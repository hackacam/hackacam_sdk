#pragma once
#ifndef _CGI_RTSP_SERVER_IFACE_H_
#define _CGI_RTSP_SERVER_IFACE_H_
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

#include <rtsp/rtsp.h>
#include "cgi_server.h"

namespace CGI {

class RTSPServerIFace : public RTSP::Application, public SDKTranslator
{

public:
    RTSPServerIFace() : _cgi_server(NULL) {}

    virtual int describe(int stream_id, StreamDesc& stream_desc);
    virtual int get_bitrate(int stream_id);
    virtual int get_stream_id(unsigned int  /* channel_num */ , unsigned int stream_id);
    virtual int get_stream_id(const char* name);
    virtual void play(int stream_id);
    virtual void teardown(int stream_id);
    virtual int pe_id() const { return 0; }
    void register_cgi_server(CGI::Server* cgi_server) { _cgi_server = cgi_server; }
    Server* cgi_server() const { return _cgi_server; }

private:

    // Validate the stream ID and makes sure that video streaming is enabled.
    bool _is_valid_stream_id(int stream_id);
    int  _parse_uri_components(char const* request_uri);

private:
    CGI::Server*   _cgi_server;
};

} // namespace CGI

extern CGI::RTSPServerIFace application;
#endif /* RTSPSERVERIFACE_H_ */



