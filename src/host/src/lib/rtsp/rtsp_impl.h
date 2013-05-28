#pragma once
#ifndef _RTSP_IMPL_H
#define _RTSP_IMPL_H
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

#include <map>
#include <cstring>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_map.h>
#include "rtsp_source.h"
#include "rtp_streamer.h"

#define RTSP_ASSERT(cond, errcode)  do { if (!(cond)) throw (errcode); } while (0)

namespace RTSP {
class Server;

extern void set_rtsp_server(Server* server);

// Supported methods
enum Method {OPTIONS, DESCRIBE, SETUP, PLAY, GET_PARAMETER, TEARDOWN};
// Either RTP/C over UDP, or interleaved RTP in RTSP stream 
enum Transport {UNKNOWN = 0, UDP, TCP};
// Error codes
enum Errcode {  OK                              = 200, 
                BAD_REQUEST                     = 400, 
                NOT_FOUND                       = 404,
                METHOD_NOT_ALLOWED              = 405, 
                REQUEST_URI_TOO_LARGE           = 414,
                SESSION_NOT_FOUND               = 454,
                METHOD_NOT_VALID_IN_THIS_STATE  = 455, 
                UNSUPPORTED_TRANSPORT           = 461,
                INTERNAL_SERVER_ERROR           = 500,
                RTSP_VERSION_NOT_SUPPORTED      = 505,
                ERROR_MISSING_FIELD_ARG         = 570,  // proprietary
                ERROR_FIELD_TOO_LONG            = 571,
                ERROR_BAD_PORT_SPEC             = 572,
                ERROR_BAD_INTERLEAVED_SPEC      = 573,
                ERROR_TCP_WITH_PORTS            = 574,
                ERROR_UDP_NO_PORTS              = 575,
                ERROR_SUPPORT_UNICAST_ONLY      = 576,
                ERROR_SESSION_ID_TOO_LONG       = 578,
                SERVER_IN_ERROR_STATE           = 579,
                ERROR_SUPPORT_ONLY_SDP          = 580,
                ERROR_MISSING_SPS               = 581,
                SERVER_BUFFER_OVERFLOW          = 582,
                SERVER_DATE_ERROR               = 583,
                ERROR_UNSUPPORTED_ENCODER       = 584
                };


}
#endif
