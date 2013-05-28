#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sbl/sbl_exception.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_thread.h>
#include <sbl/sbl_socket.h>
#include "rtsp.h"
#include "rtsp_impl.h"
#include "live_source.h"
#include "file_source.h"
#include "streaming_app.h"
#include "rtsp_sdk.h"
#include "source_map.h"
#include "build_date.h"

RTSP::App app;
StreamingApp& streaming_application() { return app; }


/* --------------------------------------------------------------------------------*/
/*                  RTP Streamer option processing                                 */
class  Options {
public:
    struct Client {
        char*   filename;
        char*   ip_address;
        int     port;
        int     count;
        Client(char* fn, char* ip, int p, int cnt) :
            filename(fn), ip_address(ip), port(p), count(cnt) {}
    };
    typedef std::vector<Client> Clients;
        
    char*   logfile_name;
    int     packet_size;
    int     fps;
    int     ts_clock;
    char*   rom_file;
    char*   sdp;
    int     stream_count;
    Clients clients;

    Options(int argc, char* argv[]) : logfile_name(0), packet_size(9900), fps(30), ts_clock(90000), 
                                      rom_file(0), sdp(0), stream_count(1) {
        int c;
        while ( (c = getopt(argc, argv, "r:l:v:a:f:s:c:h")) != -1) 
            switch (c) {
                case 'r':   rom_file       = optarg;                        break;
                case 'l':   logfile_name   = optarg;                        break;
                case 'v':   SBL::Log::set_verbosity(strtol(optarg, 0, 0));  break;
                case 'a':   packet_size    = std::strtol(optarg, 0, 0);     break;
                case 'f':   fps            = std::strtol(optarg, 0, 0);     break;
                case 's':   sdp            = optarg;                        break;
                case 'c':   stream_count   = std::strtol(optarg, 0, 0);     break;
                case 'h':   // flow thru
                default :   std::cout << _usage; 
                            exit(1);
            }
        while (optind < argc) {
            std::string save_opt(argv[optind]);
            if (parse_client_specs(argv[optind++]) != 0) {
                std::cerr << "Error in client specification: " << save_opt << std::endl;
                exit(1);
            }
        }
        if (clients.size() == 0) {
            std::cerr << "Error: at least one client must be specified" << std::endl;
            std::cerr << _usage;
            exit(1);
        }
        if (logfile_name && SBL::Log::open_logfile(logfile_name)) {
            std::cerr << "Unable to open logfile " << logfile_name << std::endl;
            exit(1);
        }
    }
private:
    static const char*   _usage;

    int parse_client_specs(char* pos) {
        char* filename = pos;
        if ( !(pos = std::strchr(filename, ',')) )
            return -1;      // need to have at least file name and client IP
        *pos++ = '\0';
        char* ip_addr = pos;
        int port = 5000;
        int count = 1;
        if ((pos = std::strchr(ip_addr, ',')))  {
            *pos++ = '\0';
            port = std::strtol(pos, &pos, 10);
            if (pos && *pos) {
                if (*pos != ',')
                    return -2;  // either count is absent or separated by comma
                count = std::strtol(++pos, 0, 10);
            }
        }
        clients.push_back(Client(filename, ip_addr, port, count));
        return 0;
    }
};

const char* Options::_usage = "\n"
    "Usage: rtp_streamer [-h] [-r rom_file] [-a packet_size] [-l <log_file>] [-v <verbosity>] [-f <fps>] [-c <stream_count>]\n"
    "                    [-t clock] [-s <sdp_dir_path>] <client_spec> ...\n"
    "       -r  <filename>      : rom file path\n"
    "       -l  <filename>      : all messages will be output to this file, default is stdout\n"
    "       -v  <int>           : verbosity level, default is 1 (no messages excepts errors)\n"
    "       -a  <int>           : packet size, default is 9900\n"
    "       -f  <int>           : frame per seconds, default is 30\n"
    "       -t  <int>           : timestamp clock frequency, default is 90000\n"
    "       -s  <dir_path>      : directory where to store .sdp files\n"
    "       -c  <int>           : stream count, must be 1, 2 or 3, default 1\n"
    "       -h                  : print this message\n"
    "Client specification <client_spec> is a comma separated list:\n"
    "   <file_name>,<ip_address>,<port>,<count> where\n"
    "       file_name           : is the name of the file which should be streamed\n"
    "       ip_address          : is the IP address of the client\n"
    "       port                : is the starting port in the client, default 5000\n"
    "       count               : is the number of ports stream will be sent to, default 1\n"
    "Stream from <filename> will be sent to <count> ports (all even) starting at <ip_address>:<port>\n"
    "To specify live stream, use a number for <filename>\n"
    "Multiple <client_spec> may be specified to support multiple source files to be streamed\n"
    "When -s option is used, .sdp files are created in the specifed directory for each stream\n"
    "\n";


std::ostream& operator<<(std::ostream& str, const Options& options) {
    str  << "Logfile:        " << (options.logfile_name ? options.logfile_name : "(stdout)") << std::endl
         << "Verbosity mask: " << SBL::Log::verbosity << std::endl
         << "FPS:            " << options.fps << std::endl
         << "TS clock        " << options.ts_clock << std::endl
         << "Packet Size:    " << options.packet_size << std::endl;
    if (options.rom_file)
    str  << "ROM file:       " << options.rom_file << std::endl;
    if (options.sdp)
    str << "SDP directory:   " << options.sdp << std::endl;

    int n = 0;
    for (Options::Clients::const_iterator client = options.clients.begin(); client != options.clients.end(); ++client) {
        str  << "Client " << n++ << ": " << std::endl
             << "   Filename:   " << client->filename << std::endl
             << "   IP address: " << client->ip_address << std::endl
             << "   Port:       " << client->port << std::endl
             << "   Count:      " << client->count << std::endl;
    }
    return str;
}

// Check to see if the filename is a number, which means live stream
static int channel_number(const char* filename) {
    if (std::strlen(filename) && std::strchr("0123456789", filename[0]))
        return std::strtol(filename, 0, 0);
    return -1;
}

static void write_sdp_file(const char* dir_name, const char* filename, RTSP::Source* source, int port) {
    char sdp_file[strlen(dir_name) + strlen(filename) + 40];
    strcpy(sdp_file, dir_name);
    strcat(sdp_file, "/");
    strcat(sdp_file, filename);
    char* pos = strrchr(sdp_file, '.');
    if (!pos)
        pos = sdp_file + strlen(sdp_file);
    sprintf(pos, "_%d.sdp", port);
    std::ofstream file(sdp_file);
    if (!file) {
        std::cerr << "Unable to open file " << sdp_file << " for writing, skipping ...\n";
        return;
    }
    file << "v=0" << std::endl
         << "s=Camera_1" << std::endl
         << "t=0 0" << std::endl
         << "m=video " << port << " RTP/AVP 96" << std::endl
         << "a=rtpmap:96 H264/90000" << std::endl;
    source->write_param_set(file) << std::endl
         << "a=control:trackID=1" << std::endl
         << "a=framerate:30" << std::endl;
    file.close();
    std::cout << "Written out " << sdp_file << std::endl;
} 

class MiniServer : public RTSP::Server {
public :
    MiniServer(int packet_size, int fps, int ts_clock) : RTSP::Server(NULL), 
                _packet_size(packet_size), _fps(fps), _ts_clock(ts_clock) {}
    RTSP::Source* get_source(const int id) {
        RTSP::Source* source = _map.find(id);
        if (!source) {
            source = new RTSP::LiveSource(id, new RTSP::Streamer(_packet_size));
            _map.save(id, source);
        }
        return source;
    }
    RTSP::Source* get_source(const char* name) {
        RTSP::Source* source = _map.find(name);
        if (!source) {
            source = RTSP::FileSource::create(name, new RTSP::Streamer(_packet_size), _fps, _ts_clock);
            _map.save(name, source);
        }
        return source;
    }
private:
    RTSP::SourceMap _map;
    int             _packet_size;
    int             _fps;
    int             _ts_clock;
};

class App : public StreamingApp {
public:
    int get_stream_id(unsigned int channel_num, unsigned int stream_num) { return 0; }
    int get_stream_id(const char* stream_name) { return 0; }
    void play(int stream_id) {}
    void teardown(int stream_id) {}
    int describe(int, StreamingApp::StreamDesc&) { return 0; }
    int pe_id() const { return 0; }
};


/* --------------------------------------------------------------------------------*/
/*                      MAIN                                                       */
int main(int argc, char* argv[]) {
    SBL::Exception::enable_backtrace(true);
    Options options(argc, argv);
    std::cout << "Stretch RTP Streamer built on" << RTSP::build_date << std::endl;
    std::cout << options;
    char streams[4];
    strncpy(streams, "hhh", options.stream_count);
    sdk_setup(options.rom_file, streams);
    MiniServer server(options.packet_size, options.fps, options.ts_clock);
    App app;
    app.set_server(&server);
    for (Options::Clients::const_iterator it = options.clients.begin(); it != options.clients.end(); ++it) {
        int port = it->port;
        for (int count = it->count; count; count--, port += 2) {
            SBL::Socket socket(SBL::Socket::UDP);
            socket.connect(it->ip_address, port);
            SBL::Socket rtcp_socket(SBL::Socket::UDP);
            socket.connect(it->ip_address, port + 1);
            int n = channel_number(it->filename);
            try {
                RTSP::Source* source = n < 0 ? server.get_source(it->filename) :
                                               server.get_source(n);
                RTSP::Client* client = source->streamer()->add_client(socket, rtcp_socket);
                client->play();
                if (options.sdp) 
                    write_sdp_file(options.sdp, it->filename, client->streamer()->source(), port);
            } catch (RTSP::Errcode e) {
                std::cerr << "Caught error code " << e << ". Exiting ..." << std::endl;
                return 1;
            }
        }
    }
        

    // TODO: need to add command line processing here
    do {
        sleep(1);
    } while (1);
    return 0;
}
