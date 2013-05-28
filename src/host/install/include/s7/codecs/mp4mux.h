/*****************************************************************************
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
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
*****************************************************************************/

#ifndef MP4MUX_H
#define MP4MUX_H

#include "mp4_common.h"

/****************************************************************************
  PACKAGE: MP4-AVMUX Stretch MP4 File Read/Write.

  DESCRIPTION:

  SECTION: Include
  {
  #include "mp4mux.h"
  }

  SECTION: Introduction
  In conjunction with the Stretch DVR SDK, there is a MP4 library that lets you
  read and write audio and video frames into mp4 file format. The generated file
  can be played using QuickTime player.

  This is an optional library that can be added to your Stretch DVR Application
  development environment. 

  This document describes the Application Programming Interface (API)
  implemented in the MP4-AVMUX library .

  This SDK provides the ability to:

    Write or synchronize audio and video frames in an .mov file.

    Read audio and video frames with their corresponding synchronized time stamps from
    the .mov file.

    Repair a corrupted .mov file so that it can be played back by QuickTime player.

  SECTION: Linking with MP4-AVMUX library
  
  This library is included as part of the SDVR-SDK library if you are linking your
  application with sdvr_sdk_dll.lib.
  Otherwise, to use MP4-AVMUX library link your DVR Application with 
  mp4lib_dll.lib for dynamic linking. This library is located in the host/lib
  folder of your RDK/EVK package .

  You must include "mp4mux.h" header file to your DVR Application before you can call 
  any of the APIs in MP4-AVMUX library. This header file is located in the
  host\src\codecs\avmux\mp4 folder.

  SECTION: Using MP4-AVMUX library to write Audio and Video frames
  This section provides the steps required to write A/V frames into an .mov file.

    The first step to write A/V frames to mp4 file is to open the file for write by
    calling mp4mux_open(). This function returns a file handle that must be used
    in all the subsequent calls to write A/V frames. 
    
    Next step is to add audio or video tracks to your mp4 file by calling 
    mp4mux_put_packet(). You can not write any type of frames unless their 
    corresponding track type is added. Currently, exactly one 
    video track optionally one audio track must be added to the mp4 file. 
    
    Call mp4mux_put_packet() to write encoded A/V frames that are sent by the DVR board.

    Close the file by calling mp4mux_close().

  SECTION: Using MP4-AVMUX library to read Audio and Video frames
  This section provides the steps required to read A/V frames from an .mov file.

    The first step to read A/V frames from an mp4 file is to open the file for read by
    calling mp4mux_open(). This function returns a file handle that must be used
    in all the subsequent calls to read A/V frames. In addition to the file handle,
    the number of A/V tracks recorded in the file is returned.

    For each track, call mp4mux_get_track_info() to get its track type, id, and CODEC. 
    In the event you are not interested in playing the audio, you can disable retrieving of
    audio frames by calling track mp4mux_disable_track().
    
    Call mp4mux_get_packet() to read one frame of audio or video at a time.

    Close the file by calling mp4mux_close().
   
  SECTION: Important Restrictions
  Note the following restrictions when using the MP4-AVMUX SDK. These
  restrictions, apart from those explicitly noted, are not permanent
  and will be removed in future versions of the SDK.
    
    Trick mode play back is not supported.

    Each MP4 file must contain exactly one video track.

    Maximum of one audio track is supported.

    It is the DVR Application's responsibility to use the time stamp and playback
    the A/V frames accordingly.

*****************************************************************************/

/**************************************************************************
   VISIBLE: A handle to MP4 file.
***************************************************************************/
#ifdef WIN32

typedef size_t      mp4_handle;

#else

#ifdef _LP64
typedef sx_uint64   mp4_handle;
#else    
typedef sx_uint32   mp4_handle;
#endif

#endif

/****************************************************************************
  VISIBLE: This enumerated type defines different file open mode used
  in mp4_info_t structure.

    MP4_RECORD      -  open a file for recording AV frames.

    MP4_READ_INFO   -  open a file to read MP4 file information.

    MP4_READ_PLAY   -  open a file for playback of AV frames.

    MP4_AUTHORIZE,  - open a file for to repair a damaged MP4 file.
****************************************************************************/
typedef enum {
    MP4_RECORD = 0,
    MP4_READ_INFO, 
    MP4_READ_PLAY, 
    MP4_AUTHORIZE, 
} mp4_mux_mode_t;

/****************************************************************************
  VISIBLE: This enumerated type defines different supported AV MUX files used
  in mp4_info_t structure.

    MODE_MP4  - mp4 file. Not supported.

    MODE_MOV  - Quicktime mov file

    MODE_3GP  - 3gp file. Not supported.
****************************************************************************/
typedef enum {
    MODE_MP4 = 0,   
    MODE_MOV,       
    MODE_3GP,       
} mp4_file_mode_t;

/****************************************************************************
  VISIBLE: This data structure is used to hold general AV MUX file information.
  Before calling mp4mux_open(), you must field this structure and set file_name,
  mux_mode, file_mode.

     file_name - The full path to the AV MUX file to be openned.

     mux_mode - The open file mode. (See mp4_mux_mode_t for different file 
     modes.)

     file_mode - The AV MUX file type. (See mp4_file_mode_t for different file 
     types.)

     timescale - This field is not used. Timescale hard coded based on 90k Hz clock.

     duration -  Duration of the longest track in file. This field is only used when
     mp4mux_get_file_info().

     nb_tracks  - umber of tracks in the file. This field is only used when
     mp4mux_get_file_info().

     error - The error code if mp4mux_open() returns zero.

     bs_buf_size - internal buffer size

     *log - Internal use only - A pointer to a file to hold the 
     debugging information.
****************************************************************************/

typedef struct {
    const char*     file_name;
    mp4_mux_mode_t  mux_mode;
    mp4_file_mode_t file_mode;
    sx_int32        timescale;
    sx_int64        duration;
    sx_int32        nb_tracks;
    sx_int32        error;
    sx_uint32       bs_buf_size;
    FILE            *log;
} mp4_info_t;

/****************************************************************************
  VISIBLE: This enumerated type defines different track types that can be
  added to an AV MUX file.

    MP4_TRACK_UNKNOWN - The track type is unknown.

    MP4_TRACK_AUDIO   - Audio track.

    MP4_TRACK_VIDEO   - Video track.

    MP4_TRACK_HINT    - Information track for streaming over the netwrok. 
    Not supported.
****************************************************************************/
typedef enum {
    MP4_TRACK_UNKNOWN = 0,  
    MP4_TRACK_AUDIO,        
    MP4_TRACK_VIDEO,        
    MP4_TRACK_HINT,         
} mp4_track_type_t;

/****************************************************************************
  VISIBLE: This data structure defines the AV track information in a MP4 file.

    type - The type of track that is added or read. (See mp4_track_type_t
    for the supported track types.)

    track_id - The track id is sequential number starting with '0' for the
    first track, '1' the second, and so on.

    timescale - The hardware time scale. When adding tracks, set this field
    to 90000 for video tracks and 8000 for audio tracks.

    duration - Duration of each frame. This field is not uset.

    width - Width of the video frame. This field is ignored for audio tracks.

    height - heigth of the video frame. This field is ignored for audio tracks.

    codec_id - The codec ID, when adding vido codec choose (CODEC_ID_H264 
    or CODEC_ID_MPEG4). For video codecs CODEC_ID_PCM_MULAW.

    language - The language code. This field is only used when adding audio
    field. To set the language code you should call mp4mux_iso639_to_lang().

    sample_size - The size of each audio sample in bytes. Since each audio
    sample is 1 byte. This field should be set to (1 * channels)

    channels -  Number of audio channels. It is should be set to 2. This field
    is ignored for video tracks.

    bit_rate - The bit rate. Not used.

    rc_buffer_size - The rate control buffer size. Not used.

    rc_min_rate - The minimum rate control. Not used.

    rc_max_rate - The maximum rate control. Not used.

    audio_vbr - The audio vbr flag. Not used.

    default_duration - The default sample duration. Set this field
    to (2560 / channels).

    default_size - The default sample size. Not used.
****************************************************************************/

typedef struct {
    mp4_track_type_t type;      // track type
    sx_int32    track_id;       // track id
    sx_int32    timescale;      // time scale
    sx_int64    duration;       // track duration
    sx_uint32   width;          // video width
    sx_uint32   height;         // video height
    codec_id_t  codec_id;       // codec id
    sx_int32    language;       // language (audio)
    sx_int32    sample_size;    // sample size (audio)
    sx_int32    channels;       // number of channels (audio)
    sx_uint32   bit_rate;       // bit rate
    sx_uint32   rc_buffer_size; // rate control buffer size
    sx_uint32   rc_min_rate;    // rate control min rate
    sx_uint32   rc_max_rate;    // rate control max rate
    sx_int32    audio_vbr;      // audio vbr flag (audio)
    // for fragment mode
    sx_uint32   default_duration;   // default sample duration
    sx_uint32   default_size;       // default sample size
    sx_int64    init_offset;    // initial offset in track time scale
} mp4_track_info_t;


/************************************************************************
  VISIBLE: The bit definition of flag field in the mp4_packet_t.

    PKT_FLAG_KEY_FRAME - Flag indicating a key frame. Following frame types
    should be marked as key frame for different CODECs:

        SDVR_VIDEO_ENC_H264: SDVR_FRAME_H264_IDR frame type is a key frame.

        SDVR_VIDEO_ENC_MPEG4: SDVR_FRAME_MPEG4_I frame type is a key frame.

        SDVR_VIDEO_ENC_JPEG: SDVR_FRAME_JPEG frame type is a key frame.

    PKT_FLAG_CONFIG_DATA - Flag indicating SPS, PPS, VOL, etc.
************************************************************************/

#define PKT_FLAG_KEY_FRAME      0x1
#define PKT_FLAG_CONFIG_DATA    0x2     // SPS/PPS, VOL, etc

/****************************************************************************
  VISIBLE: This data structure is used to write or read AV frames to MP4 file.

  Each AV frame is added as a packet which include some additionaly information
  other than the elemantary stream.

  After track is added to the MP4 file, you can add AV packets a specific track.

    @data@ - The packet data - This is the elemantary AV frame buffer.

    @size@ - The size of the data.

    @sample_count@  - The number of samples for all the channels. 
    For video this is 1 always. For PCM audio this is equal to field size.

    @flags@ - A bit map field indicating the frame type. (See above for
    the definition of these flags.)

    @duration@ -   Presentation duration in time_base units (0 if not available).
    This field is ignored when calling mp4mux_put_packet().

    @pts@ - Presendataion time stamp in time_base units when add or reading  B-frames.
    For any other video frame type it should be equal to the dts field.

    @dts@ - Date-Time stamp of the frame in time_base units. This field should be
    set to the timestamp field of the pci header when calling mp4mux_put_packet()

    @pos@ - The actual file offset of the packet within the mp4 file.
    A value of -1 means unknown. This field is only valid when calling 
    mp4mux_get_packet().

    @track_id@ - The track id of this packet. This is only used in mp4mux_get_packet().

****************************************************************************/
typedef struct {
    sx_uint8    *data;
    sx_int32    size;
    sx_int32    sample_count;
    sx_int32    flags;      
    sx_int32    duration;   
    sx_int64    pts;        
    sx_int64    dts;        
    sx_int64    pos;        
    sx_int32    track_id;   
} mp4_packet_t;

/****************************************************************************
  GROUP: Internal
****************************************************************************/
/*******************************************************************************
   A necessary evil introduced for C++ compatibility.  C source files must
   not declare a function "extern"; instead, they must declare the function
   "EXTERN".  For example:
   {
       EXTERN void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#if defined(__cplusplus)
   #define EXTERN              extern "C"
#else
   #define EXTERN              extern
#endif

/****************************************************************************
  Trick playback API - The funcions in this group are not currently supported.
****************************************************************************/
EXTERN sx_int32    mp4mux_set_play_speed(mp4_handle hdl, float speed);

// Seek flags
#define MP4_SEEK_INITIAL                0x00000001  // initial seek
#define MP4_SEEK_FLAG_BACKWARD          0x00000002  // seek backward
#define MP4_SEEK_FLAG_ANY               0x00000004  // seek for any frame
#define MP4_SEEK_FLAG_NEXT_KEY          0x00000008  // seek for the next key frame

EXTERN sx_int32    mp4mux_play_seek(mp4_handle hdl, sx_int32 track_id, sx_int64 timestamp, sx_uint32 flags);

/****************************************************************************
  GROUP: Initialization API
****************************************************************************/
EXTERN mp4_handle  mp4mux_open(mp4_info_t *mp4_open_info);
EXTERN sx_int32    mp4mux_close(mp4_handle file_handle);

/****************************************************************************
  GROUP: Recording API
****************************************************************************/
EXTERN sx_int32    mp4mux_add_track(mp4_handle hdl, mp4_track_info_t *track_info);
EXTERN sx_int32    mp4mux_put_packet(mp4_handle hdl, sx_uint32 track_id, mp4_packet_t *av_pkt);
EXTERN sx_int32    mp4mux_set_flush_period(mp4_handle hdl, sx_int64 period);

/****************************************************************************
  GROUP: Read and playback API
****************************************************************************/
EXTERN sx_int32    mp4mux_get_file_info(mp4_handle file_handle, mp4_info_t *mp4_file_info);
EXTERN sx_int32    mp4mux_get_track_info(mp4_handle file_handle, sx_int32 index, mp4_track_info_t *trk_info);
EXTERN sx_int32    mp4mux_disable_track(mp4_handle file_handle, sx_int32 track_id);
EXTERN sx_int32    mp4mux_get_packet(mp4_handle file_handle, mp4_packet_t *pkt);

/****************************************************************************
  GROUP: Utilitiy API
****************************************************************************/
EXTERN sx_int32    mp4mux_authorize(mp4_handle hdl);
EXTERN sx_int32    mp4mux_iso639_to_lang(const sx_int8 *lang, sx_int32 mp4);


#endif //#ifndef MP4MUX_H

