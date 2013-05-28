#pragma once
#ifndef _TEMPERATURE_H
#define _TEMPERATURE_H
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
#include <sbl/sbl_thread.h>

namespace CGI {
class Server;

//! This class handles temperature measurements
class Temperature : public SBL::Thread {
public:
    //! Constructor reads abs_min/max values from the temperature file
    Temperature(Server* server, const std::string& filename);
    //! returns true if the board has the sensor
    bool has_sensor() const { return _have_sensor; }
private:
    bool        _have_sensor;   //!< true if board has sensor
    float       _current;       //!< last measurement
    float       _min;           //!< min since boot
    float       _max;           //!< max since boot
    float       _abs_min;       //!< min forever
    float       _abs_max;       //!< max forever
    std::string _filename;      //!< temperature file name
    Server*     _server;        //!< pointer to server

    // the thread sleeps for SAMPLING_RATE seconds between measurements
    static const int SAMPLING_RATE = 10;

    void    start_thread();
    friend std::ostream& operator<<(std::ostream& str, const Temperature& temperature);
};

}
#endif
