#include <iostream>
#include <iomanip>
#include <csignal>
#include <cctype>
#include <cstring>
#include <rtsp/rtsp.h>
#include <sbl/sbl_logger.h>
#include <rtsp/rtsp_server.h>
#include "streaming_app.h"
#include "build_date.h"
#include "rtsp_sdk.h"

Application application;
RTSP::Application* RTSP::application() { return &::application; }

/* --------------------------------------------------------------------------------*/
/*                  Option parsing                                                 */
struct Options {
    RTSP::Server::Options   server;
    int                 port;
    char*               rom_file;
    int                 gop_size;
    int                 bitrate;
    char                encoder_type[4];
    Options(int argc, char* argv[]) : port(554), rom_file(NULL), gop_size(30), bitrate(8000)  {
        strcpy(encoder_type, "h");
        std::cout << "Stretch RTSP server built on " << RTSP::build_date  << std::endl;
        int c;
        while ( (c = getopt(argc, argv, "r:v:a:p:l:f:s:t:B:g:b:eE:Tkh")) != -1)
            switch (c) {
                case 'r':  rom_file               = optarg;                         break;
                case 'v' : SBL::Log::set_verbosity(strtol(optarg, 0, 0));           break;
                case 'a' : server.packet_size     = strtol(optarg, 0, 0);           break;
                case 'p' : port                   = strtol(optarg, 0, 0);           break;
                case 'f' : server.fps             = strtol(optarg, 0, 0);           break;
                case 's' : if (strlen(optarg) < 1 || strlen(optarg) > 3) {
                                std::cerr << "Error: -s option must be followed by 1 to 3 characters" << std::endl;
                                exit(1);
                           }
                           strncpy(encoder_type, optarg, sizeof encoder_type);
                           break;
                case 't' : server.ts_clock        = strtol(optarg, 0, 0);           break;
                case 'B' : server.send_buff_size  = strtol(optarg, 0, 0);           break;
                case 'g' : gop_size               = strtol(optarg, 0, 0);           break;
                case 'b' : bitrate                = strtol(optarg, 0, 0);           break;
                case 'e' : server.temporal_levels = true;                           break;
                case 'E' : server.increase_time   = strtol(optarg, 0, 0);           break;
                case 'T' : server.tcp_nodelay     = false;                          break;
                case 'k' : server.tcp_cork        = true;                           break;
                case 'l' : if (SBL::Log::open_logfile(optarg) < 0) {
                                std::cerr << "Error: unable to open logfile " << optarg << std::endl;
                                exit(1);
                           }
                           break;
                case 'h' : std::cerr << usage << std::endl;
                           std::cerr << "Verbosity levels (in addition to standard):" << std::endl;
                           RTSP::Server::print_verbosity_levels(std::cerr);
                default  : exit(1);
            }
        // for SVC, adjust GOP size to be multiple of 4
        if (server.temporal_levels)
            while (gop_size & 3)
                gop_size++;
    }
    static const char* usage;
};

const char* Options::usage = "\n"
    "Usage: rtsp_server -r <filename [options]\n"
    "       -r <filename>   : path to the rom file\n"
    "       -v <int>        : message verbosity (default 1, errors only)\n"
    "       -a <int>        : UDP/TCP packet size, default 9000 bytes\n"
    "       -p <port>       : port to listen on, default 554\n"
    "       -l <filename>   : logfile for messages (default stdout)\n"
    "       -f <int>        : frames per second (used only for file streams), default 30\n"
    "       -s <string>     : encoder types: h for H264, j for MJPEG\n"
    "       -t <int>        : timestamp clock (used only for file stream), default 90000\n"
    "       -b <int>        : primary stream bitrate (default 8000)\n"
    "       -B <int>        : set TCP socket buffer size\n"
    "       -T              : do not set TCP socket TCP_NODELAY flag\n"
    "       -k              : set TCP socket TCP_CORK flag\n"
    "       -e              : enable congestion control\n"
    "       -E <int>        : when congestion control is enabled, seconds to wait before increasing rate\n"
    "       -h              : print this message\n"
    "Server supports concurrent live and file streams. For file streams, file name must\n"
    "be specified by a client in the rtsp request (ex. rtsp://192.168.6.40/my_file.264)\n"
    "Live streams are specified by number 0-2 (ex. rtsp://192.168.6.50/1)\n"
    "You can specify the number and type of streams with -s option. -s must be followed by\n"
    "1, 2 or 3 characters ('h' for H264, 'j' for MJPEG). Default is -s h (one H264 stream)\n"
    "Attention: congestion control (-e) requires a special version of rom file. It also\n"
    "uses the terminal for input, therefore it shouldn't be used with verbosity more then 2.\n"
    "\n";

void segfault(int) {
    signal(SIGSEGV, SIG_DFL);
    SBL::Exception ex(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Segmentation fault");
    SBL_ERROR(ex.what());
    abort();
}

/* --------------------------------------------------------------------------------*/
/*                  MAIN                                                           */
int main(int argc, char* argv[]) {
    signal(SIGSEGV, segfault);
    SBL::Exception::enable_backtrace(true);
    Options options(argc, argv);
    RTSP::Server* server = RTSP::Server::create(options.port, options.server);
    sdk_setup(options.rom_file, options.encoder_type, options.gop_size, options.bitrate);
    do {
        if (options.server.temporal_levels) {
            int level;
            std::cout << "Enter new level: ";
            std::cout.flush();
            std::cin >> level;
            switch (level) {
                case -1: server->options()->temporal_levels = true;   break;
                case  0:
                case  1:
                case  2: server->options()->temporal_levels = false;
                         server->set_temporal_level(level);           break;
                default:
                        std::cout << "Level must be -1..2" << std::endl;
                        break;
            }
        } else
            sleep(1);
    } while (1);
    return 0;
}
