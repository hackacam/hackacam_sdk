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
#include <cstring>
#include <sys/select.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_exception.h>
#include "file_source.h"
#include "rtsp.h"

namespace RTSP {

FileSource* FileSource::create(const char* filename, Streamer* streamer, int fps, int ts_clock, int buffer_size) {
    SBL_ASSERT(streamer);
    FileSource* fs = new FileSource(filename, streamer, fps, ts_clock, buffer_size);
    Errcode errcode = fs->_errcode;
    if (errcode != OK) {
        delete fs->_buffer;
        delete streamer;
        delete fs;
        throw errcode;
    }
    return fs;
}

FileSource::FileSource(const char* filename, Streamer* streamer, int fps, int ts_clock, int buffer_size) :
    Source(filename, streamer), _ts_delta(0), _tick(0), _period(ONE_SECOND / fps), 
    _buffer(NULL), _buffer_size(buffer_size), _frame(0), _frame_size(0), _have_bytes(0), _errcode(OK) {
        
    _ts_delta = ts_clock / fps;
    SBL_ASSERT(buffer_size > BUFFER_HDR);
    _buffer = new uint8_t[_buffer_size];

    _file.open(filename);
    if (!_file) {
        SBL_ERROR("Unable to open file %s for streaming", filename);
        _errcode = NOT_FOUND;
        return;
    }
    SBL_MSG(MSG::SOURCE, "FileSource %s, fps=%d, ts_clock=%d, buffer_size=%d", 
                name(), fps,    ts_clock,   _ts_delta, _buffer_size);
    _buffer += BUFFER_HDR;
    _buffer_size -= BUFFER_HDR;
    _file.read((char*) _buffer, _buffer_size);
    _have_bytes = _file.gcount();
    if (!is_nal_header(_buffer)) {
        SBL_ERROR("File %s is not H264 elementary stream", filename);
        _errcode = BAD_REQUEST;
        return;
    }
    _frame = _buffer + 4;
    _have_bytes -= 4;
    _frame_size = frame_size(_frame, _have_bytes);
    if (frame_type(_frame[0]) != 's') {
        SBL_ERROR("File %s does not have SPS frame", filename);
        _errcode = BAD_REQUEST;
        return;
    }
    save_sps(_frame, _frame_size);
    uint8_t *pps = _frame + _frame_size + 4;
    if (!is_nal_header(pps - 4)) {
        SBL_ERROR("File %s is missing PPS", filename);
        _errcode = BAD_REQUEST;
        return;
    }
    if (frame_type(pps[0]) != 'p') {
        SBL_ERROR("File %s does not have PPS frame", filename);
        _errcode = BAD_REQUEST;
    }
    save_pps(pps, frame_size(pps, _have_bytes - _frame_size - 4));
}

int FileSource::frame_size(uint8_t* frame, int size) {
    for (int count = 0; count < size; ++count) {
        if (is_nal_header(frame + count))
            return count;
    }
    return 0;
}

void FileSource::start_thread() {
    // _timestamp and _tick initialization. This cannot be done in constructor, 
    // because thread may be started way after constructor was called.
    _playing = true;
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    _tick = time.tv_nsec;
    play_file();
    _file.close();
    delete[] (_buffer - BUFFER_HDR);
    SBL_MSG(MSG::SOURCE, "FileSource %s closed, terminating thread", name());
}

void FileSource::play_file() {
    do {
        bool sps_pps = save_if_sps_pps(_frame, _frame_size);
        SBL_MSG(MSG::SOURCE, "source %s, frame %c, size %d, ts %d", name(), frame_type(_frame[0]), _frame_size, _timestamp); 
        streamer()->send_frame(_frame, _frame_size, _timestamp);
        next_frame();
        if (!sps_pps)
            wait();
    } while (_playing);
}

void FileSource::next_frame() {
    _frame += _frame_size + 4;
    _have_bytes -= _frame_size + 4;
    _frame_size = frame_size(_frame, _have_bytes);
    if (_frame_size)
        return;
    if (_file.eof() && _have_bytes > 0) {
        _frame_size = _have_bytes; // pretend there is NAL after end of file
    } else if (_file.eof() && _have_bytes <= 0) {
        SBL_MSG(MSG::SOURCE, "Stream %s, rewinding input file", name());
        _file.clear();
        _file.seekg(0);
        _file.read((char*) _buffer, _buffer_size);
        _frame = _buffer + 4;
        _have_bytes = _file.gcount() - 4;
        SBL_ASSERT(_have_bytes > 0);
        _frame_size = frame_size(_frame, _have_bytes);
    } else {
        if (_have_bytes < 0)
            _have_bytes = 0;
        memmove(_buffer, _frame, _have_bytes);
        _frame = _buffer;

        _file.read((char*) _frame + _have_bytes, _buffer_size - _have_bytes);
        _have_bytes += _file.gcount();
        _frame_size = frame_size(_frame, _have_bytes);
        if (_frame_size == 0)
            _frame_size = _have_bytes;
    }
}

// Wait _period before sending next frame
void FileSource::wait() {
    _tick += _period;
    if (_tick >= ONE_SECOND)
        _tick -= ONE_SECOND;
    _timestamp += _ts_delta;
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    time.tv_sec  = 0;
    time.tv_nsec = _tick - time.tv_nsec;
    if (time.tv_nsec < 0)
        time.tv_nsec += ONE_SECOND;
    // account for processing time, so that frames
    // are sent precisily _period away, without cumulative error
    SBL_PERROR(::pselect(0, NULL, NULL, NULL, &time, NULL) < 0);
}

void FileSource::teardown() {
    _playing = false;
    join_thread();
    SBL_MSG(MSG::SOURCE, "File source %s teardown", name());
}

}
