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
#include <sbl/sbl_exception.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_net.h>
#include "cgi_server.h"

namespace CGI {

// Overriding reset, because device info and date are not not resettable
void ParamState::reset() {
    device_info.checkpt();
    date.checkpt(); 
    ParamSet::reset();
    device_info.revert(); 
    date.revert(); 
}

ErrorCode ParamState::read_file(const string& filename) {
    ifstream fs(filename.c_str());
    if (!fs) {
        SBL_WARN("Unable to load file %s", filename.c_str());
        return NOT_FOUND;
    }
    SBL_INFO("Loading input file %s", filename.c_str());
    ErrorCode error_code = NO_ERROR;
    while (!fs.eof() && error_code == NO_ERROR) {
        string command;
        getline(fs, command);
        if (command.length() == 0 || command[0] == '#')
            continue;
        string args = Server::split_line(command);
        error_code = _server->process(command.c_str(), args.c_str());
        if (error_code != NO_ERROR) {
            _server->read_reply();
            _server->write_status_file(Server::SERVER_INIT_FAILED, _server->buffer());
        }
    }
    fs.close();
    return error_code;
}

char Date::_buffer[30];

const char* Date::timestamp() {
    time_t current_time = time(NULL);
    ctime_r(&current_time, _buffer);
    _buffer[strlen(_buffer) - 1] = '\0';
    return _buffer;
}

bool ParamState::write_file(const string& filename) {
    /* Reading the state file uses Server::process(). Each cmd_* method called by
       process will in turn call write_file(), but we don't want to write state
       until reading of defaults and state is completed
     */
    if (!_server->initialized())
        return true;
    ofstream fs(filename.c_str());
    if (!fs) {
        SBL_ERROR("Unable to save settings into file %s", filename.c_str());
        return false;
    }
    string eol("&");
    eol = ParamSet::set_eol(eol);
    fs << "# IP Camera current settings" << endl;
    fs << "# File written on " << Date::timestamp() << " UTC" << endl;
    user.writeln(fs);
    image.writeln(fs);
    for (int i = 0; i < Stream::COUNT; ++i)
        stream[i].writeln(fs);
    snapshot.writeln(fs);
    for (int i = 0; i < Roi::COUNT; ++i)
        if (roi[i].rectangles.get().size())
            roi[i].writeln(fs);
    device_info.writeln(fs);
    date.writeln(fs);
    ParamSet::set_eol(eol);
    SBL_MSG(MSG::SERVER, "settings written to file %s", filename.c_str());
    return true;
}

void Date::writeln(ostream& str) {
    str << "date?action=set&" << timezone << '&' << dst << endl;
}

int Date::_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void Date::verify() {
    bool is_leap_year = (year.get() & 3) == 0 && !(year.get() % 100 == 0 && year.get() % 400 != 0);
    // month is already guaranteed to be 1 .. 12, but it is cheap to check and avoid core dump
    if (month.get() < 1 || month.get() > (int) (sizeof(_days) / sizeof(_days[0])))
        throw SBL::Exception(FATAL_ERROR, NULL, __FILE__, __LINE__, "month must be 1 .. 12");
    CGI_ERROR(day.get() > _days[month.get() - 1] &&
              !(month.get() == 2 && day.get() == 29 && is_leap_year),
              "incorrect day %d for month %d", day.get(), month.get());
    const char* tz = timezone.get().c_str();
    // valid format: GMTshh:mm where
    //  ss is '+' or '-'
    //  hh is 00 to 11
    //  mm is 00 or 30
    CGI_ERROR( strlen(tz) != strlen("GMTshh:mm")
            || memcmp(tz, "GMT", 3) != 0
            || !(tz[3] == '+' || tz[3] == '-')
            || !(isdigit(tz[4]) && isdigit(tz[5]) && isdigit(tz[7]) && isdigit(tz[8]))
            || tz[6] != ':',
            "incorrect timezone format %s", tz);
    int hour   = atoi(&tz[4]);
    int minute = atoi(&tz[7]);
    CGI_ERROR(hour > 12, "incorrect hour in timezone %s", tz);
    CGI_ERROR(minute >= 60, "incorrect minute in timezone %s", tz);
}

// convert Date to time_t (seconds since Epoch), ignores timezone/dst
time_t Date::get_time() const {
    struct tm t;
    t.tm_sec    = second;
    t.tm_min    = minute;
    t.tm_hour   = hour;
    t.tm_mday   = day;
    t.tm_mon    = month - 1;
    t.tm_year   = year - 1900;
    t.tm_isdst  = -1;
    return mktime(&t);
}

// convert timezone, including dst, to seconds
time_t Date::tz_seconds() const {
    int tz_hour, tz_minute;
    CGI_ERROR(sscanf(timezone.get().c_str(), "GMT%d:%d", &tz_hour, &tz_minute) != 2, "Internal error while converting timezone");
    int tz_seconds = tz_hour * 3600 + (tz_hour > 0 ? tz_minute * 60 : -tz_minute * 60);
    if (dst)
        tz_seconds += 3600;
    return tz_seconds;
}

// Convert time_t (seconds since Epoch) to Date, ignores timezone/dst
void Date::set_time(time_t time) {
    struct tm t;
    gmtime_r(&time, &t);
    second = t.tm_sec;
    minute = t.tm_min;
    hour   = t.tm_hour;
    day    = t.tm_mday;
    month  = t.tm_mon + 1;
    year   = t.tm_year + 1900;
}

ostream& operator<<(ostream& str, const Date& date) {
    str << setw(2) << setfill('0') << date.month.get() << '/'
        << setw(2) << setfill('0') << date.day.get()   << '/'
        << date.year.get() << ' '
        << setw(2) << setfill('0') << date.hour.get()   << ':'
        << setw(2) << setfill('0') << date.minute.get() << ':'
        << setw(2) << setfill('0') << date.second.get() << ' '
        << date.timezone.get();

    if (date.dst.get())
        str << " DST";
    return str;
}

// TODO: need to rework VerifyRange and VerifyEnum to throw instead of returning bool, it is inconsistent with this
bool VerifyPassword::operator()(const std::string& password) const {
    CGI_ERROR(password.size() < 1 || password.size() > 12, "password length must be 1..12");
    for (const char* s = password.c_str(); *s; s++)
        CGI_ERROR(!isalpha(*s) && !isdigit(*s) && *s != '_', "password character set is [a-zA-Z0-9_]");
    return true;
}

void DeviceInfo::verify() {
    CGI_ERROR(serial_number.changed(), "serial_number is read-only parameter");
    CGI_ERROR(firmware.changed(), "firmware is read-only parameter");
    if (dhcp) {
        CGI_ERROR(ip_address.changed(), "ip_address is read-only parameter when dhcp is true");
        CGI_ERROR(subnet.changed(), "subnet is read-only parameter when dhcp is true");
        CGI_ERROR(gateway.changed(), "gateway is read-only parameter when dhcp is true");
    } else 
        CGI_ERROR(!SBL::Net::verify_network_address(ip_address.get().c_str(), 
                                                    subnet.get().c_str(),
                                                    gateway.get().c_str()),
                  "network address is incorrect");
}


}
