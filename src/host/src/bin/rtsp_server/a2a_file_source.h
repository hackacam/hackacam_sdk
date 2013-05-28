#pragma once
#ifndef _RTSP_A2A_FILE_SOURCE_H
#define _RTSP_A2A_FILE_SOURCE_H

#include <fstream>
#include <sbl/sbl_thread.h>
#include <rtsp/rtsp.h>
#include <rtsp/rtsp_source.h>
#include <liba2a.h>
#include "rtsp_arm_common.h"

/// @cond

#define MAX_STREAMING_FILE_COUNT    16

#define MAX_STREAMING_CHAN_COUNT 32
#define MAX_A2A_BUF_COUNT        50
#define VIDEO_BUF_SIZE           256*1024

#define A2A_FILE_BUFFER_FIFO_SIZE    8

#define	STAT_REPORT_SECS	8   // set to 0 for no stats

#define SDVR_DATA_HDR_SIG    (0x00524448)

//#define REQUEST_BUF_SIZE  128*1024
#define REQUEST_BUF_SIZE  256*1024
//#define REQUEST_BUF_SIZE  0x10000L

#define AV_BUFFER_HEADER_SIZE    offsetof(sdvr_av_buffer_t,payload)

namespace RTSP {

//************************************************************

class IndexFifo 
    {
public:
    IndexFifo();
    int get_read();
    int advance_read();
    int get_write();
    int advance_write();
    int num_in();
private:
    int _read;
    int _write;
    __u32 _mask; // = size - 1
    };

//************************************************************

class BinarySemaphore 
    {
public:
    BinarySemaphore();
    ~BinarySemaphore();
    void wait();
    void post();
private:
    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
    unsigned int    _waiting;
    unsigned int    _signaled;
    };

//************************************************************

typedef struct
    {
    a2a_channel_t channel;
    uint8_t *buffer;
    } a2a_buffer_data_t;

//************************************************************

//! Reads data from a file and generates frames for streaming.
/*! It derives both from a Thread and a Source, because a
    separate thread must run for each file source. */
class A2aFileSource : public SBL::Thread, public Source {
public:

    void push_buffer(a2a_channel_t channel,uint8_t *buffer);

    //! A2aFileSource needs fps, ts_clock and buffer_size specs. Constructors open the file
    //  reads and caches sps/pps and returns.
    //  @param buffer_size is size of buffer allocated for reading from file
    //  @param fps is frame per second, common values are 25 and 30
    //  @param ts_clock is timestamp clock frequency, typically 90000 (90 KHz).
    static A2aFileSource* create(a2a_peer_t a2a_peerHandle,const int id, Streamer* streamer,            // 1 MB
                              int fps = 30, int ts_clock = 90000);
    //! Playing means starting a new thread to send out file contents
    void play()     { 
        if (!_playing) {
            create_thread(); 
        }
    }
    //! Thread entry function
    void start_thread(); 

    //! Deallocate buffers and close files (cannot be done in destructor since this runs in threads)
    void teardown();

    //! A2aFileSource doesn't use send_frame, since it call streamer->send_frame from its thread loop
    void send_frame(uint8_t* frame, int size, uint32_t timestamp, RTSP::EncoderType encoder_type) {}

    //! Server must know if this is live or file stream.
    bool is_live() const { return false; }

    uint32_t get_total_bytes_received() { return _total_bytes_received; }

    //! A2A source only works for H264
    void get_stream_desc() {
        _stream_desc.encoder_type = H264;
        _stream_desc.bitrate = 8000;
    }
private:
    enum {BUFFER_HDR = 256, ONE_SECOND = 1000000000, PAYLOAD_TYPE = 96 };
    uint32_t           _ts_delta;      // timestamp increment per frame, in nanoseconds
    uint32_t           _tick;          // current frame tick
    uint32_t           _period;        // FPS period
    uint8_t*           _buffer;        // buffer were file content is read
    uint8_t*           _frame;         // pointer to frame to send
    uint32_t           _frame_size;
    uint32_t           _have_bytes;    // how many bytes left in buffer
    // a2a-specific data:
    uint8_t*           _buffer_top;
    uint32_t           _total_bytes_received;
    uint32_t           _file_index;    // index of file on PE0 we are reading from
    uint32_t           _next_segment_file_offset;
    a2a_peer_t         _a2a_peerHandle;
    BinarySemaphore _data_ready_sem;
    IndexFifo       _index_fifo;
    a2a_buffer_data_t  _a2a_buffer_data[A2A_FILE_BUFFER_FIFO_SIZE];
    a2a_channel_t      _channel;
        
    void        next_frame();       // set up next frame to send
    static int  frame_size(uint8_t*, int);       // compute next frame size
    void        wait();             // wait _period and update _timestamp

    A2aFileSource(a2a_peer_t a2a_peerHandle,const int id, Streamer* streamer, int fps, int ts_clock);
    int payload_type() const { return PAYLOAD_TYPE; }
    void play_file();
};
/// @endcond

}
#endif
