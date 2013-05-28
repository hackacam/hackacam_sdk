#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <map>
#include <sbl/sbl_logger.h>
#include <rtsp/rtsp.h>
#include "rtsp_sdk.h"
#include "streaming_app.h"

#ifdef NO_SDK

void sdk_setup(char * romfile_path, char* streams, int gop_size, int bitrate, int quality) {}

#else
#define SBL_MSG_SDK 32
#include "a2a_communications.h"
#include <sdk/sdvr_sdk.h>

int init_a2a(int init);

#ifdef SDK_DEBUG
static int enable_sdk_debug = true;
#else
static int enable_sdk_debug = false;
#endif

#define HD_VIDEO_STD      SDVR_VIDEO_STD_NONE
#define SD_VIDEO_STD      SDVR_VIDEO_STD_NONE
#define AUDIO_RATE        SDVR_AUDIO_RATE_NONE
#define FRAME_RATE        0
#define MAX_STREAM_COUNT  4

static sdvr_board_config_t   _boardConfig;    // The board configuration. (i,e number of
                                      // video in ports, sensors, relays, etc.
static sdvr_board_attrib_t   _boardAttrib;


static void fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    exit(1);
}

static void sdk_callback(sdvr_chan_handle_t handle, sdvr_frame_type_e frame_type, sx_uint32 stream_id) {
    sdvr_av_buffer_t* av_buffer;
    sdvr_err_e status = sdvr_get_stream_buffer(handle, frame_type, stream_id, &av_buffer);
    if (status != SDVR_ERR_NONE) {
        fatal("sdvr_get_stream_buffer failed err(%d)\n", status);
    }
    RTSP::EncoderType encoder_type = RTSP::UNKNOWN_ENCODER;
    switch (frame_type) {
        case SDVR_FRAME_H264_IDR:
        case SDVR_FRAME_H264_I:
        case SDVR_FRAME_H264_P:
        case SDVR_FRAME_H264_B:
        case SDVR_FRAME_H264_SPS:
        case SDVR_FRAME_H264_PPS:
            if (encoder_type == RTSP::UNKNOWN_ENCODER)
                encoder_type = RTSP::H264;
        case SDVR_FRAME_JPEG:
            if (encoder_type == RTSP::UNKNOWN_ENCODER)
                encoder_type = RTSP::MJPEG;
//      case SDVR_FRAME_MPEG4_I:
//      case SDVR_FRAME_MPEG4_P:
//      case SDVR_FRAME_MPEG4_VOL:
        {
            sx_uint8  chan_num   = sdvr_get_chan_num(handle);
            sx_uint8* frame      = av_buffer->payload;
            sx_uint32 frame_size = av_buffer->payload_size;

            sx_uint64  timestamp64;
            sdvr_get_buffer_timestamp(av_buffer, &timestamp64);

            /* convert from hardware 100 KHz clock to 90 KHz timestamp clock */
            sx_uint32 timestamp = timestamp64 * 9 / 10;

            sx_uint32 seq_number, frame_number, drop_count;
            sdvr_av_buf_sequence(av_buffer, &seq_number, &frame_number, &drop_count);
            SBL_MSG(SBL_MSG_SDK, "Seq=%d, Frame_num=%d, drop_count=%d", seq_number, frame_number, drop_count);

            rtsp_send_frame(chan_num, stream_id, frame, frame_size, timestamp, encoder_type);
        }
            break;
        default: SBL_WARN("Received unknown frame type %d, ignoring", frame_type);
            break;
    }
    sdvr_release_av_buffer(av_buffer);
}

static void __set_sdk_params(int bEnableDebug)
{
    sdvr_sdk_params_t sdk_params;
    sdvr_get_sdk_params(&sdk_params);

    if (bEnableDebug)
    {
        sdk_params.debug_flag = (DEBUG_FLAG_ALL | DEBUG_FLAG_DEBUGGING_ON) - DEBUG_FLAG_OUTPUT_TO_SCREEN -
                                 DEBUG_FLAG_MANAGE_FRAMES;
        sdk_params.debug_file_name = (char *)"./sdvr_sdk.log";
    }
    sdvr_set_sdk_params(&sdk_params);
}

// If we are not forcing the frame rate, get the maximum frame rate supported
// for the given video standard
static sx_uint8 __get_frame_rate(sdvr_video_std_e video_std)
{
    sx_uint8  frame_rate = FRAME_RATE;
    if (FRAME_RATE == 0 )
        frame_rate =  sutil_vstd_frame_rate(video_std);
    frame_rate = sdvr_set_frame_rate(SDVR_FRS_METHOD_NONE,frame_rate);

    return frame_rate;
}

unsigned int sdk_get_num_supported_encoders()
{
    return _boardConfig.num_encoders_supported;
}

unsigned int sdk_get_num_video_encoders_per_camera()
{
    return _boardConfig.num_video_encoders_per_camera;
}

void signals_callback(sx_uint32 board_index, sdvr_signal_info_t *signal_info)
{
    switch (signal_info->sig_type)
    {
    case SDVR_SIGNAL_SDK_MISSED_FRAME:
    {
        sx_uint32 expected_seq_frame  = signal_info->data & 0xFFFF0000 >> 16;
        sx_uint32 recvd_seq_frame = signal_info->data & 0xFFFF0000;
        sx_uint32 expected_frame_type  = signal_info->extra_data & 0xFFFF0000 >> 16;
        sx_uint32 recvd_frame_type = signal_info->extra_data & 0xFFFF0000;
        printf("Signal [Missed Frame]: Channel %d - expected Frame Type/Seq#: %u/%u recvd Frame Type/Seq#: %u/%u\n",
                signal_info->chan_num,
                expected_frame_type, expected_seq_frame,
                recvd_frame_type, recvd_seq_frame);
        break;
    }
    default:
        break;

    }
}
typedef std::map<int, RTSP::Application::StreamDesc> map_stream_desc_t;
static map_stream_desc_t stream_desc;

void a2a_set_stream_desc(int chan_num,int enc_stream_id,RTSP::EncoderType encoder_type)
	{
	int image_width;
	int image_height;

	image_width = sutil_vres_width(SD_VIDEO_STD, SDVR_VIDEO_RES_DECIMATION_EQUAL);
	image_height = sutil_vres_height(SD_VIDEO_STD, SDVR_VIDEO_RES_DECIMATION_EQUAL, true);

	int index = chan_num * 10 + enc_stream_id;
	stream_desc[index].encoder_type = encoder_type;
	stream_desc[index].bitrate      = 8000;
	stream_desc[index].quality      = 50;
	stream_desc[index].width        = image_width >> enc_stream_id;
	stream_desc[index].height       = image_height >> enc_stream_id;
	}

static void __setup_video_encoder(sdvr_chan_handle_t handle, unsigned int enc_stream_id, sdvr_video_std_e video_std,
                                int gop_size, int bitrate, char encoder, int quality) {
    sdvr_video_enc_chan_params_t enc_param;
    sdvr_venc_e                  video_codec = SDVR_VIDEO_ENC_H264;
    sx_uint8                     chan_num = sdvr_get_chan_num(handle);

    memset(&enc_param, 0, sizeof(sdvr_video_enc_chan_params_t));
    bitrate >>= 2 * enc_stream_id;
    RTSP::EncoderType encoder_type = RTSP::UNKNOWN_ENCODER;
    switch (encoder) {
        case 'h':
            video_codec                            = SDVR_VIDEO_ENC_H264;
            encoder_type                           = RTSP::H264;
            enc_param.encoder.h264.enc_mode        = SDVR_H264_ENC_PROFILE_BASE;
            enc_param.encoder.h264.bitrate_control = SDVR_BITRATE_CONTROL_CBR;
            enc_param.encoder.h264.avg_bitrate     = bitrate;
            enc_param.encoder.h264.gop             = gop_size;
            break;
        case 'j':
            video_codec                            = SDVR_VIDEO_ENC_JPEG;
            encoder_type                           = RTSP::MJPEG;
            enc_param.encoder.jpeg.quality         = quality;
            enc_param.encoder.jpeg.is_image_style  = 2;
            break;
        case 'x':
            return;
        default:
            fatal("Incorrect video encoder type %c\n", encoder);
    }

    enc_param.frame_rate = __get_frame_rate(video_std);
    enc_param.flush_buf  = false;

    static const sx_uint8 decimation[] = { SDVR_VIDEO_RES_DECIMATION_EQUAL,
                                           SDVR_VIDEO_RES_DECIMATION_FOURTH,
                                           SDVR_VIDEO_RES_DECIMATION_SIXTEENTH };
    if (enc_stream_id >= sizeof(decimation) / sizeof(sx_uint8)) {
        fatal("Incorrect enc_stream_id for decimation %d\n", enc_stream_id);
    }
    enc_param.res_decimation = decimation[enc_stream_id];

    sdvr_err_e retval = sdvr_set_chan_video_codec(handle, enc_stream_id, video_codec);
    if (retval != SDVR_ERR_NONE) {
        fatal("Error setting encoder type (%s)\n", sdvr_get_error_text(retval));
    }

    retval = sdvr_set_video_encoder_channel_params(handle, enc_stream_id, &enc_param);
    if (retval != SDVR_ERR_NONE) {
        fatal("Error setting channel params (%s)\n", sdvr_get_error_text(retval));
    }

    int image_width;
    int image_height;

    image_width = sutil_vres_width(video_std, SDVR_VIDEO_RES_DECIMATION_EQUAL);
    image_height = sutil_vres_height(video_std, SDVR_VIDEO_RES_DECIMATION_EQUAL, true);

    int index = chan_num * 10 + enc_stream_id;
    stream_desc[index].encoder_type = encoder_type;
    stream_desc[index].bitrate      = bitrate;
    stream_desc[index].quality      = quality;
    stream_desc[index].width        = image_width >> enc_stream_id;
    stream_desc[index].height       = image_height >> enc_stream_id;


    retval = sdvr_enable_encoder(handle, enc_stream_id, true);
    if (retval != SDVR_ERR_NONE) {
        fatal("Error enabling encoder (%s)\n", sdvr_get_error_text(retval));
    }
}

static sdvr_err_e __readConfiguration(sx_uint32 boardIndex)
{
    sdvr_err_e     sdkErr;

    memset(&_boardConfig, 0, sizeof(_boardConfig));
    memset(&_boardAttrib, 0, sizeof(_boardAttrib));

    sdkErr = sdvr_get_board_config (boardIndex, &_boardConfig);

    if (sdkErr == SDVR_ERR_NONE)
    {
        sdkErr = sdvr_get_board_attributes (boardIndex, &_boardAttrib);
    }

    return sdkErr;
}

static void __setBoardVideoStd(sdvr_board_settings_t *boardSetting)
{
    boardSetting->hd_video_std = HD_VIDEO_STD;
    boardSetting->video_std    = SD_VIDEO_STD;

    /*
     * Determine the standard of both SD and HD, based on the specified HD
     * video standard if given by removing the supported video standard that
     * does not match with the requested HD standard.
     */
    if (boardSetting->hd_video_std != SDVR_VIDEO_STD_NONE)
    {
        sx_uint32 videoStd = SDVR_VIDEO_STD_NTSC_MASK;

        if (boardSetting->hd_video_std & SDVR_VIDEO_STD_PAL_MASK)
            videoStd = SDVR_VIDEO_STD_PAL_MASK;

        _boardAttrib.supported_video_stds &= videoStd;
    }

    if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_SD_MASK)
    {
        if (boardSetting->video_std == SDVR_VIDEO_STD_NONE ||
        !(_boardAttrib.supported_video_stds & boardSetting->video_std))
        {
            if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_NTSC_MASK)
            {
                _boardAttrib.supported_video_stds &= SDVR_VIDEO_STD_NTSC_MASK;
                boardSetting->video_std = SDVR_VIDEO_STD_4CIF_NTSC;
            }
            else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_PAL_MASK)
            {
                _boardAttrib.supported_video_stds &= SDVR_VIDEO_STD_PAL_MASK;
                boardSetting->video_std = SDVR_VIDEO_STD_4CIF_PAL;
            }
        }
    }
    else // There is no SD video standard support
    {
        boardSetting->video_std = SDVR_VIDEO_STD_NONE;
    }

    if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_HD_MASK)
    {
        if (boardSetting->hd_video_std == SDVR_VIDEO_STD_NONE ||
        !(_boardAttrib.supported_video_stds & boardSetting->hd_video_std))
        {
            if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_NTSC_MASK)
            {
                if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080P30)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080P30;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_720P60)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_720P60;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080P60)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080P60;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080I60)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080I60;
            }
            else
            {
                if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080P25)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080P25;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_720P50)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_720P50;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080P50)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080P50;
                else if (_boardAttrib.supported_video_stds & SDVR_VIDEO_STD_1080I50)
                    boardSetting->hd_video_std = SDVR_VIDEO_STD_1080I50;
            }
        }
    }
    else // There is no HD video support.
    {
        boardSetting->hd_video_std  = SDVR_VIDEO_STD_NONE;
    }

}

// Init the SDK, load firmware, create channel(s) and enable streaming.
void sdk_setup(char * romfile_path, char* streams, int gop_size, int bitrate, int quality) {
    if (application.pe_id() == 1) {
        RTSP::A2aCommunications::create();
        _boardConfig.num_video_encoders_per_camera = MAX_STREAM_COUNT;
        return;
    }
    if (romfile_path == NULL || romfile_path[0] == '\0') {
        fatal("Missing rom file path\n");
    }
    int stream_count = strlen(streams);


    sdvr_chan_handle_t           handle;
    sdvr_chan_def_t              chan_def;
    sdvr_chan_buf_def_t          chan_buf ;

    sdvr_err_e            retval               = SDVR_ERR_NONE;
    sdvr_video_std_e      video_std;
    sdvr_board_settings_t board_settings;
    sx_uint32             board_index = 0;

    memset(&board_settings, 0, sizeof(sdvr_board_settings_t));

    retval = sdvr_sdk_init();
    if ( retval != SDVR_ERR_NONE ) {
        fatal( "SDK init failed err(%d) : %s\n", retval, sdvr_get_error_text(retval) );
    }

    sdvr_set_signals_callback(signals_callback);

    // enable SDK debugging.
    __set_sdk_params(enable_sdk_debug);

    retval = sdvr_upgrade_firmware(board_index, romfile_path);
    if (retval != SDVR_ERR_NONE) {
        fatal( "Load FW failed err(%d)[file=%s] : %s\n", retval, sdvr_get_error_text(retval), romfile_path );
    }


    retval = __readConfiguration(board_index);

    if (retval != SDVR_ERR_NONE) {
        fatal( "Failed to read the board configuration. Err(%d) : %s\n", retval, sdvr_get_error_text(retval) );
    }
    if (stream_count < 1 || (sx_uint32)stream_count > sdk_get_num_video_encoders_per_camera()) {
        fatal("Number of streams must be between 1 and %d. Request stream count = %d\n",
                sdk_get_num_video_encoders_per_camera(),
                stream_count);
    }
    __setBoardVideoStd(&board_settings);
    board_settings.audio_rate   = AUDIO_RATE;
    board_settings.is_h264_SCE  = false;

    retval = sdvr_board_connect_ex(board_index, &board_settings);
    if (retval != SDVR_ERR_NONE) {
        fatal( "Board connect failed err(%d) : %s\n", retval, sdvr_get_error_text(retval) );
    }

    /*
     *  To determine the each camera width and height we need to know
     *  the currently selected video standard. If the board supports both HD
     *  and SD video standard we are going to first favor the HD and then SD
     *  video standards.
     */
    if (board_settings.hd_video_std != SDVR_VIDEO_STD_NONE)
        video_std = board_settings.hd_video_std;
    else
        video_std = board_settings.video_std;

    memset(&chan_def, 0, sizeof(chan_def));

    chan_def.board_index              = 0;
    chan_def.chan_num                 = 0;
    chan_def.chan_type                = SDVR_CHAN_TYPE_ENCODER;
    chan_def.video_format_primary     = SDVR_VIDEO_ENC_NONE;
    chan_def.video_format_secondary   = SDVR_VIDEO_ENC_NONE;
    chan_def.audio_format             = SDVR_AUDIO_ENC_NONE;
    chan_def.raw_video_stream_count   = 1;
    chan_def.set_video_encoders_count = 4;
    chan_def.video_encoders_count     = 4;

    memset(&chan_buf, 0, sizeof(sdvr_chan_buf_def_t));

    chan_buf.max_buf_count               = 0;
    chan_buf.cmd_response_buf_count      = 0;
    chan_buf.u1.encoder.video_buf_count  = 9;
    chan_buf.u1.encoder.audio_buf_count  = 0;
    chan_buf.u1.encoder.raw_vbuf_count   = 0;
    chan_buf.u1.encoder.raw_abuf_count   = 0;
    chan_buf.u1.encoder.motion_buf_count = 0;

    /* register callback before enable */
    sdvr_set_stream_callback(sdk_callback);

    for (chan_def.chan_num = 0; chan_def.chan_num < sdk_get_num_supported_encoders(); chan_def.chan_num++ )
        {
        retval = sdvr_create_chan_ex(&chan_def, &chan_buf, &handle);
        if (retval != SDVR_ERR_NONE) {
            fatal("Create channel error (%s)\n", sdvr_get_error_text(retval));
        }

        for (int n = 0; n < stream_count; n++)
            __setup_video_encoder(handle, n, video_std, gop_size, bitrate, streams[n], quality);
    }
}

int Application::get_stream_id(unsigned int channel_num, unsigned int stream_num)
{
    map_stream_desc_t::iterator it;

    int stream_id =  10 * channel_num + stream_num;
    it = stream_desc.find(stream_id);
    if (it == stream_desc.end())
    {
        SBL_ERROR("Invalid channel number %d or stream number %d", channel_num,
                stream_num);

        stream_id = -1;
    }
    return stream_id;
}
int Application::get_stream_id(const char* name)
{
    unsigned int id;
    if (sscanf(name, "%d", &id) == 0)
        return -1;
    return get_stream_id(id / 10, id % 10);
}

int Application::describe(int stream_id, RTSP::Application::StreamDesc& stream_info)
{
    // if on VRM, it must be H264
    if (application.pe_id() == 1) {
        stream_info.encoder_type = RTSP::H264;
        stream_info.bitrate = 8000;
        return 0;
    }
    map_stream_desc_t::iterator it;
    it = stream_desc.find(stream_id);
    if (it == stream_desc.end())
        return -1;
    memcpy(&stream_info, &stream_desc[stream_id], sizeof stream_info);
    return 0;
}

#if 0
#define CHAN_HANDLE_SIGNITURE 0xBEEF
#define DVR_JOB_NUM(board_id, job_type, job_id) \
    (((board_id & 0xf) << 12) | ((job_type & 0xf) << 8) | (job_id & 0xff))

int sdk_get_bitrate(int stream_num, int stream_id) {
    sdvr_chan_handle_t           handle = (DVR_JOB_NUM(0, 0, stream_num) & 0x00ffff) | (CHAN_HANDLE_SIGNITURE << 16);
    sdvr_video_enc_chan_params_t enc_param;
    if(1 == init_a2a(0)) {
        return 2000;    // VRM default
    }
    sdvr_err_e retval = sdvr_get_video_encoder_channel_params(handle, stream_id, &enc_param);
    if (retval != SDVR_ERR_NONE) {
        fatal("Error enabling encoder (%s)\n", sdvr_get_error_text(retval));
    }
    SBL_MSG(RTSP::SBL_MSG_SDK, "Stream bitrate is %d", enc_param.encoder.h264.avg_bitrate);
    return enc_param.encoder.h264.avg_bitrate;
}
#endif

#endif
