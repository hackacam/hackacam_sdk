#pragma once
#ifndef _CGI_SDK_MANAGER_H
#define _CGI_SDK_MANAGER_H
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
#include <sstream>
#include <net/if.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sbl/sbl_thread.h>
#include <sbl/sbl_socket.h>
#include <sdk/sdvr_sdk.h>
#include "sdk_translator.h"
#include "cgi_param_set.h"
#include "mv_sender.h"

namespace CGI {

class SDKManager : public SDKTranslator {
public:
    struct Options {
        sdvr_chan_buf_def_t  chan_buf_counts;
        unsigned int         debug_flags;
        bool                 enableH264Auth;
        string               firmware_name;   // The name of firmware to set on the IP-Camera board
        string               home_folder;     // The path the the Stretch folder in IP-Camera
        sdvr_video_std_e     hd_video_std;
        int                  max_video_stream_per_camera; // Number of video stream per camera
                                     // This could be less than or equal to what board supports.
        unsigned int         rtsp_port_num;
        unsigned int         sct_timeout;     // number of seconds to wait for a response
                                              // to be received from the firmware.
        string               sdk_log_filename;
        sdvr_video_std_e     sd_video_std;
        bool                 use_fosd;
        Options();
    };

    SDKManager(Server *server, const Options& options);

    void                flash_firmware(const char* filepath);
    void                get_date(Date& date);
    void                get_device_info(DeviceInfo& device_info);
    void                get_image(Image& image);
    void                get_stream(Stream& stream);
    void                get_frame_from_callback(sdvr_frame_type_e frame_type);
    void                release_callback_buffer();
    void                reset(ParamState& param_state);
    void                send_frame(sdvr_chan_handle_t sdk_chan_handle,
                                   sdvr_frame_type_e  frame_type,
                                    int stream_id);
    char*               acquire_snapshot(const Snapshot& snapshot,
                                        unsigned int *payload_size);
    void                set_date(const Date& date);
    void                set_device_info(const DeviceInfo& device_info);
    void                set_hostname(const string& device_info);

    void                set_image(const Image& image);
    void                set_rtsp_request_count(int stream_id, int count);
    void                set_stream(Stream& stream);
    void                set_test(int test_frames);
    void                enable_stream(int stream_id, bool enable);
    void                set_roi(const Roi& roi);
    MVSender&           mv_sender() const;

    char*               raw_command(const string& subsystem, const string& command, ostream& reply, unsigned int* response_size);
    char*               raw_command(const string& subsystem, const char* filename, int file_size, ostream& reply, unsigned int* response_size);
    void                ipp_command(const char* format, ...);
    bool                read_temperature(float* temperature);
    void                refresh_watchdog(int timeout);
    void                enable_watchdog(bool enable, int timeout);
    unsigned int        frame_count() const { return _frame_count; }
    void                block_callback()         {_block_callback = true; }
    bool                callback_blocked() const { return _block_callback; }
    static const char*  base_name(const char* path);
private:
    bool                init();
    struct VersionInfo {
        sdvr_firmware_ver_t sdvr_firmware_version;
        sdvr_pci_attrib_t   pci_attrib;
        string              model;
        string              firmware_version;
        string              firmware_released_date;
        string              boot_version;
        string              boot_release_date;

    };
    struct StreamInfo {
        StreamInfo() : is_enabled(false),
                       video_format(SDVR_VIDEO_ENC_NONE)
        {}
        bool                         is_enabled;    // is the encoder enabled for this stream
        sdvr_video_enc_chan_params_t video_enc_params;   // the stream's video encoder parameters
        sdvr_venc_e                  video_format;       // the video codec for this stream
    };

    void                board_connect();
    void                board_read_configuration();
    void                board_upgrade_firmware();
    // After successful connection to the board, we create one
    // encoder channel and configure one OSD to display date and time
    // at the bottom right corner. NOTE: The OSD item is hidden at this point
    void                camera_create();
    void                change_encoder_if_needed(int stream_id, sdvr_venc_e encoder);
    sdvr_chan_handle_t  get_camera_handle() const { return _camera_handle; }

    bool                isRTSP_stream_enabled(int stream_id);
    bool                is_stream_enabled(int stream_id)     const { return _stream_info[stream_id].is_enabled; }
    bool                is_stream_initialized(int stream_id) const { return _stream_info[stream_id].video_format!= SDVR_VIDEO_ENC_NONE; }

    // This function validates the HD and SD video standards that
    // were given as part of the Options with the one supported by
    // the IP-Camera and then it initializes the video standards in
    // _board_settings data structure.
    // This function must be called before calling board_connect()
    void                set_board_video_std();
    void                set_fw_date_time();
    // Used in one of test modes, read snaphot from disk
    char*               test_snapshot(const char* filename, unsigned int* size);

    char*               send_raw_command(const string& subsystem, const char* buffer, int buffer_size, ostream& reply, unsigned int* response_size);
    static const char*  flash_device(const char* filepath); // get device name from filepath

    bool                test_frame_verify(sdvr_av_buffer_t *buffer);
    bool                sdk_error(sdvr_err_e err);
    bool                is_initialized() const { return _initialized; }
    char*               serial_number();

    static const char* STRETCH_SERVER_HOME_ENV_NAME;

    bool                  _initialized;
    sdvr_board_attrib_t   _board_attrib;
    sdvr_board_config_t   _board_config;    // The board configuration. (i,e number of
                                            // video in ports, sensors, relays, etc.
    sx_uint32             _board_index;     // The board index for IP-Camera is always zero.
    sdvr_board_settings_t _board_settings;
    sdvr_chan_handle_t    _camera_handle;   // The SDK video-in channel handle.
                                            // Note that in IP-Camera there is only one
                                            // video-in port
    sdvr_video_std_e      _video_std;       // The video standard of the camera.
    Mutex                 _semaphore;       // A semaphore to synchronize receiving and
                                            // sending of the snapshot and for raw command replies
    Mutex                 _sdk_lock;        // lock around all SDK calls, to serialize them.
    Options               _options;         // Various start-up options to the SDK
    Server*               _server;          // The CGI server
    sdvr_av_buffer_t*     _callback_buffer; // The buffer holding the SDK A/V buffer
                                            // containing the snapshot or raw command reply
    StreamInfo            _stream_info[Stream::COUNT];
    VersionInfo           _version_info;    // The firmware and boot-loader version information.
    unsigned int          _drop_count;      // current drop count
    unsigned int          _frame_count;     // total frame count

    bool                  _block_callback;
    int                   _sensor_rate;

    static const char*    _local_tar_gz;
    static const char*    _startup;


}; // class SDKManager
} // namespace CGI

#endif
