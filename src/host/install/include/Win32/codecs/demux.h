#ifndef _demux_h_
#define _demux_h_

/*!
 * \file demux.h
 *      It implements demux APIs.
 *      It is the CDemuxParser C-wapper of demux_parser.cpp.
 */
//=============================================================================
// Copyright (c) 2011 Stretch Inc
// All rights reserved.
//
// No part of this document may be copied, reproduced,
// or transmitted in any form without the prior written
// consent of Stretch Inc
//=============================================================================

/******************************************************************************
    PACKAGE: demux - Audio and video frame parser

    DESCRIPTION:

    SECTION: Include
    {
        #include "demux.h"
    }

    SECTION: Introduction
    @demux@ is a collection of audio and video frame parsers for various
    container formats, such as Stretch RAW Packets (RAW),
    MPEG-2 Program Stream (MPEG2-PS), H264 Elementary Stream (H264-ES),
    Audio Video Interleave (AVI), ISO 14496 part 12 (MP4), and some
    third-party containers.

    It reads audio and video frames with their corresponding synchronized
    time stamps from a container file.

    This is an optional library that can be added to your Stretch DVR
    Application development environment. 

    This document describes the Application Programming Interface (API)
    implemented in the @demux@ library.

    SECTION: Linking with @demux@ library
    This library is included as part of the SDVR-SDK library
    if you are linking your application with sdvr_sdk_dll.lib.
    Otherwise, to use @demux@ library link your DVR Application with 
    libavmux_demux_dll.lib for dynamic linking.
    This library is located in the host/lib folder of your RDK/EVK package.

    You must include "demux.h" header file to your DVR Application
    before you can call any of the APIs in @demux@ library.
    This header file is located in the host\src\codecs\avmux\demux folder.

    SECTION: Use model
    {
        #include "demux.h"

        // configure file to open a parser and set auto detect file type
        st_demux_parser_config_t  config;
        config.file_path = input_file_name;     // of (unsigned char *)
        config.file_type = ST_DEMUX_FILE_TYPE_DETECT;

        // open a demux
        st_demux_t *demux;
        demux_open(&demux, &config);

        // loop to read audio & video frame
        st_demux_err_e err = ST_DEMUX_ERR_NONE;
        do {
            // parse next packet
            err = demux_parse_next_packet(demux);
            if(err != ST_DEMUX_ERR_NONE)
                break;

            // get payload size and allocate payload buffer
            sx_uint32 payload_size = demux_get_payload_size(demux);
            unsigned char *payload_buf = (unsigned char*)malloc(payload_size);
            err = demux_get_payload_data(demux, payload_buf);
            if(err != ST_DEMUX_ERR_NONE)
                break;

            // get time stamp
            sx_int64 timestamp = demux_get_timestamp(demux);

            // get stream id and process by stream type
            st_demux_stream_id_e stream_id = demux_get_stream_id(demux);
            if(stream_id == ST_DEMUX_STREAM_ID_VIDEO) {
                // process video packet
                ...
            } else if(stream_id == ST_DEMUX_STREAM_ID_AUDIO) {
                // process audio packet
                ...
            }

            // free payload buffer
            free(payload_buf);
        } while(err != ST_DEMUX_ERR_EOF);

        demux_close(demux);
    }


    SECITON: Restrictions
    These restrictions, apart from those explicitly noted, are not permanent
    and will be removed in future versions of the SDK.

        It does not support to write audio and video frames.

        It does not support to parse AVI, MP4, H264-ES formats yet.

        It is the DVR Application's responsibility to use the time stamp and
        playback the A/V frames accordingly.

******************************************************************************/

//=============================================================================
// COMPILATION OPTIONS
//=============================================================================
#include "demux_include.h"

//=============================================================================
// HEADER FILES
//=============================================================================
#ifndef __H_SXTYPES
#include "sx-types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// MACROS & INLINE FUNCTIONS
//=============================================================================

//=============================================================================
// TYPEDEF and DATA STRUCTURE
//=============================================================================

//=============================================================================
// CLASS INTERFACE
//=============================================================================
typedef struct st_demux_struct  st_demux_t;

//=============================================================================
// METHODS
//=============================================================================
/* create and free a demux object */
st_demux_err_e demux_open(st_demux_t **demux, st_demux_parser_config_t *config);
st_demux_err_e demux_configure(st_demux_t *demux, st_demux_parser_config_t *config);
st_demux_err_e demux_close(st_demux_t *demux);

/* Parser starts reading the next packet.
 * After this function call, the parser contains the header information 
 */
st_demux_err_e demux_parse_next_packet(st_demux_t *demux);

/* Force the parser to a location inside the file */
st_demux_err_e seek_to_offset(st_demux_t *demux, const st_offset_t offset);

/*  PKT accessor functions */
st_demux_err_e        demux_get_payload_data(st_demux_t *demux, sx_uint8 *buf);
st_demux_err_e        demux_get_first_eight_video_payload_bytes(st_demux_t *demux,
                            sx_uint8 *buf);

st_demux_stream_id_e  demux_get_stream_id(st_demux_t *demux);
st_offset_t           demux_get_header_offset(st_demux_t *demux);
st_offset_t           demux_get_payload_offset(st_demux_t *demux);
sx_int64              demux_get_timestamp(st_demux_t *demux);
sx_uint32             demux_get_timestamp_freq(st_demux_t *demux);
sx_uint32             demux_get_payload_size(st_demux_t *demux);
sx_uint32             demux_get_flags(st_demux_t *demux);
sx_bool               demux_is_key_frame(st_demux_t *demux);
sx_bool               demux_is_data_frame(st_demux_t *demux);

/*  SDT accessor functions */
st_demux_err_e demux_get_num_streams(st_demux_t *demux, sx_uint8 *num_streams);
st_demux_err_e demux_get_stream_id_at_index(st_demux_t *demux,
                            const sx_int32 index, sx_uint8 *stream_id);
st_demux_err_e demux_get_stream_codec_type_at_index(st_demux_t *demux,
                            const sx_int32 index, sx_uint8 *codec_id);
st_demux_err_e demux_get_config_size_at_index(st_demux_t *demux,
                            const sx_int32 index, sx_uint16 *config_size);

/*  config info for h264, mpeg4 and mjpeg */
st_demux_err_e demux_get_generic_video_config_at_index(st_demux_t *demux,
                            const sx_int32 index,
                            st_config_info_video_generic_t **video_config);
st_demux_err_e demux_get_generic_audio_config_at_index(st_demux_t *demux,
                            const sx_int32 index,
                            st_config_info_audio_generic_t **audio_config);
st_demux_err_e demux_get_svc_config_at_index(st_demux_t *demux,
                            const sx_int32 index,
                            st_config_info_video_svc_t **svc_config);

//=============================================================================
// PRIVATE METHODS
//=============================================================================

//=============================================================================
#ifdef __cplusplus
}
#endif

#endif /* _demux_h_ */
