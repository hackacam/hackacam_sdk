#pragma once
#ifndef _CGI_SERVER_H
#define _CGI_SERVER_H
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
#include <string>
#include <sstream>
#include <pthread.h>
#include <sbl/sbl_param_set.h>
#include "cgi_param_set.h"
#include "sdk_manager.h"
#include "gateway.h"
#include "mv_sender.h"
#include "temperature.h"
#include "watchdog.h"
#include "net_recovery.h"

namespace CGI {
using namespace SBL;
using namespace std;

class Server {
public:
    // avoid to change the numeric values of these error codes, they are written to status file
    // server checks some of them (currently WATCHDOG_REBOOT) when initializing, so if software is upgraded,
    // that can create a problem
    enum StatusCode {NO_STATUS_FILE = 99, INTERNAL, SERVER_INIT_FAILED, SERVER_START, CREATE_FILE, NETWORK_PARAMS, FIRMWARE_FLASH, REBOOT, WATCHDOG_REBOOT};
    struct Options {
        struct Files {
            string          conf;
            string          flash;
            string          tmp;
            string          state;
            string          temperature;
            string          raw_commands;
            string          status;
            string          jpeg;
            Files():        conf("/usr/local/stretch/conf"),
                            flash("/mnt/flash/cgi"),
                            tmp("/tmp"),
                            state("state.cgi"),
                            temperature("temperature.txt"),
                            raw_commands("raw_commands.cgi"),
                            status("status.txt"),
                            jpeg("")
           {}
        };
        Files               files;
        bool                logged;
        int                 watchdog_fail_count;
        bool                enable_test;
        string              net_recovery;
        Options() : 
                    logged(false),
                    watchdog_fail_count(0),
                    enable_test(false)
                    {}
    };
    Server(const Options& options, const SDKManager::Options& sdkOptions);
    void                run();
    static string       split_line(string& command);
    bool                initialized() const { return _initialized; }
    const ParamState&   param_state() const { return _param_state; }
    SDKManager*         sdk_manager() { return &_sdk; }
    MVSender&           mv_sender() { return _mv_sender; }
    ErrorCode           process(const char* command, const char* args, Gateway::Method method = Gateway::GET, bool logged = false);
    void                release_buffer() { _sdk.release_callback_buffer(); };
    bool                write_status_file(StatusCode code, const char* format, ...);
    const Stream&       stream(int n)  const { return _param_state.stream[n]; }
    unsigned int        read_reply();
    const char*         buffer() const { return _buffer; }
    static const char*  eol() { return "\r\n"; }
    void                lock()   { _mutex.lock(); }
    void                unlock() { _mutex.unlock(); }
    bool                fatal_error() const { return _fatal_error; }

    typedef Gateway::Type ContentType;
    ContentType         content_type() const { return _content_type; }
    unsigned int        content_size() const { return _content_size; }
    const char*         reply();

private:
    void cmd_login();
    void cmd_logout();
    void cmd_user();
    void cmd_date();
    void cmd_firmware();
    void cmd_reboot();
    void cmd_reset();
    void cmd_status();
    void cmd_device_info();
    void cmd_image();
    void cmd_stream();
    void cmd_snapshot();
    void cmd_roi();
    void cmd_raw_command();
    void cmd_test();

    typedef void (Server::*CmdFun)();
    typedef std::map<const char*, CmdFun, StrCompare> CmdMap;

    Gateway             _gateway;
    Options             _options;
    bool                _initialized;
    bool                _fatal_error;
    bool                _logged;
    ParamState          _param_state;
    SDKManager          _sdk;
    char                _buffer[1024];
    stringstream        _reply;
    pthread_t           _thread;
    ContentType         _content_type;
    unsigned int        _content_size;
    char*               _content_buffer;
    typedef map<const char*, const char*, StrCompare> ArgMap;
    ArgMap              _arg_map;
    MVSender            _mv_sender;
    string              _reply_filename;
    time_t              _boot_time;
    Mutex               _mutex;
    Temperature         _temperature;
    Watchdog            _watchdog;
    NetRecovery         _net_recovery;
    bool                _osd_changed;
    CmdMap              _cmd_map;

    void                split_args(const char* args);
    const char*         get_arg(const char* arg_name);
    const char*         find_arg(const char* arg_name);
    enum Action { ACTION_NONE = 0, ACTION_GET = 1, ACTION_SET = 2, ACTION_SEND = 4, ACTION_ADD = 8, ACTION_DELETE = 16, ACTION_CLEAR = 32};
    Action              get_action(unsigned int mask);

    const char*         reply_filename() const { return _reply_filename.c_str(); }
    unsigned int        flip_index(const string& abs_flip);
    void                form(const std::string& cmd);
    StatusCode          parse_status_line(const string& line, time_t& time);
    bool                watchdog_loop();
    static int          file_size(const char* filename);
    void                enable_encoders(bool enable);

    static const char*  _form[];
    static const char*  _flips[];
};

}
#endif
