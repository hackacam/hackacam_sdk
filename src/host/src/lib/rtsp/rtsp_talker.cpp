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
#include <cctype>
#include <sbl/sbl_exception.h>
#include <sbl/sbl_logger.h>

#include "rtsp_talker.h"
#include "rtsp_server.h"
#include "rtsp_parser.h"
#include "rtsp_responder.h"
#include "file_source.h"
#include "live_source.h"
#include "source_map.h"
#include "rtcp.h"

namespace RTSP {

Talker::Talker(const SBL::Socket socket, int id, Server* master) :
        _id(id),  _socket(socket), 
        _rx_bytes(0), _msg_size(0), _master(master), _rtcp_parser(NULL),
        _client(NULL), _source(NULL), _session_id("") {
    _server_port = _socket.local_address(_server_ip);
    _client_port = _socket.remote_address(_client_ip);
    SBL_MSG(MSG::SERVER, "Created RTSP talker id %d", id);
}

void Talker::start_thread() {
    SBL_INFO("RTSP talker thread %d listening to %s:%d", id(), _client_ip, _client_port);
    Parser      parser;
    Responder   responder(this, _tx_buffer, BUFFER_SIZE);
    Method      method = OPTIONS;
    do {
        try {
            MsgType msg_type = receive_msg();
            if (msg_type == MSG_RTSP) {
                SBL_MSG(MSG::SERVER, "RTSP Server thread %d received message length %d:\n%s", 
                         id(), _msg_size, _rx_buffer);
                method = parser.parse(_rx_buffer, _msg_size);
                int reply_size = responder.reply(parser.data);
                SBL_MSG(MSG::SERVER, "RTSP Server thread %d reply:\n%s", id(), _tx_buffer);
                if (!_socket.send(_tx_buffer, reply_size, false)) {
                    SBL_MSG(MSG::SERVER, "RTSP Server thread %d, socket error, exiting thread ...\n", id());
                    break;
                }
                if (method == PLAY) {
                    RTSP_ASSERT(client(), BAD_REQUEST);
                    client()->play();
                }
            } else if (msg_type == MSG_RTCP) {
                SBL_MSG(MSG::SERVER, "RTSP Server thread %d received RTCP message length %d", 
                         id(), _msg_size);
                if (_rtcp_parser) {
                    RTSP_ASSERT(_msg_size >= 4, BAD_REQUEST);
                    _rtcp_parser->parse(_rx_buffer + 4, _msg_size - 4);
                }
                else
                    SBL_WARN("Thread %d, received RTCP message, but RTCP parser is not yet ready", id());
            } else
                method = TEARDOWN;
            _rx_bytes -= _msg_size;
            if (_rx_bytes > 0)
                memmove(_rx_buffer, _rx_buffer + _msg_size, _rx_bytes);
        } catch (Errcode errcode) {
            // If we catch Errcode, it may be possible to continue, so reply with error code
            SBL_MSG(MSG::SERVER, "RTSP talker %d caught error code %d", id(), errcode);
            int reply_size = responder.reply(parser.data, errcode);
            SBL_MSG(MSG::SERVER, "RTSP talker thread %d reply:\n%s\n", id(), _tx_buffer);
            if (!_socket.send(_tx_buffer, reply_size, false)) {
                SBL_MSG(MSG::SERVER, "Talker %d unable to send reply, exiting...", id());
                method = TEARDOWN;
            }
            // We throw out any data already received if we had error
            _rx_bytes = _msg_size = 0;
        } catch (SBL::Exception& ex) {
            // if we catch Exception, it is coming from Socket, so can't send anything back    
            // log the error in the log file and terminate the thread;
            SBL_MSG(MSG::SERVER, "RTSP talker %d exiting, caught exception %s", id(), ex.what());
            method = TEARDOWN;
        }
    } while (method != TEARDOWN);
    teardown();
    SBL_INFO("RTSP talker %d terminating", id());
    delete this;
}


Talker::MsgType Talker::receive_msg() {
    if (!receive()) 
        return MSG_RESET;
    if (_rx_buffer[0] == '$')
        return receive_rtcp();
    return receive_rtsp();
}

Talker::MsgType Talker::receive_rtcp() {
    while (_rx_bytes < 4) {
        if (!receive())
            return MSG_RESET;
    }
    RTSP_ASSERT(_rx_buffer[1] == 1, BAD_REQUEST);
    _msg_size = (((_rx_buffer[2] & 0x00ff) << 8) | (_rx_buffer[3] & 0x00ff)) + 4;
    while (_rx_bytes < _msg_size) {
        if (!receive())
            return MSG_RESET;
    }
    return MSG_RTCP;
}

Talker::MsgType Talker::receive_rtsp() {
    const char eom[] = "\r\n\r\n";
    const int sizeof_eom = sizeof(eom) - 1;
    int ptr = 0;
    do {
        for (; ptr <= _rx_bytes - sizeof_eom; ptr++)
            if (!memcmp(_rx_buffer + ptr, eom, sizeof_eom)) {
                _msg_size = ptr + sizeof_eom;
                _rx_buffer[_msg_size] = '\0';
                return MSG_RTSP;
            }
        if (!receive())
            return MSG_RESET;
    } while (1);
}

int Talker::receive() {
    // Always leave space to append \0 for logging.
    RTSP_ASSERT(_rx_bytes < BUFFER_SIZE - 1, SERVER_BUFFER_OVERFLOW);
    int size = _socket.recv(_rx_buffer + _rx_bytes, BUFFER_SIZE  - _rx_bytes - 1);
    _rx_bytes += size; 
    SBL_MSG(MSG::SERVER, "Received %d bytes, have total %d bytes", size, _rx_bytes);
    return size;
}

Source* Talker::get_source(const char* stream_name) {
    if (_source)
        return _source;
    _master->lock();
    _source = _master->source_map()->find(stream_name);
    _master->unlock();
    Errcode errcode = OK;
    if (!_source) {
        int stream_id = application()->get_stream_id(stream_name);
        SBL_MSG(MSG::SERVER, "Server %d, application return id %d for stream %s", id(), stream_id, stream_name);
        _master->lock();
        try {
            if (stream_id < 0) {
                _source = FileSource::create(stream_name, new Streamer(_master->options()->packet_size), _master->options()->fps, _master->options()->ts_clock);
                _master->source_map()->save(stream_name, _source);
                SBL_MSG(MSG::SERVER, "Server %d created file source %p for stream %s", id(), _source, stream_name);
            } else {
                _source = new LiveSource(stream_id, new Streamer(_master->options()->packet_size));
                _master->source_map()->save(stream_id, _source, stream_name);
                SBL_MSG(MSG::SERVER, "Server %d created live source %p for stream %s", id(), _source, stream_name);
            }
        } catch (Errcode err) {
            errcode = err;
        }
        _master->unlock();
    }
    if (errcode != OK)
        throw errcode;
    return _source;
}

SessionID Talker::setup_tcp(const char* stream_name) {
    RTSP_ASSERT(_source, INTERNAL_SERVER_ERROR);
    if (_master->options()->tcp_nodelay)
        _socket.set_option(SBL::Socket::NO_DELAY, _master->options()->tcp_nodelay);
    if (_master->options()->send_buff_size)
        _socket.set_option(SBL::Socket::SEND_BUFF_SIZE, _master->options()->send_buff_size);
    if (_master->options()->tcp_cork)
        _socket.set_option(SBL::Socket::CORK, _master->options()->tcp_cork);
    _server_port = _socket.local_address(_server_ip);
    _client_port = _socket.remote_address(_client_ip);
    _client = _source->streamer()->add_client(_socket, _socket, this);
    _session_id = SessionID::generate();
    // For TCP, we don't need a socket for RTCP
    _rtcp_parser = new RTCP::Parser(this, SBL::Socket(SBL::Socket::NONE));
    SBL_INFO("Server %d, %s stream (TCP) on socket %d for client %s:%d", id(), _source->encoder_name(), _socket.id(), _client_ip, _client_port);
    return _session_id;
}

SessionID Talker::setup_udp(const char* stream_name, int client_port0, int client_port1) {
    RTSP_ASSERT(_source, INTERNAL_SERVER_ERROR);
    SBL::Socket rtp_socket(SBL::Socket::UDP);
    rtp_socket.connect(_client_ip, client_port0);
    _server_port = rtp_socket.local_address(_server_ip);
    _client_port = rtp_socket.remote_address(_client_ip);
    SBL::Socket rtcp_out(SBL::Socket::UDP);
    rtcp_out.connect(_client_ip, client_port1);
    int rtcp_port = rtcp_out.remote_address();
    _client = _source->streamer()->add_client(rtp_socket, rtcp_out, this);
    SBL_MSG(MSG::SERVER, "Created client %p (socket %d) for server %p (id %d)", _client, rtp_socket.id(), this, id());
    _session_id = SessionID::generate();

    SBL::Socket rtcp_in(SBL::Socket::UDP);
    rtcp_in.bind(_server_port + 1);
    _rtcp_parser = new RTCP::Parser(this, rtcp_in);
    _rtcp_parser->create_thread(Thread::Default, 64 * 1024);
    SBL_INFO("Server %d, %s stream (UDP) on socket %d/%d for client %s:%d/%d", id(), _source->encoder_name(), rtp_socket.id(), 
             rtcp_out.id(), _client_ip, _client_port, rtcp_port);
    return _session_id;
}

void Talker::teardown() {
    if (_rtcp_parser) {
        _rtcp_parser->kill();
        delete _rtcp_parser;
        _rtcp_parser = NULL;
    }
    if (_source && _client) {
        _master->lock();
        Streamer* streamer = _client->streamer();
        SBL_MSG(MSG::SERVER, "Deleting client for source %s in server %d", _source->name(), id());
        streamer->delete_client(_client);
        _client = NULL;
        if (streamer->client_count() == 0) {
            _source->teardown();
            if (!_source->is_live()) {
                _master->source_map()->erase(_source->name());
                delete _source;
                _source = NULL;
                delete streamer;
                SBL_MSG(MSG::SERVER, "Deleted source and streamer for server %d", id());
            }
        }
        _master->unlock();
    }
    _socket.close();
}

}
