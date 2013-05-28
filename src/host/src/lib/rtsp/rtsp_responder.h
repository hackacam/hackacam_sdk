#pragma once
#ifndef _RTSP_RESPONDER_H
#define _RTSP_RESPONDER_H
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
#include <sstream>
#include <sbl/sbl_socket.h>
#include "rtsp_session_id.h"
#include "rtsp_parser.h"
#include "rtp_streamer.h"

namespace RTSP {

class Talker;

//! This class issues appropriate calls (thru Server) to handle a request and builds a reply to the client.
class Responder {
public:
    //! Constructor, ties up to Server and output stream
    //  @param talker       pointer to talker. All internal requests are handled thru talker
    //  @param buffer       Responder creates a reply in this buffer
    //  @param buffer_size  buffer must be large enough to create a reply
    Responder(Talker* talker, char* buffer, int buffer_size);

    //! Create a reply for the given request, which was parsed by Parser.
    int reply(const Parser::Data& parser_data, const Errcode errcode = OK);
private:
    char*                    _buffer;
    int                      _buffer_size;
    Talker*                  _talker;
    std::stringstream        _writer;
    char                     _date[32];
    std::vector<const char*> _method_name;

    static const char*  _eol;
    static const char*  _control;
    static const char*  _version;

    void get_date();
    void reply_options();
    void reply_describe(const Parser::Data&);
    void reply_setup(const Parser::Data&);
    void reply_play(const Parser::Data&);
    void reply_get_parameter(const Parser::Data&);
    void reply_teardown(const Parser::Data&);
    int  copy_data();
};
}
#endif
