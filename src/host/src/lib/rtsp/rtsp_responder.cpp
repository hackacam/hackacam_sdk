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
#include <time.h>
#include "rtsp_responder.h"
#include "rtsp_talker.h"
#include "live_source.h"

namespace RTSP {

const char* Responder::_version = "1.0";
const char* Responder::_eol = "\r\n";
const char* Responder::_control = "track1";

Responder::Responder(Talker* talker, char* buffer, int buffer_size) : 
        _buffer(buffer), _buffer_size(buffer_size), _talker(talker) {
   memset(_date,      0, sizeof _date); 
   Parser::get_method_names(_method_name);
}

int Responder::reply(const Parser::Data& data, const Errcode errcode) {
    _writer.clear();
    _writer.seekp(0);
    _writer.seekg(0);
    get_date();
    _writer << "RTSP/1.0 " << errcode << ' ' << Parser::errcode_desc(errcode) << _eol
            << "CSeq: " << data.cseq << _eol
            << "Date: " << _date << _eol;
    // This could be done through a table, but the syntax for pointers to
    // member functions is just too ugly. Here it is:
    //      typedef void (RTSPResponder::*ReplyFun)();
    //      RTSPResponder::ReplyFun _fun_table[] = {&RTSPResponder::reply_options, &RTSPResponder::reply_describe,...}; 
    //      ((*this).*(fun_table[_parser_method]))();
    // and it would still be order dependent!
    if (errcode == OK)
        switch (data.method) {
        case OPTIONS:       reply_options();            break;
        case DESCRIBE:      reply_describe(data);       break;
        case SETUP:         reply_setup(data);          break;
        case PLAY:          reply_play(data);           break;
        case GET_PARAMETER: reply_get_parameter(data);  break;
        case TEARDOWN:      reply_teardown(data);       break;
        default: RTSP_ASSERT(0, METHOD_NOT_ALLOWED);    break;
        }
    _writer << _eol;
    return copy_data();
}

int Responder::copy_data() {
    std::streamsize size = _writer.tellp();
    RTSP_ASSERT(_writer && size < _buffer_size - 1, SERVER_BUFFER_OVERFLOW);
    _writer.read(_buffer, size);
    _buffer[size] = '\0';
    return size;
}

void Responder::get_date() {
    time_t calendar_time = time(NULL);
    tm broken_time;
    gmtime_r(&calendar_time, &broken_time);
    size_t n = strftime(&_date[0], sizeof _date - 4, "%a, %d %b %Y %T", &broken_time); 
    RTSP_ASSERT(n, SERVER_DATE_ERROR);
    strcpy(_date + n, " GMT");
}

void Responder::reply_options() {
    _writer << "Public: " << _method_name[0];
    for (unsigned int n = 1; n < _method_name.size(); n++) {
        _writer << ", " << _method_name[n];
    }
    _writer << _eol;
}

void Responder::reply_describe(const Parser::Data& data) {
    _writer << "Content-Base: " << data.url << '/' << _eol
            << "Content-Type: " << data.accept << _eol
            << "Content-Length: ";
    std::streampos len_ptr = _writer.tellp();
    _writer << "    " << _eol << _eol;
    std::streampos msg_start = _writer.tellp();
    RTSP_ASSERT(data.stream_name, BAD_REQUEST);
    Source* source = _talker->get_source(data.stream_name);
    // send play message to the application, but only if there
    // no clients currently attached, i.e. if this is the first client
    if (source->streamer()->client_count() == 0)
        source->request_app_play();
    source->get_stream_desc();
    const char* encoder_name = source->encoder_name();
    if (strcmp(encoder_name, "MPEG4")==0){
        encoder_name = "MPEG-4";
    }
    int payload_type = source->payload_type();
    _writer << "v=0" << _eol
            << "o=- " << rand() << " 1 IN IP4 " << _talker->server_ip() << _eol
            << "s=" << encoder_name << " Video, streamed by the Stretch Media Server" << _eol    
            << "i=" << data.stream_name << _eol
            << "t=0 0" << _eol
            << "a=tool:Stretch RTSP Server version " << _version << _eol
            << "a=type:broadcast" << _eol
            << "a=control:*" << _eol
            << "a=range:npt=0-" << _eol
            << "a=x-qt-text-nam:" << encoder_name << " Video, streamed by the Stretch Media Server" << _eol
            << "a=x-qt-text-inf:" << data.stream_name << _eol
            << "m=video 0 RTP/AVP " << payload_type << _eol
            << "c=IN IP4 0.0.0.0" << _eol
            << "b=AS:" << source->get_bitrate() << _eol;
    if (source->encoder_type() == H264) {
        _writer << "a=rtpmap:" << payload_type << " H264/90000" << _eol;
        source->write_param_set(_writer) << _eol;
    }else if (source->encoder_type() == MPEG4){
        _writer << "a=rtpmap:" << payload_type << " MP4V-ES/90000" << _eol;
        //        source->write_param_set(_writer) << _eol;
    }
    _writer << "a=control:" << _control;    // Missing _eol, because reply() will add it
    std::streampos pos = _writer.tellp();
    std::streampos content_length = pos - msg_start + 2; // adding the length of _eol
    _writer.seekp(len_ptr);
    _writer << content_length;
    _writer.seekp(pos);   
}

void Responder::reply_setup(const Parser::Data& data) {
    // trim _control suffix
    RTSP_ASSERT(data.stream_name, BAD_REQUEST);
    char* slash = data.stream_name + strlen(data.stream_name) - strlen(_control) - 1;
    RTSP_ASSERT(slash > data.stream_name && *slash == '/' && !strcmp(slash + 1, _control), NOT_FOUND);
    *slash = '\0';
    if (data.transport == TCP) {
        SessionID session_id = _talker->setup_tcp(data.stream_name);
        _writer << "Transport: RTP/AVP/TCP;unicast"
                << ";destination=" << _talker->client_ip()
                << ";source=" << _talker->server_ip()
                << ";interleaved=0-1" << _eol
                << "Session: " << session_id << _eol;
    } else {
        SessionID session_id = _talker->setup_udp(data.stream_name, data.client_port0, data.client_port1);
        _writer << "Transport: RTP/AVP;unicast"
                << ";destination=" << _talker->client_ip()
                << ";source=" << _talker->server_ip() 
                << ";client_port=" << data.client_port0 << '-' << data.client_port1 
                << ";server_port=" << _talker->server_port() << '-' << (_talker->server_port() + 1) << _eol
                << "Session: " << session_id << _eol;
    }
}

void Responder::reply_play(const Parser::Data& data) {
    SessionID session_id(data.session_id);
    RTSP_ASSERT(session_id == _talker->session_id(), SESSION_NOT_FOUND);
    Client* client = _talker->client();
    RTSP_ASSERT(client, INTERNAL_SERVER_ERROR);
    _writer << "Range: npt=0.000-" << _eol
            << "Session: " << data.session_id << _eol
            << "RTP-Info: url=" << data.url;
    _writer << '/' << _control
            << ";seq=" << client->seq_number()
            << ";rtptime=" << client->timestamp() << _eol;
}

void Responder::reply_get_parameter(const Parser::Data& data) {
    _writer << "Session: " << data.session_id << _eol;
}

void Responder::reply_teardown(const Parser::Data& data) {
    SessionID session_id(data.session_id);
    RTSP_ASSERT(session_id == _talker->session_id(), SESSION_NOT_FOUND);
    _talker->teardown();
}

}
