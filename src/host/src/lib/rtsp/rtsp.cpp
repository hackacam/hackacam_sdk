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
#include "rtsp.h"
#include "rtsp_impl.h"
#include "rtsp_server.h"
#include "rtsp_source.h"


#ifdef NO_SDK
void sdk_setup(const char* rom_file, int stream_count) {}
#endif

/* --------------------------------------------------------------------------------*/
/*                  SDK callback for to send a frame                               */
void rtsp_send_frame(unsigned int chan_num, unsigned int stream_num, uint8_t* frame, int size, uint32_t timestamp, RTSP::EncoderType encoder) {
    RTSP::Server* server = RTSP::application()->rtsp_server();
    if (!server)
        return;
    // find out a unique stream_id. Normally:
    //  - chan_num distinguishes between various video inputs on a board
    //  - stream_num disinguishes between various secondary streams created from a primary one
    try {
        int stream_id = RTSP::application()->get_stream_id(chan_num, stream_num);
        if (stream_id < 0) {
            SBL_ERROR("Incorrect channel number %d or stream number %d", chan_num, stream_num);
        } else {
            RTSP::Source* source = server->get_source(stream_id);
            // get_source() always returns a source (creating one if neccessary, so don't need to check for NULL
            source->send_frame(frame, size, timestamp, encoder);
        }
    } catch (RTSP::Errcode error_code) {
        SBL_ERROR("Callback caught error code %d", error_code);
    } catch (SBL::Exception& ex) {
        SBL_ERROR("Callack caught exception %s", ex.what());
    }
}

namespace RTSP {
    int MSG::SERVER       =   4;
    int MSG::SOURCE_MAP   =   8;
    int MSG::RTCP         =  16;
    int MSG::SDK          =  32;
    int MSG::SOURCE       =  64;
    int MSG::STREAMER     = 128;
}
