#pragma once
#ifndef _RTSP_FILE_SOURCE_H
#define _RTSP_FILE_SOURCE_H
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

#include <fstream>
#include <sbl/sbl_thread.h>
#include "rtsp_impl.h"
#include "rtp_streamer.h"
#include "rtsp_source.h"

namespace RTSP {

//! Reads data from a file and generates frames for streaming.
/*! It derives both from a Thread and a Source, because a
    separate thread must run for each file source. */
class FileSource : public SBL::Thread, public Source {
public:
    //! FileSource needs fps, ts_clock and buffer_size specs. Constructors open the file
    //  reads and caches sps/pps and returns.
    //  @param buffer_size is size of buffer allocated for reading from file
    //  @param fps is frame per second, common values are 25 and 30
    //  @param ts_clock is timestamp clock frequency, typically 90000 (90 KHz).
    static FileSource* create(const char* filename, Streamer* streamer,            // 1 MB
                              int fps = 30, int ts_clock = 90000, int buffer_size = 1000000);
    //! Playing means starting a new thread to send out file contents
    void play()     { 
        if (!_playing) {
            SBL_MSG(MSG::SOURCE, "Starting to play file %s", name());
            create_thread(); 
        }
    }
    //! Thread entry function
    void start_thread(); 

    //! Deallocate buffers and close files (cannot be done in destructor since this runs in threads)
    void teardown();

    //! FileSource doesn't use send_frame, since it call streamer->send_frame from its thread loop
    void send_frame(uint8_t* frame, int size, uint32_t timestamp, EncoderType encoder) {}

    //! Server must know if this is live or file stream.
    bool is_live() const { return false; }

    //! File source only works for H264
    void get_stream_desc() {
        _stream_desc.encoder_type = H264;
        _stream_desc.bitrate = 8000;
    }
private:
    enum {BUFFER_HDR = 256, ONE_SECOND = 1000000000, PAYLOAD_TYPE = 96 };
    std::ifstream   _file;          // file we are reading from
    uint32_t        _ts_delta;      // timestamp increment per frame, in nanoseconds
    uint32_t        _tick;          // current frame tick
    uint32_t        _period;        // FPS period
    uint8_t*        _buffer;        // buffer were file content is read
    uint32_t        _buffer_size;
    uint8_t*        _frame;         // pointer to frame to send
    uint32_t        _frame_size;
    int             _have_bytes;    // how many bytes left in buffer
    Errcode         _errcode;

    void        next_frame();       // set up next frame to send
    static int  frame_size(uint8_t*, int);       // compute next frame size
    void        wait();             // wait _period and update _timestamp

    FileSource(const char* filename, Streamer* streamer, int fps, int ts_clock, int buffer_size);
    int payload_type() const { return PAYLOAD_TYPE; }
    void play_file();
};

}
#endif
