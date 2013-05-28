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
#include <algorithm>
#include <math.h>
#include "sdk_manager.h"
#include "temperature.h"
#include "cgi_server.h"

namespace CGI {

Temperature::Temperature(Server* server, const std::string& filename) : 
                        _have_sensor(false), _current(0.0),
                        _min(1000.0), _max(-1000.0), _abs_min(1000.0), _abs_max(-1000.0),
                        _filename(filename), _server(server) {
    // attempt to read absolute min and max from the temperature file
    std::ifstream ifs(filename.c_str());
    if (!ifs) {
        SBL_WARN("Unable to load file %s", filename.c_str());
        return;
    }
    int temps = 0;
    while (!ifs.eof()) {
        string line;
        getline(ifs, line);
        if (line.length() == 0 || line[0] == '#')
            continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            SBL_ERROR("error reading temperature from %s, line %s", filename.c_str(), line.c_str());
            ifs.close();
            return;
        }
        if (line.substr(0, eq) == "min_temp") {
            _abs_min = strtof(line.c_str() + eq + 1, NULL); 
            temps |= 1;
        } else if (line.substr(0, eq) == "max_temp") {
            _abs_max = strtof(line.c_str() + eq + 1, NULL); 
            temps |= 2;
        }
    }
    if (temps != 3)
        SBL_WARN("Missing min or max temperature in %s", filename.c_str());
    SBL_INFO("min, max temperature at boot time is (%.1f,%.1f)", _abs_min, _abs_max);
    ifs.close();
}

std::ostream& operator<<(std::ostream& str, const Temperature& temperature) {
    // we don't print anything if we don't have a sensor
    if (temperature.has_sensor()) {
        std::ios_base::fmtflags flags = str.flags();
        std::streamsize precision = str.precision();
        str << "temperature=" << fixed << setprecision(1) 
            << temperature._current << ':'
            << temperature._min     << ':'
            << temperature._max     << ':'
            << temperature._abs_min << ':'
            << temperature._abs_max
            << setprecision(precision);
        str.flags(flags);
    }
    return str;
}

void Temperature::start_thread() {
    _have_sensor = _server->sdk_manager()->read_temperature(&_current);
    if (!_have_sensor) {
        SBL_INFO("Missing temperature sensor, thread exiting");
        return;
    }
    bool write_error = false;
    SBL_INFO("Sampling temperatures each %d seconds", SAMPLING_RATE);
    do {
        _min = min(_current, _min);
        _max = max(_current, _max);
        SBL_MSG(MSG::TEMP, "Temperature is %f:%f:%f:%f:%f", _current, _min, _max, _abs_min, _abs_max);
        if (!write_error && (_min < _abs_min || _max > _abs_max )) {
            std::ofstream ofs(_filename.c_str());
            if (ofs) {
                ofs << "# IP Camera temperature" << endl
                    << "# File written on " << Date::timestamp() << " UTC" << endl
                    <<  fixed << setprecision(1)
                    << "min_temp="  << _min << std::endl
                    << "max_temp="  << _max << std::endl;
                ofs.close();
            } else {
                SBL_ERROR("Unable to write into temperature file %s", _filename.c_str());
                write_error = true;
            }
        }
        _abs_min = min(_abs_min, _min);
        _abs_max = max(_abs_max, _max);

        sleep(SAMPLING_RATE);
        bool status = _server->sdk_manager()->read_temperature(&_current);
        if (!status) {
            SBL_ERROR("Unable to read temperature");
            return;
        }
    } while (true);
}

}
