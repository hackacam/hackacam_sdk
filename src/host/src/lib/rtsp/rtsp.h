#pragma once
#ifndef _RTSP_RTSP_H
#define _RTSP_RTSP_H
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

#include "rtsp_server.h"

//! Supported encoder types
namespace RTSP {
enum EncoderType {UNKNOWN_ENCODER, H264, MJPEG, MPEG4};

//! Applications using RTSP::Server must derive from this class and provide these services
class Application {
public:
    //! Constructor
    Application() : _rtsp_server(NULL) {}
    //! Stream descriptor is filled by application when RTSP DESCRIBE is received for inactive stream
    struct StreamDesc {
        EncoderType         encoder_type; //!< H264 or MJPEG or MPEG4
        int                 bitrate;      //!< needed for reply to RTSP DESCRIBE, alhough VLC doesn't seem to care
        int                 quality;      //!< needed only for MPJEG
        int                 width;        //!< needed only for MPJPEG
        int                 height;       //!< needed only for MPJPEG
        StreamDesc() : encoder_type(UNKNOWN_ENCODER), bitrate(0), quality(0), width(0), height(0) {}
    };
    //! Called from SDK callback with channel and stream number
    //! @param  channel_num     corresponds to a distinct video source
    //! @param  stream_num      denotes primary or secondary stream from the same source
    //! @return unique stream_id or -1 if parameters are invalid
    virtual int get_stream_id(unsigned int channel_num, unsigned int stream_num) = 0;
    //! Called from RTSP::Responder with stream name
    //! @param  stream_name     from url: rtsp://192.168.6.54:8554/stream_name
    //! @return unique stream_id or -1 if stream_name is not recognized
    virtual int get_stream_id(const char* stream_name) = 0;
    //! Start playing a given stream
    virtual void play(int stream_id) = 0;
    //! Stop playing a given stream
    virtual void teardown(int stream_id) = 0;
    //! Describe a stream
    //! @param  stream_id   stream_id of a valid stream
    //! @param  stream_desc output stream descriptor
    //! @return 0 for success, -1 for errors (if stream_id is incorrect for example)
    virtual int describe(int stream_id, StreamDesc& stream_desc) = 0;
    //! Return PE on which we run.
    //! This will be always 0 on the camera.
    virtual int pe_id() const = 0;
    //! Empty virtual destructor
    virtual ~Application() {}
    //! return global RTSP server object
    Server* rtsp_server() const { return _rtsp_server; }
    //! register global RTSP server object
    void register_rtsp_server(RTSP::Server* rtsp_server) { _rtsp_server = rtsp_server; }
private:
    Server*   _rtsp_server;
};

//! Return a Application object (which normally is a global)
extern Application* application();

//! Message bitmask
struct MSG {
    static int SERVER;
    static int SOURCE_MAP;
    static int RTCP;
    static int SDK;
    static int SOURCE;
    static int STREAMER;
};
}

//! called from a callback to send a frame
extern void rtsp_send_frame(unsigned int chan_num, unsigned int stream_id, 
                     uint8_t* frame, int size, uint32_t timestamp, RTSP::EncoderType encoder);

#endif
