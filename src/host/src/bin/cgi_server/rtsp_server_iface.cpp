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
#include <sbl/sbl_logger.h>
#include "sdk_manager.h"
#include "rtsp_server_iface.h"


/* *****************************************************************
 *   Stretch RTSP server needs the following global variable to access
 *   the streamer. This is needed to allow the application to validate
 *   the URI as well as notifying the application when a stream needs to
 *   start or stop playing.
 */
CGI::RTSPServerIFace application;
RTSP::Application* RTSP::application() { return &::application; }

namespace CGI {
/* ==================================================================
 *                          Virtual  Methods
 * ==================================================================*/

/* Describe a stream - The method is called by the RTSP server every time
 * a RTSP DESCRIBE request is received. We should validate the stream ID, and
 * return the requested information in the "stream_desc" parameter.
 *
 * Parameters:
 *
 *   stream_id  - stream_id of a valid stream
 *
 *   stream_desc - output stream descriptor
 *
 * Return:
 *
 *   0 for success, -1 for errors (if stream_id is incorrect for example)
 */
int RTSPServerIFace::describe(int stream_id, StreamDesc& stream_desc)
{
    int error = -1; // By default we assume error
    SBL_INFO("RTSP::DESCRIBE request. stream_id = %d", stream_id);
    if (!_is_valid_stream_id(stream_id))
    {
        return error;
    }

    sdvr_venc_e   encoder_type = str_encoder_to_sdk_id(cgi_server()->param_state().stream[stream_id].encoder);

    error = 0; // Success return;
    switch (encoder_type)
    {
        case SDVR_VIDEO_ENC_H264:
            stream_desc.encoder_type = RTSP::H264;
            stream_desc.width   = cgi_server()->param_state().stream[stream_id].width;
            stream_desc.height  = cgi_server()->param_state().stream[stream_id].height;
            stream_desc.bitrate = cgi_server()->param_state().stream[stream_id].bitrate;
            break;
        case SDVR_VIDEO_ENC_JPEG:
            stream_desc.encoder_type = RTSP::MJPEG;
            stream_desc.quality = cgi_server()->param_state().stream[stream_id].quality;
            stream_desc.width   = cgi_server()->param_state().stream[stream_id].width;
            stream_desc.height  = cgi_server()->param_state().stream[stream_id].height;
            stream_desc.bitrate = 4500; // kbps, estimate ??
            break;
        case SDVR_VIDEO_ENC_MPEG4:
            stream_desc.encoder_type = RTSP::MPEG4;
            stream_desc.width   = cgi_server()->param_state().stream[stream_id].width;
            stream_desc.height  = cgi_server()->param_state().stream[stream_id].height;
            stream_desc.bitrate = cgi_server()->param_state().stream[stream_id].bitrate;
            break;
        default:
            error = -1;
            stream_desc.encoder_type = RTSP::UNKNOWN_ENCODER;
            break;
    } // switch (pStreamingChan->streamingChan.video.nVideoCodecType)

    return error;
}

/*
 * Returns the video encoder bit-rate for the given encoder stream ID.
 *
 * Returns '-1' if the given stream ID does not exist
 *
 */
int RTSPServerIFace::get_bitrate(int stream_id)
{
    SBL_INFO("RTSP::get_bitrate() stream_id = %d", stream_id);
    if (!_is_valid_stream_id(stream_id))
        return -1;


    return cgi_server()->param_state().stream[stream_id].bitrate;;
}
/*
 *  Returns stream ID from channel number and stream number
 *
 *  Parameters:
 *      channel_num     corresponds to a distinct video source
 *
 *      stream_num      denotes primary or secondary stream from the same source
 *
 *  Return:
 *       -1 if the invalid stream ID or the requested stream is disabled.
 *       Otherwise a unique stream ID.
 *
 *  Remark:
 *       channel_num parameter is ignored for IP-Camera since there is only
 *       one camera. The stream ID is the camera's stream number.
 */

int RTSPServerIFace::get_stream_id(unsigned int  /* channel_num */ , unsigned int stream_id)
{
//    ParamState param_state = cgi_server()->param_state();
    int id = -1;

    // Validate that the given stream number exist.
    if (_is_valid_stream_id(stream_id))
    {
        id = stream_id;
    }

    return id;


}

/*
 *  This method validates the given request URI. If it correspond to a valid
 *  video stream, a unique stream ID is returned. Otherwise, it returns -1.
 *
 *  Parameters:
 *         request_uri - Correspond to a CGI Streaming channel URI

 *
 *  Return:
 *       -1 if the invalid stream or the requested stream is disabled.
 *       Otherwise a unique stream ID.
 */
int
RTSPServerIFace::get_stream_id(char const* request_uri)
{
    int stream_id;

    SBL_INFO("RTSP::get_stream_id() request_uri = %s", request_uri);

    stream_id = _parse_uri_components(request_uri);
    if (!_is_valid_stream_id(stream_id))
        stream_id = -1;

    SBL_INFO("RTSP::get_stream_id() returning stream_id = %d", stream_id);
    return stream_id;
}

void RTSPServerIFace::play(int stream_id)
{
    SBL_INFO("RTSP::play() stream_id = %d", stream_id);
}

void RTSPServerIFace::teardown(int stream_id)
{
    SBL_INFO("RTSP::teardown() stream_id = %d", stream_id);
}

////////////////////////////////////////////////////////////////////////////
//                             PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////

bool RTSPServerIFace::_is_valid_stream_id(int stream_id)
{
    bool valid =
    (stream_id >= 0 && stream_id < Stream::COUNT &&
                       cgi_server()->param_state().stream[stream_id].enable);
    if (!valid)
    {
        SBL_INFO("ERROR: Invalid stream_id = %d enable = %d", stream_id,
            (int)cgi_server()->param_state().stream[stream_id].enable);
    }

    return valid;
}

/****************************************************************
 *   This function parses the the components of the given
 *   request URI from the given request URI.
 *   Validates that it is in the form of:
 *     /video/<stream_id>
 *
 *   Return:
 *      -1 - If invalid request URI
 *      otherwise - The stream ID
 *
 ***************************************************************/
int RTSPServerIFace::_parse_uri_components(char const* uri)
{
    const char* slash = strrchr(uri, '/');
    if (!slash)
        return -1;
    char* end_ptr;
    int stream_id = strtol(slash + 1, &end_ptr, 10);
    if (*end_ptr || end_ptr == slash + 1)
        return -1;
    const char* ptr = slash - 1;
    while (ptr >= uri && *ptr != '/')
        --ptr;
    if (*ptr == '/')
        ++ptr;
    if (strncmp(ptr, "video", sizeof("video") - 1))
        return -1;
    return stream_id;
}

} // namespace CGI


