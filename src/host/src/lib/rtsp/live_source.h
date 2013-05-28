#pragma once
#ifndef _RTSP_LIVE_SOURCE_H
#define _RTSP_LIVE_SOURCE_H
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
#include "rtsp_source.h"

namespace RTSP {

//! Implements a LiveSource. 
/*! The send_frame() is the main method called from a callback. */
class LiveSource : public Source {
public:
    //! LiveSources don't have name, they have stream_id numbers.
    LiveSource(int stream_id, Streamer* streamer, const char* stream_name = NULL) : Source(stream_id, streamer, stream_name), _stream_id(stream_id) {
        SBL_MSG(MSG::SOURCE, "Created LiveSource %d (%p)", stream_id, this);
    }

    //! Save timestamp, sps/pps if present and call send frame to streamer if we are playing
    void send_frame(uint8_t* frame, int size, uint32_t timestamp, EncoderType encoder) {
        _timestamp = timestamp;
        _stream_desc.encoder_type = encoder;
        switch (encoder_type()) {
            case H264: 
                SBL_THROW_IF(size < NAL_HEADER_SIZE || !is_nal_header(frame), "bad NAL header, size=%d, header=%02x%02x%02x%02x",
                             size, frame[0], frame[1], frame[2], frame[3]);
                frame += NAL_HEADER_SIZE;
                size  -= NAL_HEADER_SIZE;
                SBL_MSG(MSG::SOURCE, "H264 source %d, frame %c, size %d, ts %d", _stream_id, frame_type(frame[0]), size, timestamp);
                save_if_sps_pps(frame, size);
                break;
            case MJPEG:
                SBL_MSG(MSG::SOURCE, "MJPEG source %d, frame size %d, ts %d", _stream_id, size, timestamp);
                break;
            case MPEG4:
                /* frame += NAL_HEADER_SIZE - 1; */
                /* size  -= NAL_HEADER_SIZE - 1; */
                SBL_MSG(MSG::SOURCE, "MPEG4 source %d, frame size %d, ts %d", _stream_id, size, timestamp);
                break;
            default: 
                SBL_THROW_IF(_playing, "Unknown encoder type");
        }
        if (_playing) {
            streamer()->send_frame(frame, size, timestamp);
            SBL_MSG(MSG::SOURCE, "frame sent");
        }
    }
    
    //! Play method is simply unblocking a flag for LiveSource
    void play()              { 
        SBL_INFO("Started to play stream %d", _stream_id);
        _playing = true; 
    }
    
    //! Teardown is sets a blocking flag for LiveSource and notifies application about teardown
    void teardown() { 
        SBL_INFO("Tearing down stream %d", _stream_id);
        _playing = false; 
        application()->teardown(_stream_id);
    }

    //! Server must know if we are Live or FileSource
    bool is_live()    const  { return true; }
    
    //! Return this source stream_id
    int stream_id() const { return _stream_id; }

    //! Fetch stream description and ask application play the stream
    void get_stream_desc() {
        application()->describe(_stream_id, _stream_desc);
    }

    //! Request play from the app
    void request_app_play() {
        application()->play(_stream_id);
    }

private:
    int     _stream_id;
};

}
#endif
