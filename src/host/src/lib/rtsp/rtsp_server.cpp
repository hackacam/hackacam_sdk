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
#include "rtsp_server.h"
#include "rtsp_talker.h"
#include "source_map.h"
#include "live_source.h"

namespace RTSP {

Server* Server::create(const short int port, const Options& options) {
    Server* server = new Server(port, options);
    server->create_thread(Thread::Default, STACK_SIZE);
    application()->register_rtsp_server(server);
    SBL_INFO("Master RTSP Server listening on port %d", port & 0xffff);
    return server;
}

Server::Server(const short int port, const Options& options) :
        _options(options), _socket(SBL::Socket::TCP), 
         _source_map(new SourceMap) {
    _socket.bind(port).listen(); 
    memset(&_packet_tick, 0, sizeof(_packet_tick));
}

void Server::set_temporal_level(unsigned int level) {
    SBL_MSG(MSG::SERVER, "Setting temporal level to %d", level);
    for (SourceMap::Iterator it = _source_map->begin(); it != _source_map->end(); ++it) {
        Source* source = it->second;
        source->streamer()->set_temporal_level(level);
    }
}

void Server::start_thread() {
    int thread_id = 0;
    do {
        SBL::Socket client_socket(_socket.accept());
        Talker* talker = new Talker(client_socket, ++thread_id, this);
        // new thread starts in start_thread() method
        talker->create_thread(Thread::Detached, STACK_SIZE);    
    } while (1);
}

Source* Server::get_source(const int stream_id) {
    lock();
    Source* source = _source_map->find(stream_id);
    if (!source) {
        source = new LiveSource(stream_id, new Streamer(_options.packet_size));
        _source_map->save(stream_id, source);
        SBL_MSG(MSG::SERVER, "Server created live source for stream %d", stream_id);
    }
    unlock();
    _packet_tick.tv_sec = 0; // first packet of a frame, synchronize packet tick
    return source;
}

int Server::client_count(unsigned int stream_id) const {
    Source* source = _source_map->find(stream_id);
    if (source) 
        return source->streamer()->client_count();
    return -1;
}

void Server::packet_wait() {
    if (_options.packet_gap == 0)
        return;
    // first packet of frame, don't wait
    if (_packet_tick.tv_sec == 0) {
        SBL_MSG(MSG::STREAMER, "Busy-wait skipped, first packet of a frame");
        if (::clock_gettime(CLOCK_MONOTONIC, &_packet_tick) != 0)
            SBL_ERROR("clock_gettime failed");;
        return;
    }
    _packet_tick.tv_nsec += _options.packet_gap;
    const long ONE_SECOND = 1000 * 1000 * 1000;
    if (_packet_tick.tv_nsec >= ONE_SECOND) {
        _packet_tick.tv_nsec -= ONE_SECOND;
        _packet_tick.tv_sec++;
    }
    struct timespec now;
    if (::clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        SBL_ERROR("clock_gettime failed");;
    int count = 0;
    while (now.tv_sec < _packet_tick.tv_sec || now.tv_nsec < _packet_tick.tv_nsec) {
        if (::clock_gettime(CLOCK_MONOTONIC, &now) != 0)
            SBL_ERROR("clock_gettime failed");;
        count++;
    }
    SBL_MSG(MSG::STREAMER, "Busy-wait for %d ns, %d loops, tick was %d.%09d", _options.packet_gap, count, _packet_tick.tv_sec, _packet_tick.tv_nsec);
}

void Server::print_verbosity_levels(std::ostream& str) {
    str << std::setw(12) << MSG::RTCP       << "    " << "RTCP"       << std::endl;
    str << std::setw(12) << MSG::SERVER     << "    " << "SERVER"     << std::endl;
    str << std::setw(12) << MSG::SOURCE_MAP << "    " << "SOURCE_MAP" << std::endl;
    str << std::setw(12) << MSG::SDK        << "    " << "SDK"        << std::endl;
    str << std::setw(12) << MSG::SOURCE     << "    " << "SOURCE"     << std::endl;
    str << std::setw(12) << MSG::STREAMER   << "    " << "STREAMER"   << std::endl;
}
            
}
