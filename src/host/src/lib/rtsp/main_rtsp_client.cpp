/****************************************************************************\
*  Copyright C 2013 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/
#include <iostream>
#include "rtsp_client.h"

class Options {
public:
    char*   ip_address;
    int     duration;
    bool    tcp;
    Options(int argc, char* argv[]) :
        duration(300), tcp(false) {
        int c;
        while ( (c = getopt("d:tv:l:h")) != -1 )
            switch (c) {
                case 'd':  duration = strtol(optarg, 0, 0);  break;
                case 't':  tcp      = true;                  break;
                case 'v' : RTSP::Log::set_verbosity(strtol(optarg, 0, 0));          break;
                case 'l' : if (RTSP::Log::open_logfile(optarg) < 0) {
                                std::cerr << "Error: unable to open logfile " << optarg << std::endl;
                                exit(1);
                           }
                case 'h':
                default :  std::cout << _usage << std::endl;
                           exit(1);
            }
    }
private:
    static char* _usage;
};

char* Options::_usage = "\n"
    "rtsp_client <ip_address> -d <duration> -t -v <verbosity> -l <logfile>\n"
    "Options:\n"
    "   -d <int>        : test duration in seconds, default 300\n"
    "   -t              : use TCP (default UDP)\n"
    "   -v <int>        : message verbosity (default 1, errors only)\n"
    "   -l <filename>   : logfile for messages (default stdout)\n"
    "\n";

int main(int argc, char* argv[]) {
    Options options(argc, argv);
    RTSP::RemoteClient::create(options.ip_address, options.duration, options.tcp);
    return 0;
}
