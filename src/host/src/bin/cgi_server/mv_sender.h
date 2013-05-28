#pragma once
#ifndef _CGI_MV_SENDER_H
#define _CGI_MV_SENDER_H
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
#include <sbl/sbl_socket.h>
#include <sbl/sbl_thread.h>
#include "cgi_param_set.h"

namespace CGI {

/// Motion Vector sender class
class MVSender {
public:
    MVSender() :  _ip_address(""), _port(0), _seq_num(0), 
                 _motion_rate(0), _frame_counter(0) {}
    void send_mv_packet(uint8_t* frame, unsigned int frame_size, uint32_t timestamp);
    void set_rate(const std::string& address, int motion_rate);
private:
    string           _ip_address;
    int              _port;
    Mutex            _lock;
    unsigned int     _seq_num;
    unsigned int     _motion_rate;
    unsigned int     _frame_counter;

    struct MVHdr {
        char        m;
        char        v;
        uint16_t    seq_num;
        uint32_t    timestamp;
        MVHdr() : m('M'), v('V') {}
    } _mv_hdr;
    SBL_STATIC_ASSERT(sizeof(MVHdr) == 8);
};

}
#endif
