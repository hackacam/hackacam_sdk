#include <string>
#include <iostream>
#include <fstream>
#include <sbl/sbl_exception.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_socket.h>
#include <sbl/sbl_options.h>

using namespace std;
using namespace SBL;

struct MVReceiverOptions : public OptionSet {
    Option<int>      port;
    Option<String>   mv_file;
    Option<String>   logfile;
    MVReceiverOptions(int argc, char* argv[]) : 
                port    (this, "port",     0,  "receive port number"),
                mv_file (this, "mv_file",  0,  "motion vector file"),
                logfile (this, "logfile",  0,  "log file")
                {parse(argc, argv); }
};

struct MVHdr {
    char        m;
    char        v;
    uint16_t    seq_num;
    uint32_t    timestamp;
    uint8_t     width;
    uint8_t     height;
    uint8_t     aggregate;
    uint8_t     reserverd;
};

SBL_STATIC_ASSERT(sizeof(MVHdr) == 12);

void receive(const MVReceiverOptions& options) {
    if (options.logfile)
        Log::open_logfile(options.logfile);
    ofstream mv_file;
    if (options.mv_file)
        mv_file.open(options.mv_file);
    Socket socket(Socket::UDP);
    socket.bind(options.port);
    char buffer[1000];
    char source_ip[Socket::IP_ADDR_BUFF_SIZE];
    int source_port = 0;
    int size = 0;
    int seq_num = 0;
    while ((size = socket.recv(buffer, sizeof buffer, source_ip, &source_port)) > 0) {
        MVHdr* hdr = (MVHdr*) buffer;
        if (hdr->m != 'M' || hdr->v != 'V') {
            SBL_ERROR("got bad buffer header: %c %c", buffer[0], buffer[1]);
            continue;
        }
        int n = ntohs(hdr->seq_num);
        if (seq_num && n != seq_num) 
            SBL_ERROR("got bad sequence number %d, expected %d", n, seq_num);
        seq_num = n + 1;
        uint32_t timestamp = ntohl(hdr->timestamp);
        SBL_INFO("seq_num=%d, size=%d, timestamp=%d, width=%d, height=%d", n, size, timestamp, hdr->width, hdr->height);
        if (mv_file.is_open()) {
            mv_file.write(buffer, size);
            mv_file.flush();
        }
    }

}

int main(int argc, char* argv[]) {
    Exception::catch_segfault();
    Log::set_verbosity(255);
    MVReceiverOptions options(argc, argv);
    cout << options;
    if (options.help(cout)) 
        return 0;
    receive(options);
    return 0;
}
