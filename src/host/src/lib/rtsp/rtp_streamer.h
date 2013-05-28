#pragma once
#ifndef _RTSP_RTP_STREAMER_H
#define _RTSP_RTP_STREAMER_H
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
#include <cstdlib>
#include <list>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_socket.h>
#include <sbl/sbl_thread.h>

namespace RTSP {
class Source;
class Streamer;
class Talker;

//! Represents a single remote client.
class Client { 
public:
    //! Client constructor
    // @param   sock    Socket associated with the client
    // @param   str     parent Streamer object
    Client(SBL::Socket sock, Streamer* str, SBL::Socket rtcp_socket, Talker* talker);
    //! send RTP packet
    void send(uint8_t* packet, int size, bool last_packet);
    //! return current timestamp
    uint32_t  timestamp()  const;
    //! return current sequence number
    uint16_t  seq_number() const;
    //! return socket this client sends to
    SBL::Socket    socket()     const { return _socket; }
    //! return pointer to the associated streamer
    Streamer* streamer()   const { return _streamer; }
    //! return associated talker
    Talker*   talker()     const { return _talker; }
    //! start streaming
    void play();
    //! stop streaming
    void stop() { _state = STOP; }
    //! send sender RTCP packet
    void send_sender_rtcp();
    //! set new temporal level
    void set_temporal_level(unsigned int level);
    //! increase rate
    void increase_level();
    //! decrease rate
    void reduce_level();
    //! Unique ID of this client
    int id() const;
private:
    enum State {STOP, REQUEST, PLAY};
    enum {RTCP_INTERVAL = 5 * 90000, TEMPORAL_LEVELS = 3};
    State       _state;    
    SBL::Socket _socket;  
    Streamer*   _streamer; 
    Talker*     _talker;
    // For TCP, additional data must be inserted in front of the frame
    int         _offs;  // so _offs is either 0 or 4.
    // Socket for RTCP packets out
    SBL::Socket _rtcp_socket;
    // total bytes sent since begining of time
    uint32_t    _total_bytes;
    // total packet sent since begning of time
    uint32_t    _total_packets;
    // last time (in timestamp ticks) when rtcp packet was sent
    uint32_t    _last_rtcp_packet;
    // this client sequence number
    uint16_t    _seq_number;
    // current temporal level (0, 1, 2), 0 is full, 2 is 4X
    unsigned int _temporal_level;
    // returns true if this frame should be skipped
    bool        skip_frame(unsigned int frame_index) {
        return frame_index & (3 >> (2 - _temporal_level));
    }
    friend class Streamer;
};

//! Responsible for sending frames to remote clients.
/*! Streamer supports both UDP and TCP (interleaved RTSP) transport mechanisms. For TCP, it prepends interleaved
    4-byte prefix to the frame, but otherwise doesn't distinguish between UDP and TCP.\n
    @b IMPORTANT:
    @li Each call to send_frame() should present a full frame, i.e. a NAL unit (Network Abstraction Layer)
        and frame parameter must point to NAL unit type octet.
    @li frame must have 17 bytes *in front* of it free for the first RTP header
    @li sources must send sps/pps before each I-frame
*/
class Streamer {
    friend class Client;
public:

    //! Streamer constructor
    // @param   packet_size  by default, packet_size is 1434, which prevents IP fragmentation
    // @param   ssrc         initial ssrc, by default it is a random number
    // @param   seq_number   initial sequence number, by default it is a random number   
    Streamer(int packet_size = -1, int ssrc = -1, int seq_number = -1);
    //! send a frame to all connected clients
    // @param   frame       frame pointer (must have 17 bytes in front of it free
    // @param   frame_size  size of the frame, in bytes
    // @param   timestamp   Streamer doesn't process timestamps, just forwards them in packets
    void send_frame(uint8_t* frame, int frame_size, uint32_t timestamp);

    //! add a client with a given socket
    // @param   socket  socket to use for this client
    // @details This method will wait on a mutex to avoid updating Streamer client list while 
    // packets are being sent.
    Client* add_client(SBL::Socket socket, SBL::Socket rtcp_socket, Talker* talker = NULL);
    
    //! Remove a client from a client list
    // @param   client  pointer to the client to remove.
    void delete_client(Client* client);
    
    //! return a Source associated with this Client
    Source* source()  const { return _source; }
    
    //! Set a source associated with this Client
    void set_source(Source* source) { _source = source; }
    
    //! Return how many clients with Streamer has. 
    //! This includes all client, active (play = true) or inactive (play = false)
    int client_count() const { return _clients.size(); }
    
    //! Return the current timestamp.
    //! Streamer forwards this to source, because it may not be receiving any frames yet
    uint32_t timestamp();

    //! Return the current sequence number.
    uint32_t seq_number() { return _seq_number; }

    //! Return the current frame index
    unsigned int frame_index() const { return _frame_index; }

    //! Set new temporal level for all clients (for testing)
    void set_temporal_level(unsigned int level);
private:
    // RTP header
    struct RTPHdr {
        uint16_t    flags;
        uint16_t    seq_number;
        uint32_t    timestamp;
        uint32_t    ssrc;
    };
    struct MJPEGHdr {
        uint32_t    fragment_offset; // MJPEG type is fixed
        uint8_t     type;
        uint8_t     quality;
        uint8_t     width;
        uint8_t     height;
    };
    // Constants
    enum    {
             RTP_HEADER     = sizeof(struct RTPHdr), 
             MJPEG_HEADER   = sizeof(struct MJPEGHdr),
             MJPEG_TYPE     = 1,
             RTP_SEQ_NUM    = 2,
             NAL_SPS        = 7,
             FU_INDICATOR   = 1,  // size of FU-A Indicator in bytes
             FU_HEADER      = 1,     // size of FU-A Header in bytes
             NAL_TYPE_FU_A  = 28,    // Fragmentation Unit A
             NAL_TYPE_MASK  = 0x1F,  // Mask to get NAL Type from NAL Header (5 bits) 
             NAL_START_BIT  = 1 << 7,    // Start and End bit in NAL Header
             NAL_END_BIT    = 1 << 6,
             RTP_VERSION_NUMBER = 2 // RTP version (is always 2)
             };
    typedef std::list<Client*> Clients;
    Clients         _clients;           // clients for this streamer
    Source*         _source; 
    std::string     _name;
    int             _packet_size;       // Transport protocol (UDP/TCP) packet size;
    uint32_t        _ssrc;              // synchronization source identifier
    uint32_t        _timestamp;
    uint16_t        _seq_number;        // rtp packet sequence number
    uint8_t         _fu_header;         // FU-A header
    uint8_t         _fu_indicator;      // FU-A indicator
    SBL::Mutex      _lock;              
    unsigned int    _frame_index;       // 0 for SPS/PPS/I-frame, increments thereafter
    char            _frame_type;
    bool            _mp4_starter_frame;    

    // Add RTP header 
    void write_rtp_header(uint8_t* frame, bool last_packet);
    // Add FU header and indicator bytes
    inline void write_fu_header (uint8_t* frame, bool last_packet);
    // create FU-A header and indicator
    inline void create_fu_header(uint8_t* frame);
    // send single RTP packet. 
    void send_packet(uint8_t* frame, int tx_size, bool last_packet);
    // current frame type
    char frame_type() const { return _frame_type; }
    bool is_mpeg4_starter_frame() {return _mp4_starter_frame;}
    void h264_send_frame (uint8_t* frame, int frame_size);
    void mjpeg_send_frame(uint8_t* frame, int frame_size);
    void mpeg4_send_frame (uint8_t* frame, int frame_size);
    void write_mjpeg_header(uint8_t* frame, uint32_t offset);
};
/* 
RTP_PAYLOAD size calculations:
    MTU              1500 bytes (maximum Ethernet payload)
    IP header          20 bytes (no options needed)
    UDP header          8 bytes
    UDP payload      1472 bytes
    RTP header         12 bytes
    FU header           1 byte
    FU indicator        1 byte
    RTP payload      1458 bytes - backed off to 1456
*/
}
#endif
