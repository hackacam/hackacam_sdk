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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>

#include <sbl/sbl_logger.h>
#include <sbl/sbl_net.h>
#include <sbl/sbl_map.h>

#include "sdk_manager.h"
#include "cgi_server.h"
#include "cgi_flash.h"
#include <rtsp/rtsp.h>
#include "rtsp_server_iface.h"


/* wrapper around SDK calls. 
   Acquires lock, calls function and releases lock. This is to make sure that all sdk calls are serialized.
   If function returns an error, stuffs the error in the exception code and throws.
*/
#define SDK_CALL(sdk_func) \
    do { \
        _sdk_lock.lock();             \
        sdvr_err_e err = sdk_func;    \
        _sdk_lock.unlock();           \
        if (err != SDVR_ERR_NONE)     \
            throw SBL::Exception(err, #sdk_func, __FILE__, __LINE__, "failed with error %s", sdvr_get_error_text(err)); \
    } while (0)

namespace CGI {

// wrapper around SDK calls in callback thread, we don't throw there.
bool SDKManager::sdk_error(sdvr_err_e err) {
    if (err == SDVR_ERR_NONE)
        return false;
    SBL_ERROR("Error in SDK call: %s", sdvr_get_error_text(err)); 
    return true;
} 


MVSender&   SDKManager::mv_sender() const { return _server->mv_sender(); }

/////////////////////////////////////////////////////////////////////////////////////////////////
//              C A L L    B A C K       F U N C T I O N
////////////////////////////////////////////////////////////////////////////////////////////////

void stream_callback(sdvr_chan_handle_t handle,
                     sdvr_frame_type_e frame_type,
                     sx_uint32 stream_id)
{
    CGI::Server* cgi_server = application.cgi_server();
    if (!cgi_server) {
        SBL_MSG(MSG::FRAME, "application.cgi_server()is null, callback returns");
        return;
    }
    SDKManager* sdk_manager = cgi_server->sdk_manager();

    if (sdk_manager == NULL) {
        SBL_ERROR("got null SDK manager in callback");
        return;
    }
    if (sdk_manager->callback_blocked())
        return; 
    if (frame_type == SDVR_FRAME_JPEG_SNAPSHOT || frame_type == SDVR_FRAME_CMD_RESPONSE) {
        sdk_manager->get_frame_from_callback(frame_type);
    } else {
       sdk_manager->send_frame(handle, frame_type, stream_id);
    }
}

SDKManager::Options::Options() :
    debug_flags(1),
    enableH264Auth(false),
    firmware_name("s7100ipcam_dvrfw.rom"),
    home_folder("/usr/local/stretch"),
    hd_video_std(SDVR_VIDEO_STD_1080P30),
    max_video_stream_per_camera(4),
    rtsp_port_num(554),
    sct_timeout(5),
    sdk_log_filename("cgi_sdk.log"),
    sd_video_std(SDVR_VIDEO_STD_NONE),
    use_fosd(true)
{
    memset(&chan_buf_counts, 0, sizeof(chan_buf_counts));
    chan_buf_counts.max_buf_count = 0;
    chan_buf_counts.cmd_response_buf_count = 1;
    chan_buf_counts.u1.encoder.video_buf_count = 10;

    // no encoded audio or raw video/audio support
    chan_buf_counts.u1.encoder.audio_buf_count = 0;
    chan_buf_counts.u1.encoder.raw_vbuf_count  = 0;
    chan_buf_counts.u1.encoder.raw_abuf_count  = 0;
    chan_buf_counts.u1.encoder.motion_buf_count = 5;
    chan_buf_counts.u1.encoder.data_port_buf_count = 0; // use default
}

//////////////////////////////////////////////////////////////////////////
//                         Class SDKManager Implementation
//////////////////////////////////////////////////////////////////////////

// Class constructors
SDKManager::SDKManager(Server *server, const Options& options) : _initialized(false), _board_index(0),
        _camera_handle(INVALID_CHAN_HANDLE), _options(options), _server(server), _callback_buffer(NULL),
        _drop_count(0), _frame_count(0), _block_callback(false), _sensor_rate(0)  {}

bool SDKManager::init() {
    try {
        SBL_INFO("Initializing SDK with sensor rate %d", _sensor_rate);
        sdvr_sdk_params_t sdk_params;
        string strDebugPath = _options.home_folder + "/log/" +_options.sdk_log_filename;

        // First must set the SDK parameter so that we can debug
        // in sdvr_sdk_init()

        // uDebugFlags:
        //   0 - Disable logging
        //   1 - Enable debug logging for entire SDK except logging of A/V frame
        //   2-  Enable debug logging for entire SDK
        memset(&sdk_params, 0, sizeof(sdk_params));
        SDK_CALL(sdvr_get_sdk_params(&sdk_params));
        sdk_params.timeout = _options.sct_timeout;
        if (_options.debug_flags) {
            sdk_params.debug_file_name = (char *)strDebugPath.c_str();
            if (_options.debug_flags == 1)
                sdk_params.debug_flag = DEBUG_FLAG_ALL & ~(DEBUG_FLAG_OUTPUT_TO_SCREEN | DEBUG_FLAG_VIDEO_FRAME 
                                                        | DEBUG_FLAG_MANAGE_FRAMES | DEBUG_FLAG_AUDIO_FRAME);
            else
                sdk_params.debug_flag = DEBUG_FLAG_ALL & ~(DEBUG_FLAG_OUTPUT_TO_SCREEN);

            sdk_params.debug_flag |= DEBUG_FLAG_DEBUGGING_ON | DEBUG_FLAG_LIMIT_LOGFILE_SIZE;
            sdk_params.debug_flag &= ~(DEBUG_FLAG_WATCHDOG);
        }
        SDK_CALL(sdvr_set_sdk_params(&sdk_params));
        SDK_CALL(sdvr_sdk_init());
        sdvr_set_stream_callback(stream_callback);


        // Authentication must be enabled before any board
        // is connected.
        if (_options.enableH264Auth)
            sdvr_enable_auth_key(_options.enableH264Auth);

        // Make sure that the Stretch board can be detected.
        // NOTE: In IP-Camera if the driver is not loaded prior to running
        //       CGI-Server causes the failure to detect the Stretch board
        CGI_ERROR(sdvr_get_board_count() != 1, "Failed to detect Stretch board");
        board_upgrade_firmware();
        // Get the board attribute, configuration, version, etc
        // for the current board and the firmware.
        board_read_configuration();
        board_connect();
        camera_create();
        _initialized = true;
        return true;
    } catch (SBL::Exception& ex) {
        SBL_ERROR("Error: SDK initialization error:\n%s", ex.what());
        return false;
    }
}

void SDKManager::board_connect() {
    memset(&_board_settings, 0, sizeof(_board_settings));
    _board_settings.audio_rate   = SDVR_AUDIO_RATE_NONE;
    // NOTE: is_h264_SCE should be set to false to S7.
    _board_settings.is_h264_SCE = false;
    set_board_video_std();
    SDK_CALL(sdvr_board_connect_ex(_board_index, &_board_settings));
}

void SDKManager::board_read_configuration() {
    char buffer[100];
    SDK_CALL(sdvr_get_board_config (_board_index, &_board_config));
    SDK_CALL(sdvr_get_board_attributes (_board_index, &_board_attrib));
    SDK_CALL(sdvr_get_firmware_version(_board_index,&_version_info.sdvr_firmware_version));
    SDK_CALL(sdvr_get_pci_attrib(_board_index, &_version_info.pci_attrib));
    SBL_INFO("Firmware version numbers:\n"
                "    firmware:   %d.%d.%d.%d, built on %d/%d/%d\n"
                "    bootloader: %d.%d, bsp: %d %d",
            _version_info.sdvr_firmware_version.fw_major,
            _version_info.sdvr_firmware_version.fw_minor,
            _version_info.sdvr_firmware_version.fw_revision,
            _version_info.sdvr_firmware_version.fw_build,
            _version_info.sdvr_firmware_version.fw_build_year,
            _version_info.sdvr_firmware_version.fw_build_month,
            _version_info.sdvr_firmware_version.fw_build_day,
            _version_info.sdvr_firmware_version.bootloader_major,
            _version_info.sdvr_firmware_version.bootloader_minor,
            _version_info.sdvr_firmware_version.bsp_major,
            _version_info.sdvr_firmware_version.bsp_minor);

    snprintf(buffer, sizeof buffer, "%d.%d-%d.%d", _version_info.sdvr_firmware_version.bootloader_major ,
            _version_info.sdvr_firmware_version.bootloader_minor, _version_info.sdvr_firmware_version.bsp_major,
            _version_info.sdvr_firmware_version.bsp_minor);
    _version_info.boot_version = buffer;
    //todo: This is Stretch Firmware without the fw_build. It needs to be
    //      changed with the rootFS version number.
    snprintf(buffer, sizeof buffer, "%d.%d.%d",_version_info.sdvr_firmware_version.fw_major ,
            _version_info.sdvr_firmware_version.fw_minor,_version_info.sdvr_firmware_version.fw_revision);
            //_version_info.sdvr_firmware_version.fw_build);
    _version_info.firmware_version = buffer;
    //"2000-01-02T12:34:56"
    snprintf(buffer, sizeof buffer, "%d-%02d-%02dT12:00:00", _version_info.sdvr_firmware_version.fw_build_year,
            _version_info.sdvr_firmware_version.fw_build_month,
            _version_info.sdvr_firmware_version.fw_build_day);
    _version_info.firmware_released_date = buffer;

        // todo: set the correct model
        _version_info.model = "IP-Camera7";

}

void SDKManager::board_upgrade_firmware() {
    string firmware_name = _options.home_folder + "/bin/" + _options.firmware_name;
    SBL_INFO("Upgrading the firmware from %s", firmware_name.c_str());
    SDK_CALL(sdvr_upgrade_firmware(_board_index, const_cast<char*>(firmware_name.c_str())));
}

/*
 * This method creates one encoder channel and connects it to the first data
 * port which is the only data-port supported in IP-Camera. Then, it sets up
 * all four encoder streams for this encoder channel.
 */
void SDKManager::camera_create() {
    sdvr_chan_def_t chanDef;
    sx_uint16 osd_id;
    memset(&_stream_info[0], 0, sizeof(_stream_info));
    memset(&chanDef, 0 , sizeof(sdvr_chan_def_t));

    // There is no audio support for the CGI IP-Camera
    chanDef.audio_format           = SDVR_AUDIO_ENC_NONE;
    chanDef.video_format_primary   = SDVR_VIDEO_ENC_NONE;
    chanDef.video_format_secondary = SDVR_VIDEO_ENC_NONE;

    chanDef.board_index            = _board_index;
    chanDef.chan_num               = 0;
    chanDef.chan_type              = SDVR_CHAN_TYPE_ENCODER;
    // No raw video support
    chanDef.raw_video_stream_count = 0;
    chanDef.set_video_encoders_count = 1; // Indicate that we // going to set the number of video encoders
    chanDef.video_encoders_count   = Stream::COUNT;

    SDK_CALL(sdvr_create_chan_ex (&chanDef, &_options.chan_buf_counts, &_camera_handle));
    SDK_CALL(sdvr_get_chan_vstd(_camera_handle, &_video_std));
    // Set up the camera to have a OSD on the bottom right corner
    // showing date and time. No other text

    sdvr_osd_config_ex_t osd_config;
    memset(&osd_config, 0 , sizeof (osd_config));
    osd_config.translucent   = 255;
    osd_config.location_ctrl = SDVR_LOC_BOTTOM_RIGHT;
    osd_config.dts_format    = SDVR_OSD_DTS_DMY_24H;

    SDK_CALL(sdvr_osd_text_config_ex(_camera_handle, 0, &osd_config));

    // the following setup to use fosd
    sdvr_fosd_msg_cap_t  fosd_cap;

    /* Get fosd capabilities for this channel (job) */
    memset((void *)&fosd_cap, 0, sizeof(sdvr_fosd_msg_cap_t));
    SDK_CALL(sdvr_fosd_get_cap(_camera_handle, &fosd_cap));

    /* Set spec per channel. These numbers should be equal or less than cap */
    fosd_cap.num_enc        = 5;   // the number of streams is 4 but add one more for snapshot
    fosd_cap.num_enc_osd    = 1;
    fosd_cap.num_smo        = 0;
    fosd_cap.num_smo_osd    = 0;
    fosd_cap.max_width_cap  = 600;
    fosd_cap.max_height_cap = 36;

    SDK_CALL(sdvr_fosd_spec(_camera_handle, &fosd_cap));

    // configure fosd
    sdvr_fosd_config_t fosd_config;

    memset(&fosd_config, 0, sizeof(sdvr_fosd_config_t));
    /*  prepare fosd_config, including texts  */
    fosd_config.dts_format    =  SDVR_OSD_DTS_DMY_24H;

    /*OSD positions depends on text id */
    fosd_config.position_ctrl = SDVR_LOC_BOTTOM_RIGHT;
    fosd_config.max_width     = 600;
    fosd_config.max_height    = 36;

    /* Generate osd ID */
    /* config */
    for (int i = 0; i < Stream::COUNT; i++) {
        fosd_config.font_table_id = (i>=3)? 2 : i;  // there are only 3 font tables
        osd_id = SDVR_FOSD_OSD_ID(SDVR_FOSD_MODULE_ID_ENC, i, 0);
        SDK_CALL(sdvr_fosd_config(_camera_handle, osd_id, &fosd_config));
    }
    // To be used for snapshot
    fosd_config.font_table_id = SDVR_FOSD_FONT_TABLE_LARGE;  // snapshot always default to use table 0
    osd_id = SDVR_FOSD_OSD_ID(SDVR_FOSD_MODULE_ID_ENC, 4, 0); //snapshot is stream 4
    SDK_CALL(sdvr_fosd_config(_camera_handle, osd_id, &fosd_config));

}

/*
 * This method sets the encoder for the give stream if needed. The video
 * encoder will be set if the current stream has not video encoder or the
 * video encoder is changed.
 *
 * Remarks:
 *    Since the video encoder can not be change while it is enabled. The video
 *    encoder will be disabled first. As a result, the caller of this method
 *    should re-enable it if needed. There is for this decision is that sometimes
 *    the new encoder is requested to be enabled whereas other time it maybe
 *    disabled.
 */

void  SDKManager::change_encoder_if_needed(int stream_id, sdvr_venc_e encoder) {
    // Stream not initialized means it has no video CODEC
    // before we can continue any further, we must set the video
    // codec for this stream.
    SBL_INFO("Stream %d %s initialized, encoder is %d, new one %d", stream_id,
             is_stream_initialized(stream_id) ? "is" : "is not", _stream_info[stream_id].video_format, encoder);

    if (!is_stream_initialized(stream_id) || encoder != _stream_info[stream_id].video_format ) {
        if (is_stream_initialized(stream_id) && _stream_info[stream_id].is_enabled) {
            SBL_MSG(MSG::SDK, "Disabling encoder for stream %d", stream_id);
            SDK_CALL(sdvr_enable_encoder(_camera_handle, stream_id, false));
            _stream_info[stream_id].is_enabled = false;
        }
        SDK_CALL(sdvr_set_chan_video_codec(_camera_handle, stream_id, encoder));
        // Every time we change the video stream codec type,
        // we must reset all the parameters.
        memset(&_stream_info[stream_id].video_enc_params, 0,
                sizeof(sdvr_video_enc_chan_params_t));
        _stream_info[stream_id].video_format = encoder;
    }
}

/*
 * This method sets the default HD and SD video standard for the board.
 * If a HD or SD video standard is specified in the PSIA configuration file,
 * the video standard will be used as long as it is supported by the board,
 * otherwise, a supported HD and/or SD video standard will be used according to
 * a set priority.
 *
 * NOTE: If a video standard is specified, the firmware uses this video standard
 *       when creating the camera. This is different than what happens in the
 *       DVR board, which the video standard of the video-in port is used
 *       when creating the camera.
 *
 * Remarks:
 *
 *      Both video standard must of the same standard (i,e NTSC or PAL like)
 */
void SDKManager::set_board_video_std() {
    sx_uint32 video_standard = sensor_sample_rate_to_video_standard(_sensor_rate);

    _board_settings.hd_video_std = _options.hd_video_std; //SDVR_VIDEO_STD_NONE;
    _board_settings.video_std   =  _options.sd_video_std; //SDVR_VIDEO_STD_NONE;

    /* Override with custom mode if it is supported */
    sx_bool  use_custom_vstd = (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_CUSTOM);

    /*
     * Determine the standard of both SD and HD, based on the specified HD
     * video standard if given by removing the supported video standard that
     * does not match with the requested HD standard.
     */
    if (_board_settings.hd_video_std != SDVR_VIDEO_STD_NONE) {
        if (_board_settings.hd_video_std & SDVR_VIDEO_STD_PAL_MASK)
            video_standard = SDVR_VIDEO_STD_PAL_MASK;
        _board_attrib.supported_video_stds &= video_standard;
    }

    if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_SD_MASK) {
        if (_board_settings.video_std == SDVR_VIDEO_STD_NONE ||
        !(_board_attrib.supported_video_stds & _board_settings.video_std)) {
            if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_NTSC_MASK) {
                _board_attrib.supported_video_stds &= SDVR_VIDEO_STD_NTSC_MASK;
                _board_settings.video_std = SDVR_VIDEO_STD_4CIF_NTSC;
            } else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_PAL_MASK) {
                _board_attrib.supported_video_stds &= SDVR_VIDEO_STD_PAL_MASK;
                _board_settings.video_std = SDVR_VIDEO_STD_4CIF_PAL;
            }
        }
    }
    else { // There is no SD video standard support
        _board_settings.video_std = SDVR_VIDEO_STD_NONE;
    }

    if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_HD_MASK) {
        if (_board_settings.hd_video_std == SDVR_VIDEO_STD_NONE ||
        !(_board_attrib.supported_video_stds & _board_settings.hd_video_std)) {
            if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_NTSC_MASK) {
                if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080P30)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080P30;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_720P60)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_720P60;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080P60)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080P60;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080I60)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080I60;
            } else {
                if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080P25)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080P25;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_720P50)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_720P50;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080P50)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080P50;
                else if (_board_attrib.supported_video_stds & SDVR_VIDEO_STD_1080I50)
                    _board_settings.hd_video_std = SDVR_VIDEO_STD_1080I50;
            }
        }
    }
    else { // There is no HD video support.
        _board_settings.hd_video_std  = SDVR_VIDEO_STD_NONE;
    }

    /* Override with custom mode if it is supported */
    if (use_custom_vstd) {
        _board_settings.hd_video_std = SDVR_VIDEO_STD_CUSTOM;

        // Specify custom dimensions here for example's sake
        _board_settings.custom_vstd.w = 1024;
        _board_settings.custom_vstd.h = 512;
        _board_settings.custom_vstd.frame_rate = 30;

        SBL_INFO("Using custom _board_settings: hd_video_std 0x%x, custom_vstd (%d, %d, %d)\n", 
                 _board_settings.hd_video_std, 
                 _board_settings.custom_vstd.w, 
                 _board_settings.custom_vstd.h, 
                 _board_settings.custom_vstd.frame_rate);
    }
}

char* SDKManager::test_snapshot(const char* filename, unsigned int* size) {
    struct stat stat;
    int fd = open(filename, O_RDONLY);
    if (fd < 0 || fstat(fd, &stat) != 0)
        throw Exception(USER_ERROR, 0, __FILE__, __LINE__, "unable to open or stat file %s", filename).perror();
    char* buffer = new char[stat.st_size];
    *size = read(fd, buffer, stat.st_size);
    close(fd);
    _callback_buffer = reinterpret_cast<sdvr_av_buffer_t*>(buffer);
    SBL_MSG(MSG::SDK, "Got jpeg from %s, size is %d", filename, *size);
    return buffer;
};

const char* SDKManager::base_name(const char* path) {
    const char* slash = strrchr(path, '/');
    if (slash)
        return slash + 1;
    return path;
}

const char* SDKManager::_local_tar_gz = "/mnt/local/local.tar.gz";
const char* SDKManager::_startup      = "/mnt/flash/startup";

const char* SDKManager::flash_device(const char* filepath) {
    const char* name = base_name(filepath);
    if (strcmp(name, "rootfs.img.gz") == 0) {
        return "/dev/mtd3";
    } else if (strcmp(name, "uImage") == 0) {
        return "/dev/mtd2";
    } else if (strcmp(name, "u-boot.bin") == 0) {
        return "/dev/mtd1";
    } else if (strcmp(name, base_name(_local_tar_gz)) == 0) {
        return _local_tar_gz;
    } else if (strcmp(name, base_name(_startup)) == 0) {
        return _startup;
    }
    CGI_ERROR(true, "%s is an invalid file name", name);
}


void SDKManager::flash_firmware(const char* filepath) {
    const char* device = flash_device(filepath);
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        SBL_MSG(MSG::SDK, "Unable to find %s, skipping", filepath);
        return;
    }
    close(fd);
    // local.tar.gz is simply copied to /mnt/local
    if (device == _local_tar_gz || device == _startup) {
        ifstream src(filepath);
        ofstream dst(device);
        CGI_ERROR(!src, "Error opening %s to read", filepath);
        CGI_ERROR(!dst, "Error opening %s to write", device);
        dst << src.rdbuf();
        src.close();
        dst.close();
        if (device == _startup)
            CGI_ERROR(chmod(_startup, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH), "Error changing permission of startup file");
    } else {
        // other files need erase flash
        const char* error = erase_flash(device);
        CGI_ERROR(error, error);
        SBL_MSG(MSG::SDK, "Erased %s", device);
        error = write_flash(filepath, device);
        CGI_ERROR(error, error);
    }
    SBL_MSG(MSG::SDK, "Written %s to %s", filepath, device);
}

void SDKManager::get_date(Date& date) {
    // This *may* throw, if linux date is set to something that CGI server doesn't like
    try {
        // get local time
        date.set_time(time(NULL) + date.tz_seconds());
    } catch (Exception& ex) {
        // system date is illegal per CGI specs, reset it to default.
        // This can happen if camera cannot reach NTP server (date is 12/31/1969)
        Date date(0, "");
        set_date(date);
        SBL_MSG(MSG::SDK, "Caught illegal time, setting to default %s", date.info().c_str());
    }
    SBL_MSG(MSG::SDK, "Date is:\n%s", date.info().c_str());
}

// verify that serial number has valid characters
char* SDKManager::serial_number() {
    char* s = reinterpret_cast<char*>(_version_info.pci_attrib.serial_number);
    for (int n = 0; s[n] && n < SDVR_BOARD_SERIAL_LENGTH + 1; n++) {
        if ( (s[n] & 0x0080) || (!isalnum(s[n]) && !(s[n] == '-' || s[n]  == '_' || s[n] == '.')))
            s[n] = '-';
    }
    s[SDVR_BOARD_SERIAL_LENGTH] = '\0';
    return s;
}


void SDKManager::get_device_info(DeviceInfo& device_info) {
    char host_name[HOST_NAME_MAX+5];
    memset(host_name, 0, sizeof(host_name));
    CGI_ERROR(gethostname(host_name, sizeof(host_name)) != 0, "Failed to get the host name. errno = %s", strerror( errno));

    device_info.hostname      = host_name;
    device_info.serial_number = serial_number();

    char linux_mac_addr[SBL::Net::MAC_ADDR_BUFF_SIZE];
    SBL::Net::mac_address(linux_mac_addr);
    device_info.mac_address = linux_mac_addr;
    if (is_initialized()) {
        unsigned char mac[6];
        SDK_CALL(sdvr_get_macaddr(0, mac));
        char mac_addr[18];
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        device_info.mac_address = mac_addr;
        if (strcmp(mac_addr, linux_mac_addr)) 
            SBL_WARN("Mac address error: EEPROM is %s, Linux is %s", mac_addr, linux_mac_addr);
    }

    // if dhcp=0, we are not changing ip address here
    if (device_info.dhcp) {
        char ip_addr[SBL::Net::IP_ADDR_BUFF_SIZE];
        char if_name[SBL::Net::IF_NAME_BUFF_SIZE];
        Net::ip_address(ip_addr, "eth0", if_name);
        device_info.ip_address = ip_addr;

        Net::gateway_address(ip_addr);
        device_info.gateway = ip_addr;

        Net::subnet(ip_addr, if_name);
        device_info.subnet = ip_addr;
    }
    SBL_MSG(MSG::SDK, "Retrieved  device information: %s", device_info.info().c_str());
}

void SDKManager::get_image(Image& image) {
    CGI_ERROR(!is_initialized(), "System is not initialized");

    SBL_MSG(MSG::SDK, "Retrieving camera image information from the firmware: %s", image.info().c_str());
    sdvr_img_t image_ctrl;
    memset(&image_ctrl, 0 , sizeof(image_ctrl));
    SDK_CALL(sdvr_get_img_properties(_camera_handle, &image_ctrl));

    image.brightness = image_ctrl.brightness;
    image.contrast   = image_ctrl.contrast  ;
    image.hue        = image_ctrl.hue       ;
    image.saturation = image_ctrl.saturation;
    image.sharpness  = image_ctrl.sharpness ;

    sdvr_flip_t flip;
    SDK_CALL(sdvr_get_flip_properties(_camera_handle, &flip));
    image.flip = sdk_flip_mode_to_str_flip_mode(flip.flip_mode);
}

/*
 * This method is called by the SDK A/V callback to get the callback
 * buffer. Once the frame is received the callback Mutex will be released
 * so the the acquire_snapshop() or raw_command() can continue.
 *
 * Note: The cgi-front end will call release_callback_buffer() after the 
 *       buffer is sent to the client
 */
void SDKManager::get_frame_from_callback(sdvr_frame_type_e frame_type) {
    SBL_MSG(MSG::SDK, "got frame %d in callback", frame_type);
    _callback_buffer = NULL;
    _semaphore.lock();
    sdk_error(sdvr_get_stream_buffer(_camera_handle, frame_type, 0, &_callback_buffer));
    test_frame_verify(_callback_buffer);
    _semaphore.signal();
    _semaphore.unlock();
    SBL_MSG(MSG::SDK, "Semaphore signaled and unlocked");
}

bool SDKManager::test_frame_verify(sdvr_av_buffer_t *av_frame) {
    sx_uint8*   frame;
    sx_uint32   size;
    sx_uint8    pattern;
    sx_bool     test_frame;

    if (sdk_error(sdvr_get_buffer_test_frame(av_frame, &test_frame)) || !test_frame)
        return false;

    sdk_error(sdvr_get_buffer_test_pattern(av_frame, &pattern));
    sdk_error(sdvr_av_buf_payload(av_frame, &frame, &size));
    SBL_MSG(MSG::FRAME, "verifying test frame (payload size: %d, pattern: 0x%02X)", size, pattern);
    for (unsigned int i = 0; i < size; i++) {
        if (frame[i] != pattern) {
            SBL_ERROR("Failed to verify test frame (offset: %d, payload size: %d, pattern: %02X)", 
                       i, size, pattern);
            return true;
        }
    }
    SBL_MSG(MSG::FRAME, "test frame verified (payload size: %d, pattern: %02X)", size, pattern);
    return true;
}

void SDKManager::get_stream(Stream& stream) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    SBL_MSG(MSG::SDK, "Retrieving stream information from the firmware: %s", stream.info().c_str());

    sx_uint8 stream_id = stream.id;
    CGI_ERROR (stream_id >= _board_config.num_video_encoders_per_camera,
     "Invalid stream ID %d. Maximum number of streams supported by IP-Camera is %d",
                stream_id, _board_config.num_video_encoders_per_camera);

    SDK_CALL(sdvr_get_video_encoder_channel_params(_camera_handle, stream_id, &_stream_info[stream_id].video_enc_params));

    // The encoder enable state and type is not returned by the firmware.
    // We need to return the cached versions of these parameters.
    stream.enable = _stream_info[stream_id].is_enabled;
    stream.encoder = sdk_id_to_str_encoder(_stream_info[stream_id].video_format);

    bool use_custom_size = (_board_settings.custom_vstd.w && 
                            _board_settings.custom_vstd.h &&
                            _board_settings.custom_vstd.frame_rate);

    // Since we only support reqular frame rate which is from 1 - 60 we can
    // do a straight assignment. Otherwise, we must interpret the value in
    // frame_rate before assigning to fps field.
    stream.fps = use_custom_size ? _board_settings.custom_vstd.frame_rate :
                                   _stream_info[stream_id].video_enc_params.frame_rate;

    switch (_stream_info[stream_id].video_format) {
    case SDVR_VIDEO_ENC_H264:
        stream.bitrate = _stream_info[stream_id].video_enc_params.encoder.h264.avg_bitrate;
        stream.gop = _stream_info[stream_id].video_enc_params.encoder.h264.gop;
        stream.profile = sdk_enc_mode_to_str_enc_profile(
                _stream_info[stream_id].video_enc_params.encoder.h264.enc_mode);
        break;
    case SDVR_VIDEO_ENC_JPEG:
        stream.quality = _stream_info[stream_id].video_enc_params.encoder.jpeg.quality;
        break;
    case SDVR_VIDEO_ENC_MPEG4:
        stream.bitrate = _stream_info[stream_id].video_enc_params.encoder.mpeg4.avg_bitrate;
        stream.gop = _stream_info[stream_id].video_enc_params.encoder.mpeg4.gop;
    default:
        break;
    }

    if (_stream_info[stream_id].video_enc_params.res_decimation == SDVR_VIDEO_RES_CUSTOM) {
    // todo: Currently the custom resolution are always based on the full
    //       camera resolution with given re-sized width and height and
       //       no cropping
        _stream_info[stream_id].video_enc_params.custom_res.src = SDVR_CROP_SRC_FULL;

        _stream_info[stream_id].video_enc_params.custom_res.crop_x_offset = 0;
        _stream_info[stream_id].video_enc_params.custom_res.crop_y_offset = 0;

        _stream_info[stream_id].video_enc_params.custom_res.cropped_width = 0;
        _stream_info[stream_id].video_enc_params.custom_res.cropped_height = 0;

        stream.width = _stream_info[stream_id].video_enc_params.custom_res.scaled_width;
        stream.height = _stream_info[stream_id].video_enc_params.custom_res.scaled_height;

        if (use_custom_size)
        {
            if ((_stream_info[stream_id].video_enc_params.custom_res.scaled_width > _board_settings.custom_vstd.w) ||
                (_stream_info[stream_id].video_enc_params.custom_res.scaled_height > _board_settings.custom_vstd.h))
            {
                // Don't allow scaled dimensions to be larger than custom dimensions, if specified
                stream.width = _board_settings.custom_vstd.w;
                stream.height = _board_settings.custom_vstd.h;
            }
        }
    }
    else if (use_custom_size)
    {
        stream.width = _board_settings.custom_vstd.w;
        stream.height = _board_settings.custom_vstd.h;
    } else {
        int width, height;
        decimation_to_width_height(_video_std,
                    (sdvr_video_res_decimation_e)_stream_info[stream_id].video_enc_params.res_decimation,
                    &width, &height);
        stream.width  = width;
        stream.height = height;
    }

}

void SDKManager::release_callback_buffer() {
    if (_callback_buffer) {
        if (is_initialized())
            sdvr_release_av_buffer(_callback_buffer);
        else
            delete[] _callback_buffer;
        _callback_buffer = NULL;
        SBL_MSG(MSG::SDK, "Released callback buffer");
    }
}

void SDKManager::reset(ParamState& param_state) {
    SBL_INFO("Resetting the system...");
    if (is_initialized())
        param_state.image.sensor_rate = _sensor_rate;
    set_image(param_state.image);
    for (int i = 0; i < Stream::COUNT; i++) {
        if (param_state.stream[i].fps > _sensor_rate) {
            SBL_WARN("Reducing fps for stream %d to sensor rate %d", param_state.stream[i].id.get(), _sensor_rate);
            param_state.stream[i].fps = _sensor_rate;
        }
        set_stream(param_state.stream[i]);
    }
    SBL_MSG(MSG::SDK, "Reset completed\n");
}

/*
 * This method is called by the SDK A/V callback to send the given video
 * buffer bit-stream or motion value to the client requesting the buffer.
 *
 * All the A/V frames are sent to the RTSP server. 
 *
 */
void SDKManager::send_frame(sdvr_chan_handle_t sdk_chan_handle, sdvr_frame_type_e frame_type, int stream_id) {
    sdvr_av_buffer_t* av_frame;
    sdvr_err_e err = sdvr_get_stream_buffer(sdk_chan_handle, frame_type, stream_id, &av_frame);

    // check if the stream valid and enabled.
    if (err != SDVR_ERR_NONE || _camera_handle != sdk_chan_handle)
        return;
    // If we are using Stretch RTSP library, the video frames are
    // sent as they are arrived. No queuing is done on the frame
    sx_uint8          chan_num   = sdvr_get_chan_num(sdk_chan_handle);
    sx_uint8         *frame_payload;
    sx_uint32         frame_payload_size;
    sx_bool           test_frame = false;
    sx_uint64         timestamp64;

    err = sdvr_get_buffer_test_frame(av_frame, &test_frame);
    if (err == SDVR_ERR_NONE && test_frame) {
        test_frame_verify(av_frame);
        sdvr_release_av_buffer(av_frame);
        _frame_count++;
        return;
    }

    sdvr_get_buffer_timestamp(av_frame, &timestamp64);
    /* convert from hardware 100 KHz clock to 90 KHz timestamp clock */
    sx_uint32 timestamp = timestamp64 * 9 / 10;

    sdvr_av_buf_payload( av_frame, &frame_payload, &frame_payload_size);
    sx_uint32 seq_number, frame_number, drop_count;
    sdvr_av_buf_sequence(av_frame, &seq_number, &frame_number, &drop_count);

    SBL_MSG(MSG::FRAME, "stream=%d, type=%d, size=%d, ts=%d, seq=%d, frame=%d, drops=%d", 
                         stream_id, frame_type, frame_payload_size, timestamp, seq_number, frame_number, drop_count);

    switch (frame_type) {
    case SDVR_FRAME_H264_SVC_SEI:
    case SDVR_FRAME_H264_SVC_PREFIX:
    case SDVR_FRAME_H264_SVC_SUBSET_SPS:
    case SDVR_FRAME_H264_SVC_SLICE_SCALABLE:
    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
        _frame_count++;
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
        rtsp_send_frame(chan_num, stream_id, frame_payload, frame_payload_size, timestamp, RTSP::H264);
        sdvr_release_av_buffer(av_frame);
        break;
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
        _frame_count++;
    case SDVR_FRAME_MPEG4_VOL:
        rtsp_send_frame(chan_num, stream_id, frame_payload, frame_payload_size, timestamp, RTSP::MPEG4);
        sdvr_release_av_buffer(av_frame);
        break;
    case SDVR_FRAME_MPEG2_I:
    case SDVR_FRAME_MPEG2_P:
        break;
    case SDVR_FRAME_JPEG:
        rtsp_send_frame(chan_num, stream_id, frame_payload, frame_payload_size, timestamp, RTSP::MJPEG);
        sdvr_release_av_buffer(av_frame);
        _frame_count++;
        break;
    case SDVR_FRAME_MOTION_MAP:
        mv_sender().send_mv_packet(frame_payload, frame_payload_size, timestamp);
        sdvr_release_av_buffer(av_frame);
        break;
    }
}

char* SDKManager::acquire_snapshot(const Snapshot& snapshot, unsigned int *payload_size)
{
    CGI_ERROR(!is_initialized(), "System is not initialized");
    SBL_MSG(MSG::SDK, "Sending snapshot with decimation %d...", (int)snapshot.decimation);
    *payload_size = 0;
    sdvr_video_res_decimation_e resolution = decimation_id_to_sdk(snapshot.decimation);
    _callback_buffer = NULL;

    // To be used for snapshot
    sdvr_fosd_config_t fosd_config;

    memset(&fosd_config, 0, sizeof(sdvr_fosd_config_t));
    /*  prepare fosd_config, including texts  */
    fosd_config.dts_format    =  SDVR_OSD_DTS_DMY_24H;

    /*OSD positions depends on text id */
    fosd_config.position_ctrl = SDVR_LOC_BOTTOM_RIGHT;
    fosd_config.max_width     = 600;
    fosd_config.max_height    = 36;

    fosd_config.font_table_id = decimation_font_table_id_to_sdk(snapshot.decimation); 
    sx_uint16 osd_id = SDVR_FOSD_OSD_ID(SDVR_FOSD_MODULE_ID_ENC, 4, 0); //snapshot is stream 4

    SDK_CALL(sdvr_fosd_config(_camera_handle, osd_id, &fosd_config));
    SDK_CALL(sdvr_fosd_show(_camera_handle, osd_id, true));
    _semaphore.lock();
    try {
        SDK_CALL( sdvr_snapshot(_camera_handle, resolution));
    } catch (Exception& ex) {
        _semaphore.unlock();
        throw;
    }

    // Wait for the snapshot to arrive.
    // Note: The semaphore is released once the snapshot buffer is arrived in
    // stream_callback().
    SBL_MSG(MSG::SDK, "Waiting on snapshot...");
    // NOTE: There is no need to examine the return value from wait() since
    //       if the timeout occurs _callback_buffer is going to be NULL
    //       which will generate an error to be sent to the client.
    _semaphore.wait(0,_options.sct_timeout);
    _semaphore.unlock();

    unsigned char* payload  = NULL;
    if (_callback_buffer)
        sdvr_av_buf_payload(_callback_buffer, &payload, payload_size);
    CGI_ERROR(*payload_size == 0, "Failed to receive the snapshot buffer.");
    SBL_MSG(MSG::SDK, "Received snapshot, decimation %d, size %d", (int)snapshot.decimation, *payload_size);
    return reinterpret_cast<char*>(payload);

}

/* We always keep Linux time as UTC. Timezone and DST are applied locally in CGI server only. */
void SDKManager::set_date(const Date& date) {
    SBL_MSG(MSG::SDK, "Setting up date:\n%s", date.info().c_str());
    struct timespec time;
    time.tv_sec = date.get_time() - date.tz_seconds();
    time.tv_nsec = 0;
    SBL_MSG(MSG::SDK, "Setting time to %d", time.tv_sec);
    if (clock_settime(CLOCK_REALTIME, &time) != 0)
        throw SBL::Exception(USER_ERROR, 0, __FILE__, __LINE__, "unable to set clock").perror();

    if (is_initialized()) {
        SDK_CALL(sdvr_set_date_time(_board_index,  date.get_time()));
    }
}

void SDKManager::set_hostname(const string& hostname) {
    CGI_ERROR (sethostname(hostname.c_str(), hostname.size()) != 0, "Failed to set the host name." );
};

void SDKManager::set_device_info(const DeviceInfo& device_info) {
    SBL_MSG(MSG::SDK, "Setting up device info: %s", device_info.info().c_str());
    string hostname = device_info.hostname;
    CGI_ERROR (sethostname(hostname.c_str(), hostname.size()) != 0,"Failed to set the host name." );
    if (is_initialized() && device_info.mac_address.changed()) {
        unsigned char mac[6];
        CGI_ERROR(sscanf(device_info.mac_address.get().c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6,
                  "Unable to read mac address from internal variable");
        SDK_CALL(sdvr_set_macaddr(0, mac));
        SBL_INFO("Changed mac address to %s", device_info.mac_address.get().c_str());
    }
    if (device_info.packet_gap.changed()) {
        RTSP::Server* rtsp_server = RTSP::application()->rtsp_server();
        if (rtsp_server) 
            rtsp_server->set_packet_gap(device_info.packet_gap * 1000);
        SBL_INFO("Packet gap set to %d us", device_info.packet_gap.get());
    }
}

void SDKManager::set_image(const Image& image) {
    // Video standard is set in the image, which must be the first command 
    // from the state file. Initialization is delayed until that file is read
    if (!is_initialized()) {
        _sensor_rate = image.sensor_rate;
        CGI_ERROR(!init(), "System initialization error");
    }
    SBL_MSG(MSG::SDK, "Setting up image: %s", image.info().c_str());
    // flip must be done first, since it may fail on cameras that don't support it
    // and we don't want other things changed at that time.
    if (image.flip.changed()) {
        sdvr_flip_t flip;
        flip.flip_mode  = str_flip_mode_to_sdk_flip_mode(image.flip);
        SDK_CALL(sdvr_set_flip_properties(_camera_handle, &flip));
    }
    if (image.osd.changed() && _options.use_fosd == false) 
        SDK_CALL(sdvr_osd_text_show(_camera_handle, 0, image.osd));
    // Set the image control parameters for the IP-Camera.
    if (image.brightness.changed() || image.contrast.changed() ||
        image.hue.changed()       || image.saturation.changed() ||
        image.sharpness.changed()) {

        sdvr_img_t image_ctrl;
        memset(&image_ctrl, 0 , sizeof(image_ctrl));
        image_ctrl.brightness = image.brightness;
        image_ctrl.contrast   = image.contrast;
        image_ctrl.hue        = image.hue;
        image_ctrl.saturation = image.saturation;
        image_ctrl.sharpness  = image.sharpness;

        SDK_CALL(sdvr_set_img_properties(_camera_handle, &image_ctrl));
    }

    if (image.exposure.changed()) {

        sdvr_exposure_t exposure_ctrl;

        memset(&exposure_ctrl, 0 , sizeof(exposure_ctrl));
        SDK_CALL(sdvr_get_exposure_properties(_camera_handle, &exposure_ctrl));

        if (image.exposure <= 50) {
            exposure_ctrl.setpoint = (sx_uint8)(image.exposure * 4 / 10);
        } else {
            exposure_ctrl.setpoint = (sx_uint8)((image.exposure * 16 / 10) - 60);
        }
        SDK_CALL(sdvr_set_exposure_properties(_camera_handle, &exposure_ctrl));
    }

    if (image.motion_rate.changed()) {
        SDK_CALL(sdvr_enable_motion_map(_camera_handle, image.motion_rate > 0));
        SBL_MSG(MSG::SDK, "Motion rate is %d", image.motion_rate.get());
    }

    sdvr_smo_attribute_t attrib;     // The display characteristics of this SMO port
    sdvr_smo_set_disable_mode(_board_index, 1, SDVR_SMO_DISABLE_MODE_BLANK);  // set disable mode to blank

    if (image.cvbs.changed()) {

        if (sdvr_get_smo_attributes(_board_index, 1, &attrib) == SDVR_ERR_NONE) {
            
            sdvr_smo_grid_t     grid;        // Currently we are only supporting one grid
            memset(&grid, 0, sizeof(grid)); 
            
            grid.width  = (attrib.width / 2) * 2; 
            grid.height = (attrib.height / 2) * 2; 
            grid.enable = image.cvbs; 
            
            SDK_CALL(sdvr_set_smo_grid_ex(_camera_handle, 1, &grid));
        }
        
    }
    if (image.tdn.changed()) {
        string command("tdn ");
        command += strcmp(image.tdn.get().c_str(), "hardware") == 0 ? "hw" : image.tdn.get();
        stringstream reply;
        unsigned int response_size;
        send_raw_command("ipp", command.c_str(), command.size(), reply, &response_size);
        CGI_ERROR(strcmp(reply.str().c_str(), "IPP_OK"), "changing TrueDayNight mode failed with message %s", reply.str().c_str());
        SBL_INFO("Changed TrueDayNight mode to %s", image.tdn.get().c_str());
    }
}

void SDKManager::set_stream(Stream& stream) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    bool use_custom_size = (_board_settings.custom_vstd.w && 
                            _board_settings.custom_vstd.h &&
                            _board_settings.custom_vstd.frame_rate);

    SBL_MSG(MSG::SDK, "Setting up stream: %s", stream.info().c_str());
    sx_uint8 stream_id = stream.id;
    CGI_ERROR (stream_id >= _board_config.num_video_encoders_per_camera,
     "Invalid stream ID %d. Maximum number of streams supported by IP-Camera is %d",
                stream_id, _board_config.num_video_encoders_per_camera);


    if (_options.use_fosd == true) {
        // configure fosd
        sdvr_fosd_config_t fosd_config; 
        memset(&fosd_config, 0, sizeof(sdvr_fosd_config_t)); 
        /*  prepare fosd_config, including texts  */
        fosd_config.dts_format    =  SDVR_OSD_DTS_DMY_24H; 

        /*OSD positions depends on text id */
        fosd_config.position_ctrl = SDVR_LOC_BOTTOM_RIGHT; 
        fosd_config.max_width     = 600; 
        fosd_config.max_height    = 36; 

        /* Generate osd ID */
        // config font size, table_id 0 is normal size, 1 is medium size and 2 is small size
        if (stream.width > 480) {
            fosd_config.font_table_id = SDVR_FOSD_FONT_TABLE_LARGE; 
        } else if (stream.width > 352) {
            fosd_config.font_table_id = SDVR_FOSD_FONT_TABLE_MEDIUM; 
        } else {
            fosd_config.font_table_id = SDVR_FOSD_FONT_TABLE_SMALL; 
        }
        sx_uint16 osd_id = SDVR_FOSD_OSD_ID(SDVR_FOSD_MODULE_ID_ENC, stream_id, 0); 
        SDK_CALL(sdvr_fosd_config(_camera_handle, osd_id, &fosd_config)); 
        SDK_CALL(sdvr_fosd_show(_camera_handle, osd_id, stream.osd));
    }

    // Before we can set any encoder parameter, we must the set the video
    // encoder.
    change_encoder_if_needed(stream_id, str_encoder_to_sdk_id(stream.encoder));
    if (stream.fps > _sensor_rate) {
        CGI_ERROR(_server->initialized(), "Stream fps must not be more then sensor rate");
        SBL_WARN("Resetting fps for stream %d to sensor rate %d", stream.id.get(), _sensor_rate);
        stream.fps = _sensor_rate;
    }
    _stream_info[stream_id].video_enc_params.frame_rate = stream.fps;

    /*
     * A flag to indicate whether the encoder must flush its buffer
     * immediately. Setting this flag to true causes the encoder buffer to be
     * flushed immediately. Otherwise, there is a 1 frame latency before the encoder
     * buffer is flushed. It is not recommended to set this flag to true for
     * frame rates higher than 10.
     * NOTE: Enabling the encoder flush buffer could affect
     * the system performance.
     */
    if ( _stream_info[stream_id].video_enc_params.frame_rate < 10)
        _stream_info[stream_id].video_enc_params.flush_buf = 1;
    else
        _stream_info[stream_id].video_enc_params.flush_buf = 0;


    _stream_info[stream_id].video_enc_params.res_decimation =
            frame_width_height_2_decimation(_video_std, stream.width,
                    stream.height);

    CGI_ERROR(_stream_info[stream_id].video_enc_params.res_decimation ==
            SDVR_VIDEO_RES_DECIMATION_NONE,
            "Invalid video frame width %d and height %d for stream ID = %d.",
            (int)stream.width, (int)stream.height, stream_id);

    if (_stream_info[stream_id].video_enc_params.res_decimation == SDVR_VIDEO_RES_CUSTOM) {
    // todo: Currently the custom resolution are always based on the full
    //       camera resolution with given re-sized width and height and
       //       no cropping
        _stream_info[stream_id].video_enc_params.custom_res.src = SDVR_CROP_SRC_FULL;

        _stream_info[stream_id].video_enc_params.custom_res.crop_x_offset = 0;
        _stream_info[stream_id].video_enc_params.custom_res.crop_y_offset = 0;

        _stream_info[stream_id].video_enc_params.custom_res.cropped_width = 0;
        _stream_info[stream_id].video_enc_params.custom_res.cropped_height = 0;

        if (use_custom_size && 
            ((stream.width > _board_settings.custom_vstd.w) ||
             (stream.height > _board_settings.custom_vstd.h)))
        {
            // Don't allow scaled dimensions to be larger than custom dimensions, if specified
            _stream_info[stream_id].video_enc_params.custom_res.scaled_width = _board_settings.custom_vstd.w;
            _stream_info[stream_id].video_enc_params.custom_res.scaled_height = _board_settings.custom_vstd.h;
            SBL_MSG(MSG::SDK, "reduced scaling size to (%d,%d)\n",
                    _stream_info[stream_id].video_enc_params.custom_res.scaled_width,
                    _stream_info[stream_id].video_enc_params.custom_res.scaled_height);
        }
        else
        {
            _stream_info[stream_id].video_enc_params.custom_res.scaled_width = stream.width;
            _stream_info[stream_id].video_enc_params.custom_res.scaled_height = stream.height;
        }
    }

    switch (_stream_info[stream_id].video_format) {
    case SDVR_VIDEO_ENC_H264:
        _stream_info[stream_id].video_enc_params.encoder.h264.avg_bitrate       = stream.bitrate;
        _stream_info[stream_id].video_enc_params.encoder.h264.max_bitrate       = stream.max_bitrate;
        _stream_info[stream_id].video_enc_params.encoder.h264.bitrate_control   = 
            str_enc_rate_control_to_sdk_enc_bitrate_control(stream.rate_control);
        _stream_info[stream_id].video_enc_params.encoder.h264.gop               = stream.gop;
        _stream_info[stream_id].video_enc_params.encoder.h264.quality           = stream.quality;
        _stream_info[stream_id].video_enc_params.encoder.h264.enc_mode          = 
                    str_enc_profile_to_sdk_enc_mode(stream.profile);
        // _stream_info[stream_id].video_enc_params.encoder.h264.i_frame_bitrate   = stream.i_percentage;
        break;
    case SDVR_VIDEO_ENC_JPEG:
        _stream_info[stream_id].video_enc_params.encoder.jpeg.quality         = stream.quality;
        // For JPEG encoders we need to set the image stype to 2. This is the style needed fro RTSP/RTP stream. 
        // This style can not be used for recording to file.
        _stream_info[stream_id].video_enc_params.encoder.jpeg.is_image_style    = 2;
        break;
    case SDVR_VIDEO_ENC_MPEG4:
        _stream_info[stream_id].video_enc_params.encoder.mpeg4.avg_bitrate      = stream.bitrate;
        _stream_info[stream_id].video_enc_params.encoder.mpeg4.max_bitrate      = 0;
        _stream_info[stream_id].video_enc_params.encoder.mpeg4.bitrate_control  = SDVR_BITRATE_CONTROL_CBR;
        _stream_info[stream_id].video_enc_params.encoder.mpeg4.gop              = stream.gop;
        _stream_info[stream_id].video_enc_params.encoder.mpeg4.quality          = 0;
        break;
    default:
        break;
    }
    SDK_CALL(sdvr_set_video_encoder_channel_params(_camera_handle, stream_id, &_stream_info[stream_id].video_enc_params));

    // enabling of  encoder should be done as the last step and only if it
    // changes its state. Also, we don't do it at initialization, we want
    // to configure all streams first and enable them later.
    if (_server->initialized()) {
        if (_stream_info[stream_id].is_enabled  != stream.enable) {
            SDK_CALL(sdvr_enable_encoder(_camera_handle,stream_id, stream.enable));
            SBL_INFO("sdvr_enable_encoder(Encoder %d, enable=%d)", stream_id, stream.enable.get());
        }
        // At this point we have successfully set the encoder parameters and
        // enabled/disabled the encoder as requested. Make sure to remember the
        // streaming state so that we can either reject or accept if we receive
        // RTSP request. Also this is needed need when trying to change the video
        // encoder.
        _stream_info[stream_id].is_enabled  = stream.enable;
    }
}

void SDKManager::enable_stream(int stream_id, bool enable) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    CGI_ERROR(stream_id >= Stream::COUNT, "invalid stream_id");
    SDK_CALL(sdvr_enable_encoder(_camera_handle, stream_id, enable));
    SBL_MSG(MSG::SDK, "Stream %d is %s", stream_id, enable ? "enabled" : "disabled");
    _stream_info[stream_id].is_enabled  = enable;
}

void SDKManager::set_test(int test_frames) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    SDK_CALL(sdvr_set_test_frames(test_frames));
}

char* SDKManager::raw_command(const string& subsystem, const string& command, ostream& reply, unsigned int* response_size) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    unsigned int size = command.size();
    char cmd[size + 1];
    unsigned int n = 0;
    for (; n <= size && command[n] >= ' ' && command[n] <= '~'; n++) {
        cmd[n] = command[n] == '+' ? ' ' : command[n];
    }
    cmd[n] = '\0';
    return send_raw_command(subsystem, cmd, size, reply, response_size);
}

char* SDKManager::send_raw_command(const string& subsystem, const char* buffer, int buffer_size, ostream& reply, unsigned int* response_size) {
    try {
        SBL_MSG(MSG::SDK, "Executing raw command to %s: %s", subsystem.c_str(), buffer);
        sdvr_raw_command_t raw_command;
        raw_command.cmd_data = (unsigned char*) buffer;
        raw_command.cmd_data_size = buffer_size;
        raw_command.sub_system = str_to_subsystem(subsystem);
        _semaphore.lock();
        SDK_CALL(sdvr_raw_command_channel(_camera_handle, &raw_command));
        char response[SDVR_MAX_RAW_CMD_RESPONSE_SIZE + 1];
        memcpy(response, raw_command.response, SDVR_MAX_RAW_CMD_RESPONSE_SIZE);
        response[SDVR_MAX_RAW_CMD_RESPONSE_SIZE] = '\0';
        SBL_MSG(MSG::SDK, "Raw command reply: '%s', response_size=%d", response, raw_command.response_size);
        *response_size = raw_command.response_size;
        _callback_buffer = NULL;
        unsigned char* payload  = NULL;
        if (raw_command.response_size) {
            SBL_MSG(MSG::SDK, "Waiting for callback to the raw command '%s'", buffer);
            int wait_status = _semaphore.wait(0, _options.sct_timeout);
            SBL_MSG(MSG::SDK, "Wait ended with status %d, reponse_size is %d, buffer is %p", wait_status, raw_command.response_size, _callback_buffer);
            CGI_ERROR(!_callback_buffer, "Did not receive reply to raw command in callback");
            SDK_CALL(sdvr_av_buf_payload(_callback_buffer, &payload, response_size));
        } else
            reply << response;
        _semaphore.unlock();
        return reinterpret_cast<char*>(payload);
    } catch (Exception& ex) {
        _semaphore.unlock();
        throw;
    }
}

void SDKManager::ipp_command(const char* format, ...) {
    char buffer[200];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);
    CGI_ERROR(size < 0 || size == sizeof(buffer), "error with ipp command");
    ostringstream reply;
    unsigned int reply_size = 0;
    send_raw_command("ipp", buffer, size, reply, &reply_size);
    CGI_ERROR(reply.str() != "IPP_OK", "IPP command returned error %s", reply.str().c_str());
}

char* SDKManager::raw_command(const string& subsystem, const char* filepath, int file_size, ostream& reply, unsigned int* response_size) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    *response_size = 0;
    char* buffer = new(nothrow) char[file_size];
    CGI_ERROR(buffer == NULL, "Unable to allocate buffer to read file %s", filepath);
    try {
        int fd = open(filepath, O_RDONLY);
        SBL_PERROR(fd < 0);
        CGI_ERROR(read(fd, buffer, file_size) != file_size, "Unable to read %d bytes from %s", file_size, filepath);
        return send_raw_command(subsystem, buffer, file_size, reply, response_size);
    } catch (SBL::Exception& ex) {
        delete[] buffer;
        throw;
    }
}

bool SDKManager::read_temperature(float* temperature) {
    if (!is_initialized()) {
        SBL_ERROR("Attempt to read temperature before SDK is initialized");
        return false;
    }
    try {
        SDK_CALL(sdvr_measure_temperature(_board_index, temperature));
        SBL_MSG(MSG::SDK, "Measured temperature %.1f", *temperature);
        return true;
    } catch (Exception& ex) {
        if (ex.code() == SDVR_FRMW_ERR_UNSUPPORTED_COMMAND) {
            SBL_MSG(MSG::SDK, "No temperature sensor available");
        } else
            SBL_ERROR("temperature read error: %d", ex.code());
    }
    return false;
}

void SDKManager::refresh_watchdog(int timeout) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    SDK_CALL(sdvr_set_watchdog_state_ex(_board_index, SDVR_WATCHDOG_REFRESH, timeout));
}

void SDKManager::enable_watchdog(bool enable, int timeout) {
    CGI_ERROR(!is_initialized(), "System is not initialized");
    SDK_CALL(sdvr_set_watchdog_state_ex(_board_index, enable ? SDVR_WATCHDOG_HOST : SDVR_WATCHDOG_SCP, timeout));
    SBL_MSG(MSG::SDK, "Watchdog %s", enable ? "enabled" : "disabled");
}

void SDKManager::set_roi(const Roi& roi) {
    ipp_command("privacy_color 0 255 0");
    ipp_command("set_roi 0 %d %d %d %d 0", 0, 0, Roi::WIDTH, Roi::HEIGHT);
    for (unsigned int i = 0; i < roi.size(); i++) {
        if (roi[i].w() == 0 || roi[i].h() == 0) 
            SBL_WARN("Skipping ROI with null width or height");
        else
            ipp_command("set_roi 0 %d %d %d %d 1", roi[i].x(), roi[i].y(), roi[i].w(), roi[i].h());
    }
}

}

