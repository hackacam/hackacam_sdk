/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/
/*
 * 1. Goal
 *		- Independent of PCI packet header layout.
 *		- Support audio/video/data streams.
 *		- Extensible for future use.
 *		- Low overhead and easy to parse.
 *		- Capable of resync to use in lossy environment.
 *
 * 2. Syntax
 *
 *	StretchRaw() 
 *  {
 *		FileDoneFlag (8) // For TiVo mode
 *		do {
 *			RawPacket()
 *		} while (nextbits() == sync_word)
 *	}
 *
 *	RawPacket()
 *	{
 *		RawPacketHeader()
 *		RawPacketPayload()
 *	}
 *
 *  RawPacketHeader()
 *	{
 *		sync_word (32)
 *		header_size (16)
 *		stream_id (8)
 *		version (8)
 *		flags (32)
 *		payload_size (32)
 *		if (flags & flagTimeStamp) {
 *			time_stamp_resolution (32)		// The frequency of clock that generates the time_stamp
 *			time_stamp (64)
 *		}
 *		if (flags & flagSeqNumber) {
 *			seq_number (8)
 *		}
 *		if (flags & flagHeaderExtension) {
 *			header_extension_size (16)
 *			for (i=0; i<header_extension_size; i++) {
 *				data_byte (8)
 *			}
 *		}
 *		if (flags & flagPayloadCRC) {
 *			payload_CRC (32)
 *		}
 *		if (flags & flagHeaderCRC) {
 *			header_CRC (32)
 *		}
 *	}
 *
 *  RawPacketPayload()
 *	{
 *		for (i=0; i<payload_size; i++) {
 *			data_byte (8)
 *		}
 *	}
 *
 *	StreamDefinitionTable()
 *	{
 *		table_id (8)
 *		version (8)
 *		flags (16)
 *		for (i=0; i<N; i++) {
 *			stream_id (8)
 *			codec_id (8)
 *			configuration_size (16)
 *			for (i=0; i<configuration_size; i++) {
 *				configuration_data (8)
 *			}
 *		}
 *	}
 *
 *	configuration_data()
 *	{
 *		if (codec_id == MJPEG) {
 *			configuration_data_mjpeg()
 *		}
 *		else if (codec_id == MPEG-4) {
 *			configuration_data_mpeg4()
 *		}
 *		else if (codec_id == H.264) {
 *			configuration_data_h264()
 *		}
 *		else if (codec_id == G.711) {
 *			configuration_data_g711()
 *		}
 *		else if (codec_id == G.726) {
 *			configuration_data_g726()
 *		}   
 *		else if (codec_id == SVC) {
 *			configuration_data_svc() 
 *		}
 *	}
 *
 *	configuration_data_h264()
 *	{
 *		width (16)
 *		height (16)
 *	}
 *
 *	configuration_data_g711()
 *	{
 *		sampling_rate (16)
 *		number_channels (16)
 *	}
 *
 *	configuration_data_g726()
 *	{
 *		sampling_rate (16)
 *		number_channels (16)
 *	}
 *
 *	configuration_data_svc()
 *	{
 *		num_spatial_layers(8)
 *		num_temporal_layers(8)
 *		for (i = 0; i < num_spatial_layers; i++) {
 *			for (j = 0; j < num_temporal_layers; j++) {
 *				spatial_layer_id[i][j] (8)
 *				temporal_layer_id[i][j] (8)
 *				width (16)
 *				height (16)
 *				frame_rate (16)		// frame_rate = (unsigned)floor(256.0 * fps + 0.5)
 *			}
 *		}
 *	}
 *
 *
 *	3. Semantic definition
 *
 *	3.1 Bit stream is in network order (big endian).
 *
 *	3.2 RawPacketHeader()
 *		- sync_word: 32-bit code pattern with value 0x73726666 (ASCII "srff" stands
 *		for Stretch Raw File Format)
 *		- header_size: 16-bit value specifies the size in bytes of raw packet header, 
 *		including syntax elements from version to the end of packet header.
 *		- stream_id: 8-bit stream id, see definition below.
 *		- version: 8-bit version number of packet header, it's value should be 0 for
 *		current version.
 *		- flags: 32-bit flags.
 *		- payload_size: 32-bit value specifies the size of payload in bytes.
 *		- time_stamp: 64-bit time stamp of this packet.
 *		- seq_number: 8-bit sequence number.
 *		- header_extension_size: 16-bit value specifies the size in bytes of header
 *		extension.
 *		- payload_CRC: 32-bit payload CRC value.
 *		- header_CRC: 32-bit header CRC value.
 *
 *	3.3 Definition of stream_id
 *		Value         Definition
 *	---------------------------------
 *		0x0           Forbidden
 *		0x1           SDT - Stream Definition Table
 *		0x02 - 0x1f   Reserved for Stretch use
 *		0x20 - 0x2f   Video streams
 *		0x30 - 0x3f   Reserved
 *		0x40 - 0x4f   Audio streams
 *		0x50 - 0x5f   Reserved
 *		0x60 - 0xff   Undefined
 *
 *	3.4 Definition of flags
 *		Value         Definition
 *	---------------------------------
 *		0x00000001    flagTimeStamp
 *		0x00000002    flagHeaderCRC
 *		0x00000004    flagPayloadCRC
 *		0x00000008    flagHeaderExtension
 *		0x00000010    flagKeyFrame
 *		0x00000020    flagConfigData
 *		0x00000040    flagSeqNumber
 *
 *	3.5 StreamDefinitionTable()
 *		- table_id: 8-bit table id.
 *		- version: 8-bit version.
 *		- flags: 16-bit flags.
 *		- stream_id: 8-bit stream id.
 *		- codec_id: 8-bit codec id.
 *		- configuration_size: 16-bit value specifies the size in bytes of 
 *			configuration data.
 *		- configuration_data: configuration data byte. The content of configuration
 *			data depends on the value of codec_id. Each codec has it own definition
 *			of configuration data.
 *
 *	3.6 Definition of codec ID
 *		Value         Definition
 *	---------------------------------
 *		0x00          Forbidden
 *		0x01          MJPEG
 *		0x02          MPEG-4
 *		0x03          H.264
 *		0x04		SVC
 *		0x05 - 0x7f   Reserved for video
 *		0x80          ITU-T G.711
 *		0x81          ITU-T G.726
 *		0x82 - 0xfe   Reserved for audio
 *		0xff          Undefined
 *
 *	4. Notes
 *		4.1 CRC uses MPEG-2 spec.
 *
*/
#ifndef _RAW_PARSER_H_
#define _RAW_PARSER_H_

#ifndef __H_SXTYPES
#include "sx-types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef offset_t
// support 64-bit file size
typedef sx_int64    offset_t;

#endif

#define MAX_NUM_STREAMS				255
#define MAX_SVC_LAYERS				9
#define MAX_SVC_SPATIAL				3
#define MAX_SVC_TEMPORAL			3

/////////////////////////////////////////////////////
// The Stretch Multi-media File Format Version Number
////////////////////////////////////////////////////
#define SMFF_FILE_VERSION           1


////////////////////////////////////////////////////
//  Version 1: Added time stamp frequency
//  Version 2: Fixed the header size. Prior to version 2
//             the value of header size that was written
//             by the write SDK routines was invalid and 
//             can not be used for parsing.
//
#define SMFF_STREAM_VIDEO_VERSION   2
#define SMFF_STREAM_AUDIO_VERSION   2


/*const sx_uint32*/ #define SRFF_SYNC_WORD			0x73726666

// flags
/*const sx_uint32*/ #define SRFF_FLAG_TIMESTAMP		0x00000001
/*const sx_uint32*/ #define SRFF_FLAG_HEADER_CRC	0x00000002
/*const sx_uint32*/ #define SRFF_FLAG_PAYLOAD_CRC	0x00000004
/*const sx_uint32*/ #define SRFF_FLAG_HEADER_EXT	0x00000008
/*const sx_uint32*/ #define SRFF_FLAG_KEY_FRAME		0x00000010
/*const sx_uint32*/ #define SRFF_FLAG_CONFIG_DATA	0x00000020
/*const sx_uint32*/ #define SRFF_FLAG_SEQ_NUMBER	0x00000040

// stream id
/*const sx_uint8*/ #define SRFF_STREAM_SDT			0x1
/*const sx_uint8*/ #define SRFF_STREAM_VIDEO		0x20
/*const sx_uint8*/ #define SRFF_STREAM_AUDIO		0x40

// codec id
/*const sx_uint8*/ #define SRFF_CODEC_ID_FORBIDDEN	0x00
/*const sx_uint8*/ #define SRFF_CODEC_ID_MJPEG		0x01
/*const sx_uint8*/ #define SRFF_CODEC_ID_MPEG4		0x02
/*const sx_uint8*/ #define SRFF_CODEC_ID_H264		0x03
/*const sx_uint8*/ #define SRFF_CODEC_ID_SVC		0x04
/*const sx_uint8*/ #define SRFF_CODEC_ID_G711		0x80
/*const sx_uint8*/ #define SRFF_CODEC_ID_G726		0x81

typedef enum {
	ST_RAW_PACKET_TYPE_UNKNOWN = 0,
	ST_RAW_PACKET_TYPE_TABLE,
	ST_RAW_PACKET_TYPE_VIDEO,
	ST_RAW_PACKET_TYPE_AUDIO,
	ST_RAW_PACKET_TYPE_H264_SPS,
	ST_RAW_PACKET_TYPE_H264_PPS,
	ST_RAW_PACKET_TYPE_H264_SEI
} st_raw_packet_type_e;


typedef struct {
	const char *pFilename;
} st_raw_parser_config_t;

typedef enum {
	ST_RAW_ERR_NONE = 0,
	ST_RAW_ERR_INVALID_PARAM,
	ST_RAW_ERR_NO_MEM,
	ST_RAW_ERR_NO_PACKET,	// File is not done yet, but cannot read any more data (TiVo)
	ST_RAW_ERR_EOF,			// File is done written and no more data can be read.
	ST_RAW_ERR_NO_SDT,
	ST_RAW_ERR_OPEN_FILE,
	ST_RAW_ERR_WRONG_CODEC_TYPE,
    ST_RAW_ERR_READ_ITEM   // A field value was read but containS invalid value.
} st_raw_err_e;


/* For h264, mjpeg, mpeg4 */
typedef struct config_info_video_generic_s {
	sx_uint16	width;
	sx_uint16	height;
} config_info_video_generic_t;

typedef struct config_info_audio_generic_s {
	sx_uint16	sampling_rate;
	sx_uint16	num_channels;
} config_info_audio_generic_t;

typedef struct svc_layer_info_s {
	sx_uint8	spatial_layer_id;
	sx_uint8	temporal_layer_id;
	sx_uint16	width;
	sx_uint16	height;
	sx_uint16	frame_rate;	// frame_rate = (unsigned)floor(256.0 * fps + 0.5)
} svc_layer_info_t;

typedef struct config_info_video_svc_s {
	sx_uint8	num_spatial_layers;
	sx_uint8	num_temporal_layers;
	svc_layer_info_t	layer_info[MAX_SVC_LAYERS];
} config_info_video_svc_t;

typedef struct stream_info_s {
	sx_uint8	stream_id;
	sx_uint8	codec_id;
	sx_uint16	config_size;
	void		*pConfig;
} stream_info_t;

typedef struct sdt_s {
	sx_uint8		table_id;
	sx_uint8		version;
	sx_uint16		flags;
	sx_uint8		num_streams;
	stream_info_t	streams[MAX_NUM_STREAMS];
} sdt_t;


/**********************************************************************************
 The parser context is stored in st_raw 
 *********************************************************************************/
typedef struct    st_raw_parser_s            *st_raw_parser_t;

st_raw_err_e	st_raw_parser_init(st_raw_parser_t *parser, st_raw_parser_config_t *config);
st_raw_err_e	st_raw_parser_free(st_raw_parser_t parser);

/* SDT information */
st_raw_err_e	st_raw_get_num_streams(st_raw_parser_t parser, sx_uint8 *num_streams);
st_raw_err_e	st_raw_get_stream_id_from_index(st_raw_parser_t parser, sx_int32 index, sx_uint8 *stream_id); /* index is 0 -> [num_streams - 1] */
st_raw_err_e	st_raw_get_stream_codec_type_from_index(st_raw_parser_t parser, sx_int32 index, sx_uint8 *codec_id); /* index is 0 -> [num_streams - 1] */
st_raw_err_e    st_raw_get_config_size_from_index(st_raw_parser_t parser, sx_int32 index, sx_uint16 *config_size);

/* config info for h.264, mpeg4 and mjpeg */
st_raw_err_e    st_raw_get_generic_video_config_from_index(st_raw_parser_t parser, sx_int32 index, config_info_video_generic_t *pConfig);
st_raw_err_e	st_raw_get_generic_audio_config_from_index(st_raw_parser_t parser, sx_int32 index, config_info_audio_generic_t *pConfig);
st_raw_err_e	st_raw_get_svc_config_from_index(st_raw_parser_t parser, sx_int32 index, config_info_video_svc_t *pConfig);

/* Force the parser to a location inside the file */
st_raw_err_e	st_raw_seek_to_offset(st_raw_parser_t parser, offset_t offset); 

/* 
 * Parser starts reading the next packet, after this function call, the parser contains 
 * the header information 
 */
st_raw_err_e	st_raw_parse_next_packet(st_raw_parser_t parser); 

/* Functions returning the data/information of current packet */
sx_uint8	st_raw_get_stream_id(st_raw_parser_t parser);
sx_uint8    st_raw_get_codec_type(st_raw_parser_t parser);
offset_t	st_raw_get_header_offset(st_raw_parser_t parser);
offset_t	st_raw_get_payload_offset(st_raw_parser_t parser);
sx_int64	st_raw_get_timestamp(st_raw_parser_t parser);
sx_uint32	st_raw_get_payload_size(st_raw_parser_t parser);
sx_uint32	st_raw_get_flags(st_raw_parser_t parser);
sx_bool		st_raw_is_key_frame(st_raw_parser_t parser);
sx_bool		st_raw_is_data_frame(st_raw_parser_t parser);
st_raw_err_e	st_raw_get_payload_data(st_raw_parser_t parser, sx_uint8 *buf); /* The buf should points to a buffer with at least the payload size */
st_raw_err_e	st_raw_get_first_eight_video_payload_bytes(st_raw_parser_t parser, sx_uint8 *buf);

#ifdef __cplusplus
}
#endif

#endif //_RAW_PARSER_H_


