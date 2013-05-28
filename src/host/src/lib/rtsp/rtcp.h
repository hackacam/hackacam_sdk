#pragma once
#ifndef _RTCP_H
#define _RTCP_H
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
#include <sbl/sbl_socket.h>
#include <sbl/sbl_thread.h>
/*
SR: Sender Report RTCP Packet


        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
header |V=2|P|    RC   |   PT=SR=200   |             length            |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         SSRC of sender                        |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
sender |              NTP timestamp, most significant word             |
info   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |             NTP timestamp, least significant word             |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         RTP timestamp                         |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                     sender's packet count                     |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                      sender's octet count                     |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   version (V): 2 bits
      Identifies the version of RTP, which is the same in RTCP packets
      as in RTP data packets.  The version defined by this specification
      is two (2).
   padding (P): 1 bit
   reception report count (RC): 5 bits
      The number of reception report blocks contained in this packet.  A
      value of zero is valid.

   packet type (PT): 8 bits
      Contains the constant 200 to identify this as an RTCP SR packet.


SDES: Source Description RTCP Packet
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
header |V=2|P|    SC   |  PT=SDES=202  |             length            |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
chunk  |                          SSRC/CSRC_1                          |
  1    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                           SDES items                          |
       |                              ...                              |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   source count (SC): 5 bits
      The number of SSRC/CSRC chunks contained in this SDES packet.  A
      value of zero is valid but useless.

CNAME: Canonical End-Point Identifier SDES Item
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    CNAME=1    |     length    | user and domain name        ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

RR: Receiver Report RTCP Packet
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
header |V=2|P|    RC   |   PT=RR=201   |             length            |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                     SSRC of packet sender                     |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
report |                 SSRC_1 (SSRC of first source)                 |
block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  1    | fraction lost |       cumulative number of packets lost       |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |           extended highest sequence number received           |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                      interarrival jitter                      |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         last SR (LSR)                         |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                   delay since last SR (DLSR)                  |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*/

namespace RTSP {
class Talker;

namespace RTCP {


enum {  ITEM_COUNT = 1, 
        CNAME      = 1,
        SR_PACKET_TYPE = 200, 
        RR_PACKET_TYPE = 201,
        SDES_PACKET_TYPE = 202,
        NTP_OFFSET = 2208988800U // offset between Unix time and NTP
        };

struct SR {
    uint16_t    flags;
    uint16_t    length;
    uint32_t    ssrc;
    uint32_t    ntp_h;
    uint32_t    ntp_l;
    uint32_t    rtp_ts;
    uint32_t    packets;
    uint32_t    bytes;
};

struct SDES {
    uint16_t    flags;
    uint16_t    length;
    uint32_t    ssrc;
    uint8_t     type;
    uint8_t     item_length;
    char        name[64];
} __attribute__ ((packed));

struct RR {
    uint16_t    flags;
    uint16_t    length;
    uint32_t    ssrc;
    uint32_t    fraction_lost;      // replaces SSRC_1
    int32_t     cumulative_lost;    // extended from 24 bits
    uint32_t    highest_seq;
    uint32_t    jitter;
    uint32_t    last_sr;
    uint32_t    delay_last_sr;
};

struct Sender {
    char    tcp[4];
    SR      sr;
    SDES    sdes;
} __attribute__ ((packed));

struct Receiver {
    RR      rr;
    SDES    sdes;
}  __attribute__ ((packed));

// Let's check that the structures are correctly packed
SBL_STATIC_ASSERT(sizeof(SR)     == 28);
SBL_STATIC_ASSERT(sizeof(SDES)   == 74);
SBL_STATIC_ASSERT(sizeof(Sender) == 106);
SBL_STATIC_ASSERT(sizeof(RR)     == 32);

//! Parses RTCP messages and does rudimentary bitrate control when enabled.
class Parser : public SBL::Thread {
public:
    //! Last decoded Receiver report. It is overwritten with each received RTCP packet
    Receiver  report;
    //! Parse RTCP packet from the buffer and place in report.
    //! return true if parsing was successful
    bool parse(char* buffer, unsigned int size);
    //! Associates parser with the control talker. 
    /*! Listens for RTCP packets on the socket, which can be UDP or TCP. Use model is different:
     *      - for UDP, Parser runs in a separate thread, listening on a socket
     *      - for TCP, Parser runs in the same thread as Talker, which simple calls parse() function
     *        after receiving a RTCP message.
    */
    Parser(Talker* talker, SBL::Socket socket) : 
          _talker(talker), _socket(socket), _thread_active(false),
          _packet_loss(0) {}
    //! Closes the socket on termination
    ~Parser();
    //! Enable to disable congestion control
    void set_congestion_control(bool enable);
    //! If Parser runs in a separate thread, kills that thread
    void kill();
    //! unique ID of this Parser
    int id() const;
private:
    enum {BUFF_SIZE = 200, INCREASE_PERC = 2};
    Talker*         _talker;
    SBL::Socket     _socket;
    char            _buffer[BUFF_SIZE];
    bool            _thread_active;
    unsigned int    _packet_loss; 
    int             _last_loss_time;

    void    start_thread();
    void    adjust_bitrate();
    // True if congestion control is enabled
    bool    congestion_control() const;
};
}
}
#endif
