#pragma once
#ifndef _RTSP_SERVER_H
#define _RTSP_SERVER_H
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

namespace RTSP {
class SourceMap;
class Source;

//! Main server class, listens on a port and starts Talker thread for each new client.
class Server : public SBL::Thread {
public:
    //! Server options
    struct Options {
        int   packet_size;      //!< RTP packet size
        int   fps;              //!< Frames per second (file sources only)
        int   ts_clock;         //!< Timestamp clock in Hz (file sources only)
        int   send_buff_size;   //!< TCP socket send buffer size
        int   recv_buff_size;   //!< TCP socket receive buffer size
        bool  tcp_nodelay;      //!< TCP NODELAY socket option
        bool  tcp_cork;         //!< TCP_CORK socket option
        bool  temporal_levels;  //!< enable congestion control using temporal levels
        int   increase_time;    //!< rate increase timeout (seconds) for temporal level
        int   packet_gap;       //!< time gap in nanoseconds to add between packets
        Options() : packet_size(1456), fps(30), ts_clock(90000),
                    send_buff_size(0), recv_buff_size(0),
                    tcp_nodelay(true), tcp_cork(false),
                    temporal_levels(false), increase_time(60) {}
    };
    //! Create a new Server.
    /** This is the only way to create a new server. The object will be allocated on the heap.
        This object should never be deleted and the behavior is undefined when it is */
    static Server* create(const short int port, const Options& options);

    //! Create a new Server with default options
    static Server* create(const short int port) { return create(port, Options()); }

    //! Find the Source object associated with the stream.
    Source* get_source(const int stream_id);

    //! set temporal level for all clients (testing)
    void set_temporal_level(unsigned int level);

    //! return source map
    SourceMap* source_map() { return _source_map; }

    //! return options
    Options* options() { return &_options; }
    //! return how many clients are currently attached to a given stream or
    //! -1 if the given stream_id is invalid
    int client_count(unsigned int stream_id) const;
    //! print verbosity levels
    static void print_verbosity_levels(std::ostream& str);
    //! busy wait between packets
    void packet_wait();
    //! update packet_gap
    void set_packet_gap(int packet_gap) { _options.packet_gap = packet_gap; }
    //! public lock procedure
    void lock()     { _lock.lock(); }
    //! public unlock procedure
    void unlock()   { _lock.unlock(); }
private:
    Server();                           // not implemented
    Server(const Server&);              // not implemented
    Server& operator=(const Server&);   // not implemented

    Server(const short int port, const Options& options);

    Options         _options;
    SBL::Socket      _socket;
    // Receive and transmit buffers
    static const int STACK_SIZE = 64 * 1024; 
    SBL::Mutex      _lock;
    SourceMap*      _source_map;
    struct timespec _packet_tick;

    void start_thread();
    
};

}
#endif
