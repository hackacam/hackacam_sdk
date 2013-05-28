#pragma once
#ifndef _RTSP_TALKER_H
#define _RTSP_TALKER_H
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
#include "rtsp_session_id.h"
#include "rtsp_server.h"

namespace RTSP {
class Source;
class Client;

namespace RTCP {
    class Parser;
}

//! Manages all RTSP communication with RTSP client
class Talker: public SBL::Thread {
public:
    //! Create a talker object
    //! @param  socket  socket to talk to RTSP client to
    //! @param  id      id of this talker
    //! @param  master  pointer to the master server
    Talker(const SBL::Socket socket, int thread_id, Server* master);

    //! Find the Source object associated with the stream.
    Source* get_source(const char* stream_name);

    //! Return Client attached to this server
    Client* client() const { return _client; }

    //! Setup a TCP connection to the client for this stream (TCP over RTSP)
    SessionID setup_tcp(const char* stream_name);

    //! Setup a UDP connection to the client for the stream (RTP over UDP)
    SessionID setup_udp(const char* stream_name, int client_port0,
                                                 int client_port1);

    //! Teardown a session for this stream
    void teardown();

    //! Return server ip address in 192.168.1.125 notation
    const char* server_ip() const { return _server_ip; }

    //! Return client ip address in 192.168.1.125 notation
    const char* client_ip() const { return _client_ip; }

    //! Return server port used to send RTP packets from
    unsigned int server_port() const { return _server_port; }

    //! Return client port used to send RTP packets to
    unsigned int client_port() const { return _client_port; }

    //! Return current SessionID
    SessionID session_id() const { return _session_id; }

    //! return this server id
    int id() const { return _id; }

    //! Master options
    const Server::Options* options() const { return _master->options(); }

private:
    static const int BUFFER_SIZE = 1024; 
    int             _id;
    SBL::Socket     _socket;
    char            _rx_buffer[BUFFER_SIZE];
    char            _tx_buffer[BUFFER_SIZE];
    int             _rx_bytes;    // how many bytes there are in rx_buffer
    int             _msg_size;    // size of the message in rx_buffer
    Server*         _master;      // NULL for master server
    RTCP::Parser*   _rtcp_parser;
    Client*         _client;
    Source*         _source;
    SessionID       _session_id;

    char            _server_ip[SBL::Socket::IP_ADDR_BUFF_SIZE];  // server (local) IP address
    char            _client_ip[SBL::Socket::IP_ADDR_BUFF_SIZE];  // client (remote) IP address
    unsigned int    _server_port;                           // server (local) port
    unsigned int    _client_port;                           // client (remote) port

    enum    MsgType { MSG_RESET, MSG_RTSP, MSG_RTCP};
    // Receive one full message and place it in _rx_buffer
    MsgType receive_msg();
    // Receive RTSP message and place it in _rx_buffer
    MsgType receive_rtsp();
    // Receive RTCP message and place it in _rx_buffer
    MsgType receive_rtcp();
    // Get next chunk of data in _rx_buffer, return how many got from last recv()
    int     receive();
    // Thread start procedure
    void    start_thread();
    // currently unused, prints message with readable \r\n
    void log(const char* buffer, int buffer_size);
};

}
#endif
