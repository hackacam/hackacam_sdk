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
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <sys/reboot.h>
#include <sbl/sbl_exception.h>
#include <sbl/sbl_logger.h>
#include <rtsp/rtsp.h>
#include "cgi_server.h"


namespace CGI {

//! Reboots the system after 1 second delay.
/** @details
   This is hack for reboot. Normally, it should be possible to call FCGX_Fflush() so that the a reply
   is sent to client before the system reboots. However, that doesn't seem to work, lighttpd
   is *not* sending a reply to the client, the server reboots and the client hangs.
   The hack (workaround) is to schedule reboot in a new thread. The main thread goes back
   to FCGX_Accept() and that seems to be doing the trick (i.e sending reply). Since
   FCGX_Accept() is blocking, the actual reboot requires a new thread. The sleep()
   here is another hack to ensure that response was sent.
*/
static void* reboot_system(void*) {
    sleep(1);
    // Hasta la vista!
    if (reboot(RB_AUTOBOOT) != 0)
        SBL_ERROR("unable to reboot the system");
    return 0;
}

//! Process status? command
void Server::cmd_status() {
    CGI_ERROR(_arg_map.size(), "no arguments allowed for the command");
    _content_buffer = const_cast<char*>(_options.files.status.c_str());
    _content_type   = Gateway::STATUS_FILE;
    _content_size = file_size(_content_buffer);
}

//! Process device_info command
void Server::cmd_device_info() {
    switch (get_action(ACTION_GET | ACTION_SET)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments allowed with action=get");
        _sdk.get_device_info(_param_state.device_info);
        _reply << _param_state.device_info;
        if (_temperature.has_sensor())
            _reply << _temperature << eol();
        struct timespec current_time;
        CGI_ERROR(clock_gettime(CLOCK_MONOTONIC, &current_time) != 0, "Error getting monotonic time");
        _reply << "uptime=" << (current_time.tv_sec - _boot_time) << eol();
        break;
    case ACTION_SET:
        CGI_ERROR(_arg_map.size() == 0, "missing arguments for 'set' action");
        // we ignore bad parameters during initialization, because serial number may be corrupted
        _param_state.device_info.set(_arg_map, initialized());
        if (_param_state.device_info.motion_dest.changed())
            _mv_sender.set_rate(_param_state.device_info.motion_dest, _param_state.image.motion_rate);
        // we don't execute it when initializing, because at that point
        // we only need to initialize the variables, we don't change anything
        if (initialized()) {
            _param_state.device_info.verify();
            _sdk.set_device_info(_param_state.device_info);
            _sdk.get_device_info(_param_state.device_info);
            _param_state.write_file(_options.files.state);
        }
    default: // flowing thru to avoid compiler warning
        break;
    }
    DeviceInfo& di = _param_state.device_info;
    if (initialized() && (di.dhcp.changed() || (di.dhcp == false &&
       (di.ip_address.changed() || di.subnet.changed() || di.gateway.changed())))) {
        char buffer[100];
        snprintf(buffer, sizeof buffer, "dhcp=%d,ip_address=%s,gateway=%s,subnet=%s", 
                                        di.dhcp.get(), di.ip_address.get().c_str(),
                                        di.gateway.get().c_str(), di.subnet.get().c_str());
        write_status_file(NETWORK_PARAMS, buffer);
        pthread_create(&_thread, NULL, reboot_system, NULL);
    }
}

//! Process snapshot? command
void Server::cmd_snapshot() {
    switch (get_action(ACTION_GET | ACTION_SET | ACTION_SEND)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments allowed with action=get");
        _reply << _param_state.snapshot;
        break;
    case ACTION_SET:
        CGI_ERROR(_arg_map.size() == 0, "missing arguments for 'set' action");
        _param_state.snapshot.set(_arg_map);
        _param_state.write_file(_options.files.state);
        break;
    case ACTION_SEND:
        CGI_ERROR(_arg_map.size(), "no arguments allowed with action=send");
        _content_buffer = _sdk.acquire_snapshot(_param_state.snapshot, &_content_size);
        _content_type = Gateway::JPEG;
        if (_options.files.jpeg.size()) {
            ofstream jpeg(_options.files.jpeg.c_str());
            CGI_ERROR(!jpeg, "Unable to open temporary jpeg file %s", _options.files.jpeg.c_str());
            jpeg.write(_content_buffer, _content_size);
            CGI_ERROR(jpeg.bad(), "Error writing to temporary jpeg file %s", _options.files.jpeg.c_str());
            jpeg.close();
        }
    default: // flowing thru to avoid compiler warning
        break;
    }
}

//! Process login? command
void Server::cmd_login() {
    const char* user     = get_arg("user");
    const char* password = get_arg("password");
    CGI_ERROR(_arg_map.size(), "no arguments other then 'user' and 'password' supported for 'login' command");
    CGI_ERROR(strcmp(user, "root"), "only root login is supported");
    CGI_ERROR(strcmp(password, _param_state.user.password.get().c_str()), "wrong password for user %s", user);
    _logged = true;
    SBL_INFO("User %s logged in", user);
}

//! Process logout? command
void Server::cmd_logout() {
    const char* user = get_arg("user");
    CGI_ERROR(_arg_map.size(), "no arguments other then 'user' supported for 'logout' command");
    CGI_ERROR(strcmp(user, "root"), "only root logout is supported");
    _logged = false;
    SBL_INFO("User %s logged out", user);
}

//! Process user? command
void Server::cmd_user() {
    get_action(ACTION_SET); // only valid action is set
    const char* user = get_arg("user");
    CGI_ERROR(strcmp(user, "root"), "only root user is supported");
    const char* password = get_arg("password");
    _param_state.user.password = password;
    _param_state.write_file(_options.files.state);
    SBL_INFO("Changed password for user %s", user);
}

//! Copy form[0], followed by cmd, followed by form[1] to reply stream
void Server::form(const std::string& cmd) {
    _reply << _form[0] << cmd << _form[1];
    _content_type = Gateway::HTML;
}

//! Process firmware? command
void Server::cmd_firmware() {
    if (_arg_map.size() == 0) {
        CGI_ERROR(initialized() && _gateway.method() != Gateway::GET, "HTTP method GET must be used.");
        form("firmware?multipart=true");
    } else {
        CGI_ERROR(initialized() && _gateway.method() != Gateway::POST, "HTTP method POST must be used.");
        Firmware firmware;
        firmware.set(_arg_map);
        CGI_ERROR(firmware.file.changed() && firmware.multipart, "must have either file=<name> or multipart=true argument, but not both");
        // we catch any exception here (instead of process()) so that we can re-enable encoders and watchdog if flashing fails
        try {
            // if initialization failed, we may not be able to talk to the board and disabling encoder or watchddog may throw
            // but we still want to flash the firmware in that case.
            if (initialized()) { 
                _watchdog.enable(false);
                enable_encoders(false);
            }
            _gateway.save_file(firmware.multipart, _options.files.tmp.c_str(), firmware.file.get().c_str());
            _sdk.flash_firmware(_gateway.filepath());
        } catch (Exception& ex) {
            _reply << ex.what();
        }
        if (initialized()) {
            enable_encoders(true);
            _watchdog.enable(true);
        }
        if (_reply.tellp() == 0)
            write_status_file(FIRMWARE_FLASH, "file %s written to flash", _gateway.filepath());
    }
}

//! Process reboot? command
void Server::cmd_reboot() {
    CGI_ERROR(_arg_map.size(), "command does not accept arguments");
    write_status_file(REBOOT, "received reboot command, rebooting the system...");
    // We need to send reply before reboot, reboot_system() waits for 1 second
    pthread_create(&_thread, NULL, reboot_system, NULL);
}

//! Process test? command
void Server::cmd_reset() {
    if (find_arg("dataport")) {
        _sdk.raw_command("firmware", "dataport_reset", _reply, &_content_size);
        CGI_ERROR(_content_size, "Received unexpected data with dataport reset");
        return;
    }
    CGI_ERROR(_arg_map.size(), "command does not accept arguments");
    _param_state.reset();
    _sdk.reset(_param_state);
    SBL_INFO("reset completed, current state is:\n%s", _param_state.info().c_str());
    _param_state.write_file(_options.files.state);
}

//! Process stream? command
void Server::cmd_stream() {
    int id = convert<int>(get_arg("id"));
    CGI_ERROR(id < 0 || id >= Stream::COUNT, "incorrect stream id %d", id);
    switch (get_action(ACTION_GET | ACTION_SET)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments other then 'id' allowed with action=get");
        _sdk.get_stream(_param_state.stream[id]);
        _reply << _param_state.stream[id];
        break;
    case ACTION_SET:
        CGI_ERROR(_arg_map.size() == 0, "missing arguments for 'set' action");
        _param_state.stream[id].set(_arg_map);
        _sdk.set_stream(_param_state.stream[id]);
        if (_param_state.stream[id].osd.changed())
            _osd_changed = true;
        _param_state.write_file(_options.files.state);
    default: // flowing thru to avoid compiler warning
        break;
    }
}

const char* Server::_flips[] = { "none", "horizontal", "vertical", "both" };

//! Converts flip string to integer (index in _flips table)
unsigned int Server::flip_index(const string& flip) {
    for (unsigned int i = 0; i < sizeof(_flips) / sizeof(_flips[0]); i++) 
        if (flip == _flips[i])
            return i;
    CGI_ERROR(true, "flip name %s is invalid", flip.c_str());
}

//! Process image? command
void Server::cmd_image() {
    switch (get_action(ACTION_GET | ACTION_SET)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments allowed with action=get");
        _sdk.get_image(_param_state.image);
        _reply << _param_state.image;
        break;
    case ACTION_SET:
        CGI_ERROR(_arg_map.size() == 0, "missing arguments for 'set' action");
        if (initialized()) {
            // flip if relative to the current postion
            unsigned int flip = flip_index(_param_state.image.flip.get());
            _param_state.image.flip = "none";
            _param_state.image.set(_arg_map);
            _param_state.image.flip = _flips[flip ^ flip_index(_param_state.image.flip.get())];
        } else {
            _param_state.image.set(_arg_map);
        }
        _sdk.set_image(_param_state.image);
        if (_param_state.image.osd.changed() || _osd_changed) 
            for (int i = 0; i < Stream::COUNT; i++) {
                _param_state.stream[i].osd = _param_state.image.osd.get();
                _sdk.set_stream(_param_state.stream[i]);
            }
        _osd_changed = false;
        if (_param_state.image.motion_rate.changed())
            _mv_sender.set_rate(_param_state.device_info.motion_dest, _param_state.image.motion_rate);
        _param_state.write_file(_options.files.state);
    default: // flowing thru to avoid compiler warning
        break;
    }
}

//! process date? command
void Server::cmd_date() {
    switch (get_action(ACTION_GET | ACTION_SET)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments allowed with action=get");
        _sdk.get_date(_param_state.date);
        _reply << _param_state.date;
        break;
    case ACTION_SET:    
        {
        CGI_ERROR(_arg_map.size() == 0, "missing arguments for 'set' action");
        Date& date = _param_state.date;
        _sdk.get_date(date);
        time_t timezone = date.tz_seconds();
        date.set(_arg_map);
        date.verify();
        date.set_time(date.get_time() - timezone + date.tz_seconds());
        _sdk.set_date(date);
        _param_state.write_file(_options.files.state);
        }
        break;
    default:
        break;
    } 
}

//! Process roi? command
void Server::cmd_roi() {
    int id = 0;
    if (find_arg("id"))
        id = convert<int>(get_arg("id"));
    CGI_ERROR(id < 0 || id >= Roi::COUNT, "incorrect roi id %d", id);
    switch (get_action(ACTION_GET | ACTION_SET | ACTION_ADD | ACTION_DELETE | ACTION_CLEAR)) {
    case ACTION_GET:
        CGI_ERROR(_arg_map.size(), "no arguments other then 'id' allowed with action=get");
        _reply << _param_state.roi[id];
        break;
    case ACTION_SET:
        _param_state.roi[id].set(get_arg("rectangles"));
        CGI_ERROR(_arg_map.size(), "invalid argument");
        _sdk.set_roi(_param_state.roi[id]);
        _param_state.write_file(_options.files.state);
        break;
    case ACTION_ADD:
        _param_state.roi[id].add(get_arg("rectangles"));
        CGI_ERROR(_arg_map.size(), "invalid argument");
        _sdk.set_roi(_param_state.roi[id]);
        _param_state.write_file(_options.files.state);
        break;
    case ACTION_DELETE:
        _param_state.roi[id].del(get_arg("rectangles"));
        CGI_ERROR(_arg_map.size(), "invalid argument");
        _sdk.set_roi(_param_state.roi[id]);
        _param_state.write_file(_options.files.state);
        break;
    case ACTION_CLEAR:
        CGI_ERROR(_arg_map.size(), "no arguments other then 'id' allowed with action=clear");
        _param_state.roi[id].clear();
        _sdk.set_roi(_param_state.roi[id]);
        _param_state.write_file(_options.files.state);
        break;
    default:
        break;
    }
}


//! Process raw_command? command
void Server::cmd_raw_command() {
    RawCommand raw;
    raw.set(_arg_map);
    CGI_ERROR(((raw.inp_file.get().size() != 0) + (raw.cmd.get().size() != 0)) != 1, "exactly one of 'cmd' and 'inp_file' arguments must be used");
    CGI_ERROR(raw.inp_file.get().size() && !(raw.inp_file == "cmd") && !(raw.inp_file == "conf"), "inp_file argument must be either 'cmd' or 'conf'");
    if (raw.cmd.get().size()) {
        CGI_ERROR(initialized() && _gateway.method() != Gateway::GET, "HTTP method GET must be used.");
        _content_buffer = _sdk.raw_command(raw.subsystem.get(), raw.cmd.get(), _reply, &_content_size);
    } else { // inp_file is used
        if (initialized() && _gateway.method() == Gateway::GET) {
            form("raw_command?multipart=true&inp_file=" + raw.inp_file.get());
        } else if (raw.inp_file == "cmd") {
            _gateway.save_file(raw.multipart, _options.files.tmp.c_str());
            _content_buffer = _sdk.raw_command(raw.subsystem.get(), _gateway.filepath(), _gateway.file_size(), _reply, &_content_size);
            SBL_MSG(MSG::SERVER, "Executed raw command file %s", _gateway.filepath());
        } else if (raw.inp_file == "conf") {
            _gateway.save_file(raw.multipart, _options.files.flash.c_str(), _options.files.raw_commands.c_str());
            SBL_INFO("Saved new raw command configuration file %s", _gateway.filepath());
        } // else condition here throws CGI_ERROR above (2nd one)
    }
    if (_content_buffer) {
        _content_type = Gateway::RAW;
        _reply_filename = raw.out_file.get();
    }
} 

//! Find an argument
//! @param arg_name argument name
//! @return pointer to value or NULL
const char* Server::find_arg(const char* arg_name) {
    ArgMap::iterator arg = _arg_map.find(arg_name);
    return arg == _arg_map.end() ? NULL : arg->second;
}

//! find an argument, remove it from map and return its value
//! @param  arg_name    argument name
//! @return pointer to value
//! @throw SBL::Exception() if argument or value is missing
const char* Server::get_arg(const char* arg_name) {
    ArgMap::iterator arg = _arg_map.find(arg_name);
    CGI_ERROR(arg == _arg_map.end(), "missing '%s' argument", arg_name);
    CGI_ERROR(!arg->second, "argument '%s' missing value", arg_name);
    const char* arg_value = arg->second;
    _arg_map.erase(arg);
    return arg_value;
}

//! get "action" argument and return enum
//! @param mask mask of acceptable values
//! @return Action (i.e. translates from string to enum)
//! @throw SBL::Exception() if action or its value is missing or if value is not recognized
//! also throw if value is not allowed, allowed values are passed in as 'mask' argument
Server::Action Server::get_action(unsigned int mask) {
    const char* action = get_arg("action");
    Action act = strcmp(action, "get")    == 0  ? ACTION_GET:
                 strcmp(action, "set")    == 0  ? ACTION_SET:
                 strcmp(action, "send")   == 0  ? ACTION_SEND:
                 strcmp(action, "add")    == 0  ? ACTION_ADD:
                 strcmp(action, "delete") == 0  ? ACTION_DELETE:
                 strcmp(action, "clear")  == 0  ? ACTION_CLEAR:
                                                  ACTION_NONE;
    CGI_ERROR((act & mask) == 0, "invalid argument value %s", action);
    return act; 
}

void Server::cmd_test() {
    CGI_ERROR(!_options.enable_test, "test, invalid command");
    Test test;
    test.set(_arg_map);
    if (test.kill_watchdog) {
        SBL_INFO("Killing watchdog");
        _watchdog.kill_thread();
    }
    if (test.block_callback) {
        SBL_INFO("Disabling callback");
        _sdk.block_callback();
    }
    if (test.logfile.changed()) {
        struct timespec current_time;
        CGI_ERROR(clock_gettime(CLOCK_MONOTONIC, &current_time) != 0, "Error getting monotonic time");
        _content_buffer = const_cast<char*>(SBL::Log::save_logfile(current_time.tv_sec));
        CGI_ERROR(!_content_buffer, "Unable to save old log file");
        _content_type = Gateway::LOGFILE;
        _content_size = file_size(_content_buffer);
        const char* p = strrchr(_content_buffer, '/');
        _reply_filename = p ? p + 1 : _content_buffer;
    }
    if (test.fps)
        _reply << "ave_fps=" << std::fixed << std::setprecision(1) << _watchdog.ave_fps() << eol();
    if (test.test_frames.changed()) {
        _sdk.set_test(test.test_frames);
    }
}

//! Class constructor
Server::Server(const Options& options, const SDKManager::Options& sdk_options) :
    _options(options), _initialized(false), _fatal_error(false), _logged(_options.logged), 
    _param_state(this), _sdk(this, sdk_options), 
    _temperature(this, _options.files.flash + "/" + _options.files.temperature),
    _watchdog(this, options.watchdog_fail_count),
    _net_recovery(this, options.net_recovery),
    _osd_changed(false),
    _cmd_map(SBL::CreateMap<const char*, CmdFun, SBL::StrCompare>
    ("login",       &Server::cmd_login)
    ("logout",      &Server::cmd_logout)
    ("user",        &Server::cmd_user)
    ("date",        &Server::cmd_date)
    ("firmware",    &Server::cmd_firmware)
    ("reboot",      &Server::cmd_reboot)
    ("reset",       &Server::cmd_reset)
    ("status",      &Server::cmd_status)
    ("device_info", &Server::cmd_device_info)
    ("image",       &Server::cmd_image)
    ("stream",      &Server::cmd_stream)
    ("snapshot",    &Server::cmd_snapshot)
    ("roi",         &Server::cmd_roi)
    ("raw_command", &Server::cmd_raw_command)
    ("test",        &Server::cmd_test)
    )
{
    try {
        mkdir(_options.files.flash.c_str(), S_IRWXU);
        _options.files.state       = _options.files.flash + "/" + _options.files.state;
        _options.files.status      = _options.files.flash + "/" + _options.files.status;

        struct timespec current_time;
        CGI_ERROR(clock_gettime(CLOCK_MONOTONIC, &current_time) != 0, "Error getting monotonic time");
        _boot_time = current_time.tv_sec; 
        ParamSet::set_eol(eol());

        DeviceInfo device_info(0, "");
        string hostname(device_info.hostname.get());
        _sdk.get_device_info(device_info);
        // keep default hostname if we need it later, if state file is missing
        hostname +=  device_info.mac_address.get();

        if (_param_state.read_file(_options.files.state) != NO_ERROR || watchdog_loop()) {
            SBL_WARN("Missing or invalid state file %s, resetting system to defaults", _options.files.state.c_str());
            _arg_map.clear();
            _param_state.device_info.hostname = hostname;
            cmd_reset();
        }

        // try to read raw command first from flash, then from rootfs
        if (_param_state.read_file(_options.files.flash + '/' + _options.files.raw_commands) == NOT_FOUND)
            _param_state.read_file(_options.files.conf  + '/' + _options.files.raw_commands);

        _param_state.device_info.mac_address = device_info.mac_address.get();
        _param_state.device_info.firmware    = device_info.firmware.get();
        _sdk.set_hostname(_param_state.device_info.hostname);
        SBL_MSG(MSG::SERVER, "Device info is %s", _param_state.device_info.info().c_str());
        // get correct local date/time into firmware for OSD
        _sdk.get_date(_param_state.date);
        _sdk.set_date(_param_state.date);

        const int THREAD_STACK = 64 * 1024;
        if (_net_recovery.is_enabled()) {
            _net_recovery.set_mac_address(_param_state.device_info.mac_address);
            _net_recovery.create_thread(Thread::Default, THREAD_STACK);
        } else
            SBL_INFO("net_recovery is not enabled");

        _initialized = true;
        enable_encoders(true);
        _param_state.write_file(_options.files.state);
        _watchdog.create_thread(Thread::Default, THREAD_STACK);
        _temperature.create_thread(Thread::Default, THREAD_STACK);

        SBL_INFO("CGI server initialization done");
    } catch (Exception& ex) {
        SBL_ERROR("Server initialization failed: %s", ex.what());
        strcpy(_buffer, ex.what());
        _fatal_error = true;
        write_status_file(SERVER_INIT_FAILED, "fatal error %s during initialization, system reset to defaults", ex.what());
        _param_state.reset();
        _param_state.write_file(_options.files.state);
    }
}

//! process a single command
//! @param  command command name (before '?)
//! @param  args   is the argument string (after '?')
//! @param  method HTTP method that was used
//! @param  logged bool to say that user doesn't need to be logged to process command 
ErrorCode Server::process(const char* command, const char* args, Gateway::Method method, bool logged) {
    SBL_ASSERT(command);
    SBL_ASSERT(args);
    if (*command == '/')
        command++;
    SBL_MSG(MSG::SERVER, "command %s: args: %s", command, args);
    _arg_map.clear();
    _reply.clear();
    _reply.seekp(0);
    _reply.seekg(0);
    _content_buffer = NULL;
    _content_type = Gateway::TEXT;   // this is default, so far only snapshot changes it
    _param_state.checkpt();
    ErrorCode error_code = NO_ERROR;
    try {
        CmdMap::iterator it = _cmd_map.find(command);
        CGI_ERROR(it == _cmd_map.end(), "unrecognized command");
        CmdFun cmd_fun = it->second;
        CGI_ERROR(!logged /* if logged is true, we are called from _net_recovery, so method is don't care */
               && cmd_fun != &Server::cmd_firmware 
               && cmd_fun != &Server::cmd_raw_command 
               && method != Gateway::GET, "HTTP method GET must be used");
        split_args(args);
        CGI_ERROR(!_logged && !logged && initialized() && cmd_fun != &Server::cmd_login, "user not logged in");
        CGI_ERROR(_fatal_error && cmd_fun != &Server::cmd_reboot && cmd_fun != &Server::cmd_status
                               && cmd_fun != &Server::cmd_login  && cmd_fun != &Server::cmd_firmware
                               && cmd_fun != &Server::cmd_reset,
                  "fatal error occured during server initialization, command set it restricted, "
                  "system settings were reset to defaults");
        (this->*cmd_fun)();
    } catch (SBL::Exception& ex) {
        SBL_ERROR("Error: command %s, %s", command, ex.what());
        _reply << "Error: command '" << command << "', " << ex.what();
        error_code = static_cast<ErrorCode>(ex.code());
        _param_state.revert();
    }
    return error_code;
}

//! write code and message to status file, also put it into log file
//! @param code     status code
//! @param format   printf-like format of message
//! @return true if successful, false if unable to open status file
bool Server::write_status_file(StatusCode code, const char* format, ...) {
    char buffer[512];
    size_t size = snprintf(buffer, sizeof(buffer), "[%s] %d: ", Date::timestamp(), code);
    va_list args;
    va_start(args, format);
    size += vsnprintf(buffer + size, sizeof(buffer) - size, format, args);
    va_end(args);
    SBL_INFO(buffer);
    size += snprintf(buffer + size, sizeof(buffer) - size, eol());
    int fd = open(_options.files.status.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_SYNC,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
        return false;
    write(fd, buffer, size);
    close(fd);
    return true;
}

//! parse single line from status 
//! @return timestamp and code
Server::StatusCode Server::parse_status_line(const string& line, time_t& time) {
    if (line.size() < 30)
        return NO_STATUS_FILE;
    struct tm tm;
    char* parsed = strptime(line.c_str() + 1, "%a %b %d %T %Y", &tm);
    time = mktime(&tm);
    if (parsed == NULL || *parsed == '\0')
        return NO_STATUS_FILE;
    int code = strtol(parsed + 1, 0, 10);
    return static_cast<StatusCode>(code);
}

//! parse status file
//! @return true of server keeps rebooting due to watchdog
//! @details This is when the server reboots due to watchdog within 15 seconds of start, 3 times in a row
bool Server::watchdog_loop() {
    ifstream status_file(_options.files.status.c_str());
    if (!status_file)
        return false;
    int count = 0;
    time_t prev_time = 0;
    StatusCode prev_code = NO_STATUS_FILE;
    string line;
    while (getline(status_file, line)) {
        time_t time = 0;
        StatusCode code = parse_status_line(line, time);
        if (code == WATCHDOG_REBOOT && prev_code == SERVER_START && (time - prev_time) < 15) {
            count++;
        } else if (!(code == SERVER_START && prev_code == WATCHDOG_REBOOT))
            count = 0;
        prev_code = code;
        prev_time = time;
    }
    SBL_MSG(MSG::SERVER, "Parsed status file, found %d watchdog loops", count);
    return count >= 3;
};

//! split arguments and place them in _arg_map
//! @throw SBL::Exception() if argument is duplicate
void Server::split_args(const char* args) {
    if (strlen(args) >= sizeof(_buffer))
        throw SBL::Exception(REQUEST_URI_TOO_LONG, NULL, __FILE__, __LINE__, "Request URI too long");
    char* query = strcpy(_buffer, args);
    while (query && *query) {
        char* sep = strchr(query, '&');
        if (sep)
            *sep++ = '\0';
        char* value = strchr(query, '=');
        if (value)
            *value++ = '\0';
        CGI_ERROR(_arg_map.find(query) != _arg_map.end(), "duplicate argument %s", query);
        _arg_map[query] = value;
        query = sep;
    }
}

//! Main server loop, get command and process it
//! @details acquires mutex before processing command
void Server::run() {
    write_status_file(SERVER_START, "starting CGI server...");
    lock();
    do {
        unlock();
        bool status = _gateway.accept();
        lock();
        if (!status)
            return;
        try {
            process(_gateway.command(), _gateway.args(), _gateway.method());
            const char* content = reply();
            _gateway.reply(content_type(), content, content_size(), reply_filename());
            if (content_type() == Gateway::JPEG || content_type() == Gateway::RAW)
                    release_buffer();
            else if (content_type() == Gateway::STATUS_FILE) {
                remove(_options.files.status.c_str());
                write_status_file(CREATE_FILE, "creating empty status file (status command)");
            }
        } catch (Exception& ex) {
            write_status_file(INTERNAL, "fatal error %s, server in reduced state", ex.what());
            _fatal_error = true;
        }
    } while (true);
}

//! copy reply from stream to output buffer
unsigned int Server::read_reply() {
    unsigned int size = _reply.tellp();
    if (size >= sizeof _buffer) 
        size = sizeof(_buffer) - 1;
    if (size)
        _reply.read(_buffer, size);
    _buffer[size] = '\0';
    return size;
}

//! get reply from stream to output buffer
//! @return reply type
//! insert "OK" if reply stream is empty
const char* Server::reply() {
    if (content_type() == Gateway::JPEG || content_type() == Gateway::RAW 
      || content_type() == Gateway::LOGFILE || content_type() == Gateway::STATUS_FILE)
        return _content_buffer;
    _content_size = read_reply();
    if (_content_size == 0 && content_type() != Gateway::STATUS_FILE) {
        _reply << "OK" << eol();
        _content_size = read_reply();
    }
    SBL_MSG(MSG::SERVER, "Reply size %d:\n%s", _content_size, _buffer);
    return _buffer;
}

//! split line into command and arguments
//! @param command input line, command + arguments
//! @return parameters, input is truncated to contain only command
string Server::split_line(string& command) {
    size_t q = command.find('?');
    string params;
    if (q != string::npos) {
        params  = command.substr(q + 1);
        command = command.substr(0, q);
    }
    return params;
}

//! return file size
int Server::file_size(const char* filename) {
    struct stat st;
    CGI_ERROR(stat(filename, &st) < 0, "Stat on %s failed", filename);
    return st.st_size;
}

void Server::enable_encoders(bool enable) {
    for (int i = 0; i < Stream::COUNT; i++)
        _sdk.enable_stream(i, enable && _param_state.stream[i].enable);
}

const char* Server::_form[] = {
    "<!DOCTYPE html>\r\n"
    "<html><body>\r\n"
    "<form action=\"/cgi-bin/", "\" enctype=\"multipart/form-data\" method=post>\r\n"
    "<p>Select file to upload:<br>\r\n"
        "<input type=\"file\" name=\"datafile\" size=\"40\">\r\n"
    "</p>\r\n"
    "<div><input type=\"submit\" value=\"Send\"></div>\r\n"
    "</form>\r\n"
    "</body></html>\r\n" };

}
