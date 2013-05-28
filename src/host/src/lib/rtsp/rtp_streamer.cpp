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
#include <cstring>
#include <unistd.h>
#include "rtsp_impl.h"
#include "rtp_streamer.h"
#include "rtsp_source.h"
#include "rtsp_talker.h"
#include "rtcp.h"

/*
      RTP header according to RFC 3550
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |V=2|P|X|  CC   |M|     PT      |       sequence number         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                           timestamp                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           synchronization source (SSRC) identifier            |
      +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   Marker bit (M): 1 bit
      Set for the very last packet of the access unit indicated by the
      RTP timestamp, in line with the normal use of the M bit in video
      formats, to allow an efficient playout buffer handling.  For
      aggregation packets (STAP and MTAP), the marker bit in the RTP
      header MUST be set to the value that the marker bit of the last
      NAL unit of the aggregation packet would have been if it were
      transported in its own RTP packet.  Decoders MAY use this bit as
      an early indication of the last packet of an access unit but MUST
      NOT rely on this property.

         Informative note: Only one M bit is associated with an
         aggregation packet carrying multiple NAL units.  Thus, if a
         gateway has re-packetized an aggregation packet into several
         packets, it cannot reliably set the M bit of those packets.

   Payload type (PT): 7 bits
      The assignment of an RTP payload type for this new packet format
      is outside the scope of this document and will not be specified
      here.  The assignment of a payload type has to be performed either
      through the profile used or in a dynamic way.

   Sequence number (SN): 16 bits
      Set and used in accordance with RFC 3550.  For the single NALU and
      non-interleaved packetization mode, the sequence number is used to
      determine decoding order for the NALU.

   Timestamp: 32 bits
      The RTP timestamp is set to the sampling timestamp of the content.
      A 90 kHz clock rate MUST be used.

      If the NAL unit has no timing properties of its own (e.g.,
      parameter set and SEI NAL units), the RTP timestamp is set to the
      RTP timestamp of the primary coded picture of the access unit in
      which the NAL unit is included, according to Section 7.4.1.2 of
      [1].


    RTP payload format for single NAL unit packet.
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |F|NRI|  Type   |                                               |
    +-+-+-+-+-+-+-+-+                                               |
    |                                                               |
    |               Bytes 2..n of a single NAL unit                 |
    |                                                               |
    |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                               :...OPTIONAL RTP padding        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


   RTP payload format for FU-As (Fragmented Unit A).  An FU-A
   consists of a fragmentation unit indicator of one octet, a
   fragmentation unit header of one octet, and a fragmentation unit
   payload.

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | FU indicator  |   FU header   |                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
    |                                                               |
    |                         FU payload                            |
    |                                                               |
    |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                               :...OPTIONAL RTP padding        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   The FU indicator octet has the following format:

       +---------------+
       |0|1|2|3|4|5|6|7|
       +-+-+-+-+-+-+-+-+
       |F|NRI|  Type   |
       +---------------+

   Values equal to 28 and 29 in the type field of the FU indicator octet
   identify an FU-A and an FU-B, respectively.  The use of the F bit is
   described in Section 5.3.  The value of the NRI field MUST be set
   according to the value of the NRI field in the fragmented NAL unit.

   The FU header has the following format:

      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+

   S:     1 bit
          When set to one, the Start bit indicates the start of a
          fragmented NAL unit.  When the following FU payload is not the
          start of a fragmented NAL unit payload, the Start bit is set
          to zero.

   E:     1 bit
          When set to one, the End bit indicates the end of a fragmented
          NAL unit, i.e., the last byte of the payload is also the last
          byte of the fragmented NAL unit.  When the following FU
          payload is not the last fragment of a fragmented NAL unit, the
          End bit is set to zero.

   R:     1 bit
          The Reserved bit MUST be equal to 0 and MUST be ignored by the
          receiver.

   Type:  5 bits
          The NAL unit payload type as defined in Table 7-1 of [1].
*/


namespace RTSP {

Client::Client(SBL::Socket sock, Streamer* str, SBL::Socket rtcp_socket, Talker* talker) : _state(STOP),
        _socket(sock) , _streamer(str), _talker(talker),
        _offs(sock.proto() == SBL::Socket::TCP ? 4 : 0),
        _rtcp_socket(rtcp_socket),
        _total_bytes(0), _total_packets(0),
        _last_rtcp_packet(0), _seq_number(0),
        _temporal_level(0)
        { SBL_MSG(MSG::STREAMER, "Created client %p with id %d for streamer %p and server %p",
                    this, id(), str, talker);
        }

int Client::id() const {
    return _talker->id();
}

void Client::increase_level() {
    SBL_MSG(MSG::STREAMER, "Client %d, request to increase temporal level (current %d)",
                                id(), _temporal_level);
    if (_temporal_level < TEMPORAL_LEVELS - 1) {
        _temporal_level++;
        SBL_INFO("Client %d, increasing temporal level to %d", id(), _temporal_level);
    }
}

void Client::reduce_level() {
    SBL_MSG(MSG::STREAMER, "Client %d, request to reduce temporal level (current %d)",
                                id(), _temporal_level);
    if (_temporal_level > 0) {
        _temporal_level--;
        SBL_INFO("Client %d, decreasing temporal level to %d", id(), _temporal_level);
    }
}

Streamer::Streamer(int packet_size, int ssrc, int seq_number) : _frame_index(0) {
    _packet_size  = packet_size  == -1 ? 8900   : packet_size;
    _ssrc         = ssrc         == -1 ? rand() : ssrc;
    _seq_number   = seq_number   == -1 ? rand() : seq_number;
    SBL_MSG(MSG::STREAMER, "Streamer %p: packet_size=%d, ssrc=%x, seq_num=%d", this, _packet_size, _ssrc, _seq_number);
}

void Streamer::write_rtp_header(uint8_t* frame, bool last_packet) {
    RTPHdr hdr;
    hdr.flags      = htons((RTP_VERSION_NUMBER << 14) | ((last_packet & 1) << 7) | _source->payload_type());
    hdr.seq_number = htons(_seq_number);
    hdr.timestamp  = htonl(_timestamp);
    hdr.ssrc       = htonl(_ssrc);
    memcpy(frame, &hdr, sizeof hdr);
}

inline void Streamer::write_fu_header(uint8_t* frame, bool last_packet) {
    if (last_packet)              // add End bit
        _fu_header |= NAL_END_BIT;
    frame[RTP_HEADER]     = _fu_indicator;
    frame[RTP_HEADER + 1] = _fu_header;
    _fu_header &= ~NAL_START_BIT; // reset Start bit
}

inline void Streamer::create_fu_header(uint8_t* frame) {
    _fu_indicator = (frame[0] & ~NAL_TYPE_MASK) | NAL_TYPE_FU_A;
    _fu_header    = (frame[0] & NAL_TYPE_MASK)  | NAL_START_BIT;
}

void Streamer::send_frame(uint8_t* frame, int frame_size, uint32_t timestamp) {
    _timestamp = timestamp;
    switch (_source->encoder_type()) {
        case H264:  h264_send_frame(frame, frame_size);
                    break;
        case MJPEG: mjpeg_send_frame(frame, frame_size);
                    break;
        case MPEG4: mpeg4_send_frame(frame, frame_size);
                    break;
        default:    SBL_THROW("bad encoder type");
    }
}


// send a frame
void Streamer::mpeg4_send_frame(uint8_t* frame, int frame_size) {
    _mp4_starter_frame = (frame[3]==0xb0);
    SBL_MSG(MSG::STREAMER, "MPEG4 Frame '%d', size %d, timestamp %d", _mp4_starter_frame, frame_size, _timestamp);
    if (frame_size <= _packet_size) {
        // small frame, doesn't need to be fragmented
        write_rtp_header(frame - RTP_HEADER, !_mp4_starter_frame);
        send_packet(frame - RTP_HEADER, frame_size + RTP_HEADER, !_mp4_starter_frame);
    } else {
        // large frame, will have to be segmented.
        // move frame pointer back to make place for RTP header bytes
        frame -= RTP_HEADER;
        do {
            bool last_packet = frame_size <= _packet_size;
            write_rtp_header(frame, last_packet);
            send_packet(frame, (frame_size < _packet_size ? frame_size : _packet_size)
                        + RTP_HEADER, last_packet);
            frame += _packet_size;
            frame_size -= _packet_size;
        } while (frame_size > 0);
    }
}

void Streamer::h264_send_frame(uint8_t* frame, int frame_size) {
    _frame_type = Source::frame_type(frame[0]);
    SBL_MSG(MSG::STREAMER, "H264 Frame '%c', size %d, timestamp %d", _frame_type, frame_size, _timestamp);
    if (_frame_type == 's' || _frame_type == 'p' || _frame_type == 'I')
        _frame_index = 0;
    if (frame_size <= _packet_size) {
        // small frame, doesn't need to be fragmented
        write_rtp_header(frame - RTP_HEADER, !(frame_type() == 'p' || frame_type() == 's'));
        send_packet(frame - RTP_HEADER, frame_size + RTP_HEADER, true);
    } else {
        // large frame, will have to be segmented.
        create_fu_header(frame);
        // move frame pointer back to make place for RTP header and FU-A (Fragmentation Unit type A) byte
        // FU-A header is in fact NAL header and is already in the frame, but FU-A indicator is not
        frame -= RTP_HEADER + FU_INDICATOR;
        // Only first fragment has FU-A header in the frame, subsequent ones don't and it has to be added
        int first_fragment = FU_HEADER;
        do {
            bool last_packet = frame_size <= _packet_size;
            write_rtp_header(frame, last_packet && !(frame_type() == 'p' || frame_type() == 's'));
            // It is unclear if End bit of FU Header should be set for SPS/PPS frames. I *assume* it does.
            // It probably doesn't matter, because SPS/PPS frames are small and don't need to be fragmented,
            // therefore they don't use FU Header at all ('small frame' 'if' clause above).
            write_fu_header(frame, last_packet);
            send_packet(frame, (frame_size < _packet_size ? frame_size : _packet_size)
                            + RTP_HEADER + FU_INDICATOR + FU_HEADER, last_packet);
            frame += _packet_size;
            frame_size -= _packet_size + first_fragment;
            first_fragment = 0;
        } while (frame_size > 0);
    }
    _frame_index++;
}
/*
    JPEG Header, RFC 2435

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Type-specific |              Fragment Offset                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Type     |       Q       |     Width     |     Height    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Type-specific: 8 bits

   Interpretation depends on the value of the type field.  If no
   interpretation is specified, this field MUST be zeroed on
   transmission and ignored on reception.

Fragment Offset: 24 bits
   The Fragment Offset is the offset in bytes of the current packet in
   the JPEG frame data. This value is encoded in network byte order
   (most significant byte first). The Fragment Offset plus the length of
   the payload data in the packet MUST NOT exceed 2^24 bytes.

Type: 8 bits
   The type field specifies the information that would otherwise be
   present in a JPEG abbreviated table-specification as well as the
   additional JFIF-style parameters not defined by JPEG.  Types 0-63 are
   reserved as fixed, well-known mappings to be defined by this document
   and future revisions of this document.  Types 64-127 are the same as
   types 0-63, except that restart markers are present in the JPEG data
   and a Restart Marker header appears immediately following the main
   JPEG header.  Types 128-255 are free to be dynamically defined by a
   session setup protocol (which is beyond the scope of this document).

Q: 8 bits
   The Q field defines the quantization tables for this frame.  Q values
   0-127 indicate the quantization tables are computed using an
   algorithm determined by the Type field (see below).  Q values 128-255
   indicate that a Quantization Table header appears after the main JPEG
   header (and the Restart Marker header, if present) in the first
   packet of the frame (fragment offset 0).  This header can be used to
   explicitly specify the quantization tables in-band.

Width: 8 bits
   This field encodes the width of the image in 8-pixel multiples (e.g.,
   a width of 40 denotes an image 320 pixels wide).  The maximum width
   is 2040 pixels.

Height: 8 bits
   This field encodes the height of the image in 8-pixel multiples
   (e.g., a height of 30 denotes an image 240 pixels tall). When
   encoding interlaced video, this is the height of a video field, since
   fields are individually JPEG encoded. The maximum height is 2040
   pixels.
*/
void Streamer::mjpeg_send_frame(uint8_t* frame, int frame_size) {
    SBL_MSG(MSG::STREAMER, "MJPEG frame size %d, timestamp %d", frame_size, _timestamp);
    frame -= RTP_HEADER + MJPEG_HEADER;
    uint8_t* frame_start = frame;
    do {
        bool last_packet = frame_size <= _packet_size;
        write_rtp_header(frame, last_packet);
        write_mjpeg_header(frame + RTP_HEADER, frame - frame_start);
        send_packet(frame, (frame_size < _packet_size ? frame_size : _packet_size)
                        + RTP_HEADER + MJPEG_HEADER, last_packet);
        frame      += _packet_size;
        frame_size -= _packet_size;
    } while (frame_size > 0);
}

void Streamer::write_mjpeg_header(uint8_t* frame, uint32_t offset) {
    MJPEGHdr hdr;
    hdr.fragment_offset = htonl(offset & 0x00FFFFFF);
    hdr.type    = MJPEG_TYPE;
    hdr.quality = _source->get_quality();
    hdr.width   = _source->get_width()  / 8;
    hdr.height  = _source->get_height() / 8;
    memcpy(frame, &hdr, sizeof hdr);
}

/*
   Section 10.12 of RFC 2326
   Stream data such as RTP packets is encapsulated by an ASCII dollar
   sign (24 hexadecimal), followed by a one-byte channel identifier,
   followed by the length of the encapsulated binary data as a binary,
   two-byte integer in network byte order. The stream data follows
   immediately afterwards, without a CRLF, but including the upper-layer
   protocol headers. Each $ block contains exactly one upper-layer
   protocol data unit, e.g., one RTP packet.
*/

// Send a single packet
void Streamer::send_packet(uint8_t* packet, int tx_size, bool last_packet) {
    // just in case this is interleaved TCP, prepend rtsp header
    // this avoid checking and setting it in the loop.
    *(packet - 4) = '$';
    *(packet - 3) = '\0';
    *(packet - 2) = tx_size >> 8;
    *(packet - 1) = tx_size;
    // locking so that someobody doesn't add or remove Clients on us while streaming
    _lock.lock();
    for (Clients::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        (*it)->send(packet, tx_size, last_packet);
    }
    _lock.unlock();
    _seq_number++;
}

void Streamer::set_temporal_level(unsigned int level) {
    SBL_MSG(MSG::STREAMER, "Streamer %p, setting temporal level to %d", this, level);
    for (Clients::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        (*it)->set_temporal_level(level);
    }
}

Client* Streamer::add_client(SBL::Socket socket, SBL::Socket rtcp_socket, Talker* talker) {
    Client* client = new Client(socket, this, rtcp_socket, talker);
    _lock.lock();
    _clients.push_back(client);
    _lock.unlock();
    SBL_MSG(MSG::STREAMER, "Added client %d to streamer %s", client->id(), _name.c_str());
    return client;
}

void Streamer::delete_client(Client* client) {
    SBL_MSG(MSG::STREAMER, "Removing client %d from streamer %s", client->id(), _name.c_str());
    _lock.lock();
    _clients.remove(client);
    _lock.unlock();
    delete client;
}

uint32_t Streamer::timestamp()  {
    SBL_THROW_IF(!_source, "Unable to get timestamp, source is void");
    return source()->timestamp();
}

void Client::send(uint8_t* packet, int size, bool last_packet) {
    if (_state == REQUEST && !(_streamer->source()->encoder_type() == H264 && _streamer->frame_type() != 's')
        && !(_streamer->source()->encoder_type() == MPEG4 && !_streamer->is_mpeg4_starter_frame())) {
        SBL_MSG(MSG::STREAMER, "Client %d, starting to play", id());
        _state = PLAY;
    }
    if (_state != PLAY)
        return;
    if (skip_frame(_streamer->frame_index())) {
        SBL_MSG(MSG::STREAMER, "Client %d, filtering out frame %d, current level is %d",
                id(), _streamer->frame_index(), _temporal_level);
        return;
    }
    // This implements packet gap
    application()->rtsp_server()->packet_wait();
    packet[Streamer::RTP_SEQ_NUM]     = _seq_number >> 8;
    packet[Streamer::RTP_SEQ_NUM + 1] = _seq_number;

    SBL_MSG(MSG::STREAMER, "Client %d, send packet size %d", id(), size);
    if (_socket.send(packet - _offs, size + _offs, false)) {
        _seq_number++;
        _total_bytes += size;
        _total_packets++;
        uint32_t ts = timestamp();
        if (last_packet && ts - _last_rtcp_packet > RTCP_INTERVAL) {
            send_sender_rtcp();
            _last_rtcp_packet = ts;
        }
    } else {
        _state = STOP;
        SBL_WARN("Switching off client %d due to socket error", id());
    }
}

void Client::set_temporal_level(unsigned int level) {
    SBL_MSG(MSG::STREAMER, "Client %d, setting temporal level to %d", id(), level);
    _temporal_level = level;
}

void Client::play() {
    _state = REQUEST;
    _streamer->source()->play();
}

uint32_t  Client::timestamp()  const {
    return _streamer->timestamp();
}

uint16_t  Client::seq_number() const {
    return _streamer->seq_number();
}

void Client::send_sender_rtcp() {
    RTCP::Sender hdr;
    struct timespec time;
    SBL_PERROR(clock_gettime(CLOCK_REALTIME, &time) != 0);
    hdr.sr.flags    = htons((Streamer::RTP_VERSION_NUMBER << 14) | RTCP::SR_PACKET_TYPE);
    hdr.sr.length   = htons(sizeof(hdr.sr) / 4 - 1);
    hdr.sr.ssrc     = htonl(_streamer->_ssrc);
    hdr.sr.ntp_h    = htonl(time.tv_sec + RTCP::NTP_OFFSET);
    hdr.sr.ntp_l    = htonl(time.tv_nsec);
    hdr.sr.rtp_ts   = htonl(timestamp());   // this is not quite correct, but is a good approximation
    hdr.sr.packets  = htonl(_total_packets);
    hdr.sr.bytes    = htonl(_total_bytes);
    hdr.sdes.flags  = htons((Streamer::RTP_VERSION_NUMBER << 14)
                    | (RTCP::ITEM_COUNT << 8) | RTCP::SDES_PACKET_TYPE);
    hdr.sdes.ssrc   = hdr.sr.ssrc;
    hdr.sdes.type   = RTCP::CNAME;

    // NOTE: hdr.sdes.name must be 64 bytes otherwise struct SDES will have a wrong size.
    //       But since gethostname() returns an error if the buffer passed to it
    //       can't hold the entire hostname plus the null character, we must create
    //       temporary hostname buffer that can hold the worst case hostname size and
    //       then copy the result to hdr.sdes.name
    char hostname[64+5];
    memset(hostname, 0, sizeof(hostname));
    SBL_PERROR(gethostname(hostname, sizeof(hostname)) != 0);
    memcpy(hdr.sdes.name, hostname, sizeof(hdr.sdes.name));
    hdr.sdes.item_length = strlen(hdr.sdes.name);
    unsigned int sdes_length = sizeof(hdr.sdes) - sizeof(hdr.sdes.name) + hdr.sdes.item_length + 3;
    hdr.sdes.length      = htons(sdes_length / 4 - 1);
    unsigned int size  = sizeof(hdr.sr) + sdes_length;
    hdr.tcp[0]      = '$';
    hdr.tcp[1]      = 1;
    hdr.tcp[2]      = size >> 8;
    hdr.tcp[3]      = size;
    char* buffer = (char*) &hdr.sr;
    SBL_MSG(MSG::RTCP, "RTCP Sender socket %d\n"
                          "    Bytes:       %d\n"
                          "    Packets:     %d\n"
                          "    Timestamp:   %d\n"
                          "    From sender: %s\n",
                         _rtcp_socket.id(),
                         _total_bytes,
                         _total_packets,
                         timestamp(),
                         hdr.sdes.name);
    if (!_rtcp_socket.send(buffer - _offs, size + _offs, false))
        SBL_WARN("RTCP Message send failed, socket %d", _rtcp_socket.id());
}


}
