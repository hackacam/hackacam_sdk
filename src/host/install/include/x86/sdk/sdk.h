/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/

#ifndef STRETCH_SDK_H
#define STRETCH_SDK_H
#ifndef WIN32
#include <pthread.h>
#endif
#include <sct/sct.h>
#include <sct/sct-board.h>
#include <codecs/venc264_authenticate.h>
#include <codecs/mp4mux.h>
#include <codecs/raw_parser.h>
#include <codecs/demux.h>
#include <svc/svcext.h>
#include "svcfileext.h"


/**********************************************************************
  The SCT port ID to use to send large data information to the firmware.
  Currently this is used only for sending ROI map.
**********************************************************************/
#define SCT_PORT_SEND_DATA     0xFABE

/**********************************************************************
  The SCT port ID to use to send font table information to the firmware.
  This ID will be sent to the firmware everytime SDK needs to send a
  new font table.
**********************************************************************/
#define SCT_PORT_FONT_TABLE     0xABCD

/**********************************************************************
  The SCT port ID to use to receive assertion debug data from the
  firmware.
**********************************************************************/
#define SCT_PORT_DEBUG_DATA     0xDBAC

/**********************************************************************
  The bit set in hdr_version field of sdvr_av_buffer_t which indicates
  whether the SCT buffer was allocated by the Host which means it is a send
  buffer. This is needed so that we know what method to use to free
  this buffer.

  NOTE: This bit is removed when we are ready to send the buffer.
**********************************************************************/
#define SEND_SCT_BUF     0x8000

/**********************************************************************
  The size of SCT debug data that is assinged to the the debug SCT port
  firmware.
**********************************************************************/
#define SCT_PORT_DEBUG_DATA_SIZE (256 * 1024)


/**********************************************************************
  The buffer size to allocate to send each font table chunk.
**********************************************************************/
#define XFR_SCT_FONT_TABLE_SIZE  (256 * 1024)

/**********************************************************************
  Maximum number of glyphs that can be saves in a font table.
**********************************************************************/
#define MAX_FONT_TABLE_ENTRIES 65536

/*********************************************************************
  Stretch Board Types
**********************************************************************/
#define BOARD_VRC6016      0x18A20002
#define BOARD_VRC6008      0X18A20003
#define BOARD_VRC6004      0X18A20004
#define BOARD_VCC6416      0X18A20005
#define BOARD_VRC6416      0X18A20006
#define BOARD_VRC6416_ba   0X18A20007
#define BOARD_410_10040    0X18A20008
#define BOARD_410_10041    0X18A20009
#define BOARD_VRC6016C     0X18A2000A
#define BOARD_HD           0X18A2000B
#define BOARD_VRC6016_ba   0X18A2000C
#define BOARD_VDC6004      0X18A2000F
#define BOARD_VRC6404HD    0X18A20010
#define BOARD_VRC6004HD    0X18A20011
#define BOARD_VRC6016E     0X18A20014
#define BOARD_VRC7016L     0X18A20020
#define BOARD_VDC7002L     0X18A20021

typedef struct _ssdk_font_t {
  unsigned char *code;       // The list of glyphs codes.
  int nglyph;                // Total number of glyphs in the font table.
  FILE *fp;                  // A pointer to open font table file.
  sx_bool is_send_font_as_needed;
  sx_int8 color_y;           // The y color component of the OSD character
  sx_int8 color_u;           // The u color component of the OSD character
  sx_int8 color_v;           // The v color component of the OSD character

} ssdk_font_t;
/**********************************************************************
    The hi word signature for channel handle created by the SDK
    NOTE: a channel handle consist of:
    bits 0  - 7 : Channel number
    bits 8  - 11: channel type
    bits 12 - 15: board index where the channel is on
    bits 16 - 31: The CHAN_HANDLE_SIGNATURE
***********************************************************************/
#define CHAN_HANDLE_SIGNITURE 0xBEEF
#define CH_GET_CHAN_NUM(handle)  (handle & 0xff)
#define CH_GET_CHAN_TYPE(handle) ((handle >> 8) & 0xf)
#define CH_GET_BOARD_ID(handle)  ((handle >> 12) & 0xf)
#define CH_IS_VALID_SIGNITURE(handle) (((handle >> 16) & 0x0000FFFF) == CHAN_HANDLE_SIGNITURE)

/************************************************************************
  These macros return different driver version parts given the
  32 bit driver version value.

    SDVR_MAJOR_DRV_VERSION - The driver major version number. A change in this field
    indicates major changes to functionality.

    SDVR_MINOR_DRV_VERSION - The driver minor version number. A change in this field
    indicates minor changes to functionality.

    SDVR_REVISION_DRV_VERSION - The driver revision version number. A change in this
    field indicates significant bug fixes that was introduced in the minor
    change functionality.

    SDVR_BUILD_DRV_VERSION - The driver build version number. A change in this field
    indicates only bug fixes that do not change functionality.
************************************************************************/
#define SSDK_MAJOR_DRV_VERSION(dvr_version) (0x000000FF & dvr_version)
#define SSDK_MINOR_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 8))
#define SSDK_REVISION_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 16) )
#define SSDK_BUILD_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 24) )

/* The maximum file path */
#define MAX_FILE_PATH   256


/***********************************************************************
    The maximum number different raw video stream resolution allowed per
    video channels
************************************************************************/
#define MAX_RAW_VIDEO_STREAM 2

/***********************************************************************
   The maximum size of each SDK buffer.
***********************************************************************/
#define _SSDK_MAX_ENC_BUF   22
#define _SSDK_MAX_RAW_BUF   5
#define _SSDK_MAX_DEC_BUF   5

/***********************************************************************
   The maximum number of SMO supported by each board.
***********************************************************************/
#define MAX_NUM_SMO         8
#ifdef PPC
#define SSDK_SMO_SEND_BUF_COUNT 1
#else
#define SSDK_SMO_SEND_BUF_COUNT 3
#endif


/***********************************************************************
   The maximum number of EMO supported by each board.
***********************************************************************/
#define MAX_NUM_EMO             8
#define SSDK_EMO_SCT_BUF_COUNT  10

/***********************************************************************
   The maximum number of audio-out buffers to be allocated if in case
   of sending raw audio to the audio-out port.

   NOTE: The size of each buffer is the maximum size of sct buffer for
   this board.
***********************************************************************/
#define MAX_NUM_AUDIO_OUT        4
#define SSDK_AOUT_SEND_BUF_COUNT 3

/***********************************************************************
   The size of Motion Values Queue. This is the default in case
   the DVR Application did not specify one.
***********************************************************************/
#define _SSDK_MOTION_VALUE_QUEUE_SIZE 3

/* The last legal message class ID that we put a lock on it */
#define LAST_MSG_CLASS_ID 16

/*********************************************************************
   Different channel type combination to be used for channel validation
**********************************************************************/
#define SSDK_CHAN_TYPE_ENC_DEC            -1
#define SSDK_CHAN_TYPE_ANY                -2
#define SSDK_CHAN_TYPE_ANY_VENC           -3
#define SSDK_CHAN_TYPE_ANY_VENC_VDEC      -4

#ifdef WIN32
#define SSDK_MUTEX CRITICAL_SECTION
#else
#define SSDK_MUTEX  pthread_mutex_t
#endif

/***********************************************************************
 The callback function definition used for display. This is an internal
 callback function which provides hi-level HMO display. The parameters
 to this callback function are identical to what SCT is returning
***********************************************************************/
typedef void (*sdvr_ui_callback)(sdvr_chan_handle_t handle,
                                 dvr_data_header_t *yuvHdr,
                                 void * context);

typedef struct _ssdvr_size{
    sx_uint16 width;
    sx_uint16 height;
} ssdvr_size_t;

/***********************************************************************
    A data structure holding the BDF file format font table information.
************************************************************************/
typedef struct {
  double startfont;
  char font[256];
  int size;
  int chars;
  int encoding;
  int bbw, bbh, bbxoff0x, bbyoff0y;
  int fbbx, fbby, xoff, yoff;
  int dwx0, dwy0;
} ssdk_bdf_t;

typedef struct {
    __sdvr_frame_type_e last_frame_type; // The frame type received
    sx_uint32   last_seq_number;    // Frame sequence number. This field is valid only
                                    // for RAW and encoded video frames. Every channel and
                                    // stream combination will have independent sequence numbering. The raw
                                    // and encoded video streams from the same channel will have independent
                                    // sequence numbering.
    sx_uint32   last_frame_number;  // Number of frames seen on this channel so far.
                                    // This field is valid only for raw and encoded video frames. The
                                    // frame number and sequence number will be identical when the
                                    // stream is being run at full frame rate.
    sx_uint32   frame_drop_count;   // Number of frames dropped detected by the
                                    // firmware on the current stream.

} ssdk_frame_seq_info_t;

/***********************************************************************
    Debugging - A structure to hold various frame count for a channel
************************************************************************/
typedef struct {
    struct {
        unsigned int y_u_vBufferRcvd;   // raw video frame recieved in
                                        // _ssdk_av_frame_callback_func() or
                                        // _ssdk_av_frame_decoder_callback_func()
        unsigned int y_u_vBuffersFreed; // calls to ssdk_sct_buffer_free()
        unsigned int eFrameAppRcvd;     // calls to sdvr_get_stream_buffer()
        unsigned int eFrameAppFreed;    // calls to sdvr_release_av_buffer()
        unsigned int eFrameRcvd;        // calls to _ssdk_queue_frame()
        unsigned int eFrameFreed;       // calls to ssdk_release_frame()
        unsigned int totalFramesRcvd;   // all frames received in _ssdk_av_frame_callback_func() or
                                        // _ssdk_av_frame_decoder_callback_func
        unsigned int totalFramesFreed;  // calls to ssdk_sct_buffer_free()
        unsigned int dAllocAVBuf;       // calls to sdvr_alloc_av_buffer_wait()
                                        // for decoder channels.
        unsigned int dSendAVBuf;        // calls to sdvr_send_av_frame()
                                        // for decoder channels.
        unsigned int dReleaseAVBuf;     // calls to sdvr_release_av_buffer()
                                        // for decoder channels.
        unsigned int skippedBuf;        // The number of buffer that were skipped due to any errors.
    } count;

    ssdk_frame_seq_info_t enc_seq[2];      // sequence encoded video frame information.
    ssdk_frame_seq_info_t raw_seq[2];      // sequence raw video frame inforamtion.

} ssdk_dbg_frame_t;

/************************************************************************
    Temporary memory buffer to hold avmux file information
************************************************************************/
#define AVMUX_BUF_SIGNATURE     0xFADEBABE

/***********************************************************************
    Data structure defining each item in the A/V queue.
************************************************************************/
typedef struct {
    int         next;          // The index to the next item in the queue
    sx_bool     avail;         // A boolean to indicate if this item is
                               // available to add a new frame.
    void        *data;         // The frame buffer
} ssdk_frame_queue_item_t;

/***********************************************************************
    Data structure defining the A/V queue. This data structure is shared
    between both encoded and raw A/V frames that are sent to the host.

************************************************************************/
typedef struct {
    int     start_item;          // The first item of the queue;
    int     top_item;            // The top of the queue where we should
                                 // place the new frame.
    sx_uint8 size;               // The size of the queue buffer.

    sx_uint8 num_recvd_frames;   // The number of frames recieved
                                 // and not freed yet.

    ssdk_frame_queue_item_t *frame_queue;  // A list of A/V frames that was
                                 // just received from the PCI driver.
    __sdvr_frame_type_e queue_type; // The frame type saved in this queue.
} ssdk_frame_queue_t;

typedef enum _ssdk_recording_state_e
{
    SSDK_FILE_STATE_NONE = 0,
    SSDK_FILE_STATE_READ,
    SSDK_FILE_STATE_WRITE,
    SSDK_FILE_STATE_WRITE_REWIND,
    SSDK_FILE_STATE_STOP,
    SSDK_FILE_STATE_STOP_PENDING,
    SSDK_FILE_STATE_START
} ssdk_file_state_e;


typedef struct
{
    sx_uint32 signature;        // AVMUX_BUF_SIGNATURE
    ssdk_file_state_e file_state; // The current read/write state

                                // The encoded video/audio track info
                                // in the file.
    sdvr_file_avtrack_info_t    avtrack_info;
    sx_uint64              vframe_count;    // The number of video key frames
                                      // that were written to the file.
    union {
        struct {
            sx_int32 vtrack;    // Video track handle within mp4 file
                                // if zero the video track is igonred
            sx_int32 atrack;    // Audio track handle within mp4 file
                                // if zero the audio track is igonred
            mp4_handle fh;      // The file handle to the mp4 file.
            mp4_handle new_fh;  // The new file handle to be used when
                                // we are in transission of saving the GOP
                                // from the previous file and moving to the
                                // the file.
            sx_int64 time_offset; // The correction time between audio
                                // and video frame that is needed to be synched.
        }mp4;
        struct {
            sx_int32 vtrack;    // Video track handle within smf file
                                // if zero the video track is igonred
            sx_int32 atrack;    // Audio track handle within smf file
                                // if zero the audio track is igonred
            FILE *fh;        // The file handle to the elementary file.
            FILE *new_fh;    // The new file handle to be used when
                            // we are in transission of saving the GOP
                            // from the previous file and moving to the
                            // the file.

            sx_bool             bSVCExt;
                                // If set to true, means the STD entry
                                // is not save in the .smf file. This
                                // is usually the case for h.264-svc that
                                // we get this information once SEI is recieved.
            sx_bool             bSTDEntrySaved;
            st_svcext_config_t  cfg;
            st_svcext_t         extractor_handle;
            st_raw_parser_t     parserContext;
        }smf;
        struct {
            FILE *fh;        // The file handle to the elementary file.
            FILE *new_fh;    // The new file handle to be used when
                            // we are in transission of saving the GOP
                            // from the previous file and moving to the
                            // the file.
            svc_fileext_context_t svc;
        } elementary;
        struct {
            sx_int32 vtrack;    // Video track handle within smf file
                                // if zero the video track is igonred
            sx_int32 atrack;    // Audio track handle within smf file
                                // if zero the audio track is igonred
            FILE *fh;           // The file handle to the elementary file.
            FILE *new_fh;       // The new file handle to be used when
                                // we are in transission of saving the GOP
                                // from the previous file and moving to the
                                // the file.

            sx_bool             bSVCExt;
                                // If set to true, means the STD entry
                                // is not save in the .smf file. This
                                // is usually the case for h.264-svc that
                                // we get this information once SEI is recieved.
            sx_bool             bSTDEntrySaved;
            st_svcext_config_t  cfg;
            st_svcext_t         extractor_handle;
            st_demux_t         *demuxContext;
        }demux;
    }u;
//    sx_int64 dts;
    sdvr_rec_specification_t rec_spec; // The recording specification such as
                                // File name, type, ...
                                // The mutex to handle serializing of
                                // reading and writing to the file.
#ifdef WIN32
    CRITICAL_SECTION cs_av_file;
#else
    pthread_mutex_t  cs_av_file;
#endif
} ssdk_avmux_context_t;

// removeit typedef mp4File_t ssdk_avmux_context_t ;

/***********************************************************************
    Data structure defining the SMO grid assocated to an encoder or
    decoder channel.
************************************************************************/
typedef struct _ssdk_smo_grid_t {
    sx_uint16           top_left_mb_x;
    sx_uint16           top_left_mb_y;
    sx_uint8            dwell_time;
    sx_bool             video_enable;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint8            layer;
    sx_uint8            resize_location;
    sx_uint8            instance_id;
} ssdk_smo_grid_t;

/***********************************************************************
    Data structure defining the sub-encoder definitions assocated to
    an encoder channel.

************************************************************************/

typedef struct {
                                // The type of video encoding for this
                                // sub-encoder
    dvr_vc_format_e     venc_type;

                                // A flag to indicate if the encoded A/V is
                                // streaming. If this flag is set to zero
                                // Any encoded A/V video frame on this channel
                                // will be discarded.
    sx_bool             is_enc_av_streaming;

                                // The current resolution decimation of
                                // this encoding.
    int                 res_decimation;

                                // The current state of skip frame for this channel
                                // by default this field is enabled by the firmware.
    sx_bool             is_skip_frame_enabled;


                                // A flag to make sure a recorded file
                                // always starts with SPS
    sx_bool             has_sps_vol;

    sx_int8             pe_num; // The location of secondary encoder. -1 means
                                // use the default PE location.
    sx_uint32           frame_sequence; // This counter is used for authentication
                                // tool. It is set to zero when the camera is
                                // created and gets incremented by one only
                                // when we receive any of the h.264 picture frames.

                                // The authentication handle. It is created for
                                // each sub-encoder when the channel is created.
                                // This is needed for authentication routine.
    venc264_authentication_handle_t *auth_handle;

                                // File information related to recording or
                                // parsing av file.
    sdvr_file_handle_t          avmux_file_handle;

    // The queue to hold the video frame in the SDK until the application
    // request to get it.
    // The memory for each of these buffers are allocated at
    // the time the channel is setup and then will be release when the
    // channel is destroyed.
    // NOTE: Each item in frame queue of enc_av_frame_queue,
    //       should be type casted to *sdvr_av_buffer_t.
    //       enc_av_frame_queue is and array of frame queue for
    //       each supported encoder on this channel.
    ssdk_frame_queue_t  enc_av_frame_queue;

    sx_bool                      has_enc_params; // A flag to indicate if we
                                                 // got the encoder parameters from the firmware.
    sdvr_video_enc_chan_params_t enc_params; // The current video encoder
                                //parameters settings.
} ssdk_sub_enc_info_t;

/***********************************************************************
    Data structure defining the raw video stream definitions assocated to
    an encoder/decoder channel.

************************************************************************/

typedef struct _ssdk_raw_video_stream_t {
                                // A flag to indicate if the raw video is
                                // streaming. If this flag is set to zero
                                // Any raw video frame on this channel
                                // will be discarded.
    sx_bool             is_enabled;
                                // The current resolution decimation of
                                // this raw video stream.
    sdvr_video_res_decimation_e  res_decimation;
                                // The frame rate of this raw video stream
    sx_uint32           frame_rate;
} ssdk_raw_video_stream_t;

/***********************************************************************
    Data structure defining the encoded PCI port channel
************************************************************************/

typedef struct {
                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

    sct_channel_t  pci_chan_handle; // This handle is needed for all the
                                    // communications with the SCT calls

    // A list of buffers to queue up different frame types that are sent
    // to the SDK. The memory for each of these buffers are allocated at
    // the time the channel is setup and then will be release when the
    // channel is destroyed.
    // NOTE: Each item in frame queue of enc_av_frame_queue,
    //       and raw_audio_frame_queue should be type
    //       casted to *sdvr_av_buffer_t.
    //       enc_av_frame_queue is and array of frame queue for
    //       each supported encoder on this channel.
    ssdk_frame_queue_t  yuv_frame_queue[MAX_RAW_VIDEO_STREAM];

    ssdk_frame_queue_t  raw_audio_frame_queue;

    ssdk_frame_queue_t  enc_audio_frame_queue;

    ssdk_frame_queue_t  motion_values_frame_queue;
                                // The data response data as a result of
                                // generic raw data request.
    ssdk_frame_queue_t  cmd_response_frame_queue;

                                // This is a queue of only one item to hold
                                // the last snapshot.
    ssdk_frame_queue_t  snapshot_frame_queue;


                                // Total number of frames are currently
                                // stored in all the frame queue. If
                                // this number exceed 9 we have to start
                                // dropping frames.
    sx_uint32           num_frames_in_queues;

                                // Current SMO grid setting as well as
                                // the audio/video enable state.
                                //  This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes.
    ssdk_smo_grid_t     smo_grid_info[MAX_NUM_SMO][DVR_SMO_MAX_NUM_INSTANCES];

    ssdk_raw_video_stream_t rawv_stream[MAX_RAW_VIDEO_STREAM];

                                // It indicates the number of audio out ports
                                // that are used to recieve audio from this
                                // audio channel.
    sx_uint8            audio_out_enable_count;
                                // It indicates which audio-out port is enabled
                                // for this channel.
    sx_bool             audio_out_enable_map[MAX_NUM_AUDIO_OUT];

                                // A flag to indicate whether to stream raw or
                                // encoded audio. Once the raw audio is enabled
                                // for a channel, the encoded audio will be paused.
    sx_bool             is_raw_audio_streaming;

                                // information specific to a sub-encoder on this
                                // encoder channel. An array of sub_enc_info will
                                // be created equal to the max number encoder supported
                                // for each camera when the channel is created.
                                // This memory for this array is release when the channel
                                // is destroyed.
    ssdk_sub_enc_info_t *sub_enc_info;

                                // A flag to indicate if the channels is in
                                // middle of raw A/V, encoded streaming or
                                // SMO is enabled.
    sx_bool             is_channel_running;


                                // The type of audio encoder;
    dvr_ac_format_e     aenc_type;

                                // Structure to hold different frames
                                // information for debugging purposes.
    ssdk_dbg_frame_t dbg_frame;

                                // Arrays holding which regions are defined
                                // Any array item with value of 1 means
                                // the region is defined.
    sx_bool         md_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool         bd_defined_regions[SDVR_MAX_BD_REGIONS];
    sx_bool         pr_defined_regions[SDVR_MAX_PR_REGIONS];

    struct {
        sdvr_video_std_e std; // The video standard detected of deteced
                              // on the video input where this camera is
                              // connected. This would be the default video
                              // standard on the board if no source is
                              // connected to this video input.

                              // The width and number of lines of the video
                              // frame associated with this standard
        sx_uint16 width;
        sx_uint16 lines;


    } vstd;
    sx_uint64       user_data;  // A 64 bit user specified data associated
                                // to this channel.

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be shown.
    sx_bool         osd_defined_map[SDVR_MAX_OSD];
    sx_uint32       fosd_defined_map[DVR_FOSD_MODULE_ID_NUM];

    sdvr_ui_callback    ui_video_cb_func;
    void *              ui_video_cb_context[MAX_RAW_VIDEO_STREAM];
    sdvr_ui_callback    ui_audio_cb_func;
    void *              ui_audio_cb_context;

                                // The mutex to handle changing of
                                // current state of recoridng.
#ifdef WIN32
    CRITICAL_SECTION cs_chan_rec;
#else
    pthread_mutex_t  cs_chan_rec;
#endif

                                // The SDK caches the vpp actions
                                // so that the DVR Application can enable or
                                // disable each one individually without
                                // affecting other actions. By default
                                // all the actions are enabled.
    sx_uint32       vpp_action_flags;

} ssdk_recv_chan_info_t;

/***********************************************************************
    Data structure defining the decoded PCI port channel
************************************************************************/

typedef struct _ssdk_send_chan_info_t {
                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

    sct_channel_t  pci_chan_handle; // This handle is needed for all the
                                    // message communications with the SCT
                                    // calls
    sct_channel_t pci_send_chan_handle; // The handle to be used to send
                                    // A/V frames using SCT calls.

    sdvr_video_std_e  vStd;         // The decoder's video standard. To indicate
                                    // whether we are decoding HD or SD.

    // A list of buffers to queue up different frame types that are sent
    // to the SDK. The memory for each of these buffers are allocated at
    // the time the channel is setup and then will be released when the
    // channel is destroyed.
    // NOTE: Each item in frame queue of raw_audio_frame_queue should
    //       be type casted to *sdvr_av_buffer_t
    ssdk_frame_queue_t  yuv_frame_queue;

    ssdk_frame_queue_t  raw_audio_frame_queue;
                                // Total number of frames are currently
                                // stored in all the frame queue. If
                                // this number exceed 9 we have to start
                                // dropping frames.

                                // The data response data as a result of
                                // generic raw data request.
    ssdk_frame_queue_t  cmd_response_frame_queue;

    sx_uint32           num_frames_in_queues;

                                // A flag to indicate if the decoding of A/V
                                // is enabled. This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes
    sx_bool             is_dec_av_enable;


                                // Current SMO grid setting as well as
                                // the audio/video enable state.
                                //  This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes.
    ssdk_smo_grid_t     smo_grid_info[MAX_NUM_SMO][DVR_SMO_MAX_NUM_INSTANCES];

                                // Raw video attributes and state
    ssdk_raw_video_stream_t rawv_stream;

    struct {
                                // A flag to indicate if any decoder parameters
                                // have changed since the last time the firmware
                                // was updated.
        sx_bool                 is_modified;
                                // The video size supported by this decoder
                                // The decoder can not decode frame that
                                // are not exactly this size. These are
                                // set at the time the decoder is created.
        sx_uint16               vsize_width;
        sx_uint16               vsize_lines;
                                // Setting this field to a nonzero value will
                                // disable the decoder's internal pipeline.
        sx_uint8                    disable_pipeline;
        sdvr_h264_decoder_mode_e    decoder_mode;
    } properties;


                                // Number of buffers availabe to send to FW
                                // for decoding. If this number is zero
                                // we should not attempt to send any new
                                // decode buffer.
    sx_uint32           avail_decode_buf_count;

                                // A flag to indicate if the channels is in
                                // middle of raw A/V, encoded streaming or
                                // SMO is enabled.
    sx_bool             is_channel_running;

                                // The type of video decoder;
    dvr_vc_format_e     vdecoder_type;

                                // The type of audio decoder;
    dvr_ac_format_e     adecoder_type;

                                // Indicates if this audio data to be decoded should
                                // be stereo or mono. It defaults to stereo.
    sdvr_enc_audio_mode_e enc_audio_mode;

                                // Structure to hold different frames
                                // information for debugging purposes.
    ssdk_dbg_frame_t dbg_frame;

                                // Arrays holding which regions are defined
                                // Any array item with value of 1 means
                                // the region is defined.
    sx_bool             md_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool             bd_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool             pr_defined_regions[SDVR_MAX_MD_REGIONS];

    sx_uint64           user_data; // A 64 bit user specified data associated
                                   // to this channel.

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be showned.
    sx_bool             osd_defined_map[SDVR_MAX_OSD];
    sx_uint32           fosd_defined_map[DVR_FOSD_MODULE_ID_NUM];

    sdvr_ui_callback    ui_video_cb_func;
    void *              ui_video_cb_context;
    sdvr_ui_callback    ui_audio_cb_func;
    void *              ui_audio_cb_context;

    sx_uint32           sct_send_payload_size;  // Size of each buffer allocated
                                // on this channel in order to send
                                // encoded or raw A/V frames.
                                // NOTE: This size does not include the added
                                //       PCI header the 5K padding. This is
                                //       the maximum size that can be used for
                                //       the payload

    sx_uint32           send_seq_no;  // The frame sequence number of each
                                // buffer we send to the Firmware.

                                // The mutex to handle changing of
                                // current state of recoridng.
#ifdef WIN32
    CRITICAL_SECTION    cs_chan_rec;
#else
    pthread_mutex_t     cs_chan_rec;
#endif
} ssdk_send_chan_info_t;

typedef struct _ssdk_smo_info_t {
    sct_channel_t pci_send_chan_handle; // The handle to be used to send
                                        // A/V frames using SCT calls.

                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

                                // Number of buffers availabe to send to FW.
                                // If this number is zero, we should not attempt
                                //  to send any new frame buffer.
    sx_uint32     avail_send_buf_count;

                                // A bitmap of supported video format on this
                                // SMO, see sdvr_rawv_formats_e for details.
                                // The raw video format that is being sent to
                                // the output monitor. (i.e. 4-2-0)
    sx_uint16     supported_video_format;
    sx_uint8      video_format; // Current overlay video format started on this
                                // SMO port.
    sx_uint16     width;        // Width of the SMO screen.
    sx_uint16     height;       // Height of the SMO screen.

    sx_uint32   sct_buf_size;   // The sct send buffer size allocated for this SMO.
                                // NOTE: This size maybe smaller than allowed larger
                                // frame for the selected video format, if that size
                                // is less than the SCT buffer size for this board.

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be showned.
    sx_bool       osd_defined_map[SDVR_MAX_OSD];
    sx_uint32     fosd_defined_map[DVR_FOSD_MODULE_ID_NUM];

    sx_uint32  send_voverlay_buf_count; // calls to sdvr_send_video_overlay()
    sx_uint32  get_voverlay_buf_count; // calls to sdvr_get_video_overlay_buffer()
    sx_uint32  release_voverlay_buf_count; // calls to sdvr_release_av_buffer()

                                // mutext to handle synchronization of SMO
                                // overlay buffer handling.
    int         voveraly_start_count; // The number of time sdvr_start_video_overlay()
                                // was called for this SMO port. This is needed
                                // to know when to actually stop the video
                                // overlay. Since both the overlay and HMO mirror
                                // usage the same mechanism.
#ifdef WIN32
    CRITICAL_SECTION cs_smo_buf;
#else
    pthread_mutex_t  cs_smo_buf;
#endif
    //only used in ppc for host dma
    sx_uint32       dma_addr;
    void    *       dma_src;
    sx_uint32       dma_size;
    sx_uint8        dma_id;
    sx_uint8        dma_init;
}ssdk_smo_info_t;


typedef struct _ssdk_emo_info_t {
    sx_uint16     width;        // Maximum width of an EMO buffer
    sx_uint16     height;       // Maximum height of an EMO buffer

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be showned.
    sx_bool       osd_defined_map[SDVR_MAX_OSD];
    sx_uint32     fosd_defined_map[DVR_FOSD_MODULE_ID_NUM];

}ssdk_emo_info_t;

typedef struct _ssdk_aout_info_t {
    sct_channel_t pci_send_chan_handle; // The handle to be used to send
                                        // A/V frames using SCT calls.

                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

                                // Number of buffers availabe to send to FW.
                                // If this number is zero, we should not attempt
                                //  to send any new frame buffer.
    sx_uint32     avail_send_buf_count;
}ssdk_aout_info_t;

/***********************************************************************
    Data structure defining the encoded/decoded PCI port channels
    description per DVR PCI board
************************************************************************/

typedef struct {
    const char *name;           // The PCI board name that that is returned
                                // from scr_get_board_name

    sct_board_t handle;         // The board handle needed to make SCT calls
                                // for general commands

    sx_uint32       board_type;
    sdvr_chip_rev_e chip_revision;

    sct_channel_t pci_debug_handle; // The SCT channel handle to receive
                                    // extra debug messages from the firmware.

    sct_channel_t pci_send_data_handle; // The SCT channel handle to be used
                                // to send large buffers to the firmware.
                                // This mostly used for sending of different
                                // ROIs map. NOTE: This handle is SCT channel
                                // gets openned on the first call to send any of the
                                // ROIs map and keeps open until the board is closed.
    sx_uint32 pci_send_data_buf_size; // The maximum size of each sct send data buffer.
                                // This is usually the size of a D1 video frame for
                                // the current video standard.

    ml_versions boot_loader_ver; // The boot-loader version.
                                 // This gets sets everytime we load a firmware.

    dvr_ver_info_t version;      // The firmware, bsp, boot-loader version. This gets
                                 // set every time we connect to a board.

    sdvr_video_std_e default_sd_video_std; // The default SD video standard for this board
                                // in the event no video input is connected for all
                                // the SD ports
    sdvr_video_std_e default_hd_video_std; // The default HD video standard for this board
                                // in the event no video input is connected for all
                                // the HD ports

    sdvr_enc_audio_mode_e enc_audio_mode; // This field specifies whether the SDK should encode
                                // or decode audio data as 8 bit mono or 16 bit stereo.

    dvr_audio_data_type_e audio_data_type; // Indicates whether the firmware sends
                                //raw or encoded audio data when the host requests audio. 
                                // This is the audio type that will be send whether 
                                // the host request for encoded or raw audio. The SDK may or 
                                // may not need to encode or decode the received audio data.

    sx_uint32 board_index;      // The current board index. The address of
                                // this field is used to send to the driver
                                // to save board_index for callback functions.

    sx_uint16    max_num_video_in;   // The maximum number of video-in channels
                                    // on the board. (i.e camera connections)

    sx_uint16    max_num_host_encoders; // The maximum number of host video encoding 
                                // channels supported by the DVR board.

    sx_uint16    max_num_decoders; // The maximum number of decoding channel
                                // supported by the DVR board.

    sx_uint16    max_num_relays; // The maximum number of relays supported by
                                 // the DVR board.

    sx_uint16    max_num_sensors;// The maximum number of sensors supported by
                                 // the DVR board.

    sx_int32 slot_number;       // The PCI slot number that this boars is on

                                // A list containing different PCI buffer
                                // information for each camera channel.
                                // NOTE: Associated to each camera we
                                //       receive A/V frame for the primary
                                //       or secondary encoded channel or just
                                //       raw A/V frame.
                                // The index to this array corresponds to
                                // the items in the ssdk_recv_chan_type_e
    ssdk_recv_chan_info_t *recv_channels;

                                // A list containing all the decode channels
                                // informat.
                                // NOTE: Associated to each decoder we can
                                //       recieve raw A/V frames. The
                                //       encode A/V frames to be sent for
                                //       decoding are not being buffered.
    ssdk_send_chan_info_t *send_channels;

    sx_bool            isDriverValidate; // A flag to indicate if the Driver and
                                // SCT is validated for this board.

                                // A flag to indicate whether on board connect
                                // we must issue sct_reset.
    sx_bool            is_reset_required;

                                // A flag to indicate the last call to
                                // recieve a reply, timed-out. We assume
                                // the firmware is crashed. All the future
                                // calls to _ssdk_message_recv will be ingnored
    sx_bool         is_fw_dead;
    sx_uint8        dump_frame_info_flag;

    sx_bool         is_h264_SCE;// If true, the SDK performs  start code
                                // emulation for all the h.264 video frames;
                                // except SPS and PPS frames.

                                // The key to be used in H.264 frame authentication.
    sx_uint8        auth_key[DVR_AUTH_KEY_LEN];
    sx_uint32       auth_key_len; // Number of bytes used in auth_key[]

    sx_uint32       max_num_smos; // Number of SMOs supported by this board
    ssdk_smo_info_t  smo_info[MAX_NUM_SMO];
    sx_uint16       max_num_encoders;

    sx_uint32 max_sct_buf_size;       // The size of largest SCT buffer on this board.
    sx_uint32 max_sct_recv_buf_count; // The total number of buffers for all the
                                      // channels supported by the SCT for this board.
                                      // NOTE: The buffers inside the driver is shared
                                      //       amoung all channels. So if the total
                                      //       number unfree buffers in all the queues
                                      //       for this board, exceed the max_sct_buf_count - 1,
                                      //       then we should start dropping frames.

    sx_uint32 num_frames_in_all_queues; // Total number of frames in all the frame queues
                                      // for this board.

    sdvr_audio_rate_e audio_rate; // The sampling audio rate used on this board
                                  // default is DVR_AUDIO_RATE_8KHZ

    sx_uint32       last_err_code; // The last error code sent by DVR_SIGNAL_LAST_ERROR

    sx_uint32       last_sct_msg;  // The last SCT message request that was sent to the firmware.
                                   // This is needed to know the response received is as a
                                   // result of SET or GET message.

    sx_uint32       max_num_audio_out; // Number of audio-out ports supported by this board
    ssdk_aout_info_t aout_info[MAX_NUM_AUDIO_OUT];
    sx_uint8        serial_number[SDVR_BOARD_SERIAL_LENGTH + 1]; // The board serial number

    sx_uint32       max_num_emos; // Number of EMOs supported by this board
    ssdk_emo_info_t  emo_info[MAX_NUM_EMO];

    sx_uint32       max_num_video_enc_per_camera; // Maximum number of video stream
                                  // for each camera on this board. Each video stream
                                  // can have different CODEC.
    sx_uint32       max_num_rvideo_stream_per_camera; // Maximum number of raw vidoe
                                  // supported per camera

    sx_bool         omit_blank_frames; // Whether to drop frames after video is lost

    dvr_vstd_custom_t custom_vstd; // Size/refresh parameters for custom video
} ssdk_dvr_board_info_t;

/***********************************************************************
    Data structure defining the PCI driver and boards information.
************************************************************************/
typedef struct {

    sx_uint8 board_num;        // Number of boards installed


                                // An array holding all information about
                                // channels on every  PCIe boards that are
                                // connected.
    ssdk_dvr_board_info_t *boards;

                                // Mutex variable to handle critical section.
                                // Note: There is only one mutex to handle
                                //       frame data for all the boards.
#ifdef WIN32
    CRITICAL_SECTION cs_board_info;
#else
    pthread_mutex_t  cs_board_info;
#endif

                                // Mutex variable to handle synchronzing of
                                // sending messages to the firmware.
SSDK_MUTEX cs_msg[LAST_MSG_CLASS_ID];


                                // Mutex variable to handle critical section.
                                // Note: There is only one mutex to handle
                                //       decode buffer management count for
                                //       all the deocoders.
#ifdef WIN32
    CRITICAL_SECTION    cs_decode_buf_count;
#else
    pthread_mutex_t     cs_decode_buf_count;
#endif
                                // Mutex to synchronize set/checking of any
                                // of the system callbacks.
    SSDK_MUTEX          cs_callback_func;

                                // The callback function provided by the DVR
                                // application to be called everytime
                                // a new A/V frame is arrived.
                                // This is kept only for backward compatibility
    void (*av_frame_callback)(sdvr_chan_handle_t handle,
                              sdvr_frame_type_e frame_type,
                              sx_bool primary_frame);

    void (*stream_callback)(sdvr_chan_handle_t handle,
                              sdvr_frame_type_e frame_type,
                              sx_uint32 stream_id);

                                // The callback function provided by the DVR
                                // application to be called every time any
                                // sensor is activated on this board
    void (*sensor_callback) (sx_uint32 board_index,
                             sx_uint32 sensor_map);
                                // The callback function provided by the DVR
                                // application to be called every time any
                                // sensor is activated on this board
    void (*sensor_64_callback) (sx_uint32 board_index,
                                sx_uint64 sensor_map);

                                // The callback function provided by the DVR
                                // application to be called every time an
                                // alarm is detected on this board
    void (*video_alarm_callback) (sdvr_chan_handle_t handle,
                                  sdvr_video_alarm_e alarm_type,
                                  sx_uint32 data);


                                // The callback function provided by the DVR
                                // application to be called every time
                                // sct confirms the frame sent.
    void (*conf_callback) (sdvr_chan_handle_t handle);

                                // The callback function provided by the DVR
                                // application to be called every an asynch
                                // message is sent on this board
    void (*signals_callback) (sx_uint32 board_index,
                             sdvr_signal_info_t * signal_info);

                                // The callback function provided by the DVR
                                // application to be called every time
                                // we need to display a tracing message.
    void (*disp_debug_callback) (char *message);

                                // The full path to the log file.
    char log_file_name[MAX_FILE_PATH + 1];

                                // The last custom font table defined.
                                // Current there can only be one font
                                // table defined. The range of this
                                // field is 8 - 16. 0 means not defined yet.
    sx_uint8 custom_font_table_id;
    sx_uint8 current_font_table_id; // The ID of currently used font table.
                                // This could be different than custom_font_table_id,
                                // if custom font table was created but the user switched
                                // to one of the pre-loaded font table in the firmware.


    unsigned char *temp_venc_buf; // The buffer which holds video encoded
                                  // SCE.
    sx_uint32 temp_venc_buf_size; // The size of temp venc buffer.

                                // This flag only is kept within the SDK
                                // and never sent to the board.
    sx_bool         is_auth_key_enabled;

                                // This flag indicates whether SDK to convert
                                // The raw video stream from 4-2-0 to 4-2-2
    sx_uint8        yuv_format;

    ssdk_font_t *font_table_handle;

    sx_uint16       highest_video_frame_rate; // The highest frame rate of
                                // the video standard associated with all
                                // the connected video inputs. This is mostly
                                // needed for the Mode 2 of UI-SDK updating.

                                // Mutex variable to handle writing of the
                                // message into the logfile. This is needed
                                // to synchronize message writing from
                                // different threads. (e,g a/v callback)
    SSDK_MUTEX    cs_trace_log;

                                // HMO mirror related state variables
    sdvr_chan_handle_t    hmo_mirror_handle;
    sx_int32              hmo_mirror_width;
    sx_int32              hmo_mirror_height;
    sx_uint8              hmo_mirror_id;

} ssdk_dvr_boards_info_t;

#ifdef STRETCH_SDK_C
#define SDK_EXTERN

/***********************************************************************
    The user specified SDK settings
************************************************************************/
sdvr_sdk_params_t m_sdkParams = {
#if defined(ARM)
	2,             // enc_buf_num
    0,              // raw_buf_num
    2,              // dec_buf_num - maximum #buffer allowed by sct
#else
    10,             // enc_buf_num
    2,              // raw_buf_num
    5,              // dec_buf_num - maximum #buffer allowed by sct
#endif
#if defined(PPC) || defined(ARM)
    262144 - (DVR_DATA_HDR_LEN + (1024 * 5)), // dec_buf_size = 256 x 1024
#else
    (256 * 1024),   // dec_buf_size:
                    // size of a SD encoded buffer     = 256K
                    // size of a HD 720 encoded buffer = 3 * 256K
                    // size of a HD 1080 encoded buffer = 6 * 256K

#endif
#if defined(ARM)
    20,             // timeout -- Needed for ARM, otherwise channel creation
                    // failes priodically
#else
    10,             // timeout
#endif
    0,              // debug_flag
    NULL,           // debug_file_name;
};

static sdvr_err_e _ssdk_validate_drv_sct_version(sx_uint32 board_index);
static void _ssdk_encode_audio_g711(sdvr_av_buffer_t *frame_buf, sdvr_enc_audio_mode_e audio_mode);
static void _ssdk_do_board_cleanup(sx_uint32 board_index);
static void _ssdk_decoder_send_conf_callback_func( void * context, void * buffer, int size);
static void _ssdk_av_frame_callback_func( void * context, void * buffer, int size);
static void _ssdk_av_frame_decoder_callback_func( void * context, void * buffer, int size);
static void _ssdk_message_callback_func( void * context, sx_int32  msg_class, void * buffer);
static sx_uint32 _ssdk_remove_yuv_padding( sdvr_chan_handle_t handle,
                              sx_uint16 padded_width,
                              sx_uint16 active_width,
                              sx_uint16 active_height,
                              sx_uint8 *dest_buf,
                              sx_uint8 *src_buf);
static void _ssdk_convert_YUV420_YUV422(dvr_data_header_t *yuvHdr);
static ssdk_frame_queue_t *_ssdk_get_frame_queue(sdvr_chan_handle_t handle,
                                                 __sdvr_frame_type_e frame_catagory,
                                                 sx_uint32 stream_id);
static sdvr_err_e _ssdk_queue_frame(sdvr_chan_handle_t handle,
                                    void * buffer,
                                    __sdvr_frame_type_e frame_catagory,
                                    sx_uint32 stream_id);
static sdvr_err_e _ssdk_message_send(sdvr_chan_handle_t handle, sx_uint8 b_index, sx_int32  msg_class, void * buffer, sx_int32 buf_size );
static sdvr_err_e _ssdk_message_recv( sdvr_chan_handle_t handle, sx_uint8 b_index,
      sx_int32 msg_class, void *buffer, sct_board_t *psrc_board, sx_int32 *pmsg_class );
static void  _ssdk_do_message_byte_swap(sx_uint32 board_index, sx_int32  msg_class, void * buffer);
static sx_bool _is_byte_swap = false;
sx_bool ssdk_is_Big_Endian = false;

sx_uint32 sdbg_totalFrame_count_rcvd;
sx_uint32 sdbg_totalFrame_count_freed;
#else
#if defined(__cplusplus)
   #define SDK_EXTERN              extern "C"
#else
   #define SDK_EXTERN              extern
#endif

SDK_EXTERN sdvr_sdk_params_t m_sdkParams;
SDK_EXTERN sx_bool ssdk_is_Big_Endian;

#endif


/***********************************************************************
    The following two macros allow you to either free an sct buffer that
    was added to one of the channel queues or skip it by freeing before
    it has ever been added to any of the sct buffers.
************************************************************************/
#define ssdk_sct_buffer_free(channel, buffer) ssdk_do_sct_buffer_free(channel, buffer, false)
#define ssdk_sct_buffer_skip(channel, buffer) ssdk_do_sct_buffer_free(channel, buffer, true)

/***********************************************************************
    The PCI DVR Board informations. Such as the number of boards, etc.
************************************************************************/
SDK_EXTERN ssdk_dvr_boards_info_t _ssdvr_boards_info;

/***********************************************************************
    A flag indicating if we are in the message or A/V frame callback.
    If this flag is set, no debugging information will be send to display.
    This is needed to fix a deadlock situation between SCT and DVR app.
************************************************************************/
SDK_EXTERN sx_bool ssdvr_in_callback;
/************************************************************************
                      Internal SDK Function Prototypes
*************************************************************************/

SDK_EXTERN ssdk_dbg_frame_t *ssdk_get_dbg_frame(sdvr_chan_handle_t handle);
SDK_EXTERN sx_bool ssdk_is_byte_swap();

SDK_EXTERN void ssdk_adjust_hdr_little_to_big_endian(void *buf_hdr);
SDK_EXTERN sx_int16 ssdk_endian_byte_swap_int16(sx_int16 nValue);
SDK_EXTERN sx_int32 ssdk_endian_byte_swap_int32(sx_int32 nLongNumber);
SDK_EXTERN void ssdk_change_endianness(sx_uint64 *uLongNumber, size_t size) ;



SDK_EXTERN sdvr_err_e ssdk_validate_board_ndx(sx_uint32 board_index);
SDK_EXTERN sdvr_err_e ssdk_validate_chan_handle(sdvr_chan_handle_t handle, int chk_chan_type);
SDK_EXTERN sdvr_err_e ssdk_validate_board_chan_type(sx_uint8 board_index, sx_uint8 chan_num, sx_uint8 chan_type);
SDK_EXTERN int ssdk_get_max_chan_num(int board_index, sx_uint8 chan_type);

SDK_EXTERN sdvr_err_e ssdk_validate_enc_sub_chan(sdvr_chan_handle_t handle,
                                              sdvr_sub_encoders_e sub_chan_enc);
SDK_EXTERN dvr_job_type_e ssdk_chan_to_job_type(sdvr_chan_type_e chan_type);
SDK_EXTERN dvr_vc_format_e ssdk_to_fw_video_frmt(sdvr_venc_e video_format);
SDK_EXTERN dvr_ac_format_e ssdk_to_fw_audio_frmt(sdvr_aenc_e audio_format);
SDK_EXTERN void ssdk_register_sdk_chan_callback(sct_channel_t pci_handle,sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_set_board_config(sx_uint32 board_index,
                                 sdvr_board_settings_t *board_settings);

SDK_EXTERN sdvr_err_e ssdvr_get_supported_vstd(sx_uint32 board_index,
                                               sx_uint16  *video_stds);
SDK_EXTERN sdvr_err_e ssdk_drv_open_rcv(sdvr_chan_handle_t handle,
                                        sdvr_chan_def_t *chan_def,
                                        sdvr_chan_buf_def_t *buf_def,
                                        dvr_job_t *cmd_strct);
SDK_EXTERN sdvr_err_e ssdk_drv_close_rcv(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_drv_open_decode_xfr(sdvr_chan_handle_t handle,
                                               sdvr_chan_def_t *chan_def,
                                               sdvr_chan_buf_def_t *buf_def,
                                               dvr_job_t *cmd_strct);
SDK_EXTERN sdvr_err_e ssdk_drv_close_decode_xfr(sdvr_chan_handle_t handle);

SDK_EXTERN void ssdk_reset_enc_chan_info(sdvr_chan_handle_t handle);

SDK_EXTERN sdvr_err_e ssdk_dequeue_frame( sdvr_chan_handle_t handle, void **buffer,
                                __sdvr_frame_type_e frame_catagory,
                                sx_uint32 stream_id);
SDK_EXTERN sdvr_err_e ssdk_release_frame(void *frame_buf);

SDK_EXTERN void _ssdk_mutex_msg_lock(int msgId);
SDK_EXTERN void _ssdk_mutex_msg_unlock(int msgId);
SDK_EXTERN void _ssdk_mutex_trace_log_lock();
SDK_EXTERN void _ssdk_mutex_trace_log_unlock();
SDK_EXTERN void _ssdk_mutex_lock();
SDK_EXTERN void _ssdk_mutex_unlock();
SDK_EXTERN void _ssdk_mutex_decode_buf_lock();
SDK_EXTERN void _ssdk_mutex_decode_buf_unlock();
SDK_EXTERN void _ssdk_mutex_chan_rec_lock(sdvr_chan_handle_t handle);
SDK_EXTERN void _ssdk_mutex_chan_rec_unlock(sdvr_chan_handle_t handle);
SDK_EXTERN void _ssdk_mutex_av_file_lock(SSDK_MUTEX cs_av_file);
SDK_EXTERN void _ssdk_mutex_av_file_unlock(SSDK_MUTEX cs_av_file);
SDK_EXTERN void _ssdk_mutex_callback_lock();
SDK_EXTERN void _ssdk_mutex_callback_unlock();

SDK_EXTERN void _ssdk_mutex_smo_overlaybuf_lock(sdvr_chan_handle_t handle);
SDK_EXTERN void _ssdk_mutex_smo_overlaybuf_unlock(sdvr_chan_handle_t handle);

SDK_EXTERN sdvr_display_debug_callback ssdvr_get_display_bebug_callback();
SDK_EXTERN sx_uint32 _ssdk_get_debug_flag();
SDK_EXTERN sdvr_chan_handle_t ssdk_create_chan_handle(sx_uint8 board_index,
                                                  sdvr_chan_type_e  chan_type,
                                                  sx_uint8          chan_num);
SDK_EXTERN sdvr_err_e ssdk_disable_channel(sdvr_chan_handle_t handle,
                                           sx_bool *was_contrl_running);
SDK_EXTERN sdvr_err_e ssdk_enable_channel(sdvr_chan_handle_t handle);

ssdk_raw_video_stream_t *ssdk_get_rawv_stream_properties(sdvr_chan_handle_t handle,
                                                         sx_uint8 stream_id);
void ssdk_enable_raw_av(sdvr_chan_handle_t handle, sx_uint8 stream_id, sx_bool enable);

SDK_EXTERN void ssdk_enable_enc_av(sdvr_chan_handle_t handle,
                                   sx_uint8 sub_chan_enc,
                                   sx_bool enable);
SDK_EXTERN sdvr_err_e ssdvr_enable_chan(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_message_chan_send(sdvr_chan_handle_t handle,
                       sx_int32  msg_class,
                       void * buffer,
                       sx_int32 buf_size );
SDK_EXTERN sdvr_err_e ssdk_message_board_send(sx_uint8 b_index,
                       sx_int32  msg_class,
                       void * buffer,
                       sx_int32 buf_size );
SDK_EXTERN sdvr_err_e ssdk_message_chan_recv(sdvr_chan_handle_t handle,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class );
SDK_EXTERN sdvr_err_e ssdk_message_board_recv(sx_uint8 b_index,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class );
SDK_EXTERN void ssdk_do_sct_buffer_free( sct_channel_t    channel, void * p, sx_bool is_skip_buffer );

SDK_EXTERN sx_bool ssdk_is_chan_streaming(sdvr_chan_handle_t handle);
SDK_EXTERN sx_bool ssdk_is_chan_encoding(sdvr_chan_handle_t handle);
SDK_EXTERN sx_bool ssdk_is_chan_previewing(sdvr_chan_handle_t handle);

SDK_EXTERN sx_bool ssdk_is_any_chan_streaming(sx_uint32 board_index);
SDK_EXTERN sx_bool ssdk_is_chan_running(sdvr_chan_handle_t handle);

SDK_EXTERN sdvr_err_e ssdk_get_chan_av_codec(sdvr_chan_handle_t handle,
                                        sdvr_sub_encoders_e   stream_id,
                                        sdvr_venc_e *video_codec,
                                        sdvr_aenc_e *audio_codec);

SDK_EXTERN sx_bool ssdk_is_min_fw_version(sx_uint8 board_index,  sx_uint8 majorChk, sx_uint8 minorChk,
                               sx_uint8 revisionChk, sx_uint8 buildChk);
SDK_EXTERN int sdvr_avail_decoder_buf_count(sdvr_chan_handle_t handle);

SDK_EXTERN ssdk_send_chan_info_t * ssdk_get_send_chan_strct(sx_uint8 board_index, sx_uint8 chan_num);
SDK_EXTERN ssdk_send_chan_info_t * ssdk_get_send_chan_strct_ex(sx_uint8 board_index, sx_uint8 chan_type, sx_uint8 chan_num);
SDK_EXTERN ssdk_recv_chan_info_t * ssdk_get_recv_chan_strct(sx_uint8 board_index, sx_uint8 chan_num);
SDK_EXTERN ssdk_recv_chan_info_t * ssdk_get_recv_chan_strct_ex(sx_uint8 board_index, sx_uint8 chan_type, sx_uint8 chan_num);
SDK_EXTERN unsigned int ssdk_get_venc_per_camera(sx_uint8 board_index);

SDK_EXTERN  int       ssdk_highest_system_frame_rate();

SDK_EXTERN sdvr_err_e ssdvr_write_Enc_VFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
SDK_EXTERN sdvr_err_e ssdvr_write_Enc_AFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);

SDK_EXTERN sdvr_err_e ssdvr_get_auth_key(sx_uint32 board_index,
                                         sx_uint8 *auth_key,
                                         sx_uint8 *auth_key_len);
SDK_EXTERN sdvr_err_e ssdvr_read_auth_key(sx_uint32 board_index);

SDK_EXTERN sdvr_err_e ssdk_validate_has_buf(sdvr_chan_handle_t handle,
                                            __sdvr_frame_type_e frame_catagory,
                                            sx_uint32 stream_id);
SDK_EXTERN void ssdk_decode_audio_g711(sdvr_av_buffer_t *frame_buf, sdvr_enc_audio_mode_e audio_mode);
SDK_EXTERN sdvr_err_e sdisplay_do_send_vpp_action( sdvr_chan_handle_t handle,
                                         sx_bool enable,
                                         sx_uint8 threshold,
                                         sx_uint32 vpp_function);
SDK_EXTERN sdvr_err_e ssdvr_do_create_sct_send_chan(sx_uint32 board_index);
SDK_EXTERN sdvr_err_e ssdvr_do_create_chan(sdvr_chan_def_t *chan_def,
                            sdvr_chan_buf_def_t *buf_def,
                            sx_uint32            smo_port, 
                            sdvr_chan_handle_t *handle_ptr,
                            int func_name);
SDK_EXTERN sdvr_err_e ssdvr_do_destroy_chan(sdvr_chan_handle_t handle,
                                            int func_name);
SDK_EXTERN sdvr_err_e ssdvr_send_venc_params(sdvr_chan_handle_t handle,
                             sx_int32 sub_chan,
                             dvr_vc_format_e venc_type,
                             sdvr_video_enc_chan_params_t *video_enc_params,
                             int dbg_func_name);
SDK_EXTERN sdvr_err_e ssdvr_do_osd_text_show(sdvr_chan_handle_t handle,
                              sx_uint8 osd_id,
                              sx_bool show,
                              sx_uint8 smo_port);
SDK_EXTERN sdvr_err_e ssdvr_do_osd_text_config(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config,
                             sx_uint8 smo_port);

SDK_EXTERN sdvr_err_e ssdvr_st_raw_err_to_sdk(st_raw_err_e st_raw_err);
SDK_EXTERN sdvr_err_e ssdvr_st_demux_err_to_sdk(st_demux_err_e st_demux_err);
SDK_EXTERN sdvr_err_e ssdvr_svcext_err_to_sdk(st_svcext_error_e err);
SDK_EXTERN sdvr_err_e ssdvr_avmux_err_to_sdk(error_code_t err);

SDK_EXTERN sdvr_err_e ssdvr_update_font_table_as_needed( sx_uint16 *unistring, sx_uint8 unistring_len);

/****************************************************************************
  Internal API used only by ui display and other DLLs
****************************************************************************/
SDK_EXTERN sx_bool ssdk_set_ui_video_callback(sdvr_chan_handle_t handle, int raw_vstream_id, sdvr_ui_callback callback_func, void * context);
SDK_EXTERN void *ssdk_get_ui_video_context(sdvr_chan_handle_t handle, int raw_vstream_id);
SDK_EXTERN sx_bool ssdk_set_ui_audio_callback(sdvr_chan_handle_t handle, sdvr_ui_callback callback_func, void * context);
SDK_EXTERN void *ssdk_get_ui_audio_context(sdvr_chan_handle_t handle);
SDK_EXTERN void ssdk_free_ui_context(void *pdi);
SDK_EXTERN void *ssdk_malloc_ui_context(size_t size);
SDK_EXTERN int     ssdk_get_frame_rate(sx_uint8 board_index);
SDK_EXTERN sx_bool ssdk_is_hmo_mirror_enabled();
SDK_EXTERN sx_bool ssdk_get_hmo_mirror_size( int * pw, int* ph);
SDK_EXTERN sx_bool ssdk_send_hmo_mirror_buffer( sx_uint8 * y, sx_uint8 * u, sx_uint8 * v, int pitch );

#endif //STRETCH_SDK_H


