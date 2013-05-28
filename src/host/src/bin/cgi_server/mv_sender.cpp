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
#include "mv_sender.h"

namespace CGI {

void MVSender::send_mv_packet(uint8_t* frame, unsigned int frame_size, uint32_t timestamp) {
    if (_motion_rate == 0 || ++_frame_counter < _motion_rate)
        return;
    SBL_MSG(MSG::MV, "sender %p, dest %s:%d, frame size %d, ts %d, rate %d, counter %d", this, _ip_address.c_str(), _port,
                        frame_size, timestamp, _motion_rate, _frame_counter);
    _frame_counter = 0;
    _mv_hdr.seq_num     = htons(_seq_num++);
    _mv_hdr.timestamp   = htonl(timestamp);    
    frame -= sizeof(_mv_hdr);
    memcpy(frame, &_mv_hdr, sizeof(_mv_hdr));
    Socket socket(Socket::UDP);
    _lock.lock();
    socket.send(frame, frame_size + sizeof(_mv_hdr), _ip_address.c_str(), _port, false);
    _lock.unlock();
    socket.close();
}

void MVSender::set_rate(const string& address, int motion_rate) {
    size_t colon = address.find(':');
    CGI_ERROR(colon == std::string::npos, "missing port number");
    int port = convert<int, const char*>(address.substr(colon + 1).c_str());
    _lock.lock();
    _ip_address = address.substr(0, colon);
    _port = port;
    _motion_rate = motion_rate;
    _lock.unlock();
    SBL_INFO("sender is %p, motion_dest is %s:%d, rate is %d", this, _ip_address.c_str(), _port, _motion_rate);
}

}
