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
#include <unistd.h>
#include <sbl/sbl_exception.h>
#include "rtsp_source.h"
#include "rtsp_impl.h"
#include "rtp_streamer.h"

namespace RTSP {

//                         0123456789ABCDEF0123456789ABCDEF
const char* Source::_frame_type = "UPUUUIUspUUUUUUUUUUUUUUUUUUUUUUU";

Source::Source(const char* name, Streamer* streamer) :
            _sps(NULL), _sps_size(0),
            _pps(NULL), _pps_size(0),
            _timestamp(0), _playing(false),
            // encoder_type needs to be set up to unknown when PSIA server is updated
            _name(name), _streamer(streamer)  { 
    streamer->set_source(this);
}

Source::Source(const int id, Streamer* streamer, const char* stream_name)  :
            _sps(NULL), _sps_size(0),
            _pps(NULL), _pps_size(0),
            _timestamp(0), _playing(false),
            // encoder_type needs to be set up to unknown when PSIA server is updated
             _streamer(streamer) {
    streamer->set_source(this);
    char buffer[16];
    if (stream_name == NULL) {
        snprintf(buffer, sizeof buffer, "%d", id);
        _name = buffer;
    } else 
        _name = stream_name;
}

const char* Source::encoder_name() const {
    switch (encoder_type()) {
        case H264:  return "H264";
        case MJPEG: return "MJPEG";
        case MPEG4: return "MPEG4";
        default:    SBL_ERROR("Unsupported encoder %d", encoder_type());
        RTSP_ASSERT(0, ERROR_UNSUPPORTED_ENCODER);
    }
}

int Source::payload_type() const {
    switch (encoder_type()) {
        case H264:  return 96;
        case MJPEG: return 26;
        case MPEG4:  return 96;
        default:    SBL_ERROR("Unsupported encoder");
                    RTSP_ASSERT(0, ERROR_UNSUPPORTED_ENCODER);
    }
}

Source::~Source() {
    _sps_lock.lock();
    if (_sps) {
        delete[] _sps;
        _sps = NULL;
        _sps_size = 0;
    }
    if (_pps) {
        delete[] _pps;
        _pps = NULL;
        _pps_size = 0;
    }
    _sps_lock.unlock();
    SBL_MSG(MSG::SOURCE, "Deleted source %s", _name.c_str());
}

void Source::save_sps(uint8_t* frame, int frame_size) {
    SBL_MSG(MSG::SOURCE, "Saving SPS for source %s", _name.c_str());
    save_params(frame, frame_size, _sps, _sps_size);
}
//! save PPS frame
void Source::save_pps(uint8_t* frame, int frame_size) {
    SBL_MSG(MSG::SOURCE, "Saving PPS for source %s", _name.c_str());
    save_params(frame, frame_size, _pps, _pps_size);
}

bool Source::save_if_sps_pps(uint8_t* frame, int size) {
    switch (frame_type(frame[0])) {
        case 's':   save_sps(frame, size);  return true;
        case 'p':   save_pps(frame, size);  return true;
    }
    return false;
}

int Source::seq_number() {
    return _streamer->seq_number();
}

void Source::save_params(uint8_t* frame, int frame_size, uint8_t*& buffer, int& buffer_size) {
    _sps_lock.lock();
    if (frame_size > buffer_size) {
        if (buffer)
            delete[] buffer;
        buffer = new uint8_t[frame_size];
    }
    memcpy(buffer, frame, frame_size);
    buffer_size = frame_size;
    _sps_lock.unlock();
}

bool Source::is_nal_header(uint8_t* frame) {
    return frame[0] == 0 && frame[1] == 0 && frame[2] == 0 && frame[3] == 1;
}

int Source::profile_level(uint8_t* sps) {
    SBL_THROW_IF(sps == 0, "missing sps");
    return ((sps[1] & 0x0ff) << 16)
         | ((sps[2] & 0x0ff) << 8)
         |  (sps[3] & 0x0ff); // profile_idc|constraint_setN_flag|level_idc
}

std::ostream& Source::write_param_set(std::ostream& str) {
    // when used with PSIA server, we may be asked for param set before sps & pps were cached,
    // so we wait for about 2 GOPs before giving up
    for (int n = 0; n < 60 && ! ((volatile uint8_t*) _sps && (volatile uint8_t*) _pps); ++n) {
        SBL_MSG(MSG::SOURCE, "Waiting for sps/pps");
        usleep(30000);
    }
    RTSP_ASSERT(_sps && _pps, ERROR_MISSING_SPS);
    // Can't see a scenario where client would be deleted between _sps is checked
    // (above) and used (below). The profile_level() will assert in that case.
    _sps_lock.lock();
    std::ios_base::fmtflags flags = str.flags();
    str << "a=fmtp:" << payload_type()
        << " packetization-mode=1;profile-level-id=" << std::hex << profile_level(_sps)
        << ";sprop-parameter-sets=";
    str.flags(flags);
    encode64(str, _sps, _sps_size) << ',' ;
    encode64(str, _pps, _pps_size);
    _sps_lock.unlock();
    return str;
}


std::ostream& Source::encode64(std::ostream& str, uint8_t* data, int size) {
    static const char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    SBL_THROW_IF(!data, "attempt to encode void data"); 
    SBL_THROW_IF(!size, "attempt to encode empty data");

    int triplets = 3 * ((size - 1) / 3 + 1);
    // Map each full group of 3 input bytes into 4 output base-64 characters:
    for (int i = 0; i < triplets; i += 3) {
        str << encode_table[(data[i] >> 2) & 0x3F]
            << (i + 1 < size ? encode_table[(((data[i] & 0x3) << 4) | (data[i + 1] >> 4)) & 0x3F] :
                               encode_table[((data[i] & 0x3) << 4) & 0x3F])
            << (i + 2 < size ? encode_table[((data[i + 1] << 2) | (data[i + 2] >>6)) & 0x3F] :
                i + 1 < size ? encode_table[(data[i + 1] << 2) &0x3F] :
                               '=')
            << (i + 2 < size ? encode_table[data[i + 2] & 0x3F] :
                               '=');
    }
    return str;
}

int Source::get_bitrate() const { 
    SBL_THROW_IF(_stream_desc.encoder_type == UNKNOWN_ENCODER, "unknown encoder type"); 
    return _stream_desc.bitrate; 
}

int Source::get_quality() const { 
    SBL_THROW_IF(_stream_desc.encoder_type == UNKNOWN_ENCODER, "unknown encoder type");  
    return _stream_desc.quality; 
}

int Source::get_width()   const { 
    SBL_THROW_IF(_stream_desc.encoder_type == UNKNOWN_ENCODER, "unknown encoder type");  
    return _stream_desc.width; 
}

int Source::get_height()  const { 
    SBL_THROW_IF(_stream_desc.encoder_type == UNKNOWN_ENCODER, "unknown encoder type");  
    return _stream_desc.height; 
}
Streamer* Source::create_streamer(int packet_size, int ssrc, int seq_num) {
    return new Streamer(packet_size, ssrc, seq_num);
}
}
