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
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fcgi/fcgiapp.h>
#include <sbl/sbl_logger.h>
#include "cgi_server.h"
#include "cgi_param_set.h"
#include <rtsp/rtsp.h>
#include "rtsp_server_iface.h"
#include "gateway.h"
#include "build_date.h"

using namespace std;
/* ---------------------------------------------------------------------------*/
/*                  Option parsing                                            */
class Options {
public:
    CGI::Server::Options     cgi;
    CGI::SDKManager::Options sdk;
    RTSP::Server::Options    rtsp;    
    Options(int argc, char* argv[]):  _logfile_name(0), _logfile_size(1024 * 1024) {
        int value = 0;
        getenv("CGI_SERVER_LOGFILE", _logfile_name);
        if (getenv("CGI_SERVER_LOGFILE_SIZE", _logfile_size))
            _logfile_size *= 1024 * 1024;
        if (getenv("CGI_SERVER_VERBOSITY", value))
            SBL::Log::set_verbosity(value);
        getenv("CGI_SERVER_ROMFILE", sdk.firmware_name);
        getenv("CGI_SERVER_WATCHDOG", cgi.watchdog_fail_count);
        if (getenv("CGI_SERVER_TEST", value))
            cgi.enable_test = value;
        getenv("CGI_SERVER_JPEG_FILE", cgi.files.jpeg);
        if (getenv("CGI_SERVER_PACKET_SIZE", rtsp.packet_size))
            rtsp.packet_size = rtsp.packet_size < 500  ? 500 :
                               rtsp.packet_size > 9000 ? 9000 :
                               rtsp.packet_size;
        getenv("CGI_SERVER_PACKET_GAP", rtsp.packet_gap);
        getenv("CGI_SERVER_BCAST", cgi.net_recovery);

        set_rtsp_verbosity();
        if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-v") == 0)) {
            std::cout << "CGI Server version " << CGI::version_string << " built on " << CGI::build_date << std::endl;
            if (strcmp(argv[1], "-h") == 0) {
                std::cerr << _usage << std::endl;
                print_verbosity_levels();
            }
            exit(1);
        }
        if (_logfile_name && SBL::Log::open_logfile(_logfile_name, true, _logfile_size) < 0) {
                // Can't really put that in the logfile, can we?
                std::cerr << "Error: unable to open logfile " << optarg << std::endl;
                exit(1);
        }
        SBL_INFO("CGI Server started on %s\n"
                 "Server version %s (built on %s)\n"
                 "    state_file:     %s\n"
                 "    firmware:       %s\n",
                 CGI::Date::timestamp(),
                 CGI::version_string, CGI::build_date, 
                 cgi.files.state.c_str(),
                 sdk.firmware_name.c_str());

        _buffer << "Version numbers:\n";
        print_version("local ", "/usr/local/stretch/etc/os-release");
        print_version("rootfs", "/etc/os-release");
        print_version("kernel", "/proc/build_version");
        SBL_INFO("Environment:\n%s", _buffer.str().c_str());
    }
private:
    std::ostringstream  _buffer;
    const char*         _logfile_name;
    int                 _logfile_size;
    static const char*  _usage;
    static void print_verbosity_levels();
    static void set_rtsp_verbosity();
    bool getenv(const char* name, const char*& value);
    bool getenv(const char* name, string& value);
    bool getenv(const char* name, int& value);
    void print_version(const char* what, const char* filename);
};

bool Options::getenv(const char* name, const char*& value) {
    const char* env = ::getenv(name);
    if (!env)
        return false;
    value = env;
    _buffer << "    " << setw(24) << left << name << ' ' << value << "\n"; 
    return true;
}

bool Options::getenv(const char* name, string& value) {
    const char* v;
    if (getenv(name, v)) {
        value = v;
        return true;
    }
    return false;
}

bool Options::getenv(const char* name, int& value) {
    const char* str;
    if (getenv(name, str)) {
        value = strtol(str, 0, 10);
        return true;
    }
    return false;
}

void Options::print_version(const char* what, const char* filename) {
    ifstream ifs(filename);
    if (!ifs)
        return;
    string line, build_version, build_date;
    while (getline(ifs, line)) {
        unsigned int pos = line.find('=');
        if (pos != string::npos) {
            if (line.substr(0, pos) == "BUILD_VERSION") 
                build_version = line.substr(pos + 1);
            else if (line.substr(0, pos) == "BUILD_DATE") 
                build_date    = line.substr(pos + 1);
        }
    }
    if (build_version.size() || build_date.size())
        _buffer << "    " << what << " version " << build_version << ", built on " << build_date << "\n";
}


namespace CGI    {
    int MSG::SERVER      = 1 << 2;
    int MSG::SDK         = 1 << 3;
    int MSG::WATCHDOG    = 1 << 5;
    int MSG::TEMP        = 1 << 6;
    int MSG::MV          = 1 << 7;
    int MSG::FRAME       = 1 << 8;
}

void Options::set_rtsp_verbosity() {
    RTSP::MSG::SERVER     = 1 << 4;
    RTSP::MSG::SOURCE_MAP = 1 << 9;
    RTSP::MSG::RTCP       = 1 << 10;
    RTSP::MSG::SDK        = 1 << 11;
    RTSP::MSG::SOURCE     = 1 << 12;
    RTSP::MSG::STREAMER   = 1 << 13;
}

void Options::print_verbosity_levels() {
    std::cerr << "Standard verbosity levels:" << std::endl
              << "           0    no messages" << std::endl
              << "           1    errors"      << std::endl
              << "           2    errors and warnings" << std::endl
              << "           3    errors, warnings and info" << std::endl;
    std::cerr << "CGI server custom verbosity levels:" << std::endl
              << std::setw(12) << CGI::MSG::SERVER   << "    " << "SERVER"   << std::endl
              << std::setw(12) << CGI::MSG::SDK      << "    " << "SDK"      << std::endl
              << std::setw(12) << CGI::MSG::WATCHDOG << "    " << "WATCHDOG" << std::endl
              << std::setw(12) << CGI::MSG::TEMP     << "    " << "TEMP"     << std::endl
              << std::setw(12) << CGI::MSG::MV       << "    " << "MV"       << std::endl
              << std::setw(12) << CGI::MSG::FRAME    << "    " << "FRAME"    << std::endl;
    std::cerr << "RTSP library custom verbosity levels:" << std::endl;
    RTSP::Server::print_verbosity_levels(std::cerr);          
}

const char* Options::_usage = "\n"
    "CGI Server is a module that must started by a web server program, such as lighttpd.\n"
    "It cannot be started from command line. It uses the following environment variables:\n"
    "   CGI_SERVER_LOGFILE      path to the log file\n"
    "   CGI_SERVER_LOGFILE_SIZE max size of the logfile in MBytes\n"
    "   CGI_SERVER_VERBOSITY    verbosity level in the log file\n"
    "   CGI_SERVER_ROMFILE      path to the firmware rom file\n"
    "   CGI_SERVER_WATCHDOG     watchdog timeout, 0 to disable\n"
    ;

int main(int argc, char* argv[]) {
    Options options(argc, argv);
#ifdef DEBUG
    SBL::Exception::catch_segfault();
#endif

    CGI::Server cgi_server(options.cgi, options.sdk);
    application.register_cgi_server(&cgi_server);
    if (cgi_server.initialized()) // if initialization failed, we don't create RTSP server
        RTSP::Server::create(options.sdk.rtsp_port_num, options.rtsp);
    cgi_server.run();
    return 0;
}
