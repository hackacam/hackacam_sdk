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
#include <cstring>
#include <sbl/sbl_logger.h>
#include "rtcp.h"
#include "rtsp_impl.h"
#include "rtsp_talker.h"
#include "rtp_streamer.h"

namespace RTSP {
namespace RTCP {

int Parser::id() const {
    return _talker->id();
}

// Very simplistic parsing, should really check packet type and process accordingly
// This assumes that RTCP messages is always RR followed by SDES.
bool Parser::parse(char* buffer, unsigned int size) {
    if (size < sizeof(report) - sizeof(report.sdes.name) || size > sizeof(report) ) {
        SBL_WARN("RTCP %d, got packet size %d, expected %d-%d, ignoring", 
                 id(), size, sizeof(report) - sizeof(report.sdes.name), sizeof(report));
        return false;
    }
    memcpy(&report, buffer, size);
    report.rr.flags             = ntohs(report.rr.flags);
    report.rr.length            = ntohs(report.rr.length);
    report.rr.ssrc              = ntohl(report.rr.ssrc);
    report.rr.fraction_lost     = ntohl(report.rr.fraction_lost);
    report.rr.cumulative_lost   = ntohl(report.rr.cumulative_lost);
    report.rr.highest_seq       = ntohl(report.rr.highest_seq);
    report.rr.last_sr           = ntohl(report.rr.last_sr);
    report.rr.delay_last_sr     = ntohl(report.rr.delay_last_sr);

    report.sdes.flags           = ntohs(report.sdes.flags);
    report.sdes.length          = ntohs(report.sdes.length);
    report.sdes.ssrc            = ntohl(report.sdes.ssrc);

    report.rr.fraction_lost = (report.rr.cumulative_lost >> 24) & 0x00FF;
    report.rr.cumulative_lost = (report.rr.cumulative_lost << 8) >> 8;
    if ((report.rr.flags & 0x00FF) != RR_PACKET_TYPE) {
        SBL_WARN("RTCP %d, expected RR packet (%d), got %d, ignoring", 
                 id(), RR_PACKET_TYPE, report.rr.flags & 0x00FF);
        return false;
    }
    if ((report.sdes.flags & 0x00FF) != SDES_PACKET_TYPE) {
        SBL_WARN("RTCP %d, expected SDES packet (%d), got %d, ignoring", 
                 id(), SDES_PACKET_TYPE, report.sdes.flags & 0x00FF);
        return false;
    }
    SBL_MSG(MSG::RTCP,
             "Received RTCP Packet for thread %d:\n"
             "  Fraction packets lost:   %%%.1f\n"
             "  Cumulative packet lost:  %d\n"
             "  Highest sequence number: %d\n"
             "  Jitter:                  %d\n"
             "  Last SR:                 %d\n"
             "  Delay last SR:           %d\n"
             "  From receiver:           %s\n",
             id(),
             100.0 * report.rr.fraction_lost / 256.0, 
             report.rr.cumulative_lost,
             report.rr.highest_seq,
             report.rr.jitter,
             report.rr.last_sr,
             report.rr.delay_last_sr,
             report.sdes.name);
    return true;
}


// This is a simplistic processing, assumes RTCP RR/SDES arrives always in a single UDP packet.
void Parser::start_thread() {
    _thread_active = true;
    SBL_MSG(MSG::RTCP, "Starting thread to listen to RTCP messages for thread %d", id());
    do {
        int recv = _socket.recv(_buffer, BUFF_SIZE);
        SBL_MSG(MSG::RTCP, "Received RTCP message size %d", recv);
        if (parse(_buffer, recv) && congestion_control())
            adjust_bitrate();
    } while (1);
}

bool Parser::congestion_control() const {
    return _talker->options()->temporal_levels;
}

void Parser::set_congestion_control(bool enable) {
    SBL_MSG(MSG::RTCP, "RTCP %d, setting congestion control to %s", id(), enable ? "true" : "false");
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    _last_loss_time = time.tv_sec;
    _packet_loss = 0;
}

void Parser::adjust_bitrate() {
    RTSP_ASSERT(_talker->client(), INTERNAL_SERVER_ERROR);
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    //  - decrease level is packet loss == 0 for 1 minute
    if (report.rr.fraction_lost == 0) {
        if (time.tv_sec - _last_loss_time > _talker->options()->increase_time) {
            SBL_MSG(MSG::RTCP, "RTCP %d, reducing level for client %p", id(), _talker->client());
            _talker->client()->reduce_level();
            _last_loss_time = time.tv_sec;
        }
    } else {
        _last_loss_time = time.tv_sec;
    }
    /* increase level if packet loss > 2%, but we need to be careful to not increase too fast
       -2  -1   0
       ----------
            0   0   nothing
            0   1   increase
            1   0   nothing
       0    1   1   nothing
       1    1   1   increase
    */
    _packet_loss = (_packet_loss << 1) | ((report.rr.fraction_lost > INCREASE_PERC * 256 / 100) ? 1 : 0);
    SBL_MSG(MSG::RTCP, "RTCP %d, packet_loss flags %d", id(), _packet_loss & 7);
    if ((_packet_loss & 3) == 1 || (_packet_loss & 7) == 7) {
        SBL_MSG(MSG::RTCP, "RTCP %d, increasing level for client %p", id(), _talker->client());
        _talker->client()->increase_level();
    }
}

Parser::~Parser() {
    SBL_MSG(MSG::RTCP, "Destroying RTCP Parser %d", id());
    _socket.close();
}

void Parser::kill() {
    if (_thread_active) {
        SBL_MSG(MSG::RTCP, "Killing RTCP Parser %d", id());
        kill_thread();
    }
}

}
}
