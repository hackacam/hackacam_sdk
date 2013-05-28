#pragma once
#ifndef _CGI_PARAM_SET_H
#define _CGI_PARAM_SET_H
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
#include <sbl/sbl_param_set.h>
#include "cgi.h"
#include "roi.h"
#include "build_date.h"

/// CGI Server
namespace CGI {
class Server;

using namespace SBL;
using namespace std;

/// Image command
struct Image : public ParamSet {
    Param<bool>     osd;
    Param<int>      brightness;
    Param<int>      exposure;
    Param<int>      hue;
    Param<int>      saturation;
    Param<int>      contrast;
    Param<int>      sharpness;
    Param<int>      motion_rate;
    Param<string>   flip;
    Param<bool>     cvbs;
    Param<int>      sensor_rate;
    Param<string>   tdn;

    Image(ParamSet* parent, const char* name) : ParamSet(parent, name),
              PARAM(osd, false),
              PARAM(brightness,  50, VerifyRange(0, 100)),
              PARAM(exposure,    50, VerifyRange(0, 100)),
              PARAM(hue,         50, VerifyRange(0, 100)),
              PARAM(saturation,  50, VerifyRange(0, 100)),
              PARAM(contrast,    50, VerifyRange(0, 100)),
              PARAM(sharpness,   50, VerifyRange(0, 100)),
              PARAM(motion_rate,  0, VerifyRange(0, 100)),
              PARAM(flip, "none", VerifyEnum("none", "horizontal", "vertical","both")),
              PARAM(cvbs, false),
              PARAM(sensor_rate, 30, VerifyEnum(25, 30)),
              PARAM(tdn,         "auto", VerifyEnum("auto", "hardware", "day", "night"))
    {}
};

/// Normally, height must be multiple of 16, but 1080 and 540 and 270 aren't, so a special version
/// of range verification is needed to to support that special case
class VerifyResolution {
public:
    bool operator()(int n) {
        return n >= _min && n <= _max && (n % _mult == 0 || n == _max || n == _max / 2 || n == _max / 4);
    }
    VerifyResolution(int min, int max, int mult) : _min(min), _max(max), _mult(mult) {}
private:
    int _min;
    int _max;
    int _mult;
};

/// Stream command
struct Stream : public ParamSet {
    static const int COUNT = 4;
    Param<int>      id;
    Param<bool>     enable;
    Param<string>   encoder;
    Param<int>      fps;
    Param<int>      width;
    Param<int>      height;
    Param<int>      bitrate;
    Param<int>      quality;
    Param<int>      gop;
    Param<string>   profile;
    Param<string>   rate_control;
    Param<int>      max_bitrate;
    Param<bool>     osd;


    Stream() : ParamSet("stream"),     
               PARAM(id,            0,      VerifyRange(0, COUNT - 1)),
               PARAM(enable,        false),
               PARAM(encoder,       "h264", VerifyEnum("h264", "mjpeg", "mpeg4")),
               PARAM(fps,           30,     VerifyRange(1, 30)),
               PARAM(width,         1920,   VerifyResolution(176, 1920, 16)),
               PARAM(height,        1080,   VerifyResolution(120, 1080, 16)),
               PARAM(bitrate,       5000,   VerifyRange(100, 15000)),
               PARAM(quality,       70,     VerifyRange(10, 90)),
               PARAM(gop,           30,     VerifyRange(1, 30)),
               PARAM(profile,       "main", VerifyEnum("base", "main", "high")),
               PARAM(rate_control,  "cbr", VerifyEnum("cbr", "vbr", "cq")),
               PARAM(max_bitrate,  12000, VerifyRange(100, 15000)),
               PARAM(osd, false)
    {}
};

/// Snapshot command
struct Snapshot : public ParamSet {
    Param<int>  decimation;
    Snapshot(ParamSet* parent, const char* name) : ParamSet(parent, name),
                 PARAM(decimation, 16, VerifyEnum(1, 4, 16))
    {}
};

/// Test command
struct Test : public ParamSet {
    Param<bool>   kill_watchdog;
    Param<bool>   block_callback;
    Param<string> logfile;
    Param<bool>   fps;
    Param<int>    packet_gap;
    Param<int>   test_frames;
    Test() : ParamSet("test"),     
        PARAM(kill_watchdog, false),
        PARAM(block_callback, false),
        PARAM(logfile, ""),
        PARAM(fps, false),
        PARAM(packet_gap, 0),
        PARAM(test_frames, -1, VerifyEnum(-1, 0, 1))
    {}
};

/// Password has special verification requirements
struct VerifyPassword {
    bool operator()(const std::string& password) const;
};

struct User : public ParamSet {
    Param<string>   user;
    Param<string>   password;
    User(ParamSet* parent, const char* name) : ParamSet(parent, name),
            PARAM(user,     "root"),
            PARAM(password, "password", VerifyPassword())
    {}
};

/// Date command
class Date : public ParamSet {
public:
    Param<int>      year;
    Param<int>      month;
    Param<int>      day;
    Param<int>      hour;
    Param<int>      minute;
    Param<int>      second;
    Param<string>   timezone;
    Param<bool>     dst;

    Date(ParamSet* parent, const char* name) : ParamSet(parent, name),
                PARAM(year,     2010,   VerifyRange(2010, 2037)),   // to avoid year 2038 problem
                PARAM(month,    1,      VerifyRange(1, 12)),
                PARAM(day,      1,      VerifyRange(1, 31)),
                PARAM(hour,     0,      VerifyRange(0, 23)),
                PARAM(minute,   0,      VerifyRange(0, 59)),
                PARAM(second,   0,      VerifyRange(0, 59)),
                PARAM(timezone, "GMT-08:00"),
                PARAM(dst,      false)
    { }

    /// date needs a special verification method
    void verify();
    /// return time in seconds since Epoch, without timezone
    time_t get_time() const;
    /// set from time in seconds since Epoch, ignore timezone
    void set_time(time_t);
    /// convert timezone and dst to seconds
    time_t tz_seconds() const;
    /// date output is special
    friend ostream& operator<<(ostream& str, const Date& date);
    /// current timestampe
    static const char* timestamp();
    /// write to state file
    void writeln(std::ostream& str);
private:
    static int  _days[];
    static char _buffer[];
};

/// device_info command
struct DeviceInfo : public ParamSet {
    Param<string>   firmware;
    Param<string>   hostname;
    Param<string>   mac_address;
    Param<string>   ip_address;
    Param<string>   serial_number;
    Param<bool>     dhcp;
    Param<string>   subnet;
    Param<string>   gateway;
    Param<string>   model;
    Param<string>   motion_dest;
    Param<string>   user_data;
    Param<int>      packet_gap;

    DeviceInfo(ParamSet* parent, const char* name) : ParamSet(parent, name),
                PARAM(firmware,     version_string,     VerifyDots(2)),
                PARAM(hostname,     "ipcam-"),
                PARAM(mac_address,  "000000000000"),
                PARAM(ip_address,   "192.168.1.60",     VerifyDots(3)),
                PARAM(serial_number, "1000"),
                PARAM(dhcp,         true),
                PARAM(subnet,       "255.255.255.0",    VerifyDots(3)),
                PARAM(gateway,      "192.168.1.1",      VerifyDots(3)),
                PARAM(model,        "ipcam"),
                PARAM(motion_dest,  "192.168.1.1:9000", VerifyDots(3, true)),
                PARAM(user_data,    ""),
                PARAM(packet_gap,   0,                  VerifyRange(0, 2000))
    {}
    /// device_info needs a special verification method
    void verify();
};

/// Top paramater state structure
class ParamState  : public ParamSet {
public:
    Image       image;
    Stream      stream[Stream::COUNT];
    Date        date;
    User        user;
    DeviceInfo  device_info;
    Snapshot    snapshot;
    Roi         roi[Roi::COUNT];
    ParamState(Server* server) : 
                PARAM(image),
                PARAM(date),
                PARAM(user),
                PARAM(device_info),
                PARAM(snapshot),
                _server(server)
    {
        // since stream and roi are arrays, they need special intitializaion
        stream[0].init("id=0&bitrate=5000&enable=1&encoder=h264&fps=25&gop=25&height=1080&profile=main&quality=70&width=1920", this, 0); 
        stream[1].init("id=1&bitrate=5000&enable=0&encoder=mjpeg&fps=15&gop=30&height=270&profile=main&quality=70&width=480",  this, 1);
        stream[2].init("id=2&bitrate=500&enable=0&encoder=h264&fps=30&gop=30&height=240&profile=main&quality=70&width=352",    this, 2);
        stream[3].init("id=3&bitrate=250&enable=0&encoder=h264&fps=15&gop=15&height=240&profile=main&quality=70&width=352",    this, 3);
        for (int i = 0; i < Roi::COUNT; i++)
            roi[i].init("", this, i);
    }


    ErrorCode read_file(const string& filename);
    bool write_file(const string& filename);
    void reset();
private:
    Server* _server;
};

struct Firmware : public ParamSet {
    Param<string>   file;
    Param<bool>     multipart;
    Firmware() :
        PARAM(file, ""),
        PARAM(multipart, false) 
    {}
};

struct RawCommand: public ParamSet {
    Param<string>   subsystem;
    Param<string>   cmd;
    Param<string>   inp_file;
    Param<string>   out_file;
    Param<bool>     multipart;
    RawCommand() :
        PARAM(subsystem, "ipp", VerifyEnum("ipp", "firmware")),
        PARAM(cmd,       ""),
        PARAM(inp_file,  ""),
        PARAM(out_file,  "raw_reply"),
        PARAM(multipart, false)
    {}
};

#undef PARAM

}

#endif
