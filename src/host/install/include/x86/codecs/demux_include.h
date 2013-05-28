#ifndef _demux_include_h_
#define _demux_include_h_

/*!
 * \file demux_include.h
 *      It defines public definitions of demux.
 */
//=============================================================================
// Copyright (c) 2011 Stretch Inc
// All rights reserved.
//
// No part of this document may be copied, reproduced,
// or transmitted in any form without the prior written
// consent of Stretch Inc
//=============================================================================

//=============================================================================
// COMPILATION OPTIONS
//=============================================================================

//=============================================================================
// HEADER FILES
//=============================================================================
#ifndef __H_SXTYPES
#include "sx-types.h"
#endif

//=============================================================================
// MACROS & INLINE FUNCTIONS
//=============================================================================
// supporting macros and constants
#define MAX_NUM_STREAMS         255
#define MAX_SVC_LAYERS            9
#define MAX_SVC_SPATIAL           3
#define MAX_SVC_TEMPORAL          3

//=============================================================================
// TYPEDEF and DATA STRUCTURE - interface
//=============================================================================
/**
    Enumerate err codes of demux
**/
typedef enum {
    ST_DEMUX_ERR_NONE                 =  0,     // success or OK
    ST_DEMUX_ERR_INVALID_FUNC         = -1,
    ST_DEMUX_ERR_INVALID_PARAM        = -2,
    ST_DEMUX_ERR_NO_MEM               = -3,
    ST_DEMUX_ERR_NO_PACKET            = -4,   // File is not done yet, but cannot read any more data (TiVo)
    ST_DEMUX_ERR_EOF                  = -5,   // File is done written and no more data can be read.
    ST_DEMUX_ERR_NO_SDT               = -6,
    ST_DEMUX_ERR_OPEN_FILE            = -7,
    ST_DEMUX_ERR_WRONG_CODEC_TYPE     = -8,
    ST_DEMUX_ERR_READ_ITEM            = -9,   // A field value was read but containS invalid value.
    ST_DEMUX_ERR_FILE_TYPE_UNKNOWN    = -10,
    ST_DEMUX_ERR_READ_LESS_PAYLOAD    = -11,
    ST_DEMUX_ERR_READ_NO_PAYLOAD      = -12,
    ST_DEMUX_ERR_PARSE_SPS            = -13,
    ST_DEMUX_ERR_PARSE_PPS            = -14,
} st_demux_err_e;

/**
    Enumerate stream IDs of demux
**/
typedef enum {
    ST_DEMUX_STREAM_ID_UNKNOWN        = 0x00,
    ST_DEMUX_STREAM_ID_SDT            = 0x01,
    ST_DEMUX_STREAM_ID_VIDEO          = 0x20,
    ST_DEMUX_STREAM_ID_AUDIO          = 0x40,
} st_demux_stream_id_e;

/**
    Enumerate codec IDs (types) of demux
**/
/* codec id */
typedef enum {
    ST_DEMUX_CODEC_ID_FORBIDDEN	    = 0x00,
    ST_DEMUX_CODEC_ID_MJPEG		    = 0x01,
    ST_DEMUX_CODEC_ID_MPEG4		    = 0x02,
    ST_DEMUX_CODEC_ID_H264		    = 0x03,
    ST_DEMUX_CODEC_ID_SVC		    = 0x04,
    ST_DEMUX_CODEC_ID_G711		    = 0x80,
    ST_DEMUX_CODEC_ID_G726		    = 0x81,
} st_demux_codec_id_e;

/**
    Enumerate file types handled by demux
**/
typedef enum {
    ST_DEMUX_FILE_TYPE_MOV,
    ST_DEMUX_FILE_TYPE_SRFF,
    ST_DEMUX_FILE_TYPE_SVC,
    ST_DEMUX_FILE_TYPE_AVI,
    ST_DEMUX_FILE_TYPE_H264_ES,
    ST_DEMUX_FILE_TYPE_MJPEG_ES,
    ST_DEMUX_FILE_TYPE_MPEG4_ES,
    ST_DEMUX_FILE_TYPE_MPEG_PS,   /*  MPEG Program stream, 13818-1 */
    ST_DEMUX_FILE_TYPE_HIK_MPEG,
    ST_DEMUX_FILE_TYPE_HIK_H264,
    ST_DEMUX_FILE_TYPE_DETECT,
    ST_DEMUX_FILE_TYPE_UNKNOWN,
} st_demux_file_type_e;

/**
    Define demux configuration structure
    @Fields@:
        "file_path" - file path of this parsed file

        "file_type" - specify file type of this parsed file
**/
typedef struct {
	const char             *file_path;
    st_demux_file_type_e    file_type;
} st_demux_parser_config_t;

#if 1   /*  remove me later */
#ifndef st_offset_t
// support 32-bit file size
typedef sx_int32    st_offset_t;
#endif

/* For h264, mjpeg, mpeg4 */
/**
    Define video configuration structure
**/
typedef struct st_config_info_video_generic_s {
    sx_uint16   width;
    sx_uint16   height;
} st_config_info_video_generic_t;

/**
    Define audio configuration structure
**/
typedef struct st_config_info_audio_generic_s {
    sx_uint16   sampling_rate;
    sx_uint16   num_channels;
} st_config_info_audio_generic_t;

/**
    Define SVC layer information structure
**/
typedef struct st_svc_layer_info_s {
    sx_uint8    spatial_layer_id;
    sx_uint8    temporal_layer_id;
    sx_uint16   width;
    sx_uint16   height;
    sx_uint16   frame_rate;    // frame_rate = (unsigned)floor(256.0 * fps + 0.5)
} st_svc_layer_info_t;

/**
    Define video SVC configuration structure
**/
typedef struct st_config_info_video_svc_s {
    sx_uint8    num_spatial_layers;
    sx_uint8    num_temporal_layers;
    st_svc_layer_info_t    layer_info[MAX_SVC_LAYERS];
} st_config_info_video_svc_t;

#endif

//=============================================================================
// CLASS INTERFACE
//=============================================================================

//=============================================================================
// METHODS
//=============================================================================

//=============================================================================
// PRIVATE METHODS
//=============================================================================

//=============================================================================
#endif /* _demux_include_h_ */
