#pragma once
#ifndef _RTSP_STREAM_SOURCE_H
#define _RTSP_STREAM_SOURCE_H
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
#include <ostream>
#include "rtsp.h"
#include "rtsp_session_id.h"
#include <sbl/sbl_socket.h>
#include <sbl/sbl_thread.h>
#include <sbl/sbl_logger.h>

namespace RTSP {
class Streamer;

//! An abstract base clase for LiveSource and FileSource classes.
/*! It implements sps/pps caching and writing sdp to a Responder stream. */
class Source  {
public:
    //! Constructor requires stream name and pointer to the streamer.
    Source(const char* name, Streamer* streamer);
    //! Constructor used by live source (no name, just id)
    Source(const int id, Streamer* streamer, const char* stream_name = NULL);
    //! This method is called when PLAY request is received.
    virtual void play() = 0;
    //! This is called from the callback, when a frame is ready for this source.
    //! FileSource doesn't use this method, because it calls streamer->send_frame directly.
    virtual void send_frame(uint8_t* frame, int size, uint32_t timestamp, EncoderType encoder) = 0;
    //! This is called when TEARDOWN method is received.
    virtual void teardown() = 0;
    //! Need to distinguish between Live and FileSources.
    virtual bool is_live() const = 0;
    //! Ask application to play (only live source use this)
    virtual void request_app_play() {}
    //! Check if source is playing
    virtual bool is_playing() const { return _playing; }
    //! Saving timestamps is implemented by concrete classes, this is just an accessor 
    //! which returns it.
    virtual int timestamp() { return _timestamp; }
    //! Sequence number is maintained by Streamer, this is a convienience method that
    //! forwards the request there.
    virtual int seq_number();
    //! Write out parameter set line of the .sdp file to the Responder output stream.
    //! @param str  output stream to write data to
    std::ostream& write_param_set(std::ostream& str);
    //! Return this source name
    const char* name() const { return _name.c_str(); }
    //! Return the Streamer associated with this Source.
    virtual Streamer* streamer() const {return _streamer; }
    //! Fetch stream description
    virtual void get_stream_desc() = 0;
    //! Return this source bitrate
    int get_bitrate() const;
    //! Return this source quality (MJPEG only)
    int get_quality() const;
    //! return this source width in pixels
    int get_width()   const;
    //! return this source height in pixels
    int get_height()  const;
    //! Return this source encoder type
    EncoderType encoder_type() const { return _stream_desc.encoder_type; }
    //! Return this source payload type
    int payload_type() const;
    //! Return this source encoder name
    const char* encoder_name() const;
    //! Abstract base classes must have virtual destructor by definition.
    virtual ~Source();
    //! return frame type ('s', 'p', 'I', 'P')
    static char frame_type(uint8_t frame) {
        return _frame_type[frame & 0x1F];
    }
    //! Helper to create new streamer
    static Streamer* create_streamer(int packet_size = -1, int ssrc = -1, int seq_num = -1);
protected:
    enum {NAL_HEADER_SIZE /*!< Size of NAL header */ = 4};
    uint8_t*                  _sps;       //!< cached sps frame
    int                       _sps_size;  //!< size of the cached sps frame
    uint8_t*                  _pps;       //!< cached pps frame
    int                       _pps_size;  //!< size of the cached pps frame
    uint32_t                  _timestamp; //!< current frame timestamp
    bool                      _playing;   //!< true if this source is playing
    Application::StreamDesc   _stream_desc;   //!< description of stream associated with this source

    //! save SPS frame
    void save_sps(uint8_t* frame, int frame_size);
    
    //! save PPS frame
    void save_pps(uint8_t* frame, int frame_size);

    //! true if frame points to a valid NAL header (0001)
    static bool is_nal_header(uint8_t* frame);
    //! save SPS or PPS, return true if either
    bool save_if_sps_pps(uint8_t* frame, int frame_size);
private:
    std::string _name;
    Streamer*   _streamer;
    SBL::Mutex  _sps_lock;

    static const char* _frame_type;

    std::ostream& encode64(std::ostream& str, uint8_t* data, int data_size);
    int profile_level(uint8_t*);
    void save_params(uint8_t* frame, int frame_size, uint8_t*& buffer, int& buffer_size);
};

}
#endif
