/****************************************************************************\
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
\****************************************************************************/

#ifndef ARM_COMMON_H_
#define ARM_COMMON_H_

/// @cond

//#include "liba2a.h"
#include <sdk/sdvr_sdk.h>

/*
 *  The status error code when communicating using a2a library.
 *  NOTE: The status code should not exceed 255.
 */
typedef enum _arm_common_status_e {
    ARM_ERR_NONE = 0,

    // The following errors are from A2A mapped to be under 256
    // this is needed for us to transmit the errors.
    ARM_A2A_ERR_INVALID_PARAMETER,
    ARM_A2A_ERR_DEVICE_CLOSED,
    ARM_A2A_ERR_SYSTEM, /* misc system errors, check errno */
    ARM_A2A_ERR_NO_PEER,
    ARM_A2A_ERR_PEER_CLOSED,
    ARM_A2A_ERR_NO_CHANNELS,
    ARM_A2A_ERR_CHANNEL_IN_USE,
    ARM_A2A_ERR_PORT_IN_USE,
    ARM_A2A_ERR_CHANNEL_CREATE,
    ARM_A2A_ERR_CHANNEL_CONNECT,
    ARM_A2A_ERR_CHANNEL_CLOSING,
    ARM_A2A_ERR_CHANNEL_NOT_ACTIVE,
    ARM_A2A_ERR_CHANNEL_DEAD,
    ARM_A2A_ERR_NO_RECV_BUFFERS,
    ARM_A2A_ERR_NO_SEND_BUFFERS,
    ARM_A2A_ERR_MSG_SEND,
    ARM_A2A_ERR_MSG_RECV,


    // Following are the error code specific to the ARM application
    ARM_ERR_ARM_CONNECT,
    ARM_ERR_MEM_ALLOC,
    ARM_SDK_CHAN_NOT_FOUND,
    ARM_ERR_CHAN_CREATE,
    ARM_ERR_CHAN_CONNECT,
    ARM_STREAM_NOT_FOUND,
    ARM_ENV_VAR_MISSING,
    ARM_CONFIG_FILE,
    ARM_INVALID_DEVICE_ID,
    ARM_UNKNOWN_ARM_MSG,
    ARM_INTERNAL_ERROR,
    ARM_INVALID_STREAM_ID,

    ARM_A2A_INIT_FAILED,
} arm_common_status_e;

/*
 *  The message class to be used for communication between ARM process in PE0
 *  and PE1. The application in PE0 should send these messages to the PE1 when
 *  it starts up and as cameras are created, destroyed, or anything changes
 *  about the camera (such as video encoder being enabled). In most cases,
 *  the Application in PE1 will only receive messages with some exception of
 *  sending the messages to enable or disable a particular video encoder stream
 *  on a camera or some information such as the version number.
 *
 *
 *  VRM_CREATE_MSG_CLASS - Request to create/add a device
 *
 *  VRM_GET_MSG_CLASS - Request to get some information according to the opcode for
 *  the device.
 *
 *  VRM_DELETE_MSG_CLASS -- Request to destroy/delete a device
 *
 *  VRM_UPDATE_MSG_CLASS -- Request to change some existing attribute/parameters
 *  for the device.
 *
 *  VRM_RESPONSE_MSG_CLASS -- The status response to create, delete, update in order
 *  to indicate success or failure.
 *
 *  VRM_RESPONSE_GET_MSG_CLASS -- The data returned for the requested get message including
 *  the success of failure status.
 */
typedef enum _arm_message_e {
    VRM_MSG_CLASS_NONE = 0,
    VRM_CREATE_MSG_CLASS,
    VRM_GET_MSG_CLASS,
    VRM_DELETE_MSG_CLASS,
    VRM_UPDATE_MSG_CLASS,
    VRM_RESPONSE_MSG_CLASS,
    VRM_RESPONSE_GET_MSG_CLASS,
    VRM_FILE_REQ_MSG_CLASS
} arm_message_e;

typedef struct _msg_file_req_info_t {
    sx_uint32        fileIndex;
    sx_uint32        fileOffset;
    sx_uint32        segmentLen;
} msg_file_req_info_t;

#define	S7_SEG_LEN_MASK		0x7FFFFFFF
#define	S7_FILE_END_FLAG	0x80000000

typedef struct _file_buf_header_t {
    sx_uint32        magic;
    sx_uint32        fileIndex;
    sx_uint32        fileOffset;
    sx_uint32        segmentLen;
} file_buf_header_t;

/* The operation code ID to be used with the specified message class.
 *
 * OPCODE_DEVICE - To connect or disconnect an ARM process, encoder channel,
 * decoder channel, etc.
 * This opcode should be used with message class VRM_CREATE_MSG_CLASS
 * in order to connect and
 * used with message class VRM_DELETE_MSG_CLASS in order to disconnect.
 *
 * OPCODE_STREAM_CODEC - To assign, change, or query the audio/video codec associated
 * with a device (camera or decoder). This opcode to be used with
 * VRM_CREATE_MSG_CLASS to add a new stream and VRM_DELETE_MSG_CLASS to
 * delete a video stream from a encoder/decoder device.
 *
 * OPCODE_CODEC_INFO - To set the A/V CODEC parameters such as frame rate and
 * frame size. This opcode is to be used with VRM_UPDATE_MSG_CLASS.
 *
 * OPCODE_ENABLE - To enabled Audio/Video stream on a given encoder/decoder device.
 */
typedef enum _arm_opcode_e {
    OPCODE_NONE,
    OPCODE_DEVICE,
    OPCODE_STREAM_CODEC,
    OPCODE_CODEC_INFO,
    OPCODE_ENABLE,
    OPCODE_FILE_STREAMS
} arm_opcode_e;

/****************************************************************************
  This enumerated type describes the kinds of devices supported
  by SDVR. To create a channel that only allows it to be used in HMO or
  SMO, you must use DEVICE_TYPE_CHAN_ENCODER, and set the encoder type to
  SDVR_VIDEO_ENC_NONE.

    DEVICE_TYPE_NONE       - Device type not specified.

    DEVICE_TYPE_CHAN_ENCODER - Encoder channel.

    DEVICE_TYPE_CHAN_DECODER - Decoder channel.

    DEVICE_TYPE_ARM          - ARM process.


****************************************************************************/
typedef enum _device_type_e{
    DEVICE_TYPE_NONE           = 255,
    DEVICE_TYPE_CHAN_ENCODER   = 0,
    DEVICE_TYPE_CHAN_DECODER   = 2,
    DEVICE_TYPE_ARM            = 4,

} device_type_e;

/*
 *  The header data structure associated with all of the ARM message opcodes.
 *
 *  Parameters:
 *
 *     opcode - The command to perform.
 *
 *     device_id - Zero based device ID. (i,e. for ARM operation it is the
 *     ARM index, for the camera it is the camera number, etc).
 *
 *     device_type - The type of device to get/set its property.
 */
typedef struct _arm_msg_hdr_t {
    sx_uint8           status;
    arm_opcode_e       opcode:8;
    device_type_e      device_type:8;
    sx_uint8           device_id;

} arm_msg_hdr_t;

/*
 * This data structure is used by PE0 in order to connect/disconnect to/from PE1.
 * PE1 will not response to any messages or receives any video buffer to stream
 * until this message is sent to it.
 *
 * CONNECT request from ARM Process on PE0 to PE1, the Application on PE0 sends
 * the following message to PE1.
 * VRM_CREATE_MSG_CLASS
 *
 * Sends Request (PE0->PE1):
 *     hdr.opcode       = OPCODE_DEVICE
 *     hdr.device_type  = DEVICE_TYPE_ARM
 *     hdr.device_id    = 0 -- NOTE: PE0's peer ID
 *     arm_major_ver    = The ARM's Application major version
       arm_minor_ver    = The ARM's Application minor version
       arm_revision_ver = The ARM's Application revision version
       arm_build_ver    = The ARM's Application build version
       fw_major_ver     = The Firmware's major version
       fw_minor_ver     = The Firmware's minor version
       fw_revision_ver  = The Firmware's revision version
       fw_build_ver     = The Firmware's build version
       maxCameraCount   = Total number of cameras that can be create on this ARM
 *     maxVideoStreamPerCamera = Maximum number of video stream allowed per
 *     camera. NOTE: The stream ID is zero based when adding video streams to
 *     each camera.
 *     a2a_chan_num     = The system wide A2A channel that PE0 send and receive
 *                        A/V buffers.
 *
 * Receive Response (PE1->PE0):
 *     hdr.status = 0 for success or status code
 *
 *
 * Disconnect request ARM Process on PE0 from PE1, the Application on PE0 sends
 * the following message to PE1.
 *
 * VRM_DELETE_MSG_CLASS
 *
 * Sends Request (PE0->PE1):
 *     hdr.opcode       = OPCODE_DEVICE
 *     hdr.device_type  = DEVICE_TYPE_ARM
 *     hdr.device_id    = 0 -- NOTE: PE0's peer ID
 *
 * Receive Response (PE1->PE0):
 *     hdr.status = 0 for success or status code
 */
typedef struct _msg_connect_t {
    arm_msg_hdr_t  hdr;
    sx_uint8       arm_major_ver;
    sx_uint8       arm_minor_ver;
    sx_uint8       arm_revision_ver;
    sx_uint8       arm_build_ver;
    sx_uint8       fw_major_ver;
    sx_uint8       fw_minor_ver;
    sx_uint8       fw_revision_ver;
    sx_uint8       fw_build_ver;
    sx_uint8       maxCameraCount;
    sx_uint8       maxVideoStreamPerCamera;
    sx_uint8       a2a_chan_num;
    sx_uint8      reserverd;
} msg_connect_t;

/*
 * The data structure to be used by OPCODE_DEVICE,
 * every time a device (i,e, camera, decoder) is created or destroyed by the
 * master ARM process.
 *
 * Use VRM_CREATE_MSG_CLASS when the device is created and VRM_DELETE_MSG_CLASS
 * when the device is removed.
 *
 *  Parameters:
 *
       hdr.opcode         - OPCODE_DEVICE

       hdr.device_type    - DEVICE_TYPE_CHAN_ENCODER or DEVICE_TYPE_CHAN_DECODER

       hdr.device_id      - For encoder channel it is the associated A/V data port,
       For the decoder channel is the decode channel ID

       vStd               - The video standard of the encoder channel. It is
       ignored for the decoder channel types. This field is not used when
       VRM_DELETE_MSG_CLASS.


 */
typedef struct _msg_device_info_t {
    arm_msg_hdr_t    hdr;

    sdvr_video_std_e vStd;
    sx_uint32        reserverd[2];

} msg_device_info_t;

/*
 * The data structure to be used to add, remove, update, or get a CODEC video or
 * audio stream to the given encoder or decoder device.
 *
 * Use VRM_UPDATE_MSG_CLASS to add, remove, or update the CODEC and
 * VRM_GET_MSG_CLASS to get the current codec type.
 *
 *  Parameters:
 *
    hdr.opcode        = OPCODE_STREAM_CODEC

    hdr.device_type   = DEVICE_TYPE_CHAN_ENCODER or DEVICE_TYPE_CHAN_DECODER

    hdr.device_id     = <the device ID associated with the device type>
 *
 *  streamId          = The nTh encoder or decoder associated with the device
 *                      For decoder currently only one stream is support and
 *                      this field should be set to zero.
 *                      For encoders, it is 0 - 3 assuming your board type supports
 *                      4 encoder stream.
 *
 *  videoCodecType   - Set to SDVR_VIDEO_ENC_NONE if removing this video stream
 *                     otherwise, specify the video codec for this stream.
 *
 *  audioCodecType   - Set to SDVR_AUDIO_ENC_NONE if removing this audio stream
 *                     otherwise, specify the audio codec for this stream.
 */
typedef struct _msg_stream_codec_t {
    arm_msg_hdr_t    hdr;
    sx_uint8         streamId;
    sdvr_venc_e      videoCodecType:8;
    sdvr_aenc_e      audioCodeCType:8;
    sx_uint8         reserve;
    sx_uint32        reserve1[2];
} msg_stream_codec_t;

/*
 * The data structure to be used to get or update video CODEC attribute
 * associated with a specific video stream of a device (encoder or decoder channel).
 *
 * Use VRM_UPDATE_MSG_CLASS update and
 * VRM_GET_MSG_CLASS to get the current video codec attributes.
 *
 *  Parameters:
 *
    hdr.opcode        = OPCODE_STREAM_CODEC_INFO

    hdr.device_type   = DEVICE_TYPE_CHAN_ENCODER or DEVICE_TYPE_CHAN_DECODER

    hdr.device_id     = <the device ID associated with the device type>
 *
 *  streamId          = The nTh video encoder or decoder stream associated with
 *                      the device.
 *                      For decoder currently only one stream is support and
 *                      this field should be set to zero.
 *                      For encoders, it is 0 - 3 assuming your board type supports
 *                      4 encoder stream.
 *
 *  frameRate        - The video frame for this video stream
 *
 *  videoWidth       - The video frame width for this stream
 *
 *  videoHeight      - The video frame height for this stream.
 */
typedef struct _msg_stream_codec_info_t {
    arm_msg_hdr_t    hdr;
    sx_uint8         streamId;
    sx_uint8         frameRate;
    sx_uint8         reserve1[2];
    sx_uint32        videoWidth;
    sx_uint32        videoHeight;
} msg_stream_codec_info_t;

/*
 * The data structure to be used with VRM_UPDATE_MSG_CLASS in order  set the
 * current streaming state of associated with a specific video stream of a device
 * (encoder or decoder channel).
 *
 * In general this message is sent from ARM 1 to ARM 0 any time an RTSP request
 * is received. The application will redirect all the video frames for the
 * requested stream ID on the device once the stream is enabled.
 *
 *  Parameters:  (PE1 ==> PE0)
 *
    hdr.opcode        = OPCODE_ENABLE

    hdr.device_type   = DEVICE_TYPE_CHAN_ENCODER

    hdr.device_id     = <the device ID associated with the device type>
 *
 *  streamId          = The nTh video encoder stream associated with
 *                      the camera.
 *                      The valid range is 0 - 3 assuming your board type supports
 *                      4 encoder streams.
 *
 *  bEnable           = Set to 'true' for the PE1 to send the video frames to PE0
 *                      Set to 'false' for PE1 to stop sending any video frames.
 *
 *  Remarks:
 *
 *     This message will not have any response to be sent from PE0 to PE1
 */
typedef struct _msg_stream_enable_t {
    arm_msg_hdr_t    hdr;
    sx_uint8         streamId;
    sx_uint8         bEnable;
    sx_uint8         reserve1[2];
    sx_uint32        reserve2[2];
} msg_stream_enable_t;

/// @endcond

#endif /* ARM_COMMON_H_ */
