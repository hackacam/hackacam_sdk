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

#ifndef STRETCH_SDVR_SDK_H
#define STRETCH_SDVR_SDK_H

#include <stdio.h>
#include <stretch_version.h>

/****************************************************************************
  PACKAGE: sdvr_sdk Stretch DVR SDK

  DESCRIPTION:

  SECTION: Include
  {
  #include "sdvr_sdk.h"
  }

  SECTION: Introduction
  Stretch has developed a reference design for PC-based and embedded
  DVRs and for IP cameras. The architecture of a DVR (either PC-based or embedded) 
  or IP-camera using components supplied by Stretch is as follows:
  {
         Host                                 Hardware Codec
                                              consisting of
       Application                          one or more Stretch
           |                                  chips running
          SDK                                 the Stretch-developed
           |                                      Codec
        Driver                                    Firmware
           |----------------------------------------|
                   Host-to-hardware interface
  }
  The Stretch (SDVR) hardware consists of one or more Stretch processors
  running the @Firmware@.
  Together, the hardware and the firmware are responsible
  for encoding, decoding, and interfacing to the cameras, microphones,
  sensors, and relays, and communicating with the host.

  On the host there is a @Driver@ that handles the
  low-level communication with the firmware. Sitting on top of the
  driver is the @SDK@, which provides an Application-level Programming
  Interface (API) to the SDVR hardware. It is through this API that
  an application talks to the hardware. The SDK provides the
  ability to:

    Discover the capabilities of the hardware CODEC.

    Configure the hardware.

    Encode and decode video and audio.

    Control other capabilities like on-screen display, spot monitor, and so on.

  This document describes the Application Programming Interface (API)
  implemented in the SDK.
  
  Your Application can link with the SDK library either statically or dynamically.
  On Windows, use _dll.lib libraries to link dynamically. On Linux, you can choose
  to link with shared (.so) libraries or static (.a) libraries.

  If you need to rebuild the SDK, please read the host/README.txt file for build requirements.

  Throughout this document, we refer to the application that uses the
  SDK as the @Application@.

  SECTION: Function Groups
  The functions in the SDK API are divided into several major groups.

    @System Set Up@. Functions in
    this group enable you to initialize the SDK, set up the system
    parameters, and discover and initialize boards at start-up. When you want
    to exit the application, the APIs in this group allow you to
    shut down the system gracefully and to free all system resources.

    @Channel Set Up@. Functions in this group
    enable you to initialize each channel at start-up or to reinitialize
    a channel during operation.

    @Encoding@. Functions in this group allow you to start
    and stop encoding, and to receive encoded and raw audio/video (A/V) buffers.

    @Frame Buffer Field Access@. Functions in this group allow you to access
    different attributes associated with an encoded or decoded frame buffer
    that is received from the firmware. (i.e. timestamp, channel number, and so on.)

    @Decoding@. Functions in this group allow you to send encoded
    A/V streams to the decoder for decoding and display.

    @Video Display and Sound@. Functions in this group allow you to
    control raw audio and video streaming. Stretch provides a separate
    UI SDK library that helps you to display video frames in any regions
    within a display window handle. (See sdvr_ui_sdk.html for details.)

    @On-Screen Display@. Functions in this group allow you to
    control how on-screen information is displayed and to enable or disable
    on-screen display (OSD).

    @Spot Monitor@. Functions in this group allow you to control
    the grid pattern and channels appearing in the spot monitor output (SMO)
    as well as sending full size raw video frames to the video out port.

    @Encode Monitor Output@. Functions in this group provide
    functionality similar to the SMO except that the formatted  output is an encoded H.264
    video frame which is sent to the Application instead of being displayed on
    the spot monitor.

    @RS485, GPIO, and TWI Communication API@. Functions in this group allow you to
    control the RS-485 interface over which you can implement any PTZ
    protocol that works over RS-485. Additionally, in some board you can read and
    write to GPIO pins and TWI registers.

    @Sensors and Relays@. Functions in this group allow you to
    set and reset sensors and relays attached to the hardware.

    @Audio/Video File Container API@. Functions in this group allow you to
    save the encoded audio and video frames in to a file.

  Naming conventions. Functions and data structures start with @sdvr_@
  so as to avoid conflict with symbols in the Application.

  All enumerated types end with @_e@ and all data structures end with @_t@.

  SECTION: Important Notes about the SDK
  Note the following:

    The SDK and the firmware on the board is stateless, i.e. parameters
    used to configure the SDK and the firmware are not stored across
    sessions (a session starts with initialization and ends when the SDK
    is closed or the system rebooted). Remember that every time you call
    sdvr_sdk_init() at the beginning of your application you must
    reinitialize all the system and channel parameters after you connect
    to the hardware.

    To set the time in firmware, the Application needs to compute the
    number of seconds elapsed since January 1, 1970 UTC and to present it to
    each board using sdvr_set_date_time(). When this function is used,
    the firmware takes the date and time and sets it in hardware.
    It is required that the Application set the
    time whenever the time changes (e.g., Daylight  Savings Time). We also
    recommend that the Application monitor the time on
    the hardware, and if the drift between the host clock and the
    clock on the hardware gets beyond a certain limit, then reset the time
    on the hardware.

    Depending on the type of DVR (embedded or PC-based), the hardware
    may consist of one or more boards, each having one or more Stretch chips,
    and each running a copy of the CODEC firmware.
    If there are N boards in a system, the SDK numbers them from 0 to N-1.

    When configuring the system, the Application is required to set
    the default system-wide video standards and resolution. This is the maximum
    video resolution that is supported by the SDK. This video standard  will
    be used only in cases  when no cameras are connected to a video input ports
    at the time of board  connect. Lesser video
    resolutions can be supported, but the lesser resolutions can only be 1/4
    of or 1/16 of the maximum resolution. Other supported resolutions are 2CIF
    that is the same width but half the number of lines of maximum resolution and
    DCIF that is 3/4 of the width but 2/3 of the number of lines of
    maximum resolution. @NOTE: 2CIF and DCIF are always based on 4CIF or D1
    video standard.@

    If you mark a region as private, that region is blocked out in both
    the live and the encoded stream.

    When decoding a frame, you must enable raw video or SMO for the decoder
    channel in order to receive and display the decoded raw video frames.

    For any alarm detection (i.e., motion detection, blind detection, and so on)
    to be active, the camera must be either encoding video frames or streaming
    raw video.

    If the specified decimation and x,y coordinates of an SMO grid do not correctly
    fit on the monitor, the grid is not displayed.

    All the reserved fields on all data structures must be set to zero. In general,
    it is recommended that you memset the data structures to zero before initializing
    their fields and passing them to a function.

    The OSD font table must be changed only when there is no video stream is in progress.
    (i.e, no encoding, no raw video streaming, and so on.)

    OSD features are supported by two different sets of APIs. One is flexible OSD
    (FOSD), and the other is fixed OSD.

    The raw video frame width is always truncated to be multiple of 16.

    The recorded MOV file does not support recording mono audio.
    In this case the SMF format must be used to record a camera with Mono audio.

    The maximum allowed encoded audio and video frame buffer count varies depending
    on the board. It maybe much less than the suggested one. Use sdvr_get_board_attributes()
    to decide what is the best number or call sdvr_get_sdk_params() to get the default
    values.

    The live video timestamp is based on 100Khz clock.

    To decode and display the frames on the SMO, the encoded bit-stream timestamp must
    be set based on 90Khz clock.

    The video timestamp retrieved from the MOV or SMF files are based on 90Khz.

  SECTION: Important Restrictions
  Note the following restrictions when using the SDK. The
  restrictions, apart from those explicitly noted, are not permanent
  and will be removed in future versions of the SDK.

    Only one Application can use the SDK on one host. Multiple
    copies of the SDK and associated driver cannot reside on the same host.
    This is a permanent restriction.

    Currently it is recommended to call any SDK API which requires
    communication with Stretch boards from the main application and not
    within a thread. See "Multi-threaded Application Support" section for
    more details.


    All the cameras on an SDVR card must have the same video standards and camera
    resolution although on some DVR boards it is permitted
    to have a mixture of SD and HD video standards. For example, it is permissible to have
    SD D1 NTSC and HD 1080I60 video standards on some boards but you may not connect
    SD D1 NTSC and SD D1/4CIF PAL nor it is allowed to connect SD D1 PAL and HD 720P60.

    The video standard of decoders must match one of the supported video standards.
    For example, it is not allowed to create an HD decoder  on a DVR board that
    only supports one of the SD video standards (PAL or NTSC).

    Currently up to five OSD texts per channel are supported.

    For OSD text only the ASCII character set is supported.
    Double-byte and Unicode character sets are supported only if the corresponding
    font table is loaded in the firmware.

    OSD text must be positioned such that it does not cause wrapping
    within the video frame. Otherwise, the behavior is not deterministic.

    Only H.264 decoding is supported on when connecting to a board prior to
    version 7 of the firmware.

    SMO dwell time field is not supported. This means only one video channel can be
    specified in any SMO grid.

    There can be a maximum of 4 instances of the same channel (encoder or decoder) on
    a SMO port.

    A channel must be streaming video (encoder, raw, or SMO) in order for
    any of the alarm detection occurs.

    The three classic resolution CIF, 2CIF and 4CIF are only supported for
    streaming raw video frames to the host Application and only by firmware
    versions prior to 7.x.

    It is recommended to limit the number of video overlay objects to 64 per DVR system although
    more maybe added depending on the availability of memory on the DVR board.

    Currently the video overlay objects only support YUV-4-2-0, SDVR_RAWV_FORMAT_RGB_565
    (little endean and big endean) and SDVR_RAWV_FORMAT_RGB_655 video format.

    H.264-SVC video encoder does not support DCIF, HALF, 2CIF decimation.

    Decimation 2CIF is not supported if the current video standard is set to D1.

    Decimation HALF is not supported if the current video standard is set to 4CIF.

    Currently the firmware requires that all the encoded video frames
    to have the same video resolution during any one session of decoding.
    This means that the video frames size should remain the same size as when
    sdvr_set_decoder_size() was called prior to enabling the decoder by
    calling  sdvr_enable_decoder() until it was disabled.

    Currently SDVR_BITRATE_CONTROL_CBR_S is only supported by H.264 AVC and
    SVC video CODECs.

    Currently getting or setting of any of the video-in image parameters is not
    supported by the HD boards.

    Privacy blocking is not supported by firmware version 7 for decoder channels.

    Currently EEPROM APIs can only be used in conjunction with Cryptographic EEPROM.

    The size for all encoder types needs to be a minimum of 64x64.

    For all encoder types, the width needs to be a multiple of 16 and the
    height needs to be a multiple of 2.

    7.x firmwares don't support SDVR_RAWV_FORMAT_YUV_4_2_2 and SDVR_RAWV_FORMAT_YUYV_4_2_2i
    raw video formats.

    7.x firmwares don't support SDVR_VIDEO_ENC_MPEG2 CODEC.

    MPEG4 encoder in 7.x firmwares does not support SDVR_BITRATE_CONTROL_CONSTANT_QUALITY.

    sdvr_set_chan_codec_pe() is currently only supported by boards including S6 chip-sets.

    Only single channel (0) and single stream (primary) host encoding is supported.  Only
    H.264 encoding is supported for host encoding.  OSD, FOSD, SMO, HMO, audio and alarms
    are not supported with host encoding.

  SECTION: Format of data buffer exchange between SDK and Application

  sdvr_av_buffer_t is used to exchange audio and video frames as well as
  other data buffers such as analytics between the SDK and the host Application.

  The raw A/V frame buffer can be
  either from the live or decoded video stream.
  In most DVR boards, the format of a raw video is YUV 4:2:0, but some DVR boards
  may support YUV 4:2:2. The raw video format can be determined by examining
  the frame header.
  A raw video frame is of type sdvr_av_buffer_t that
  consists of a header followed by a payload. The payload includes the Y, U, and V
  planes of the YUV frame.

  The format of a raw audio is PCM and the audio rate can be set when calling
  sdvr_board_connect_ex(). A raw audio frame is of type sdvr_av_buffer_t,
  which consists of a header followed by a payload.

  The encoder encodes the incoming video and audio
  frame-by-frame and each encoded
  frame is sent to the host. A frame consists of a header generated by
  the encoder and a payload. The header format is Stretch proprietary
  and has information that may be relevant to the Application (e.g.,
  whether motion was detected). The header format is sdvr_av_buffer_t described later
  in this document.

  There are a number of A/V buffer APIs that allow you to access the fields
  within the sdvr_av_buffer_t. Refer to "Frame Buffer Field Access API" for a list
  of these APIs and their usage.

  @NOTE: You should not directly access the
  fields in this data structure. Instead, use the functions specified
  in the "Frame Buffer Field Access" group.@

  The payload format contains the video or audio stream in
  elementary stream format. Therefore, if the video or audio payload is stored in
  its own file, it can be played by any player that supports the elementary
  stream format and has the corresponding decoder. There is an exception for alarm
  values frame types; in this case,  the payload contains the alarm values per
  macro blocks of the video frame.


  SECTION: Using the SDK API
  This section provides a high-level overview of the various DVR
  Application tasks and the functions used to accomplish those tasks.

  SUBSECTION: SDK and Board Initialization and Set-Up
  The Application needs to perform the following tasks during
  initialization and set-up.

    Initialize the SDK using sdvr_sdk_init().

    Load firmware into every DVR board that needs to be connected by
    calling sdvr_upgrade_firmware(). This is required only in cases when
    no firmware is
    burned onto the board, or when you want to use a different firmware than
    that already burned. You can get the board type as well as other PCI information
    in order to decide which version of firmware to load by calling
    sdvr_get_pci_attrib().

    For a PC-based DVR, get the number of boards using sdvr_get_board_count().
    For an embedded DVR, the board count is almost always 1.

    For both PC-based and embedded DVRs, get the board attributes using
    sdvr_get_board_attributes().

    Set the amount of memory allocated for communication with the
    hardware using sdvr_set_sdk_params(). We recommend that you use default
    video buffer settings for that call sdvr_get_sdk_params(), and only
    change the fields of interest to you. During development, we
    recommend that you enable debugging using many different flags in the SDK parameter
    structure, and specify a log file to record tracing of communication between
    the Application and the DVR firmware.

    Connect to each board using sdvr_board_connect_ex() and set various board
    system settings. You must specify the default SD and HD
    video standard and maximum resolution. (You can get a list of
    supported video standards from sdvr_get_board_attributes().) Additionally,
    specify whether the host PC should perform start code emulation for H.264
    CODECS. For performance reasons, it is highly recommended to always set
    this field to 1 except for the embedded Applications which must be set
    to 0. (See sdvr_board_settings_t for all the available system settings.)
    This establishes a connection to the board for further control and data
    communication.

    If required, the SDK, driver, boot loader, and firmware versions can be obtained
    using sdvr_get_sdk_version(), sdvr_get_driver_version() and
    sdvr_get_firmware_version(). Stretch will publish a matrix of which
    SDK, driver, boot loader, and firmware versions are compatible. This information
    can be used to check that compatible versions are used.

    Get the capabilities of each board, i.e., the number of cameras it
    can have, the number of sensors, etc. using sdvr_get_board_config().

    Set the date and time in the firmware using sdvr_set_date_time().
    Subsequently, the Application should periodically monitor the
    time on the firmware using sdvr_get_date_time(), and if there is
    drift between the host and the firmware clock, reset the clock on
    the firmware using sdvr_set_date_time().

    Set the callback function for the SDK to call when an error is
    encountered in the firmware
    using sdvr_set_signals_callback(). This is not mandatory, but is a very
    useful tool for debugging.

    Set the callback function for the SDK to call when sensors are
    triggered using sdvr_set_sensor_64_callback(). This is mandatory if the
    Application wants to be notified about sensors being triggered.

    Set the callback function for the SDK to call when video alarms are
    triggered using sdvr_set_video_alarm_callback(). This is not mandatory as
    you can get some of this information from the A/V buffer header, but we
    highly recommend that you register a callback for video alarms.

    Set the callback function for the SDK to call when AV frames are available
    from the hardware using sdvr_set_stream_callback(). This callback
    function is not mandatory, as described later.

    Set the callback function for the SDK to call send AV frames confirmation
    from the DVR Host Application to the DVR firmware
    using sdvr_set_confirmation_callback(). This callback
    function is not mandatory and only is used in conjunction with decoders
    or sending raw video to video out port.

    If you need to display OSD text other than English language, load
    the corresponding font file using sdvr_osd_set_font_table().

    If necessary, set the watchdog timer in the hardware using
    sdvr_set_watchdog_state_ex().
    The watchdog timer is used to reset the
    entire DVR system to prevent it from hanging. If the software on the
    host and the firmware is alive, then the watchdog should be periodically
    reset before it expires (the current value can be obtained using
    sdvr_get_watchdog_state()).

    If necessary, run diagnostics on the hardware using sdvr_run_diagnostics().
    The diagnostics run pertain only to the Stretch Chips-part of the
    system, as described later.

  SUBSECTION: Multi-threaded Application Support
  Although it is possible to use Stretch SDK in a multi-threaded
  application but it is highly discouraged as there are cases that
  may cause difficulties. As a result, we recommend that you redirect
  all the Stretch SDK calls to your main application which forces serialized
  calls to the firmware. Like any client/server environment whether your API
  call is made from the main application or multiple threads the actual request
  to the Stretch firmware is executed in serial.

  There are exception to the above stated limitation where you can safely
  call some of the SDK APIs from a thread. In general any API that does not require
  a returned response from the firmware is thread safe.
  This means most of the A/V buffer management APIs can be called from any
  thread in order to get, record, send, or manipulate any of A/V buffers with
  the exception that A/V buffers associated with an encoder or decoder
  channel should not be released from a thread while the same channel is
  being destroyed. Additionally, it is safe to call file recording APIs
  from any thread.

  To summarize, you can call any API needed to record or playback A/V buffers
  from any thread but the APIs that create, setup, enable streams, and destroy
  encoder/decoders channels should not be used from multiple threads.

  SUBSECTION: Channel Set-Up
  Before using the system, you must create different channel type to perform
  encoding or decoding tasks. An encoder channel provides encoded video and
  audio and raw video and audio data. Decoder channel provide decoding of an encoded
  video frame. Additionally, decoder channels are also used to play raw video frames
  that are sent from the Application on the SMO monitors.

  To create and configure each channel type, the Application needs to perform the
  following tasks during initialization.

    If there are N cameras supported by the system, then channels 0 to
    N-1 are encoding channels. Each encoding channel number corresponds to the
    camera position number in the back panel.
    Set each of these channels as encoding
    channels unless you want to leave some cameras unconnected and
    use the processing power for decoding. To create an encoding
    channel, use sdvr_create_chan_ex(). You can dynamically
    turn off encoding channels to free up processing power for decoding.
    Additionally, if there are channels created with secondary encoding support,
    you may not be able to connect to all the cameras for recording. You can
    also specify audio encoding for all cameras that have corresponding
    audio channels connected. (The audio rate is set at the time of connecting
    to the board by calling sdvr_board_connect_ex().) Additionally, you may want to create some
    streaming only channels that do not need to have any encoding capabilities.
    These are called HMO- or SMO-only channels. To create HMO- or SMO-only
    channel, set the channel type to SDVR_CHAN_TYPE_ENCODER
    and the primary video format to SDVR_VIDEO_ENC_NONE.
    @Note: The HMO- or SMO-only channels take away from the maximum number
    of encoding channels that are allowed by the system. This means that if there are N
    cameras supported and you create M HMO- or SMO-only
    channels, you can only create N minus M number of encoding channels.@

    There maybe scenarios where you do not have enough processing power on one PE to
    perform both primary and secondary encoding for all channels on that PE, but
    there are other PEs that are not heavily loaded. In such cases, you can
    direct the firmware to move the secondary encoder for a specific channel to
    another PE. This can be done by calling sdvr_set_chan_codec_pe() before the
    channel is created.

    If necessary, you can change the video-in image parameters (i.e., hue,
    saturation) using sdvr_set_video_in_params(). These parameters can
    be changed only for encoder or HMO- or SMO-only channels.

    Each camera only supports displaying and encoding of video frames that are
    the same as the video standard detected at the time of channel creation. In case
    no video signal is detected when the camera was being created, the default video
    standard specified on the board connect is used. You can query the video
    standard associated with each camera by calling sdvr_get_chan_vstd(). (Note: some
    boards allow mixing of SD and HD video input, but each video input board on
    those boards only allows one video standard. Refer to the DVR board's specification
    document for more information.)

    After encoding channels are set up, set up decoding channels using
    sdvr_create_chan_ex(). The maximum number of
    decoding channels depends
    on the processing power left after setting up the encoder channels.
    If the call to sdvr_create_chan_ex() returns successfully, then the decoding
    channel was set up. Otherwise, there is no more processing power available and
    the decoding channel could not be set up. NOTE: The actual processing power
    calculation is done when encoders or decoders are enabled. This implies that,
    even though decoders  were able to  be created, they may fail to be
    enabled depending on the number of  decoders and encoders that were created at
    the time channels were set up. Otherwise, there is no more processing power available and
    the decoding channel could not be set up. Currently, the decoding channel number
    indicates in which Stretch chip the decoder is created but the actual number of
    decoding  channel that can be created on each Stretch chip depends on the chip model.
    As with encoder channels,
    decoder channel numbers run from 0 to N-1. Where N is the maximum number of
    decoders supported as returned by sdvr_get_board_config().

    Similar to cameras that only support encoding of one video standard, each decoder
    can only decode similar video standard frames, e,g, SD or HD video frames. You may
    create HD, SD, or mixture of HD and SD decoders on each DVR board depending on
    the video standards supported by that board. You can call sdvr_get_supported_vstd()
    to get a list of supported video standard by each DVR board and determine what type
    of decoders can be created.

    You may send raw video frames from the Application to be displayed on a
    SMO display. In such cases, you create a decoder channel using sdvr_create_chan_ex()
    and specify the video decoder of SDVR_VIDEO_ENC_RAW_YUV_420. Once this channel type
    is created successfully, you can assign this channel to a SMO grid (refer to
    section "Spot Monitor Output" for setting up the SMO grid), enable the decoder,
    and start sending packed YUV 4-2-0 video frames (refer to section "Decode" for
    details of decoding).

    When you set a channel to be encoding or decoding, you receive
    a unique channel handle of type sdvr_chan_handle_t. Subsequently,
    you use this handle in all function calls requiring a channel
    identifier. You can always extract the channel number, type, and the board
    number that this channel is created on by calling sdvr_get_chan_num(),
    sdvr_get_chan_type, and sdvr_get_board_index() respectively.

    Existing encoding or decoding channels can be destroyed at any time by
    calling sdvr_destroy_chan(). This call lets you rebalance
    channel types as needed.

    Set or change the video encoder of an encoding or decoding channel using
    sdvr_set_chan_video_codec().

    Set the parameters of each video encoding channel using
    sdvr_set_video_encoder_channel_params().

    Set the parameters of each alarm-triggered video encoding channel using
    sdvr_set_alarm_video_encoder_params().

    Set the parameters of each audio encoding channel using
    sdvr_set_audio_encoder_channel_params(). (Currently, there are no audio
    encoder parameters but the audio rate and whether to encode stereo or mono
    is set at the time of connecting to the board by calling sdvr_board_connect_ex().)

    Set regions for motion and blind detection, and privacy regions
    using sdvr_set_regions_map(), sdvr_set_regions(), or sdvr_add_region().

    Set how often you want to receive motion values buffers using
    sdvr_set_motion_value_frequency(). By default, motion values buffers
    are sent when a motion alarm is triggered.

    You can associate application-defined data with each encoding or
    decoding channel by calling sdvr_set_chan_user_data(). This data
    can be retrieved at a later time by calling sdvr_get_chan_user_data().

    Enable motion, blind, and night detection using
    sdvr_enable_motion_detection(), sdvr_enable_blind_detection(), and
    sdvr_enable_night_detection(), respectively.

    Enable privacy regions using sdvr_enable_privacy_regions().

  The hardware is now set up for encoding, decoding, and display.

  SUBSECTION: Video Analytics and Privacy Blocking
  Stretch provides variety of video frame analysis to be used with various
  video alarm detection, as well as blocking different regions within
  video frames for privacy. These video frame analytics include the Motion
  and Blind Detection within a
  video frame according to some user defined Regions of Interest (ROI) that
  are based on Macro Blocks (MB). As well as the Night Detection that applies
  to the entire video frame. The size of a MB is 16x16 pixels. The
  ROIs are always marked within a 1:1 decimation of a video standard size. Refer
  to sdvr_set_regions_map() for more information.

  To start video alarm detections or to set up privacy blocking, you
  must specify ROIs within a 1:1 decimation of a video standard size for each
  encoder channel.
  Do this by calling either sdvr_set_regions_map() or
  sdvr_set_regions(). By default, the entire video frame is used for any of
  the alarm detection.

  The next step is to enable different alarm detections based on the currently
  specified ROIs for each channel and a specific threshold.  Enable motion, blind,
  and night detection using  sdvr_enable_motion_detection() or sdvr_enable_motion_detection_ex(),
  sdvr_enable_blind_detection(), and  sdvr_enable_night_detection(), respectively.
  Enable privacy regions using sdvr_enable_privacy_regions().
  After alarm detection is enabled, the Application gets notified through
  the alarm callback as alarms are triggered according to the given threshold. By default
  all of the video alarms are disabled.
  Additionally, you receive different alarm values
  with every video frame (encoded or RAW) as part of the video frame buffer.
  You may choose to receive  motion values every N video
  frames by calling sdvr_set_motion_value_frequency().

  @Note:@ Motion values are always generated once an alarm is detected by
  the firmware regardless of the setting in sdvr_set_motion_value_frequency().

  Having a motion value buffer, you can perform more detailed analysis
  by calling sdvr_motion_value_analyzer() and passing it different list of
  ROIs to detect motions in various regions of the current video frame.

  In some cases, running video analytics may affect the system performance.
  In these cases, the video analytics can be disabled per channel by calling
  sdvr_enable_analytics() to get better system performance.
  By default, video analytics are enabled.

  In addition to Stretch-provided video analytics, the Application
  can perform custom video analytic.
  In this method, the Application must rebuild the DVR firmware to
  overwrite the video analytics module stub. The Application can communicate
  with this module using sdvr_set_va_data() and sdvr_get_va_data().
  Subsequently, the video analytics
  data are sent to the Application as SDVR_FRAME_APP_ANALYTIC frame types.
  As these data buffers are received in the A/V callback function, the Application
  can retrieve them by calling sdvr_get_stream_buffer() with frame type of SDVR_FRAME_ANALYTIC.
  It then needs to verify that the frame type in the buffer is of SDVR_FRAME_APP_ANALYTIC.
  The data in the payload of this message is sent by the provided application
  video analytics module in the firmware and is not known to Stretch.

  SUBSECTION: Encode
  The Application needs to perform the following tasks during
  encoding.

    Enable the encoder for each channel using sdvr_enable_encoder().

    Get a frame of encoded video for a particular channel using
    sdvr_get_stream_buffer(). If a callback was registered for AV frames,
    then the Application should have received one or more callbacks
    with information about the channels for which encoded frames are
    available. (See sdvr_set_stream_callback()
    for a usage example of such a callback). That information can be used to
    determine which channels have
    data and to request frames only from those channels. If, however, a callback
    was not registered, sdvr_get_stream_buffer() can be used as a polling
    function - when channels have encoded AV frames this function returns
    valid buffers, but returns appropriate error codes for channels
    that do not have new frames available.

    Stretch has implemented a one-copy buffer management policy. The buffers
    required to hold incoming data from the board (and data going to the
    board) are allocated in the SDK. The driver, however,
    allocates a few buffers that are contiguous in physical memory to
    enable efficient DMA. Data coming from the board is first stored in the
    driver buffers and then copied over to the SDK buffers. Similarly,
    data going to the board is first stored in the SDK buffers and then
    copied to the driver buffers before being DMAed to the board.

    It is important to release buffers obtained from the SDK using
    sdvr_release_av_buffer(). The SDK then recycles these buffers
    and uses them for holding future incoming frames. Under no
    circumstances should a buffer be freed by the Application.
    Also, holding on to the buffer for too long causes the SDK
    to run out of buffers, and frames are lost. It is therefore
    important that enough buffers be allocated and that buffers are
    released in a timely manner.

  It is the responsibility of the Application to save the data to disk.

  SUBSECTION: Decode
  The Application needs to perform the following tasks during decoding.

    Set the encoded video frame size for each channel
    to be decoded by calling sdvr_set_decoder_size().

    Enable the decoder for each channel using sdvr_enable_decoder().

    In keeping with Stretch's buffer management policy whereby the
    SDK manages all the buffers, the
    Application must request a free frame buffer from the SDK using
    sdvr_alloc_av_buffer().

    The Application should fill the payload associated with this buffer
    with encoded data from the disk. The pointer to the payload of the
    buffer can be accessed by calling sdvr_get_buffer_payload_ptr().
    If the video type of the decoder channel is SDVR_VIDEO_ENC_RAW_YUV_420,
    the Application needs to fill the Y, U, and V plane buffers associated
    with the 4-2-0 raw YUV buffers. A pointer to each YUV plane within
    the allocated buffer can be accessed by calling sdvr_get_buffer_yuv_payloads().

    @NOTE: In the event that default send buffer size associated with each
    decoder is not large enough to send YUV 4-2-0 or HD encoded buffer,
    you should call sdvr_create_chan_ex() and set the send buffer size
    of buf_def parameter to be large enough.@

    In case decoding audio to be played on the board, you must call
    sdvr_set_decoder_audio_mode() to specify the encoded audio data is Mono
    or Stereo. This call is not needed if no audio is being decoded.

    After this buffer is full, it can be sent to the
    hardware for decoding using sdvr_send_av_frame(). Decoder buffers are
    released as part of the call to sdvr_send_av_frame(). In the event that
    a decoder buffer is acquired, but needs to be released without sending,
    you can call sdvr_release_av_buffer().

    The decoded raw video frames can either be displayed on the SMO by calling
    sdvr_smo_set_grid() or requested to be sent to the Application
    for displaying on the host monitor by calling sdvr_stream_raw_video().
    Once the streaming of raw video frames is enabled for the decoded channel, the
    raw video frames will be sent for the corresponding decoder via the
    av_frame_callback function (refer to the next section, Raw Video and Audio Data,
    for detailed information).

  SUBSECTION: Raw video and audio data
  The SDK provides raw (unencoded) audio and video for each channel
  as a separate stream to the Application. These raw A/V frames can be
  either from the live or decoded video stream. For live video, you may choose
  to have two different raw video streams with different video resolutions.
  The Application can request to receive the raw audio frames associated with
  the live
  audio channels, or decode channels or it can redirect the raw audio frames to be
  played from any of the audio-out ports that may  exist on your DVR board.

  The Application needs to do the following to get raw live video and audio data.

    Enable streaming of video and audio from the hardware or decoder to the host
    using sdvr_stream_raw_video() or sdvr_stream_raw_video_secondary()
    and sdvr_stream_raw_audio() calls.
    Although video for various channels can be enabled for streaming, it
    makes sense to enable streaming for only one audio channel
    (although the SDK supports streaming of multiple audio channels).
    To conserve communication bandwidth between the host and the board, we
    recommend that raw video streaming be enabled only for channels
    that are being displayed, and not for all channels.

    Register a callback function using sdvr_set_stream_callback()
    so that the Application can be informed
    when raw audio and video frames are available. This is not
    strictly required (see preceding discussion).

    Call sdvr_get_stream_buffer() to get audio/video
    frames from the SDK.
    The format of raw video is YUV 4:2:0, and will be
    received as one buffer consisting of three separate Y, U, and V planes.
    Use appropriate rendering and sound playback software
    and hardware to display the video and to play the sound. Video for
    each channel is obtained separately, and it is the responsibility of the
    Application to display the video in its appropriate window. Typically,
    the Application creates tiled windows for each channel to be
    displayed, and the video for each channel is rendered in its own
    window. If interested, Stretch provides a UI SDK library that helps you
    to display video frames in any regions within a display window handle.
    (See sdvr_ui_sdk.html for details.)

    Release the frame buffer obtained in the previous step using
    sdvr_release_av_buffer().
    Raw video buffers are large, so there will
    usually not be too many of these buffers per channel. Therefore, it
    is extremely important that these buffers be promptly released.

  The Application needs to do the following to play raw live or encoded audio
  associated with an encoder or decoder channel through any of the audio-out ports.
  (This functionality is only available if your
  DVR board is equipped with one or more audio-out port.)

    Enable playing of audio through any of the audio-out ports by calling
    sdvr_enable_audio_out(). If the handle passed to sdvr_enable_audio_out()
    corresponds to an encoder channel, the live audio associated with that
    channel will be played immediately; if the handle corresponds to a decoder
    channel, then the encoded audio associated with that decoder will be
    played when the decoder channel is enabled and the encoded audio
    frames are sent to the board. For the live or decoded
    audio to be played, there is no need for the corresponding video to
    be enabled.

  The Application needs to do the following to play raw linear PCM audio
  data through any of the audio-out ports.
  (This functionality is only available if your
  DVR board is equipped with one or more audio-out port.)

    Call sdvr_start_audio_out() to initialize the audio output port in order
    to send raw linear PCM audio data. After the audio output port is initialized,
    you should call sdvr_alloc_av_buffer(), fill it with raw linear PCM audio data, and send
    it to the board by calling sdvr_send_av_frame(). NOTE: To send
    encoded G711 audio data, you should call sdvr_enable_audio_out().

  SUBSECTION: On-Screen Display (OSD)
  Each encoder or decoder channel can display a number of OSD text items
  at any position in the video frame. (See "Important Restrictions"
  for the supported number of OSD text items.) To show OSD text item,
  the Application needs to do the following:

    It first needs to configure each OSD text item using sdvr_osd_text_config_ex().
    The "osd_text_config" data structure allows the Application to
    specify the text string, its location, whether date and time should
    be appended to the string, and if so, the style in which the date and
    time are displayed. Stretch provides multi-language support through multiple
    font tables. You can load a non-English language font by calling
    sdvr_osd_set_font_table(). If no font table is loaded, the default English
    font is used.

    After an OSD text item is configured, it can be shown or hidden using
    sdvr_osd_text_show(). In
    the case of encoder channels, OSD text is blended into the video before the video is
    encoded. Therefore, the OSD text present at the time of encoding and is
    displayed during decoding. In the case of decoder channels, OSD text is added
    after the video frame is decoded. Therefore, it is possible to have two different
    OSD text items on the decoded frame, one when it was encoded and the other
    when it is decoded. You may specify that an OSD text item is only
    displayed on any of the SMO ports on the board using sdvr_osd_text_config_ex().
    This text will not be displayed in the raw or encoded video.

  SUBSECTION: Spot Monitor Output (SMO)
  Stretch supports displaying live video output from each encoder channel or the
  decode frames output of each decode channel anywhere within the spot monitor display.
  The combination of SMO display location and its video decimation for each channel is
  called an SMO grid. Each SMO grid can be defined and enabled by calling
  sdvr_smo_set_grid().  You can achieve different
  display pattern by placing the channel video output in various display locations.
  Additionally, an SMO port may have a video out capability that means you can send
  raw video frames from the DVR Host Application to be displayed on that
  SMO port. It is possible to overlay these video frames on top of each other. In this
  case, you must specify the order of the layers. You can get the capabilities,
  screen size, supported raw video format, and video standards associated
  with each SMO port using sdvr_get_smo_attributes(). If your SMO port supports multiple
  video standard, you can change the video standard by calling sdvr_set_smo_video_vstd().
  Changing of SMO video standard is allowed as long as no channels (encoders or decoders)
  are created and no other attributes of the SMO is changed. OSD text can be added to SMO
  streams by using the Flexible On Screen Display (FOSD) API.

  To enable SMO, the Application needs to do the following:

    Specify the grid pattern using sdvr_smo_set_grid().
    The SMO grid pattern is flexible and is defined by the user. The size of
    each tile can be set independent of any other tiles and it can
    be either enabled or disabled. When enabled, there are one or more
    channels to display at this tile. If disabled, the tile is not used.
    The channel specified at each location can be either an encoding
    or decoding channel. If it is an encoding channel, live video is
    displayed, and if it is a decode channel then playback video is displayed.
    The rate at which live video is displayed is based on the SMO's video standard.
    Whereas the playback display of decoded channel is based on the timestamp
    associated with the encoded buffer sent for decoding. This timestamp should be
    based on 90Khz clock. Setting the timestamp associated with decoded frames to
    zero (0) forces the frame to be displayed immediately.

    Setting the SMO configuration using sdvr_smo_set_grid() enables displaying
    of SMO. No further action is required on the part of the Application.

  To rotate the display between multiple channels (i.e. implement "dwell time")
  the host application must implement the logic to do so, by disabling one channel
  and enabling the next one every time period.

  You may send raw video frames from the Host Application to be displayed on any SMO
  port with video out capability. On each SMO port, you can define up to 255 different
  video overlay objects of varying size. Each one of these overlay objects can receive
  different raw video frames, they can be moved around the SMO display, and be shown or hidden
  independently of each other. These video overlay objects can be used for many purposes
  such as menu items, mouse cursor, picture in picture, and so on.
  Following are the steps required to create and display video overlay objects on the SMO
  display.

    Open a video output channel using sdvr_start_video_overlay(). The channel handle
    that is returned will be used for any subsequent call to send raw video
    frames. Add one or more video overlay objects using sdvr_smo_add_overlay(). This
    call returns an overlay ID that must be used for all subsequent calls to refer
    to this overlay. Alternatively, in the embedded environment, you can use
    sdvr_smo_add_direct_mem_overlay() to add overlay objects.

    After SMO overlay objects are created, you can display YUV frames in them,
    show or hide the overlay at anytime by calling sdvr_smo_show_overlay(),
    or move them around the SMO display using sdvr_smo_set_overlay_pos(). The order of these
    actions varies depending on the usage of the video overlay objects.

    Before sending any YUV frames to an overlay object, the
    Application must request a free frame buffer from the SDK using
    sdvr_get_video_overlay_buffer(). At this time, you must specify the size of the
    video frame and the position where the video frame should be displayed within the
    overlay object.

    The Application should fill the payload field of this buffer with
    raw YUV 4-2-0 video format. The YUV planes at the payload must
    be compacted meaning no spaces between the Y, U, and V plane buffers and
    their size must match the specified video format when sdvr_start_video_overlay()
    was called.

    After this buffer is full, it can be sent to the
    overlay object on the SMO port using sdvr_send_video_overlay(). The buffers are
    released as part of the call to sdvr_send_video_overlay(). In the event that
    a send buffer is acquired, but needs to be released without sending,
    you can call sdvr_release_av_buffer().

    You can be notified as each raw video frame is
    sent to the DVR board and processed by registering a confirmation
    callback using sdvr_set_confirmation_callback().

    To show the contents of an overlay object, you must call sdvr_smo_show_overlay().

    You can call
    sdvr_smo_clear_overlay() to clear buffers associated with a specific layer in the
    event you are displaying
    multi-layer video frames and are interested in clearing one layer before others.

    After you are done sending video frame, close
    the video output channel using sdvr_stop_video_overlay().

  You can mirror the layout of grids displayed on the HMO on any one of the SMO
  ports in your DVR system. The HMO layout can be redirected to only one SMO port
  at a time. This feature requires
  using of the Stretch UI SDK mode 2 and an SMO port that support SDVR_RAWV_FORMAT_YUV_4_2_0
  video overlays. Following are the steps required to mirror
  and redirect the HMO layout to a specific SMO display:

    Call sdvr_open_hmo_mirror() to enable HMO mirroring on a specific SMO port.

    Setup the HMO grids and start preview of raw video for each channel. Refer to
    "Stretch DVR Display SDK (sdvr_ui_sdk)" documentation. (Note: you must use
    UI SDK mode 2.)

    Start streaming of raw video as described in section "Raw video and audio data".
    At this point the raw video for each channel on the HMO grids will be displayed
    on the host monitor and the specified SMO display.

    To redirect the HMO output to a different SMO port, you must first close the
    previous HMO mirror SMO port by calling sdvr_close_hmo_mirror() and then
    specify the new SMO port by calling sdvr_open_hmo_mirror() again.

    Note that the displaying of raw video on the SMO will not occur until
    the raw video is streamed for each channel on the HMO grids.

  HMO mirroring covers the entire SMO display. As a result, you should not use
  any other SMO grid displaying feature on the same SMO port. Although, it is
  permitted to use the overlay on the same SMO port with the exception of
  overlay zero.

  SUBSECTION: Encode Monitor Output (EMO)
  Stretch supports formatting and recording of the live video output from
  each encoder channel or the decoded frames output of each decode channel within
  a video frame of size equal to the current SMO video standard.
  This video frame is called an Encode Monitor Output (EMO), and the EMO frame belongs
  to the EMO channel.

  Creating and formatting of EMO video frames depends on whether firmware
  versions 7.0 or higher is being used or not.

  When using firmware version prior to 7.0:

  Before receiving EMO video frames in your Application, you must create an EMO channel
  by calling sdvr_create_emo() and enable its video encoder using sdvr_enable_encoder().
  Not all the DVR firmware support this type of channel, so you must first call
  sdvr_get_board_config() to determine if such a channel can be created using your current
  DVR firmware. You can change the video encoder parameters of an EMO channel by calling
  sdvr_set_emo_video_enc_params(). All video encoder parameters such as bit rate, frame rate,
  and so on can be changed, with the exception of video resolution. The video
  resolution is fixed, and its size can be read using sdvr_get_board_config().

  The combination of video location within an EMO video frame and
  its video decimation for each channel is called an EMO grid. Each EMO grid
  can be defined and enabled by calling sdvr_set_emo_grid(). You can achieve different
  display patterns by placing the channel video output in various display locations.
  You can get the capabilities and video frame size associated with an
  EMO channel using sdvr_get_emo_attributes(). If your EMO channel is capable of OSD,
  you can display up to five different texts on that video frame. Note: The
  displaying of OSD text is not related to streaming of any video on that EMO channel.

  You can stop receiving EMO frames without losing your existing EMO grid
  configuration by calling sdvr_enable_encoder() and setting its enable
  parameter to false.

  Before terminating the Application, you must disable all the EMO grids by
  calling sdvr_set_emo_grid() and setting its enable flag to false, disable the video
  encoder on the EMO channel by calling sdvr_enable_encoder() and setting its
  enable parameter to false, and destroy the EMO channel by calling sdvr_destroy_emo().

  When using firmware version 7.0 or higher:

  Before receiving EMO video frames in your Application, you must create an EMO channel
  by calling sdvr_create_emo_port() and enable its video encoder using sdvr_enable_encoder().
  Not all the DVR firmware support this type of channel, so you must first call
  sdvr_get_board_config() to determine if such a channel can be created using your current
  DVR firmware. You can change the video encoder parameters of an EMO channel by calling
  sdvr_set_emo_video_enc_params(). All video encoder parameters such as bit rate, frame rate,
  and so on can be changed, with the exception of video resolution. The video
  resolution is fixed, and its size can be read using sdvr_get_board_config().

  The EMO video frames are mirror of the SMO grids on the SMO port that is defined.

  You can stop receiving EMO frames without losing your existing EMO grid
  configuration by calling sdvr_enable_encoder() and setting its enable
  parameter to false.

  Before terminating the Application, you must disable the video
  encoder on the EMO channel by calling sdvr_enable_encoder() and setting its
  enable parameter to false, and destroy the EMO channel by calling sdvr_destroy_emo().

  SUBSECTION: Audio/Video File Container
  Stretch SDK provides a number of routines to facilitate saving of A/V frames
  into a A/V file container. These file containers vary from the standard types such
  as .mov (QuickTime version of mp4 file format) to the Stretch Multi-media File (SMF)
  format .smf. The purpose of these APIs is to simplify
  recording of the encoded
  A/V frames from a specific video/audio input to the file container as well
  retrieving them for playback.

  The A/V File container APIs are grouped into two sections:

  1) Saving/Recording to an A/V file container -

  Using this set of APIs, you can specify
  the location and type of the A/V container. As well as controlling the amount
  of A/V frames recorded in each container.

  Start recording of encoded A/V frames associated to a camera by calling
  sdvr_file_start_recording().
  As part of this call, you specify the camera handle and a number of recording policies.
  Additionally, you can optionally choose to enable the encoder associated  to this
  camera or call sdvr_enable_encoder() separately. Once the recording starts for a
  camera, the DVR SDK
  starts saving all of the encoded A/V frames into the requested container.
  In the event that it is required to create multiple files as requested
  in the recording policy, the
  SDK ensures that each file starts with a key frame.
  Stop the recording by calling sdvr_file_stop_recording().

  2) Playing back/accessing the A/V frames in the A/V file container -

  Using this set of APIs, you can retrieve one frame at a time from the A/V
  container. The encoded frame can be sent to the
  Stretch decoder and the result be displayed. See section "Decoding API" for
  how to decode encoded video frames.

  Before accessing the A/V frames in the A/V file container,
  the file must be opened for reading by calling sdvr_file_open_read(). This call
  requires you to specify the path to the file, its type, and whether you are interested
  to retrieve video, audio, or both video and audio frames. Up on success return,
  a handle to the opened file is returned that needs to be used
  for the subsequent file read calls.  By calling sdvr_file_get_avtrack_from_file() and
  using the returned file handle, you can find various information stored in the
  A/V file container such as the number of audio or video tracks and
  their CODEC types.
  Each call to sdvr_file_read_frame() returns the next A/V frame stored in the file.
  In the event the encoded frames are going to be sent to
  the Stretch Decoder, it is recommended to call sdvr_alloc_av_buffer_wait() in order
  to acquire a buffer then pass its payload to sdvr_file_read_frame(). (You can use
  APIs listed in the "Frame Buffer Field Access API" section to get more detailed
  information regarding each frame that was read.)

  If reading frames from a file that is being written to (Tivo Mode),
  sdvr_file_read_frame() may return SDVR_ERR_NO_MORE_FRAMES. This return code
  means currently there are not any new frames in the file but the file is not close. You should
  call sdvr_file_read_frame() again after waiting a few milliseconds.

  At the end, call sdvr_file_close() when you are done reading A/V frames or
  sdvr_file_read_frame() returned SDVR_ERR_EOF.

  SUBSECTION: Pan, Tilt, and Zoom
  Typically, the PTZ controller for a camera is connected to the processor
  implementing the PTZ protocol through an RS-485 interface. The Stretch
  processor does not have an RS-485 interface, so the Stretch reference
  design provides the mechanism to talk to the RS-485 port on the alarm
  I/O card.

  The SDK does not support any particular PTZ protocol. Instead, it gives
  access to the RS-485 interface through the RS-232C interface. The PTZ
  section of the SDK allows the Application to set up the RS232C port
  on the Stretch to talk to the RS-485. The following functions are available:

    sdvr_init_uart() allows the Application to set the baud rate,
    stop bits, etc. for the UART interface.

    sdvr_read_uart() returns up to 255 characters from the UART port.

    sdvr_write_uart() writes up to 255 characters to the UART port.

  SUBSECTION: Sensors and Relays
  Sensors are external inputs that can be triggered, for example, by a
  door opening. The SDK allows the Application to register a callback
  so that any time one or more sensors are triggered, the callback function
  is called. This callback is registered using sdvr_set_sensor_64_callback()
  and the type of the callback function is sdvr_sensor_64_callback. Some sensors
  are edge triggered whereas others are level sensitive. You can specify
  how each sensor should be triggered, as well as its initial enable status
  by calling sdvr_config_sensors_64(). You can also enable or disable an
  individual sensor at different times by calling sdvr_enable_sensor().
  The initial state of the sensors can be determined by calling
  sdvr_get_sensors_state_64() after they are configured and enabled.

  Relays are actuators that are activated by the Application. To
  activate or deactivate a relay, the Application calls
  sdvr_trigger_relay() with the proper value for the "is_triggered"
  flag.

  SUBSECTION: EEPROM Programming
  Stretch SDK provides a number of APIs for accessing the EEPROM on the DVR
  boards. The SDK supports reading and writing of
  up to 8 bytes of data on the EEPROM as well as setting a number of security
  privileges for accessing various zones within the EEPROM. There are three (3) different
  zones as described in sdvr_crypto_eeprom_zones_e on the EEPROM. Two are user
  accessible zone; whereas, the third one is only used for the EEPROM internal
  settings.

  Any user zone that needs to be secured can be password protected for
  reading and writing. This is accomplished by calling sdvr_eeprom_set_security().
  The password for any password protected zone is set by calling sdvr_eeprom_set_password().
  This password must be exactly 3 bytes long. Passwords must be
  verified before any reading or writing of the data in that zone is allowed.
  This verification is done by calling sdvr_eeprom_verify_password(). Once the password
  is verified, you can call sdvr_eeprom_write() to write up to 8
  bytes of data in to that zone. Additionally, you can read back the data by
  calling sdvr_eeprom_read().

  The final EEPROM configuration will be protected from ever being
  altered by any user once sdvr_eeprom_blow_fuse() is called.
  You must call sdvr_eeprom_blow_fuse() with caution
  as this operation is not reversible.

  SUBSECTION: System Shutdown
  The Application needs to perform the following tasks during
  shutdown:

    Disable encoding on all channels using sdvr_enable_encoder(), with the
    "enable" flag set to false.

    Disable decoding on all channels using sdvr_enable_decoder(), with the
    "enable" flag set to false.

    Disable all relays using sdvr_trigger_relay(), with the "is_triggered"
    flag set to false.

    Disconnect from boards using sdvr_board_disconnect(). This frees up
    all board-specific resources in the SDK and driver.
    @NOTE:  After you call sdvr_board_disconnect(), you must load the firmware
    by calling sdvr_upgrade_firmware() every time prior to reconnecting
    to the DVR board if no firmware is
    loaded into the board's non-volatile memory.@

    Close the SDK and free up all system resources using sdvr_sdk_close().
    Although disabling and destroying all the channels, as well as disconnecting
    from all boards is good programming  practice,
    it is not required because sdvr_sdk_close() performs these actions.
****************************************************************************/

#ifndef __H_SXTYPES
#define __H_SXTYPES


/****************************************************************************
    8-bit unsigned integer.
****************************************************************************/
typedef unsigned char  sx_uint8;

/****************************************************************************
    16-bit unsigned integer.
****************************************************************************/
typedef unsigned short sx_uint16;

/****************************************************************************
    32-bit unsigned integer.
****************************************************************************/
typedef unsigned int   sx_uint32;

/****************************************************************************
    64-bit unsigned integer.
****************************************************************************/
typedef unsigned long long sx_uint64;

/****************************************************************************
    Boolean value.
****************************************************************************/
typedef unsigned int   sx_bool;

/****************************************************************************
    true is a non-zero value
****************************************************************************/

#ifndef true
#define true 1
#endif

/****************************************************************************
    false is zero value
****************************************************************************/

#ifndef false
#define false 0
#endif

/****************************************************************************
    8-bit signed integer.
****************************************************************************/
typedef signed char               sx_int8;     // 8  bit signed integer

/****************************************************************************
    16-bit signed integer.
****************************************************************************/
typedef  short              sx_int16;    // 16 bit signed integer

/****************************************************************************
    32-bit signed integer.
****************************************************************************/
typedef  int                sx_int32;    // 32 bit signed integer

/****************************************************************************
    64-bit signed integer.
****************************************************************************/
typedef  long long          sx_int64;    // 64 bit signed integer
#endif

/****************************************************************************
    The largest un signed integer supported by the OS.
****************************************************************************/
#ifdef WIN32
typedef size_t      sx_size_t;

#else

#ifdef _LP64
typedef sx_uint64   sx_size_t;
#else
typedef sx_uint32   sx_size_t;
#endif

#endif

/****************************************************************************
    32 bit time value, the number of seconds elapsed since midnight 01/01/1970.
****************************************************************************/
#ifndef _TIME_T_DEFINED
typedef    long    time_t;
#define    _TIME_T_DEFINED
#endif


/****************************************************************************
  VERSION INFORMATION RELATED TO THE SDK
****************************************************************************/


/****************************************************************************
    SDK version

      SDVR_SDK_VERSION_MAJOR - The major version of this SDK.

      SDVR_SDK_VERSION_MINOR - The minor version of this SDK.

      SDVR_SDK_VERSION_REVISION - The revision version of this SDK.

      SDVR_SDK_VERSION_BUILD - The build version of this SDK.

****************************************************************************/
#define SDVR_SDK_VERSION_MAJOR      STRETCH_MAJOR_VERSION
#define SDVR_SDK_VERSION_MINOR      STRETCH_MINOR_VERSION
#define SDVR_SDK_VERSION_REVISION   STRETCH_REVISION_VERSION
#define SDVR_SDK_VERSION_BUILD      4

/****************************************************************************
    Minimum Firmware version required by this SDK

      SSDK_FW_MIN_VERSION_MAJOR - The major version of the firmware required
      by this SDK.

      SSDK_FW_MIN_VERSION_MINOR - The minor version of the firmware required
      by this SDK.

      SSDK_FW_MIN_VERSION_REVISION - The revision version of the firmware
      required by this SDK.

      SSDK_FW_MIN_VERSION_BUILD - The build version of the firmware required
      by this SDK.

****************************************************************************/
#define SSDK_FW_MIN_VERSION_MAJOR      7
#define SSDK_FW_MIN_VERSION_MINOR      6
#define SSDK_FW_MIN_VERSION_REVISION   0
#define SSDK_FW_MIN_VERSION_BUILD      23

/****************************************************************************
    Minimum SCT version required by this SDK

      SSDK_SCT_MIN_VERSION_MAJOR - The major version of SCT required by this
      SDK.

      SSDK_SCT_MIN_VERSION_MINOR - The minor version of SCT required by this
      SDK.

      SSDK_SCT_MIN_VERSION_REVISION - The revision version of SCT required by
      this SDK.

      SSDK_SCT_MIN_VERSION_BUILD - The build version of SCT required by this
      SDK.

****************************************************************************/
#define SSDK_SCT_MIN_VERSION_MAJOR      7
#define SSDK_SCT_MIN_VERSION_MINOR      5
#define SSDK_SCT_MIN_VERSION_REVISION   0
#define SSDK_SCT_MIN_VERSION_BUILD      1

/****************************************************************************
    Minimum Driver version required by this SDK

      SSDK_DRV_MIN_VERSION_MAJOR - The major version of the driver required
      by this SDK.

      SSDK_DRV_MIN_VERSION_MINOR - The minor version of the driver required
      by this SDK.

      SSDK_DRV_MIN_VERSION_REVISION - The revision version of the driver
      required by this SDK.

      SSDK_DRV_MIN_VERSION_BUILD - The build version of the driver required
      by this SDK.

****************************************************************************/
#define SSDK_DRV_MIN_VERSION_MAJOR      7
#define SSDK_DRV_MIN_VERSION_MINOR      5
#define SSDK_DRV_MIN_VERSION_REVISION   0
#define SSDK_DRV_MIN_VERSION_BUILD      1


/****************************************************************************
  VISIBLE: Typedef for the errors returned by the SDK.

    SDVR_ERR_NONE - No error, or in other words, success!

    @The following error codes are generated by the DVR firmware:@

    SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER - The given camera number is invalid.

    SDVR_FRMW_ERR_WRONG_CAMERA_TYPE - Error code if the specified video standard
    is not supported by the firmware.

    SDVR_FRMW_ERR_WRONG_CODEC_FORMAT - Error code if the specified video
    codec is not supported.

    SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION - Error code if the size of the
    picture (width and/or height) to be encoded is invalid.

    SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE

    SDVR_FRMW_ERR_WRONG_CHANNEL_ID

    SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT

    SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT

    SDVR_FRMW_ERR_EXCEED_CPU_LIMIT

    SDVR_FRMW_ERR_CHANNEL_NOT_CREATED

    SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED

    SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED

    SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED

    SDVR_FRMW_ERR_INVALID_TIME

    SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS

    SDVR_FRMW_ERR_SMO_NOT_SUPPORTED - There is no SMO support in the firmware.

    SDVR_FRMW_ERR_VDET_ERROR - This is a warning when the firmware detects cameras
    connected with mixed video standard connected. (i.e., some NTSC cameras and
    some PAL cameras). The function should still return all the requested information.

    SDVR_FRMW_ERR_RUNTIME_ERROR

    SDVR_FRMW_ERR_VPP_RUNTIME_ERROR

    SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR

    SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR

    SDVR_FRMW_ERR_ILLEGAL_PARAMETER

    SDVR_FRMW_ERR_INTERNAL_ERROR

    SDVR_FRMW_ERR_ILLEGAL_COMMAND

    SDVR_FRMW_ERR_SMO_NOT_DISABLED - Error code if you tried to reset channel
    parameters to factory default while SMO was enabled.

    SDVR_FRMW_ERR_OUT_OF_MEMORY - Error code if the firmware runs out of memory
    in the middle of the current operation.

    SDVR_FRMW_ERR_NO_IO_BOARD - Error code if the current command requires
    an I/O board for the operation but the I/O board is not connected to the
    DVR board.

    SDVR_FRMW_ERR_AUDIO_RUNTIME

    SDVR_FRMW_ERR_UNSUPPORTED_COMMAND - Error code if the command is not
    supported by the current version of the DVR firmware.

    SDVR_FRMW_ERR_SMO_CHAN_FAILED -

    SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED - Error code if you exceeded
    the processing capabilities for the current operation. (i.e. If you
    try to start 4 channels of D1 dual encoding.)

    SDVR_FRMW_ERR_UNSUPPORTED_VIDEO_RES - Error code if you try to select
    a video resolution that is not supported under the current video
    standard. For example, if you try to set HMO output resolution to D1 when the
    video standard is 4CIF.

    SDVR_FRMW_ERR_OUTPUT_BUFFER_OVERRUN - Error code indicating internal error
    in the firmware (encoder output buffer overrun).

    SDVR_FRMW_WATCHDOG_EXPIRED -

    SDVR_FRMW_ERR_ASSERT_FAIL -

    SDVR_FRMW_ERR_AUDIOOUT_NOT_SUPPORTED -

    SDVR_FRMW_ERR_AUDIOOUT_NOT_DISABLED -

    SDVR_FRMW_ERR_AUDIOOUT_CHAN_FAILED -

    SDVR_FRMW_ERR_EMO_NOT_SUPPORTED -

    SDVR_FRMW_ERR_INVALID_DEFAULT_VSTD - Error code indicating the default board
    video standard is not supported.

    SDVR_FRMW_ERR_REMOTE_ENC_NOT_SUPPORTED - Encoding on a remote PE (i.e. not on
    the PE that receives the video input) is not supported for this encoder.

    SDVR_FRMW_ERR_SMO_ALREADY_CREATED - Error code when SMO port video standard
    is being set after any kind of channels were created.

    SDVR_FRMW_ERR_IPP_NOT_PRESENT - Error code if the Image Sensor Pipeline (IPP)
    is not supported by the DVR/IP Camera board.

    SDVR_FRMW_ERR_NO_CODEC_CHANGE_WHEN_ACTIVE - Error code if trying to change
    the CODEC type of an encoder or decoder channel for a sub-stream while it
    is enabled.

    SDVR_FRMW_ERR_UNSUPPORTED_VIDEO_FORMAT - Error code if the specified video
    codec is not supported.

    SDVR_FRMW_ERR_UNSUPPORTED_AUDIO_FORMAT -

    SDVR_FRMW_ERR_ILLEGAL_OPCODE - Error code if the requested operation is not
    supported by the current version of the firmware.

    SDVR_FRMW_ERR_ILLEGAL_STREAM_ID - Error code if requested video stream
    is out of range of supported video CODECs for a specific channel.

    SDVR_FRMW_ERR_STREAM_NOT_DISABLED -

    SDVR_FRMW_ERR_ILLEGAL_PE_ID -

    SDVR_FRMW_ERR_TWI_ERROR -

    SDVR_FRMW_ERR_ILLEGAL_GPIO_NUM -

    SDVR_FRMW_ERR_EMO_NOT_CREATED -

    SDVR_FRMW_ERR_INVALID_IMAGE_W_H - Error code if the specified output image
    width or height is invalid, or if crop/scale size and offset are invalid.

    SDVR_FRMW_ERR_BUFFER_TOO_SMALL - Error code if the specified decoder size
    is smaller that the actual encoded bit stream size.

    SDVR_FRMW_ERR_ENC_AUDIO_MODE -

    SDVR_FRMW_ERR_BUFFER_TOO_LARGE -

    SDVR_FRMW_ERR_INVALID_ENC_MODE -

    SDVR_FRMW_ERR_INVALID_PORT_NUM -

    SDVR_FRMW_ERR_INVALID_LAYER -

    SDVR_FRMW_ERR_INVALID_SIZE -

    SDVR_FRMW_ERR_INVALID_INDEX -
    SDVR_FRMW_ERR_INVALID_RATE -

    SDVR_FRMW_ERR_INVALID_EEPROM_ZONE -

    SDVR_FRMW_ERR_INVALID_EEPROM_ACCESS_TYPE -

    SDVR_FRMW_ERR_INVALID_EEPROM_PASS_TYPE -

    SDVR_FRMW_ERR_INVALID_EEPROM_FUSE_ID -

    SDVR_FRMW_ERR_INVALID_TEMP_THRESHOLD -

    SDVR_FRMW_ERR_INVALID_TEMP_INTERVAL -

    SDVR_FRMW_ERR_IOCHAN_ALREADY_CREATED -

    SDVR_FRMW_ERR_IOCHAN_NOT_CREATED -

    SDVR_FRMW_UNSUPPORTED_BITSTREAM - Error code when bit-stream contains
    features not supported by the Stretch decoder. This error may be returned
    as part of signal SDVR_SIGNAL_RUNTIME_ERROR.

    SDVR_FRMW_ERR_INVALID_MEM_INTERVAL -

    SDVR_FRMW_ERR_INVALID_SMO_INSTANCE - Error code if the given SMO instance
    ID exceeds the maximum number of instances that a camera or player can
    be set on different grid's of a SMO port.

    @The following error codes are generated by the PCI driver interface:@

    SDVR_DRV_ERR_INVALID_PARAMETER - Refer to the function prototype for information.

    SDVR_DRV_ERR_BOARD_IN_USE - The given board index is already in use.

    SDVR_DRV_ERR_BOARD_CONNECT - Failed to connect to the board.

    SDVR_DRV_ERR_BOARD_CLOSE - Failed while trying to close the board.

    SDVR_DRV_ERR_BOARD_RESET - Error code if failed to reset the DVR board.

    SDVR_DRV_ERR_IPC_INIT

    SDVR_DRV_ERR_NO_CHANNELS

    SDVR_DRV_ERR_CHANNEL_IN_USE - If a receive channel has not been closed by
    the firmware.

    SDVR_DRV_ERR_CHANNEL_CREATE

    SDVR_DRV_ERR_CHANNEL_CONNECT

    SDVR_DRV_ERR_CHANNEL_CLOSE - If an error occurred while closing the
    channel, or the firmware did not respond correctly to the close request.

    SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE

    SDVR_DRV_ERR_CHANNEL_DEAD

    SDVR_DRV_ERR_NO_RECV_BUFFERS

    SDVR_DRV_ERR_NO_SEND_BUFFERS

    SDVR_DRV_ERR_MSG_SEND - Error code if the driver failed to send the
    command to the firmware.

    SDVR_DRV_ERR_MSG_RECV - Error code if the driver timeout while waiting
    to receive a response from the firmware. This error could be an indication
    that the code on the firmware hung.

    SDVR_DRV_BOARD_BOOT_FAIL - Failed to boot PCI board.

    @Following error codes are generated by the DVR SDK:@

    SDVR_ERR_OUT_OF_MEMORY - System is out of memory.

    SDVR_ERR_INVALID_HANDLE - Invalid buffer handle.

    SDVR_ERR_INVALID_ARG - Invalid argument to a function call.

    SDVR_ERR_INVALID_BOARD - Invalid board number.

    SDVR_ERR_BOARD_CONNECTED - The current operation is invalid while
    connecting to a board (i.e., setting SDK parameters or connect to a board
    that is already connected).

    SDVR_ERR_INVALID_CHANNEL - Invalid channel number.

    SDVR_ERR_CHANNEL_CLOSED - Channel is closed - cannot communicate with it.

    SDVR_ERR_BOARD_CLOSED - Board is closed - cannot communicate with it.

    SDVR_ERR_NO_VFRAME - No video frame is available.

    SDVR_ERR_NO_AFRAME - No audio frame is available.

    SDVR_ERR_INTERNAL - Internal error in the SDK. Please contact
    Stretch support for assistance.

    SDVR_ERR_BOARD_NOT_CONNECTED - The specified board index was not
    connected.

    SDVR_ERR_IN_STREAMING - Failed to close the board because some channels
    are still active (encoding or decoding).

    SDVR_ERR_NO_DVR_BOARD - No PCIe DVR board was found on the PC.

    SDVR_ERR_WRONG_DRIVER_VERSION - The current DVR PCIe driver is
    not supported.

    SDVR_ERR_DBG_FILE - Failed to open the debug file.

    SDVR_ERR_ENCODER_NOT_ENABLED - Error code if failed to start the video
    encoder for the given channel or the video encoder is not enabled for
    an operation that required it to be running.

    SDVR_ERR_ENCODER_NOT_DISABLED - Failed to stop the encoder on the
    given channel.

    SDVR_ERR_SDK_NO_FRAME_BUF - There is not enough buffer allocated to
    receive encoded or raw frame buffers. Or the frame size is zero.

    SDVR_ERR_INVALID_FRAME_TYPE - The given frame type is not supported.

    SDVR_ERR_NOBUF - No A/V buffer is available.

    SDVR_ERR_CALLBACK_FAILED - Failed to register callback with the driver.

    SDVR_ERR_INVALID_CHAN_HANDLE - The given channel handle is invalid.

    SDVR_ERR_COMMAND_NOT_SUPPORTED - The function is not implemented.

    SDVR_ERR_ODD_SMO_COORDINATES - Error code if either of the x or y value of the
    SMO grid is an odd number.

    SDVR_ERR_LOAD_FIRMWARE - Error code if failed to load the firmware. This
    could be as result of invalid file path or failure to load from PCIe driver.

    SDVR_ERR_WRONG_CHANNEL_TYPE - The channel handle belongs to wrong channel
    type for the current operation.

    SDVR_ERR_DECODER_NOT_ENABLED - Error code if we are trying to send
    frames to be decoded but the decoder is not enabled.

    SDVR_ERR_BUF_NOT_AVAIL - Error code if no buffer is available to send
    frames.

    SDVR_ERR_MAX_REGIONS - Error code if the maximum allowed regions is reached
    when you request to add a new motion or blind detection as well as
    privacy region.

    SDVR_ERR_INVALID_REGION - Error code if the given region is either does
    not exist or is invalid.

    SDVR_ERR_INVALID_GOP - Error code if the specified GOP value of the
    encoder parameter to be set is zero.

    SDVR_ERR_INVALID_BITRATE - Error code if the specified maximum bit rate is
    less than the average bit rate while setting the encoder parameters.

    SDVR_ERR_INVALID_BITRATE_CONTROL - Error code if an unknown encoder bit rate
    control is specified.

    SDVR_ERR_INVALID_QUALITY - Error code if the encoder quality parameter
    is out of range for the current video encoder.

    SDVR_ERR_INVALID_FPS - Error code if the specified encoder frame rate is
    not supported by the current video standard.

    SDVR_ERR_UNSUPPORTED_FIRMWARE - Error code if the DVR firmware version is
    not supported by the current version of SDK.

    SDVR_ERR_INVALID_OSD_ID - Error code if the specified OSD ID has not been
    configured before using or is out of the valid range of OSD items on a
    channel.

    SDVR_ERR_OSD_LENGTH - Error code if the given OSD text length is too large.

    SDVR_ERR_OSD_FONT_FILE - Error code if the given font table file does not
    exist, is invalid, or can not be opened.

    SDVR_ERR_FONT_ID - Error code if the given font ID does not fall within
    the valid range or the font table with this ID does not exist.

    SDVR_ERR_CAMERA_IN_REC - Error code if the camera that is being
    requested for recording, is currently in recording.

    SDVR_ERR_OPEN_FILE - Error code for failure to open the given
    file for the current operation.

    SDVR_ERR_FAILED_ADD_VIDEO_TRACK - Error code for failure to add
    the video track to the recording file.

    SDVR_ERR_FAILED_ADD_AUDIO_TRACK - Error code for failure  to add
    the audio track to the recording file.

    SDVR_ERR_SDK_BUF_EXCEEDED - Error code if any buffer size, while
    setting up SDK buffers, exceeds the maximum allowed.

    SDVR_ERR_AUDIO_RUNTIME - Error code if audio buffers overflow.

    SDVR_ERR_AUTH_KEY_MISSING - Error code if a required authentication key
    is missing on the DVR board while connecting to it.

    SDVR_ERR_AUTH_KEY_LEN - Error code if the size of authentication key is
    larger than what the SDK can support.

    SDVR_ERR_INVALID_RESOLUTION - Error code if the specified resolution
    decimation is not supported by the current operation. (i.e. DCIF or
    2CIF resolution is not supported in SMO or HMO).

    SDVR_ERR_SMO_PORT_NUM - Error code if the given SMO port number does not
    exist on the DVR board.

    SDVR_ERR_UN_INIT_RSRVD_FIELD - Error code if the reserved fields in a
    data structure were not set to zero prior calling the SDK API. You should
    always memset the data structure to zero with the size of the data structure
    before calling any SDK API.

    SDVR_ERR_SMO_GRID_NOT_ENABLED - Error code if activating audio on a SMO
    grid that was not enabled.

    SDVR_ERR_SMO_GRID_SIZE - Error code if the given width and height of the
    SMO grid while being resized is wrong. One possible cause of this error
    is that width and height are not multiple of 2.

    SDVR_ERR_SMO_ALPHA_VALUE - Error code if the alpha value specified for
    the video overlay buffer is not supported.

    SDVR_ERR_FIRMWARE_FILE - Error code if the specified firmware file does
    not exist or is invalid.

    SDVR_ERR_NULL_PARAM - Error code if a NULL pointer parameter is passed to
    the SDK function.

    SDVR_ERR_WRONG_CODEC - Error code if the specified CODEC is either invalid or not
    supported by the current SDK function.

    SDVR_ERR_DECODER_IS_ENABLED - Error code if the decoder is enabled while the
    called SDK function requires the decoder to be stopped.

    SDVR_ERR_AOUT_PORT_NUM - Error code if the given audio out port number does not
    exist on the DVR board.

    SDVR_ERR_BMP_FILE - Error code reading the BMP file due to unsupported format.
    See sdvr_rawv_formats_e for supported BMP file formats.

    SDVR_ERR_FILE_NOT_FOUND - Error code if failed to open the file.

    SDVR_ERR_RAWV_FILE - Error code if failed to read the raw video file.

    SDVR_ERR_EXCEEDED_MAX_EMO - Error code if exceeded maximum number of
    EMO ports that can be created on a give DVR board.

    SDVR_ERR_IN_PREVIEW - Error code if trying to close the channel which is
    in the middle of the UI preview.

    SDVR_ERR_INVALID_FRAME_SIZE - Error code if the specified video frame size
    is invalid in the context of the current operation.

    SDVR_ERR_NO_VIDEO_TRACK - Error code if no supported video codec track
    is found in the file.

    SDVR_ERR_NO_SDT - Error code if no SDT track exist in the SMF file.

    SDVR_ERR_NO_MORE_FRAMES - Error code if there is either no frame in the A/V
    container file or we reached to the end of the file but still expecting
    frames to be written. This is usually means we are in Tivo mode
    recording.

    SDVR_ERR_EOF - Error code if we reached to the end of the A/V container
    file and we don't expect any more frames to be written.

    SDVR_ERR_MOV_PARSE - Error code if in countered error while parsing the
    .mov file.

    SDVR_ERR_FILE_ADD_FRAME - Error code if failed to add more A/V frames to
    the file.

    SDVR_ERR_INVALID_FILE_TYPE - Error code if the specified file type is not
    supported by the operation.

    SDVR_ERR_FILE_FRAME_TYPE - Error code if the specified frame type is
    not supported by the A/V file operation. (i,e, Adding audio frame to
    an elementary stream file).

    SDVR_ERR_NAL_START_CODE - Error code if the H.264 NAL start code is
    not found within the specified video frame.

    SDVR_ERR_INVALID_WIDTH_HEIGHT - Error code if either the width or height
    passed to the API does not fall within the specified range as documented
    by the function call.

    SDVR_ERR_INVALID_AV_TRACK - Error code if audio or video frames are being
    added to a file which its corresponding track was not added to the file.

    SDVR_ERR_SEND_BUF_TOO_SMALL - Error code if the send buffer size assigned
    on the current channel is too small to hold the video frame.

    SDVR_ERR_SEND_BUF_TOO_LARGE - Error code if the send buffer size assigned
    on the current channel is larger than what it is supported by driver for
    the connecting DVR board.

    SDVR_ERR_INVALID_VSTD - Error code if the specified video standard is not
    valid for the operation. It could be specifying a HD video standard when
    an SD video standard is expected or vice versa.

    SDVR_ERR_MIXED_VSTD - Error code if both the SD and HD video standards are
    not the same.

    SDVR_ERR_SMF_READ_ITEM - Error code if an item read from SMF
    file contains an invalid value.

    SDVR_ERR_MISSING_CONFIG - Error code if any of the configuration video frames
    (SPS/PPS/VOL) is missing in the recorded file.

    SDVR_ERR_MULTIPLE_HMO_MIRRORS - Error code if multiple HMO mirrors is started.
    Make sure to close the previous HMO mirror before opening a new one.

    SDVR_ERR_OVERLAY_VIDEO_FORMAT - Error code if either the specified video
    format is not supported by the SMO, or the video overlay is being restarted
    with a different video format.

    SDVR_ERR_INVALID_AV_BUFFER - Error code if the specified A/V buffer is
    not valid. (i,e. A buffer that was not allocated by one of the SDK APIs)

    SDVR_ERR_OVERLAY_NUM - Error code if the overlay number exceeds the range.

    SDVR_ERR_INVALID_SUB_ENCODER - Error code if the requested encoder
    on the video channel is either invalid or has no video CODEC assigned to it.

    SDVR_ERR_DATA_SIZE - Error code if the specified size of buffer is not valid
    for the operation.

    SDVR_ERR_WRONG_SCT_VERSION - Error code if the current version of the SCT
    library is not supported by this SDK.

    SDVR_ERR_INVALID_RELAY_NUM - Error code if the specified relay number exceeds
    the maximum number of relays on the current board. NOTE: Relay numbers are
    zero based.

    SDVR_ERR_INVALID_SENSOR_NUM - Error code if the specified sensor number exceeds
    the maximum number of sensors on the current board. NOTE: Sensor numbers are
    zero based.

    SDVR_ERR_INVALID_LUMA_STRENGTH - Error code if the specified value exceeds
    the maximum value of the luma strength.

    SDVR_ERR_INVALID_CHROMA_STRENGTH - Error code if the specified value exceeds
    the maximum value of the chroma strength.

    SDVR_ERR_INVALID_SMO_INSTANCE - Error code if the given SMO instance
    ID exceeds the maximum number of instances that a camera or player can
    be set on different grid's of a SMO port.

    SDVR_ERR_INVALID_I_FRAME_BITRATE - Error code if the specified video encoder
    I-frame-bitrate value is more than 99.

    @The following error codes are generated by the svcext library:@

    SDVR_SVCEXT_ERR_OUT_OF_MEMORY - Unable to allocate memory.

    SDVR_SVCEXT_ERR_INVALID_POINTER - Invalid pointer is passed to the API.

    SDVR_SVCEXT_ERR_SEI_UNSUPPORTED - Only Scalable SEI messages are recognized. All other
    messages are not recognized by the extraction process.

    SDVR_SVCEXT_ERR_NO_STARTCODE - Cannot find the 4 bytes 0x00 0x00 0x00 0x01 startcode.

    SDVR_SVCEXT_ERR_UNSUPPORTED_NAL - Nal type is not supported.

    SDVR_SVCEXT_ERR_LAYERS_EXCEED_RANGE - Number of spatial layers exceed supported range.

    SDVR_SVCEXT_ERR_MISSING_SEI - No SEI founded in the stream.

    SDVR_SVCEXT_ERR_UNEXPECTED_PARAMSET - Inconsistent paramset.

    SDVR_SVCEXT_ERR_MISSING_PARAMSET - No paramset for the layer is found.

    SDVR_SVCEXT_ERR_LAYER_NOT_FOUND - A spatial layer is missing.

    SDVR_SVCEXT_ERR_FAIL - General failure

    SDVR_SVCEXT_ERR_NOT_SUPPORTED - The SVC extraction is not supported.

    SDVR_SVCEXT_ERR_BAD_FRAME - SVC extractor detects corrupted frame data.

****************************************************************************/
typedef enum _sdvr_err_e {
    SDVR_ERR_NONE = 0,
    SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER,
    SDVR_FRMW_ERR_WRONG_CAMERA_TYPE,
    SDVR_FRMW_ERR_WRONG_CODEC_FORMAT,
    SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION,
    SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE,
    SDVR_FRMW_ERR_WRONG_CHANNEL_ID,
    SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT,
    SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT,
    SDVR_FRMW_ERR_EXCEED_CPU_LIMIT,
    SDVR_FRMW_ERR_CHANNEL_NOT_CREATED,
    SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED,
    SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED,
    SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED,
    SDVR_FRMW_ERR_SMO_NOT_CREATED,
    SDVR_FRMW_ERR_INVALID_TIME,
    SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS,
    SDVR_FRMW_ERR_SMO_NOT_SUPPORTED,
    SDVR_FRMW_ERR_VDET_ERROR,
    SDVR_FRMW_ERR_RUNTIME_ERROR,
    SDVR_FRMW_ERR_VPP_RUNTIME_ERROR,
    SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR,
    SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR,
    SDVR_FRMW_ERR_ILLEGAL_PARAMETER,
    SDVR_FRMW_ERR_INTERNAL_ERROR,
    SDVR_FRMW_ERR_ILLEGAL_COMMAND,
    SDVR_FRMW_ERR_SMO_NOT_DISABLED,
    SDVR_FRMW_ERR_OUT_OF_MEMORY,
    SDVR_FRMW_ERR_NO_IO_BOARD,
    SDVR_FRMW_ERR_AUDIO_RUNTIME,
    SDVR_FRMW_ERR_UNSUPPORTED_COMMAND,
    SDVR_FRMW_ERR_SMO_CHAN_FAILED,
    SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED,
    SDVR_FRMW_ERR_UNSUPPORTED_VIDEO_RES,
    SDVR_FRMW_ERR_OUTPUT_BUFFER_OVERRUN,
    SDVR_FRMW_WATCHDOG_EXPIRED,
    SDVR_FRMW_ERR_ASSERT_FAIL,
    SDVR_FRMW_ERR_AUDIOOUT_NOT_SUPPORTED,
    SDVR_FRMW_ERR_AUDIOOUT_NOT_DISABLED,
    SDVR_FRMW_ERR_AUDIOOUT_CHAN_FAILED,
    SDVR_FRMW_ERR_EMO_NOT_SUPPORTED,
    SDVR_FRMW_ERR_INVALID_DEFAULT_VSTD,
    SDVR_FRMW_ERR_REMOTE_ENC_NOT_SUPPORTED,
    SDVR_FRMW_ERR_SMO_ALREADY_CREATED,
    SDVR_FRMW_ERR_IPP_NOT_PRESENT,
    SDVR_FRMW_ERR_NO_CODEC_CHANGE_WHEN_ACTIVE,
    SDVR_FRMW_ERR_UNSUPPORTED_VIDEO_FORMAT,
    SDVR_FRMW_ERR_UNSUPPORTED_AUDIO_FORMAT,
    SDVR_FRMW_ERR_ILLEGAL_OPCODE,
    SDVR_FRMW_ERR_ILLEGAL_STREAM_ID,
    SDVR_FRMW_ERR_STREAM_NOT_DISABLED,
    SDVR_FRMW_ERR_ILLEGAL_PE_ID,
    SDVR_FRMW_ERR_TWI_ERROR,
    SDVR_FRMW_ERR_ILLEGAL_GPIO_NUM,
    SDVR_FRMW_ERR_EMO_NOT_CREATED,
    SDVR_FRMW_ERR_INVALID_IMAGE_W_H,
    SDVR_FRMW_ERR_BUFFER_TOO_SMALL,
    SDVR_FRMW_ERR_ENC_AUDIO_MODE,
    SDVR_FRMW_ERR_BUFFER_TOO_LARGE,
    SDVR_FRMW_ERR_INVALID_ENC_MODE,
    SDVR_FRMW_ERR_INVALID_PORT_NUM,
    SDVR_FRMW_ERR_INVALID_LAYER,
    SDVR_FRMW_ERR_INVALID_SIZE,
    SDVR_FRMW_ERR_INVALID_INDEX,
    SDVR_FRMW_ERR_INVALID_RATE,
    SDVR_FRMW_ERR_INVALID_EEPROM_ZONE,
    SDVR_FRMW_ERR_INVALID_EEPROM_ACCESS_TYPE,
    SDVR_FRMW_ERR_INVALID_EEPROM_PASS_TYPE,
    SDVR_FRMW_ERR_INVALID_EEPROM_FUSE_ID,
    SDVR_FRMW_ERR_INVALID_TEMP_THRESHOLD,
    SDVR_FRMW_ERR_INVALID_TEMP_INTERVAL,
    SDVR_FRMW_ERR_IOCHAN_ALREADY_CREATED,
    SDVR_FRMW_ERR_IOCHAN_NOT_CREATED,
    SDVR_FRMW_UNSUPPORTED_BITSTREAM,
    SDVR_FRMW_ERR_INVALID_MEM_INTERVAL,
    SDVR_FRMW_ERR_INVALID_SMO_INSTANCE,
    SDVR_FRMW_ERR_INVALID_SIGNAL,
    SDVR_FRMW_ERR_UNSUPPORT_FLIP,
    SDVR_FRMW_ERR_INVALID_EEPROM_FMT,
    SDVR_FRMW_ERR_INVALID_VALUE_IN_SPEC,
    SDVR_FRMW_ERR_FOSD_ERROR,
    SDVR_FRMW_ERR_FONT_NOT_AVAILABLE,
    SDVR_FRMW_ERR_PROTECTION_ERROR,
    SDVR_FRMW_ERR_SNAPSHOT_NOT_READY,

    SDVR_DRV_ERR_MSG_RECV = 255,

    SDVR_DRV_ERR_INVALID_PARAMETER = 1000,
    SDVR_DRV_ERR_BOARD_IN_USE,
    SDVR_DRV_ERR_BOARD_CONNECT,
    SDVR_DRV_ERR_BOARD_CLOSE,
    SDVR_DRV_ERR_BOARD_RESET,
    SDVR_DRV_ERR_IPC_INIT,
    SDVR_DRV_ERR_NO_CHANNELS,
    SDVR_DRV_ERR_CHANNEL_IN_USE,
    SDVR_DRV_ERR_CHANNEL_CREATE,
    SDVR_DRV_ERR_CHANNEL_CONNECT,
    SDVR_DRV_ERR_CHANNEL_CLOSE,
    SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE,
    SDVR_DRV_ERR_CHANNEL_DEAD,
    SDVR_DRV_ERR_NO_RECV_BUFFERS,
    SDVR_DRV_ERR_NO_SEND_BUFFERS,
    SDVR_DRV_ERR_MSG_SEND = 1015,
    /* 1016 is left missing*/
    SDVR_DRV_BOARD_BOOT_FAIL = 1017,

    SDVR_ERR_OUT_OF_MEMORY = 2000,
    SDVR_ERR_INVALID_HANDLE,
    SDVR_ERR_INVALID_ARG,
    SDVR_ERR_INVALID_BOARD,
    SDVR_ERR_BOARD_CONNECTED,
    SDVR_ERR_INVALID_CHANNEL,
    SDVR_ERR_CHANNEL_CLOSED,
    SDVR_ERR_BOARD_CLOSED,
    SDVR_ERR_NO_VFRAME,
    SDVR_ERR_NO_AFRAME,
    SDVR_ERR_INTERNAL,
    SDVR_ERR_BOARD_NOT_CONNECTED,
    SDVR_ERR_IN_STREAMING,
    SDVR_ERR_NO_DVR_BOARD,
    SDVR_ERR_WRONG_DRIVER_VERSION,
    SDVR_ERR_DBG_FILE,
    SDVR_ERR_ENCODER_NOT_ENABLED,
    SDVR_ERR_ENCODER_NOT_DISABLED,
    SDVR_ERR_SDK_NO_FRAME_BUF,
    SDVR_ERR_INVALID_FRAME_TYPE,
    SDVR_ERR_NOBUF,
    SDVR_ERR_CALLBACK_FAILED,
    SDVR_ERR_INVALID_CHAN_HANDLE,
    SDVR_ERR_COMMAND_NOT_SUPPORTED,
    SDVR_ERR_ODD_SMO_COORDINATES,
    SDVR_ERR_LOAD_FIRMWARE,
    SDVR_ERR_WRONG_CHANNEL_TYPE,
    SDVR_ERR_DECODER_NOT_ENABLED,
    SDVR_ERR_BUF_NOT_AVAIL,
    SDVR_ERR_MAX_REGIONS,
    SDVR_ERR_INVALID_REGION,
    SDVR_ERR_INVALID_GOP,
    SDVR_ERR_INVALID_BITRATE,
    SDVR_ERR_INVALID_BITRATE_CONTROL,
    SDVR_ERR_INVALID_QUALITY,
    SDVR_ERR_INVALID_FPS,
    SDVR_ERR_UNSUPPORTED_FIRMWARE,
    SDVR_ERR_INVALID_OSD_ID,
    SDVR_ERR_OSD_LENGTH,
    SDVR_ERR_OSD_FONT_FILE,
    SDVR_ERR_FONT_ID,
    SDVR_ERR_CAMERA_IN_REC,
    SDVR_ERR_OPEN_FILE,
    SDVR_ERR_FAILED_ADD_VIDEO_TRACK,
    SDVR_ERR_FAILED_ADD_AUDIO_TRACK,
    SDVR_ERR_SDK_BUF_EXCEEDED,
    SDVR_ERR_AUTH_KEY_MISSING,
    SDVR_ERR_AUTH_KEY_LEN,
    SDVR_ERR_INVALID_RESOLUTION,
    SDVR_ERR_SMO_PORT_NUM,
    SDVR_ERR_UN_INIT_RSRVD_FIELD,
    SDVR_ERR_SMO_GRID_NOT_ENABLED,
    SDVR_ERR_SMO_GRID_SIZE,
    SDVR_ERR_SMO_ALPHA_VALUE,
    SDVR_ERR_FIRMWARE_FILE,
    SDVR_ERR_NULL_PARAM,
    SDVR_ERR_WRONG_CODEC,
    SDVR_ERR_DECODER_IS_ENABLED,
    SDVR_ERR_AOUT_PORT_NUM,
    SDVR_ERR_BMP_FILE,
    SDVR_ERR_FILE_NOT_FOUND,
    SDVR_ERR_RAWV_FILE,
    SDVR_ERR_EXCEEDED_MAX_EMO,
    SDVR_ERR_IN_PREVIEW,
    SDVR_ERR_INVALID_FRAME_SIZE,
    SDVR_ERR_NO_VIDEO_TRACK,
    SDVR_ERR_NO_SDT,
    SDVR_ERR_NO_MORE_FRAMES,
    SDVR_ERR_EOF,
    SDVR_ERR_MOV_PARSE,
    SDVR_ERR_FILE_ADD_FRAME,
    SDVR_ERR_INVALID_FILE_TYPE,
    SDVR_ERR_NAL_START_CODE,
    SDVR_ERR_INVALID_WIDTH_HEIGHT,
    SDVR_ERR_INVALID_AV_TRACK,
    SDVR_ERR_SEND_BUF_TOO_SMALL,
    SDVR_ERR_SEND_BUF_TOO_LARGE,
    SDVR_ERR_INVALID_VSTD,
    SDVR_ERR_MIXED_VSTD,
    SDVR_ERR_SMF_READ_ITEM,
    SDVR_ERR_MISSING_CONFIG,
    SDVR_ERR_MULTIPLE_HMO_MIRRORS,
    SDVR_ERR_OVERLAY_VIDEO_FORMAT,
    SDVR_ERR_INVALID_AV_BUFFER,
    SDVR_ERR_OVERLAY_NUM,
    SDVR_ERR_INVALID_SUB_ENCODER,
    SDVR_ERR_DATA_SIZE,
    SDVR_ERR_WRONG_SCT_VERSION,
    SDVR_ERR_INVALID_RELAY_NUM,
    SDVR_ERR_INVALID_SENSOR_NUM,
    SDVR_ERR_INVALID_LUMA_STRENGTH,
    SDVR_ERR_INVALID_CHROMA_STRENGTH,
    SDVR_ERR_INVALID_SMO_INSTANCE,
    SDVR_ERR_INVALID_I_FRAME_BITRATE,
    SDVR_ERR_INVALID_GOP_STYLE,

    SDVR_SVCEXT_ERR_OUT_OF_MEMORY = 3000,
    SDVR_SVCEXT_ERR_INVALID_POINTER,
    SDVR_SVCEXT_ERR_UNSUPPORTED_SEI,
    SDVR_SVCEXT_ERR_NO_STARTCODE,
    SDVR_SVCEXT_ERR_UNSUPPORTED_NAL,
    SDVR_SVCEXT_ERR_LAYERS_EXCEED_RANGE,
    SDVR_SVCEXT_ERR_MISSING_SEI,
    SDVR_SVCEXT_ERR_UNEXPECTED_PARAMSET,
    SDVR_SVCEXT_ERR_MISSING_PARAMSET,
    SDVR_SVCEXT_ERR_LAYER_NOT_FOUND,
    SDVR_SVCEXT_ERR_FAIL,
    SDVR_SVCEXT_ERR_NOT_SUPPORTED,
    SDVR_SVCEXT_ERR_BAD_FRAME

} sdvr_err_e;

/****************************************************************************
    VISIBLE: Typedef for the board diagnostics codes.

        SDVR_DIAG_OK - All the diagnostic tests passed.

    @Diagnostics failure codes for Boot loader DDR test:@

        SDVR_DIAG_DDR_WRITEREAD_FAIL - DDR write/read test failed.

        SDVR_DIAG_DDR_ADDRLINES_FAIL - DDR address lines test failed.

        SDVR_DIAG_DDR_BITFLIP_FAIL   - DDR bit-flip test failed.

        SDVR_DIAG_DDR_DMA_FAIL       - DDR DMA test failed.

        SDVR_DIAG_DDR_READ_DMA_FAIL  - DDR read/DMA test failed.

    @Diagnostics failure codes for PLL test:@

        SDVR_DIAG_PLL_TEST_MHZ       - Processor speed test failed.

        SDVR_DIAG_PLL_TEST_SYS       - PLL_SYS test failed.

        SDVR_DIAG_PLL_TEST_IO        - PLL_IO test failed.

        SDVR_DIAG_PLL_TEST_AIM       - PLL_AIM test failed.

        SDVR_DIAG_PLL_TEST_DP0       - PLL_DP0 test failed.

        SDVR_DIAG_PLL_TEST_DP2       - PLL_DP2 test failed.

        SDVR_DIAG_PLL_TEST_DDR       - DLL_DDR test failed.

    @Diagnostics failure codes for SPI Flash test:@

        SDVR_DIAG_SPI_TEST_READ      - Flash read error.

        SDVR_DIAG_SPI_TEST_ERASE     - Flash erase error.

        SDVR_DIAG_SPI_TEST_PROG      - Flash program error.

        SDVR_DIAG_SPI_TEST_UNLOCK    - Flash unlock error.

        SDVR_DIAG_SPI_TEST_COMPARE   - Flash data compare error.

        SDVR_DIAG_SPI_TEST_MAINT     - Flash maintenance command error.

        SDVR_DIAG_SPI_TEST_MISC      - Miscellaneous Flash error.

    @Diagnostics failure codes for TWI EEPROM test:@

        SDVR_DIAG_TWI_EEPROM_TEST_READ     - TWI EEPROM read error.

        SDVR_DIAG_TWI_EEPROM_TEST_WRITE    - TWI EEPROM write error.

        SDVR_DIAG_TWI_EEPROM_TEST_INIT     - TWI EEPROM initialization error.

        SDVR_DIAG_TWI_EEPROM_TEST_COMPARE  - TWI EEPROM data compare error.

        SDVR_DIAG_TWI_EEPROM_TEST_WP_COMPARE - TWI EEPROM write-protect error.

    @Diagnostics failure codes for Epson test:@

        SDVR_DIAG_EPSON_REG_TEST_INIT     - Epson test initialization error.

        SDVR_DIAG_EPSON_REG_TEST_WALKING  - Epson register bit-walk error.

    @Diagnostics failure codes for Decoder test:@

        SDVR_DIAG_DECODER_AUDIO_TEST_INIT      - Decoder audio test init error.

        SDVR_DIAG_DECODER_AUDIO_TEST_NO_AUDIO  - Decoder audio not received error.

        SDVR_DIAG_TW2815_REG_TEST             - Techwell register test error.

        SDVR_DIAG_TW2864_REG_TEST             - Techwell register test error.

        SDVR_DIAG_DECODER_VIDEO_TEST_INIT      - Decoder video test init error.

        SDVR_DIAG_DECODER_VIDEO_TEST_NO_VIDEO  - Decoder video not received error.

        SDVR_DIAG_DECODER_VIDEO_TEST_TIMEOUT   - Decoder video test timeout error.

        SDVR_DIAG_DECODER_VIDDET_TEST_INIT_ERR     - Decoder video detect test init error.

        SDVR_DIAG_DECODER_VIDDET_TEST_UNKNOWN_CHIP - Decoder video detect test unknown chip error.

        SDVR_DIAG_DECODER_VIDDET_TEST_NO_INPUT_ERR - Decoder video detect test no input error.

        SDVR_DIAG_DECODER_VIDDET_TEST_CONFLICT_ERR - Decoder video detect test conflict error.

        SDVR_DIAG_DECODER_VIDDET_TEST_NO_SYNC_ERR  - Decoder video detect test no sync. error.

        SDVR_DIAG_DECODER_AUDDET_TEST_NO_SYNC_ERR  - Decoder audio detect test no sync. error.

        SDVR_DIAG_DECODER_UNIQUE_VIDEO_TEST - Video inputs are not unique

        SDVR_DIAG_NVP1104_REG_TEST - Nextchip register test error

        SDVR_DIAG_NVP1114_REG_TEST - Nextchip register test error

        SDVR_DIAG_DECODER_AUDIO_TEST_TIMEOUT - Decoder audio test timeout error.

        SDVR_DIAG_GV7601_REG_TEST             - Gennum register test error.


    @Diagnostics failure codes for PCIe test:@

        SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB    - PCIe did not detect the CBB test board.

        SDVR_DIAG_PCIE_EYEMASK_TEST_ERR       - PCIe eye mask test failure.

        SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT   - PCIe eye mask test timeout.


    @Diagnostics failure codes for AIM test:@

        SDVR_DIAG_AIM_TEST_ERRORS             - AIM errors were detected.


    @Diagnostics failure codes for SMO output tests:@

        SDVR_DIAG_ASWITCH_ENABLE_ERR          - Analog switch output enable error
        SDVR_DIAG_SMO_ENABLE_ERR              - SMO output enable error

    @Diagnostics failure codes for I/O tests:@

        SDVR_DIAG_IOBOARD_LOOPBACK_ERR        - I/O loopback test error

*****************************************************************************/
typedef enum _sdvr_diag_code_e {
    SDVR_DIAG_OK                        = 0x00000000,

    /* Boot-loader DDR test errors */

    SDVR_DIAG_DDR_WRITEREAD_FAIL        = 0xb007e001,
    SDVR_DIAG_DDR_ADDRLINES_FAIL        = 0xb007e002,
    SDVR_DIAG_DDR_BITFLIP_FAIL          = 0xb007e003,
    SDVR_DIAG_DDR_DMA_FAIL              = 0xb007e004,
    SDVR_DIAG_DDR_READ_DMA_FAIL         = 0xb007e005,

    /* PLL test errors */

    SDVR_DIAG_PLL_TEST_MHZ               = 0x1000e001,
    SDVR_DIAG_PLL_TEST_SYS               = 0x1000e002,
    SDVR_DIAG_PLL_TEST_IO                = 0x1000e003,
    SDVR_DIAG_PLL_TEST_AIM               = 0x1000e004,
    SDVR_DIAG_PLL_TEST_DP0               = 0x1000e005,
    SDVR_DIAG_PLL_TEST_DP2               = 0x1000e006,
    SDVR_DIAG_PLL_TEST_DDR               = 0x1000e007,

    /* SPI flash test errors */

    SDVR_DIAG_SPI_TEST_READ              = 0x1001e001,
    SDVR_DIAG_SPI_TEST_ERASE             = 0x1001e002,
    SDVR_DIAG_SPI_TEST_PROG              = 0x1001e003,
    SDVR_DIAG_SPI_TEST_UNLOCK            = 0x1001e004,
    SDVR_DIAG_SPI_TEST_COMPARE           = 0x1001e005,
    SDVR_DIAG_SPI_TEST_MAINT             = 0x1001e006,
    SDVR_DIAG_SPI_TEST_MISC              = 0x1001e007,

    /* TWI EEPROM test errors */

    SDVR_DIAG_TWI_EEPROM_TEST_READ       = 0x1002e001,
    SDVR_DIAG_TWI_EEPROM_TEST_WRITE      = 0x1002e002,
    SDVR_DIAG_TWI_EEPROM_TEST_INIT       = 0x1002e003,
    SDVR_DIAG_TWI_EEPROM_TEST_COMPARE    = 0x1002e004,
    SDVR_DIAG_TWI_EEPROM_TEST_WP_COMPARE = 0x1002e005,

    /* Epson test errors */

    SDVR_DIAG_EPSON_REG_TEST_INIT       = 0x1003e001,
    SDVR_DIAG_EPSON_REG_TEST_WALKING    = 0x1003e002,

    /* Techwell test errors */

    SDVR_DIAG_DECODER_AUDIO_TEST_INIT     = 0x1004e001,
    SDVR_DIAG_DECODER_AUDIO_TEST_NO_AUDIO = 0x1004e002,
    SDVR_DIAG_TW2815_REG_TEST             = 0x1004e003,
    SDVR_DIAG_TW2864_REG_TEST             = 0x1004e013,
    SDVR_DIAG_DECODER_VIDEO_TEST_INIT     = 0x1004e004,
    SDVR_DIAG_DECODER_VIDEO_TEST_NO_VIDEO = 0x1004e005,
    SDVR_DIAG_DECODER_VIDEO_TEST_TIMEOUT  = 0x1004e015,

    SDVR_DIAG_DECODER_VIDDET_TEST_INIT_ERR     = 0x1004e006,
    SDVR_DIAG_DECODER_VIDDET_TEST_UNKNOWN_CHIP = 0x1004e007,
    SDVR_DIAG_DECODER_VIDDET_TEST_NO_INPUT_ERR = 0x1004e008,
    SDVR_DIAG_DECODER_VIDDET_TEST_CONFLICT_ERR = 0x1004e009,
    SDVR_DIAG_DECODER_VIDDET_TEST_NO_SYNC_ERR  = 0x1004e00a,
    SDVR_DIAG_DECODER_AUDDET_TEST_NO_SYNC_ERR  = 0x1004e00b,
    SDVR_DIAG_DECODER_UNIQUE_VIDEO_TEST        = 0x1004e00c,
    SDVR_DIAG_NVP1104_REG_TEST                 = 0x1004e01d,
    SDVR_DIAG_NVP1114_REG_TEST                 = 0x1004e00d,
    SDVR_DIAG_DECODER_AUDIO_TEST_TIMEOUT       = 0x1004e00e,
    SDVR_DIAG_GV7601_REG_TEST                  = 0x1004e010,

    /* PCIe test errors */

    SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB   = 0x1005e001,
    SDVR_DIAG_PCIE_EYEMASK_TEST_ERR      = 0x1005e002,
    SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT  = 0x1005e003,

    /* AIM test errors */

    SDVR_DIAG_AIM_TEST_ERRORS            = 0x1006e001,

    /* SMO output test errors */

    SDVR_DIAG_ASWITCH_ENABLE_ERR         = 0x1007e001,
    SDVR_DIAG_SMO_ENABLE_ERR             = 0x1007e002,

    /* I/O test errors */

    SDVR_DIAG_IOBOARD_LOOPBACK_ERR       = 0x1008e001

} sdvr_diag_code_e;

/**************************************************************************
   VISIBLE: A handle to a channel.
***************************************************************************/
typedef sx_int32 sdvr_chan_handle_t;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_signals_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_signals_type_e;

/************************************************************************
  VISIBLE: This enumerated type defines the types of asynchronous messages
  that can be sent from the DVR firmware to the Application.

    SDVR_SIGNAL_RUNTIME_ERROR - Indicates that a non-fatal runtime
    error has occurred on the board. There is extra data associated with
    this signal that gives more information regarding the error. The
    meaning of those values varies depending on where the error occurred.

    SDVR_SIGNAL_FATAL_ERROR - Indicates that a fatal error has
    occurred on the board. If this signal is received, the board must
    be reset. There is extra data associated with
    this signal that gives more information regarding the error. The
    meaning of those values varies depending on where the error occurred.

    SDVR_SIGNAL_WATCHDOG_EXPIRED - Indicates that the watchdog timer has
    expired and that the board is about to be reset. If the PC reset function
    is enabled, then the host PC will also be reset.

    SDVR_SIGNAL_PCI_CONGESTION - Indicates that the firmware is dropping
    data due to congestion on the PCI bus. This usually happens only if the
    DVR Host Application is not picking up the arriving data fast enough.

    SDVR_SIGNAL_TEMPERATURE - Reports the current core temperature of PE 0,
    as measured from the silicon. This signal is enabled by default.
    The default measurement interval is 15 seconds and the default threshold
    is 85 degrees centigrade (C). The signal is sent only if the threshold is exceeded.

    SDVR_SIGNAL_FRAME_TOO_LARGE - Indicates that the firmware dropped an
    encoded frame because it was too large to send to the host.

    SDVR_SIGNAL_MEMORY_USAGE - Reports the current PE wise memory usage,
    value in MB. This is disabled by default. The default measurement
    interval is 30 seconds

    SDVR_SIGNAL_SDK_MISSED_FRAME - Indicates that the SDK detected
    either a gap between two consecutive encoded frames or an out of order
    condition.  The data field associated with this signal indicates
    frame sequence number information. The high 2 bytes hold the expected
    sequence number; whereas, the low 2 bytes the received sequence number.
    Additionally, the extra_data field hold the frame type information.
    The high 2 bytes hold the last frame type received and the low 2
    bytes the current frame type.

    SDVR_SIGNAL_USER_1 - This is a user-defined signal for those who wish
    to create their own custom firmware that communicates with the SDK.

    SDVR_SIGNAL_USER_2 - This is a user-defined signal for those who wish
    to create their own custom firmware that communicates with the SDK.
************************************************************************/
typedef enum _sdvr_signals_type_e {
    SDVR_SIGNAL_RUNTIME_ERROR = 1,
    SDVR_SIGNAL_FATAL_ERROR,
    SDVR_SIGNAL_WATCHDOG_EXPIRED,
    SDVR_SIGNAL_PCI_CONGESTION,
    SDVR_SIGNAL_TEMPERATURE,
    SDVR_SIGNAL_FRAME_TOO_LARGE,
    SDVR_SIGNAL_MEMORY_USAGE,

    SDVR_SIGNAL_USER_1 = 50,
    SDVR_SIGNAL_USER_2,

    SDVR_SIGNAL_SDK_MISSED_FRAME = 60,
} __sdvr_signals_type_e;

/****************************************************************************
  VISIBLE: This enumerated type defines the types of events that
  can be detected by the video encoder.

    SDVR_VIDEO_ALARM_NONE - No event or alarm.

    SDVR_VIDEO_ALARM_MOTION - Motion detected. See sdvr_get_alarm_motion_value()
    to get the motion values one motion is detected.

    SDVR_VIDEO_ALARM_BLIND - Blind detected.

    SDVR_VIDEO_ALARM_NIGHT - Night detected.

    SDVR_VIDEO_ALARM_LOSS - Video loss detected. If a channel is set
    to be an encoding channel and no camera is connected to it, this
    video alarm is triggered whenever the encoder is enabled.

    SDVR_VIDEO_ALARM_DETECTED - This alarm is sent when a video signal
    is detected on an encoding channel that was created but had no video
    signal previously. The "data" parameter in
    the alarm callback associated with this video alarm detected
    contains the new video standard.
****************************************************************************/
typedef enum _sdvr_video_alarm_e {
    SDVR_VIDEO_ALARM_NONE,
    SDVR_VIDEO_ALARM_MOTION,
    SDVR_VIDEO_ALARM_BLIND,
    SDVR_VIDEO_ALARM_NIGHT,
    SDVR_VIDEO_ALARM_LOSS,
    SDVR_VIDEO_ALARM_DETECTED
} sdvr_video_alarm_e;

/****************************************************************************
  VISIBLE: This enumerated type defines the types of regions that
  can be defined for a camera.

    SDVR_REGION_MOTION - Motion detected region.

    SDVR_REGION_BLIND - Blind detected region.

    SDVR_REGION_PRIVACY - Privacy region.
****************************************************************************/

typedef enum _sdvr_regions_type_e {
    SDVR_REGION_MOTION = 1,
    SDVR_REGION_BLIND = 2,
    SDVR_REGION_PRIVACY = 4,
} sdvr_regions_type_e;

/****************************************************************************
  VISIBLE: The width and number of lines to be used to calculate regions of
  interest (ROI) map buffer for different video standards. The size of ROI
  map buffer is calculated by (width x lines) for the currently specified
  video standard.

    SDVR_REGION_MAP_D1_WIDTH - The width of ROI map for a D1 NTSC or PAL
    video standard.

    SDVR_REGION_MAP_4CIF_WIDTH - The width of ROI map for a 4-CIF NTSC or
    PAL video standard.

    SDVR_REGION_MAP_LINE_NTSC - The number of lines in ROI map for a NTSC video
    standard.

    SDVR_REGION_MAP_LINE_PAL - The number of lines in ROI map for a PAL video
    standard.

****************************************************************************/

#define SDVR_REGION_MAP_D1_WIDTH       45
#define SDVR_REGION_MAP_4CIF_WIDTH     44
#define SDVR_REGION_MAP_LINE_NTSC      30
#define SDVR_REGION_MAP_LINE_PAL       36

/************************************************************************
  VISIBLE: The current audio/video signal state on the channel in the
  sdvr_av_buffer_t structure.

    SDVR_AV_STATE_VIDEO_LOST - No video signal is being detected on the channel.

    SDVR_AV_STATE_AUDIO_LOST - No audio signal is being detected on the channel.
************************************************************************/
#define    SDVR_AV_STATE_VIDEO_LOST    0x01
#define SDVR_AV_STATE_AUDIO_LOST    0x02

/****************************************************************************
  VISIBLE: This enumerated type defines the kind of frames that can
  be exchanged between the SDK and the Application.
  This is the type to pass to sdvr_get_stream_buffer() to get the
  specific buffer associated with a frame category.

    SDVR_FRAME_RAW_VIDEO - Generic Raw video frame type.

    SDVR_FRAME_RAW_AUDIO - Generic Raw audio PCM frame.

    SDVR_FRAME_H264_IDR - Encoded H.264 IDR frame.

    SDVR_FRAME_H264_I   - Encoded H.264 I frame.

    SDVR_FRAME_H264_P   - Encoded H.264 P frame.

    SDVR_FRAME_H264_B   - Encoded H.264 B frame.

    SDVR_FRAME_H264_SPS - Encoded H.264 SPS frame.

    SDVR_FRAME_H264_PPS - Encoded H.264 PPS frame.

    SDVR_FRAME_JPEG - Encoded JPEG image frame.

    SDVR_FRAME_G711 - Encoded G.711 audio frame. Note used by
    the DVR Host Application.

    SDVR_FRAME_MPEG4_I   - Encoded MPEG4 I frame.

    SDVR_FRAME_MPEG4_P   - Encoded MPEG4 P frame.

    SDVR_FRAME_MPEG4_VOL - Encoded MPEG4 VOL frame which contains
    the video frame header information.

    SDVR_FRAME_MOTION_VALUES - Macro Block (MB) based motion values
    buffer. Each element in this buffer contains the motion value
    from 0 to 255 indicating amount of motion for a particular
    MB.

    SDVR_FRAME_H264_SVC_SEI - Encoded H.264-SVC SEI frame.

    SDVR_FRAME_H264_SVC_PREFIX - Encoded H.264-SVC PREFIX frame.

    SDVR_FRAME_H264_SVC_SUBSET_SPS - Encoded H.264-SVC SPS frame.

    SDVR_FRAME_H264_SVC_SLICE_SCALABLE - Encoded H.264-SVC SCALABLE frame.

    SDVR_FRAME_MPEG2_I   - Encoded MPEG2 I frame.

    SDVR_FRAME_MPEG2_P   - Encoded MPEG2 P frame.

    SDVR_FRAME_CMD_RESPONSE - A buffer containing the response to the
    raw command sent from the Host Application to a sub-system
    within the firmware. The format of the payload is only known
    to the sub-system for the specific command.

    SDVR_FRAME_JPEG_SNAPSHOT - JPEG-compressed single-frame snapshot image.

    SDVR_FRAME_MOTION_MAP - A buffer containing motion values
    represented as 8 bits per one 128x128 block in network byte order,
    scanning the image from left to right and from top to bottom.
    (See sdvr_enable_motion_map() for more details.)

    SDVR_FRAME_APP_ANALYTIC - The video analytic data by the
    application provided video analytic module on the firmware.

    SDVR_FRAME_VIDEO_ENCODED - Any encoded video frame.
    This is the type to pass to sdvr_get_stream_buffer() to get an
    encoded video frame for a particular stream if you try to
    poll for video frames instead of calling it from callback.

    The following frame types should be used when calling
    sdvr_get_stream_buffer() to get various frame/data buffers:

    SDVR_FRAME_VIDEO_ENCODED_PRIMARY - This is a deprecated type.
    Call sdvr_get_stream_buffer() with stream ID 0 and any of the
    encoded buffer fame type to get the primary encoded video frames.

    SDVR_FRAME_VIDEO_ENCODED_SECONDARY - This is a deprecated type.
    Call sdvr_get_stream_buffer() with stream ID 1 and any of the
    encoded buffer fame type to get the secondary encoded video frames.

    SDVR_FRAME_AUDIO_ENCODED - Any encoded audio data
    that was received. This is the
    type to pass to sdvr_get_stream_buffer() to get the encoded audio data.

    SDVR_FRAME_ANALYTIC - Any video analytic frames. This is the
    type to pass to sdvr_get_stream_buffer() to get
    SDVR_FRAME_MOTION_VALUES or SDVR_FRAME_APP_ANALYTIC.

    SDVR_FRAME_RAW_VIDEO_SECONDARY - This is a deprecated type.
    Call sdvr_get_stream_buffer() with frame type if SDVR_FRAME_RAW_VIDEO and
    stream id 1 to get the secondary raw video stream.

  Remarks:

    Currently B frames are not support for any of the video CODECs.
****************************************************************************/
typedef enum _sdvr_frame_type_e {
    SDVR_FRAME_RAW_AUDIO = 3,
    SDVR_FRAME_H264_IDR,
    SDVR_FRAME_H264_I,
    SDVR_FRAME_H264_P,
    SDVR_FRAME_H264_B,
    SDVR_FRAME_H264_SPS,
    SDVR_FRAME_H264_PPS,
    SDVR_FRAME_JPEG,
    SDVR_FRAME_G711,
    SDVR_FRAME_MPEG4_I,
    SDVR_FRAME_MPEG4_P,
    SDVR_FRAME_MPEG4_B,
    SDVR_FRAME_MPEG4_VOL,
    SDVR_FRAME_MOTION_VALUES = 18,
    SDVR_FRAME_RAW_VIDEO,
    SDVR_FRAME_H264_SVC_SEI,
    SDVR_FRAME_H264_SVC_PREFIX,
    SDVR_FRAME_H264_SVC_SUBSET_SPS,
    SDVR_FRAME_H264_SVC_SLICE_SCALABLE,
    SDVR_FRAME_MPEG2_I,
    SDVR_FRAME_MPEG2_P,
    SDVR_FRAME_MPEG2_B,
    SDVR_FRAME_CMD_RESPONSE,
    SDVR_FRAME_JPEG_SNAPSHOT,
    SDVR_FRAME_MOTION_MAP,

    SDVR_FRAME_APP_ANALYTIC = 90,

    SDVR_FRAME_VIDEO_ENCODED_PRIMARY = 100,
    SDVR_FRAME_VIDEO_ENCODED_SECONDARY,
    SDVR_FRAME_AUDIO_ENCODED,
    SDVR_FRAME_ANALYTIC,
    SDVR_FRAME_RAW_VIDEO_SECONDARY,
    SDVR_FRAME_VIDEO_ENCODED
}__sdvr_frame_type_e;
/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_frame_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_frame_type_e;
/****************************************************************************
  VISIBLE: Typedef describing locations for OSD.

    SDVR_LOC_TOP_LEFT - Top left of the screen

    SDVR_LOC_TOP_RIGHT - Top right of the screen

    SDVR_LOC_BOTTOM_LEFT - Bottom left of the screen

    SDVR_LOC_BOTTOM_RIGHT - Bottom right of the screen

    SDVR_LOC_CUSTOM - A user-defined position. The upper left corner of
    the video frame is the origin (0,0).
****************************************************************************/
typedef enum _sdvr_location_e {
    SDVR_LOC_TOP_LEFT = 0,
    SDVR_LOC_BOTTOM_LEFT,
    SDVR_LOC_TOP_RIGHT,
    SDVR_LOC_BOTTOM_RIGHT,
    SDVR_LOC_CUSTOM
} sdvr_location_e;

/****************************************************************************
  VISIBLE: This enumerated type describes the video standards supported
  by SDVR.

    SDVR_VIDEO_STD_NONE - No standard defined

    SDVR_VIDEO_STD_D1_PAL  - PAL 720x576 at 25 fps.

    SDVR_VIDEO_STD_D1_NTSC - NTSC 720x480 at 30 fps.

    SDVR_VIDEO_STD_CIF_PAL - PAL 352x288 at 25 fps.

    SDVR_VIDEO_STD_CIF_NTSC - NTSC 352x240 at 30 fps.

    SDVR_VIDEO_STD_2CIF_PAL - PAL 704x288 at 25 fps.

    SDVR_VIDEO_STD_2CIF_NTSC - NTSC 704x240 at 30 fps.

    SDVR_VIDEO_STD_4CIF_PAL - PAL 704x576 at 25 fps.

    SDVR_VIDEO_STD_4CIF_NTSC - NTSC 704x480 at 30 fps.

    SDVR_VIDEO_STD_1080P30 - HD 1920x1080 at 30 frames per second.

    SDVR_VIDEO_STD_1080P25 - HD 1920x1080 at 25 frames per second.

    SDVR_VIDEO_STD_720P60 - HD 1280x720 at 60 fps(NTSC).

    SDVR_VIDEO_STD_720P50 - HD 1280x720 at 50 fps(PAL).

    SDVR_VIDEO_STD_1080I60 - HD 1920x540 at 60 fields per second or 30 fps.

    SDVR_VIDEO_STD_1080I50 - HD 1920x540 at 50 fields per second or 25 fps.

    SDVR_VIDEO_STD_1080P60 - HD 1920x1080 at 60 frames per second (NTSC).

    SDVR_VIDEO_STD_1080P50 - HD 1920x1080 at 50 frames per second (PAL).

    SDVR_VIDEO_STD_720P30 - HD 1280x720 at 30 fps(NTSC).

    SDVR_VIDEO_STD_720P25 - HD 1280x720 at 25 fps(PAL).

    SDVR_VIDEO_STD_CUSTOM - Custom size and frame rate, specified elsewhere.

************************************************************************/
typedef enum _sdvr_video_std_e {
    SDVR_VIDEO_STD_NONE = 0,
    SDVR_VIDEO_STD_D1_PAL    = (1 << 0),
    SDVR_VIDEO_STD_D1_NTSC   = (1 << 1),
    SDVR_VIDEO_STD_CIF_PAL   = (1 << 2),
    SDVR_VIDEO_STD_CIF_NTSC  = (1 << 3),
    SDVR_VIDEO_STD_2CIF_PAL  = (1 << 4),
    SDVR_VIDEO_STD_2CIF_NTSC = (1 << 5),
    SDVR_VIDEO_STD_4CIF_PAL  = (1 << 6),
    SDVR_VIDEO_STD_4CIF_NTSC = (1 << 7),
    SDVR_VIDEO_STD_1080P30   = (1 << 8),
    SDVR_VIDEO_STD_1080P25   = (1 << 9),
    SDVR_VIDEO_STD_720P60    = (1 << 10),
    SDVR_VIDEO_STD_720P50    = (1 << 11),
    SDVR_VIDEO_STD_1080I60   = (1 << 12),
    SDVR_VIDEO_STD_1080I50   = (1 << 13),
    SDVR_VIDEO_STD_1080P60   = (1 << 14),
    SDVR_VIDEO_STD_1080P50   = (1 << 15),
    SDVR_VIDEO_STD_720P30   =  (1 << 16),
    SDVR_VIDEO_STD_720P25   =  (1 << 17),
    SDVR_VIDEO_STD_CUSTOM   =  (1 << 18)
} sdvr_video_std_e;

/***********************************************************************
  VISIBLE: This defines the last supported video standard.

************************************************************************/
#define SDVR_VIDEO_STD_MAX SDVR_VIDEO_STD_CUSTOM

/****************************************************************************
  VISIBLE: The video standards masks:

    SDVR_VIDEO_STD_PAL_MASK   - All PAL types (SD and HD)

    SDVR_VIDEO_STD_NTSC_MASK  - All NTSC types (SD and HD)

    SDVR_VIDEO_STD_SD_MASK    - All SD types

    SDVR_VIDEO_STD_HD_MASK    - All HD types

    SDVR_VIDEO_STD_1080HD_MASK - All 1080 HD types

    SDVR_VIDEO_STD_720HD_MASK  - All 720 HD types
************************************************************************/
#define SDVR_VIDEO_STD_PAL_MASK  (SDVR_VIDEO_STD_D1_PAL | SDVR_VIDEO_STD_CIF_PAL | \
                                 SDVR_VIDEO_STD_2CIF_PAL | SDVR_VIDEO_STD_4CIF_PAL | \
                                 SDVR_VIDEO_STD_1080P25 | SDVR_VIDEO_STD_720P50 | \
                                 SDVR_VIDEO_STD_720P25 | \
                                 SDVR_VIDEO_STD_1080I50 | SDVR_VIDEO_STD_1080P50)
#define SDVR_VIDEO_STD_NTSC_MASK (SDVR_VIDEO_STD_D1_NTSC | SDVR_VIDEO_STD_CIF_NTSC | \
                                 SDVR_VIDEO_STD_2CIF_NTSC | SDVR_VIDEO_STD_4CIF_NTSC | \
                                 SDVR_VIDEO_STD_1080P30 | SDVR_VIDEO_STD_720P60 | \
                                 SDVR_VIDEO_STD_720P30  | \
                                 SDVR_VIDEO_STD_1080I60 | SDVR_VIDEO_STD_1080P60)
#define SDVR_VIDEO_STD_SD_MASK   (SDVR_VIDEO_STD_D1_PAL | SDVR_VIDEO_STD_D1_NTSC | \
                                 SDVR_VIDEO_STD_CIF_PAL | SDVR_VIDEO_STD_CIF_NTSC | \
                                 SDVR_VIDEO_STD_2CIF_PAL | SDVR_VIDEO_STD_2CIF_NTSC | \
                                 SDVR_VIDEO_STD_4CIF_PAL | SDVR_VIDEO_STD_4CIF_NTSC)
#define SDVR_VIDEO_STD_HD_MASK   (SDVR_VIDEO_STD_1080P30 | SDVR_VIDEO_STD_1080P25 | \
                                 SDVR_VIDEO_STD_720P60 | SDVR_VIDEO_STD_720P50 | \
                                 SDVR_VIDEO_STD_720P30  | SDVR_VIDEO_STD_720P25 |\
                                 SDVR_VIDEO_STD_1080I60 | SDVR_VIDEO_STD_1080I50 | \
                                 SDVR_VIDEO_STD_1080P60 | SDVR_VIDEO_STD_1080P50)

#define SDVR_VIDEO_STD_1080HD_MASK (SDVR_VIDEO_STD_1080P30 | SDVR_VIDEO_STD_1080P25 | \
                                 SDVR_VIDEO_STD_1080I60 | SDVR_VIDEO_STD_1080I50 | \
                                 SDVR_VIDEO_STD_1080P60 | SDVR_VIDEO_STD_1080P50)

#define SDVR_VIDEO_STD_720HD_MASK  (SDVR_VIDEO_STD_720P60 | SDVR_VIDEO_STD_720P50 | \
                                 SDVR_VIDEO_STD_720P30  | SDVR_VIDEO_STD_720P25)


/****************************************************************************
  VISIBLE: This enumerated type describes the various supported video sizes.

    SDVR_VIDEO_SIZE_CUSTOM  - A non-standard video frame size

    SDVR_VIDEO_SIZE_720x576 - D1-PAL video width of 720 and number of
    lines of 576.

    SDVR_VIDEO_SIZE_720x480 - D1-NTSC video width of 720 and number of
    lines of 480.

    SDVR_VIDEO_SIZE_352x288 - CIF-PAL video width of 352 and number of
    lines of 288. @NOTE: This is not a standard CIF-PAL.@

    SDVR_VIDEO_SIZE_352x240 - CIF-NTSC video width of 352 and number of
    lines of 240. @NOTE: This is not a standard CIF-NTSC.@

    SDVR_VIDEO_SIZE_704x288 - 2CIF-PAL video width of 704 and number of
    lines of 288.

    SDVR_VIDEO_SIZE_704x240 - 2CIF-NTSC video width of 704 and number of
    lines of 240.

    SDVR_VIDEO_SIZE_704x576 - 4CIF-PAL video width of 704 and number of
    lines of 576.

    SDVR_VIDEO_SIZE_704x480 - 4CIF-NTSC video width of 704 and number of
    lines of 480.

    SDVR_VIDEO_SIZE_176x144 - QCIF_PAL video width of 176 and number of
    lines of 144.  @NOTE: This is not a standard QCIF-PAL.@

    SDVR_VIDEO_SIZE_176x112 - QCIF_NTSC video width of 176 and number of
    lines of 112. @NOTE: This is not a standard QCIF-NTSC. This is the
    size of QCIF_NTSC video frames produced by any system prior to version 7.0.@

    SDVR_VIDEO_SIZE_528x320 - DCIF based on 4CIF NTSC video width of 528 and number of
    lines of 320.

    SDVR_VIDEO_SIZE_528x384 - DCIF based on 4CIF PAL video width of 528 and number of
    lines of 384.

    SDVR_VIDEO_SIZE_720x288 - HALF-D1-PAL video width of 720 and number of
    lines of 288.

    SDVR_VIDEO_SIZE_720x240 - HALF_D1-NTSC video width of 720 and number of
    lines of 240.

    SDVR_VIDEO_SIZE_1280x720 - HD video width of 1280 and number of lines of
    720. This could be either 720P60, 720P50, 720P30, or 720P25.

    SDVR_VIDEO_SIZE_1920x1080 - HD video width of 1920 and number of lines of
    1080. This could be 1080P25, 1080P30, 1080P50, 1080P60, 1080I50, or
    1080I60. @NOTE: The actual frame size sent by the DVR board is 1920x1072
    if connected to any firmware prior to version 7.0.@

    SDVR_VIDEO_SIZE_960x528 - One fourth of 1080P or 1080I HD video with
    width of 960 and number of lines of 528. @NOTE: This is not a standard
    size that should be 960x540.@

    SDVR_VIDEO_SIZE_480x256 - One sixteenth of 1080P or 1080I HD video with
    width of 480 and number of lines of 256. @NOTE: This is not a standard
    size that should be 480x270.@

    SDVR_VIDEO_SIZE_640x352 - One fourth of 720P or 720I HD video with
    width of 640 and number of lines of 352. @NOTE: This is not a standard
    size that should be 640x360.@

    SDVR_VIDEO_SIZE_320x176 - One sixteenth of 720P or 720I HD video with
    width of 320 and number of lines of 176. @NOTE: This is not a standard
    size that should be 320x180.@
************************************************************************/
typedef enum _sdvr_video_size_e {
    SDVR_VIDEO_SIZE_CUSTOM  = 0,
    SDVR_VIDEO_SIZE_720x576 = (1 << 0),
    SDVR_VIDEO_SIZE_720x480 = (1 << 1),
    SDVR_VIDEO_SIZE_352x288 = (1 << 2),
    SDVR_VIDEO_SIZE_352x240 = (1 << 3),
    SDVR_VIDEO_SIZE_704x288 = (1 << 4),
    SDVR_VIDEO_SIZE_704x240 = (1 << 5),
    SDVR_VIDEO_SIZE_704x576 = (1 << 6),
    SDVR_VIDEO_SIZE_704x480 = (1 << 7),
    SDVR_VIDEO_SIZE_176x144 = (1 << 8),
    SDVR_VIDEO_SIZE_176x112 = (1 << 9),
    SDVR_VIDEO_SIZE_528x320 = (1 << 10),
    SDVR_VIDEO_SIZE_528x384 = (1 << 11),
    SDVR_VIDEO_SIZE_720x288 = (1 << 12),
    SDVR_VIDEO_SIZE_720x240 = (1 << 13),
    SDVR_VIDEO_SIZE_1280x720 = (1 << 14),
    SDVR_VIDEO_SIZE_1920x1080 = (1 << 15),
    SDVR_VIDEO_SIZE_960x528 = (1 << 16),
    SDVR_VIDEO_SIZE_480x256 = (1 << 17),
    SDVR_VIDEO_SIZE_640x352 = (1 << 18),
    SDVR_VIDEO_SIZE_320x176 = (1 << 19),
    SDVR_VIDEO_SIZE_960x540 = (1 << 20),
    SDVR_VIDEO_SIZE_480x270 = (1 << 21),
    SDVR_VIDEO_SIZE_176x120 = (1 << 22),
    SDVR_VIDEO_SIZE_320x180 = (1 << 23),
    SDVR_VIDEO_SIZE_640x360 = (1 << 24)

} sdvr_video_size_e;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_chan_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_chan_type_e;

/****************************************************************************
  VISIBLE: This enumerated type describes the kinds of channels supported
  by SDVR. To create a channel that only allows it to be used in HMO or
  SMO, you must use SDVR_CHAN_TYPE_ENCODER, and set the encoder type to
  SDVR_VIDEO_ENC_NONE.

    SDVR_CHAN_TYPE_NONE - Channel type not specified.

    SDVR_CHAN_TYPE_ENCODER - Encoder channel.

    SDVR_CHAN_TYPE_HOST_ENCODER - Encoder channel for host-supplied video.

    SDVR_CHAN_TYPE_DECODER - Decoder channel.

    SDVR_CHAN_TYPE_EMO     - Encodes tiled monitor video derived from
    multiple cameras or decoders. See sdvr_create_emo_port() for detailed
    description.

    SDVR_CHAN_TYPE_AOUT - To be used only by the SDK. The Application
    should not use this channel type.

    SDVR_CHAN_TYPE_OUTPUT - To be used only by the SDK. The Application
    should not use this channel type.
****************************************************************************/
typedef enum _sdvr_chan_type_e {
    SDVR_CHAN_TYPE_NONE = 255,
    SDVR_CHAN_TYPE_ENCODER = 0,
    SDVR_CHAN_TYPE_HOST_ENCODER = 1,
    SDVR_CHAN_TYPE_DECODER = 2,
    SDVR_CHAN_TYPE_EMO     = 0xD,
    SDVR_CHAN_TYPE_AOUT      = 0xE,
    SDVR_CHAN_TYPE_OUTPUT    = 0xF
}__sdvr_chan_type_e;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_vpp_mode_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_vpp_mode_e;

/************************************************************************
    VISIBLE: Enumerated type describing video pre-processing modes.

    @SDVR_VPP_MODE_ANALYTICS@    Run VPP in analytics mode.

    @SDVR_VPP_MODE_SLATERAL@     Run VPP in Stretch-lateral-filter mode.

    @NOTE@: These enums are deprecated and should not be used.
************************************************************************/
enum _sdvr_vpp_mode_e {
    SDVR_VPP_MODE_SLATERAL   = 0,
    SDVR_VPP_MODE_ANALYTICS  = 1,
};


/************************************************************************
    VISIBLE: Enumerated type describing different CODEC actions that can
    be redirected to a specific PE per channel.

    @SDVR_CODEC_ACTION_SECONDARY_VENCODER@ - Secondary Video Encoding.

    @SDVR_CODEC_ACTION_VDECODER@ - Video Decoding. This action is
    not currently supported.

************************************************************************/
typedef enum _sdvr_codec_action_e {
    SDVR_CODEC_ACTION_SECONDARY_VENCODER,
    SDVR_CODEC_ACTION_VDECODER
}sdvr_codec_action_e;

/****************************************************************************
  VISIBLE: Enumerated type describing encoder subchannels supported
  by the SDVR.

    SDVR_ENC_PRIMARY - The primary encoder.

    SDVR_ENC_SECONDARY - The secondary encoder.

  Remarks:

    You should avoid using this enum starting version 7.2.

    @NOTE@: This enums is deprecated and should not be used.
****************************************************************************/
typedef enum _sdvr_sub_encoders_e {
    SDVR_ENC_PRIMARY = 0,
    SDVR_ENC_SECONDARY
} sdvr_sub_encoders_e;

/****************************************************************************
  VISIBLE: Enumerated type describing video encoders supported by the SDVR.

    SDVR_VIDEO_ENC_NONE        - No video encoder specified.

    SDVR_VIDEO_ENC_H264        - H.264 encoder.

    SDVR_VIDEO_ENC_JPEG        - Motion JPEG encoder.

    SDVR_VIDEO_ENC_MPEG4       - MPEG4 encoder.

    SDVR_VIDEO_ENC_H264_SVC    - H264 scalable video codec.  Not supported by the
    decoder channels.

    SDVR_VIDEO_ENC_MPEG2       - MPEG2 video codec.  Not supported by the
    decoder channels; nor, it is supported by 7.x firmwares.

    SDVR_VIDEO_ENC_RAW_YUV_420 - Raw YUV 4:2:0 video - video-out decoder
    channel type only.

****************************************************************************/
typedef enum _sdvr_venc_e {
    SDVR_VIDEO_ENC_NONE         = 0,
    SDVR_VIDEO_ENC_H264         = 1,
    SDVR_VIDEO_ENC_JPEG         = 2,
    SDVR_VIDEO_ENC_MPEG4        = 3,
    SDVR_VIDEO_ENC_H264_SVC     = 4,
    SDVR_VIDEO_ENC_MPEG2        = 5,
    SDVR_VIDEO_ENC_RAW_YUV_420  = 6
} sdvr_venc_e;

/************************************************************************
  VISIBLE: Enumerated type describing audio sampling rates supported by
  the SDVR.

    SDVR_AUDIO_RATE_NONE    - Not specified.

    SDVR_AUDIO_RATE_8KHZ    - 8KHz audio sampling.

    SDVR_AUDIO_RATE_16KHZ   - 16KHz audio sampling.

    SDVR_AUDIO_RATE_32KHZ   - 32KHz audio sampling.
************************************************************************/
typedef enum _sdvr_audio_rate_e {
    SDVR_AUDIO_RATE_NONE  = 0,
    SDVR_AUDIO_RATE_8KHZ  = 1,
    SDVR_AUDIO_RATE_16KHZ = 2,
    SDVR_AUDIO_RATE_32KHZ = 3
} sdvr_audio_rate_e;


/****************************************************************************
  VISIBLE: Enumerated type describing audio encoders supported by the SDVR.

    SDVR_AUDIO_ENC_NONE - No audio encoder specified.

    SDVR_AUDIO_ENC_G711 - G.711 audio encoder. This is the only audio CODEC
    currently supported.

    SDVR_AUDIO_ENC_G726_16 - G.726 audio encoder at 16K bits/sec.

    SDVR_AUDIO_ENC_G726_24 - G.726 audio encoder at 24K bits/sec.

    SDVR_AUDIO_ENC_G726_32 - G.726 audio encoder at 32K bits/sec.

    SDVR_AUDIO_ENC_G726_48 - G.726 audio encoder at 48K bits/sec.

****************************************************************************/
typedef enum _sdvr_aenc_e {
    SDVR_AUDIO_ENC_NONE,
    SDVR_AUDIO_ENC_G711,
    SDVR_AUDIO_ENC_G726_16K,
    SDVR_AUDIO_ENC_G726_24K,
    SDVR_AUDIO_ENC_G726_32K,
    SDVR_AUDIO_ENC_G726_48K
} sdvr_aenc_e;

/****************************************************************************
    VISIBLE: The following enum describes various encoded audio modes

    SDVR_ENC_AUDIO_MODE_NONE - Un-specified audio encoding mode.

    SDVR_ENC_AUDIO_MODE_MONO - The encoded audio is always 8 bit mono.

    SDVR_ENC_AUDIO_MODE_STEREO - The encoded audio is always 16 bit stereo.
****************************************************************************/
typedef enum _sdvr_enc_audio_mode_e {
    SDVR_ENC_AUDIO_MODE_NONE = 0,
    SDVR_ENC_AUDIO_MODE_MONO    = 1,
    SDVR_ENC_AUDIO_MODE_STEREO  = 2
} sdvr_enc_audio_mode_e;

/****************************************************************************
    VISIBLE: The following enum describes various FOSD font table ID

    SDVR_FOSD_FONT_TABLE_LARGE - use FOSD large font talbe.

    SDVR_FOSD_FONT_TABLE_MEDIUM - use FOSD medium font talbe.

    SDVR_FOSD_FONT_TABLE_SMALL - use FOSD small font talbe.
****************************************************************************/
typedef enum _sdvr_font_table_id_e {
    SDVR_FOSD_FONT_TABLE_LARGE  = 0,
    SDVR_FOSD_FONT_TABLE_MEDIUM = 1,
    SDVR_FOSD_FONT_TABLE_SMALL  = 2
} sdvr_font_table_id_e;


/****************************************************************************
  VISIBLE: Enumerated type describing the various encoding and display
  resolution decimation.

  When configuring the DVR, the Application sets the maximum
  system-wide resolution. The video resolution for a particular channel
  is described in terms of this maximum resolution. The video resolution
  for a channel can be decimated to be equal to, 1/4 of (CIF), 1/16 (QCIF), HALF of,
  or DCIF of the maximum system-wide resolution. It can also be set as an absolute
  resolution of D1, 4CIF, or 2CIF that is not tied to the video standard except
  whether it is PAL or NTSC. In the case of classic
  CIF, 2CIF, or 4CIF resolutions, the video sizes will be generated
  by scaling (if needed) followed by cropping. Classic CIF and 2CIF
  require scaling before cropping, Classic 4CIF requires cropping only.

    SDVR_VIDEO_RES_DECIMATION_NONE - No resolution set.

    SDVR_VIDEO_RES_DECIMATION_EQUAL - Same resolution as the currently specified
    video standard.
    For D1, it is 720x480 for NTSC and 720x576 for PAL. For 4CIF, it is 704x480
    for NTSC and 704x576 for PAL. For CIF, it is
    352x240 for NTSC and 352x288 for PAL.

    SDVR_VIDEO_RES_DECIMATION_CIF - Absolute CIF size. CIF
    size is 352x240 for NTSC and 352x288 for PAL. Use SDVR_VIDEO_RES_CIF instead.

    SDVR_VIDEO_RES_DECIMATION_FOURTH - Same as SDVR_VIDEO_RES_DECIMATION_CIF.
    Kept for backward compatibility. Use SDVR_VIDEO_RES_CIF instead.

    SDVR_VIDEO_RES_CIF -   Absolute CIF size.  352x240 (NTSC) or 352x288 (PAL).
    Same as SDVR_VIDEO_RES_DECIMATION_CIF. In case of HD, it is 1/4 of the
    HD resolution. 640x360 (NTSC or PAL). This resolution is only supported if
    the current video standard is 4CIF (NTSC or PAL) or HD.

    SDVR_VIDEO_RES_DECIMATION_QCIF - Absolute QCIF size.
    QCIF size is 176x120 for NTSC and 176x144 for PAL. Use SDVR_VIDEO_RES_QCIF
    instead. @NOTE: The actual NTSC frame size sent by the DVR board is 176x112
    if connected to any firmware prior to version 7.0.@

    SDVR_VIDEO_RES_DECIMATION_SIXTEENTH - Same as SDVR_VIDEO_RES_DECIMATION_QCIF.
    Kept for backward compatibility. Use SDVR_VIDEO_RES_QCIF instead.

    SDVR_VIDEO_RES_QCIF -  Absolute QCIF size. 176x120 (NTSC) or 176x144 (PAL).
    Same as SDVR_VIDEO_RES_DECIMATION_QCIF. In case of HD, it is 1/16 of the
    HD resolution. 320x176 (NTSC or PAL). This resolution is only supported if
    the current video standard is 4CIF (NTSC or PAL) or HD.
    @NOTE: The actual NTSC frame size sent by the DVR board is 176x112
    if connected to any firmware prior to version 7.0.@

    SDVR_VIDEO_RES_DECIMATION_HALF - It is same width as
    SDVR_VIDEO_RES_DECIMATION_EQUAL but one-half the height. Not supported by
    the H.264- SVC video encoder.

    SDVR_VIDEO_RES_DECIMATION_DCIF - DCIF size. It is 528x320 for NTSC and
    528x384 for PAL. This resolution is only supported for video encoding except
    H.264-SVC.

    SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF - 320x240 - Only supported for
    streaming raw video frames by firmware versions prior to 7.x.

    SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF - 640x240  - Only supported for
    streaming raw video frames versions prior to 7.x.

    SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF - 640x480  - Only supported for
    streaming raw video frames versions prior to 7.x.

    SDVR_VIDEO_RES_D1 - Absolute D1 size.  720x480 (NTSC) or 720x576 (PAL).

    SDVR_VIDEO_RES_4CIF - Absolute 4CIF size. 704x480 (NTSC) or 704x576 (PAL).

    SDVR_VIDEO_RES_2CIF -  Absolute 2CIF size. 704x240 (NTSC) or 704x288 (PAL).
    Only supported if current video standard is 4CIF (NTSC or PAL). Not supported
    by the H.264-SVC encoder.

    SDVR_VIDEO_RES_720P - 1280x720 resolution for both 720P50 and 720P60.

    SDVR_VIDEO_RES_1080I - 1920x1080 resolution for both 1080I50 and 1080I60.
    Not supported for encoding.

    SDVR_VIDEO_RES_1080P - 1920x1080 resolution for 1080P25, 1080P30, 1080P50,
    and 1080P60.

    SDVR_VIDEO_RES_CUSTOM - Arbitrary output size. This resolution decimation
    is only supported for video encoding. The video frame can be cropped and
    resized based on full, 1/4, or 1/16 of source video frame. These information
    are provided at the time of setting the video encoder parameters.
    Not all boards may support custom output size.

****************************************************************************/
typedef enum _sdvr_video_res_decimation_e {
    SDVR_VIDEO_RES_DECIMATION_NONE = 0,
    SDVR_VIDEO_RES_DECIMATION_EQUAL = 1,
    SDVR_VIDEO_RES_DECIMATION_CIF = 2,
    SDVR_VIDEO_RES_DECIMATION_FOURTH = SDVR_VIDEO_RES_DECIMATION_CIF,
    SDVR_VIDEO_RES_CIF = SDVR_VIDEO_RES_DECIMATION_CIF,
    SDVR_VIDEO_RES_DECIMATION_QCIF = 4,
    SDVR_VIDEO_RES_DECIMATION_SIXTEENTH = SDVR_VIDEO_RES_DECIMATION_QCIF,
    SDVR_VIDEO_RES_QCIF = SDVR_VIDEO_RES_DECIMATION_QCIF,
    SDVR_VIDEO_RES_DECIMATION_HALF = 5,
    SDVR_VIDEO_RES_DECIMATION_DCIF = 6,
    SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF = 7,
    SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF = 8,
    SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF = 9,
    SDVR_VIDEO_RES_D1 = 10,
    SDVR_VIDEO_RES_4CIF = 11,
    SDVR_VIDEO_RES_2CIF = 12,
    SDVR_VIDEO_RES_720P = 13,
    SDVR_VIDEO_RES_1080I = 14,
    SDVR_VIDEO_RES_1080P = 15,
    SDVR_VIDEO_RES_CUSTOM = 128

} sdvr_video_res_decimation_e;

/************************************************************************
    VISIBLE: Image source selections for cropping the video frame.

    SDVR_CROP_SRC_FULL - Select the full input picture as source.

    SDVR_CROP_SRC_FOURTH - Select the 1/4 of the input picture as source.

    SDVR_CROP_SRC_SIXTEENTH - Select the 1/16 of the input picture as source.
************************************************************************/
typedef enum _sdvr_crop_src_e {
    SDVR_CROP_SRC_FULL,
    SDVR_CROP_SRC_FOURTH,
    SDVR_CROP_SRC_SIXTEENTH
} sdvr_crop_src_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various bit rate control schemes
  available in the SDVR.

    SDVR_BITRATE_CONTROL_NONE - No bit rate control.

    SDVR_BITRATE_CONTROL_VBR - Variable bit rate.

    SDVR_BITRATE_CONTROL_CBR - Constant bit rate.

    SDVR_BITRATE_CONTROL_CBR_S - Constant bit rate with strict limit on
    bitrate fluctuation.

    SDVR_BITRATE_CONTROL_CQP - Quantization parameter - Not Supported.

    SDVR_BITRATE_CONTROL_CONSTANT_QUALITY - Constant quality bit rate.
    Not supported by MPEG4 encoder in any 7.x firmware.

****************************************************************************/
typedef enum _sdvr_br_control_e {
    SDVR_BITRATE_CONTROL_NONE = 255,
    SDVR_BITRATE_CONTROL_VBR = 0,
    SDVR_BITRATE_CONTROL_CBR,
    SDVR_BITRATE_CONTROL_CQP,
    SDVR_BITRATE_CONTROL_CONSTANT_QUALITY,
    SDVR_BITRATE_CONTROL_CBR_S,
} sdvr_br_control_e;

/****************************************************************************
    VISIBLE: Enumerated type describing the date and time display styles
    supported by the SDVR.

    SDVR_OSD_DTS_NONE - No date and time is displayed after the
    OSD text.

    SDVR_OSD_DTS_DEBUG - Enables a special debug display mode.
    @NOTE@: The debug display is for testing only and must not be exposed
    to end users. The format of the debug display can change at any time.
    Not all boards or output devices may support the debug display mode.

    SDVR_OSD_DTS_DEBUG_2 - See notes above.

    SDVR_OSD_DTS_DEBUG_3 - See notes above.

    The following styles display the name of the month in 3-letter format
    and the year in 2-digit format.

    SDVR_OSD_DTS_MDY_12H - Displays the date in MMM-DD-YY format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_DMY_12H - Displays the date in DD-MMM-YY format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_YMD_12H - Displays the date in YY-MMM-DD format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_MDY_24H - Displays the date in MMM-DD-YY format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_DMY_24H - Displays the date in DD-MMM-YY format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_YMD_24H - Displays the date in YY-MMM-DD format
    followed by time in 24 hour HH:MM:SS format.

    The following styles print the month in numeric format and the year
    in full 4-digit format.

    SDVR_OSD_DTS_MDY_12H_NUM - Displays the date in MM-DD-YYYY format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_DMY_12H_NUM - Displays the date in DD-MM-YYYY format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_YMD_12H_NUM - Displays the date in YYYY-MM-DD format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_MDY_24H_NUM - Displays the date in MM-DD-YYYY format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_DMY_24H_NUM - Displays the date in DD-MM-YYYY format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_YMD_24H_NUM - Displays the date in YYYY-MM-DD format
    followed by time in 24 hour HH:MM:SS format.
****************************************************************************/
typedef enum _sdvr_dts_style_e {
    SDVR_OSD_DTS_NONE = 0,
    SDVR_OSD_DTS_DEBUG,
    SDVR_OSD_DTS_MDY_12H,
    SDVR_OSD_DTS_DMY_12H,
    SDVR_OSD_DTS_YMD_12H,
    SDVR_OSD_DTS_MDY_24H,
    SDVR_OSD_DTS_DMY_24H,
    SDVR_OSD_DTS_YMD_24H,
    SDVR_OSD_DTS_DEBUG_2,
    SDVR_OSD_DTS_MDY_12H_NUM,
    SDVR_OSD_DTS_DMY_12H_NUM,
    SDVR_OSD_DTS_YMD_12H_NUM,
    SDVR_OSD_DTS_MDY_24H_NUM,
    SDVR_OSD_DTS_DMY_24H_NUM,
    SDVR_OSD_DTS_YMD_24H_NUM,
    SDVR_OSD_DTS_DEBUG_3
} sdvr_dts_style_e;

/************************************************************************
    VISIBLE: Chip revision definitions.
************************************************************************/
typedef enum _sdvr_chip_rev_e {
    SDVR_CHIP_S6100_3_REV_C = 0,
    SDVR_CHIP_S6105_3_REV_C,
    SDVR_CHIP_S6106_3_REV_C,
    SDVR_CHIP_S6100_3_REV_D = 16,
    SDVR_CHIP_S6105_3_REV_D,
    SDVR_CHIP_S6106_3_REV_D,
    SDVR_CHIP_S6100_3_REV_F = 32,
    SDVR_CHIP_S6105_3_REV_F,
    SDVR_CHIP_S6106_3_REV_F,
    SDVR_CHIP_S6100_3_UNKNOWN = 48,
    SDVR_CHIP_S6105_3_UNKNOWN,
    SDVR_CHIP_S6106_3_UNKNOWN,
    SDVR_CHIP_S7100_UNKNOWN = 64,
    SDVR_CHIP_S7100_REV_A,
    SDVR_CHIP_S7100_REV_B,
    SDVR_CHIP_S7100_REV_C,
    SDVR_CHIP_UNKNOWN = 255
} sdvr_chip_rev_e;

/************************************************************************
    VISIBLE: Enumerated type describing watchdog control options.

    @SDVR_WATCHDOG_DISABLE@ - Disable WDT.

    @SDVR_WATCHDOG_SCP@ - Enable WDT with SCP controlling refresh.

    @SDVR_WATCHDOG_HOST@ - Enable WDT with host controlling refresh.

    @SDVR_WATCHDOG_REFRESH@ - Refresh host-controlled WDT.

    NOTE: DISABLE and SCP enums must line up with previous API's false/true
    defines; to preserve backward-compatibility, do not change these values.
************************************************************************/
typedef enum _sdvr_watchdog_control_e {
    SDVR_WATCHDOG_DISABLE   = 0,
    SDVR_WATCHDOG_SCP       = 1,
    SDVR_WATCHDOG_HOST      = 2,
    SDVR_WATCHDOG_REFRESH   = 3
} sdvr_watchdog_control_e;

/*******************************************************************************
    Length of the serial number string.
*******************************************************************************/
#define SDVR_BOARD_SERIAL_LENGTH      16

/****************************************************************************
  VISIBLE: This data structure holds board attributes.

    @pci_slot_num@ - The PCI slot number in which the board is located.

    @board_type@ - The board type that is combination of device_id and
    the subsystem_vendor.

    @supported_video_stds@ - Specifies what cameras are supported by the board.
    It is a bit-wise OR of video standards, such as @SDVR_VIDEO_STD_D1_PAL@
    or @SDVR_VIDEO_STD_D1_NTSC@.

    @chip_revision@ - The Stretch chip number and revision. Refer to
    @sdvr_chip_rev_e@ for a list of Stretch chip numbers.

    @board_revision@ - The board revision.

    @board_sub_rev@- A character in the range of A-Z representing the board
    sub revision. This field is not used prior to firmware version 4.1.x.x.

    @max_recv_buf_count@ - The total count per channels to receive A/V or
    any other kind of buffer from the firmware. NOTE: The buffer counts when
    calling sdvr_create_chan_ex() can not exceed this number.

****************************************************************************/
typedef struct _sdvr_board_attrib_t {
    sx_uint32      pci_slot_num;
    sx_uint32      board_type;
    sx_uint32      supported_video_stds;
    sdvr_chip_rev_e chip_revision;
    sx_uint32      board_revision;
    sx_uint8       board_sub_rev;
    sx_uint32      max_recv_buf_count;
} sdvr_board_attrib_t;

/****************************************************************************
  VISIBLE: This data structure holds PCIe board attributes. You get these
  attributes by calling sdvr_get_pci_attrib(). This function can be called
  before or after loading of the firmware into the DVR board.

    @pci_slot_num@ - The PCI slot number in which the board is located.

    @board_type@ - The board type which is a combination of device_id and
    the subsystem_vendor.

    @vendor_id@ - Always Stretch (0x18A2)

    @device_id@ - Board ID per each vendor.

    @subsystem_vendor@ - Vendor ID. For Stretch boards it is (0x18A2).

    @subsystem_id@ - Currently is always set to zero.

    @serial_number@ - A null terminated serial number string.
****************************************************************************/
typedef struct _sdvr_pci_attrib_t {
    sx_uint32       pci_slot_num;
    sx_uint32       board_type;
    sx_uint16       vendor_id;
    sx_uint16       device_id;
    sx_uint16       subsystem_vendor;
    sx_uint16       subsystem_id;
    sx_uint8        serial_number[SDVR_BOARD_SERIAL_LENGTH + 1];
} sdvr_pci_attrib_t;

/****************************************************************************
  VISIBLE: These debug flags are available to help you turn on
  debugging in the SDK.

    @DEBUG_FLAG_DEBUGGING_ON@ - Turn on debugging.

    @DEBUG_FLAG_ALL@ - Turn on all debugging flags.

    @DEBUG_FLAG_WRITE_TO_FILE@ - Turn on writing debug information to file.

    @DEBUG_FLAG_LIMIT_LOGFILE_SIZE@ - Debug info file size is limited to 2 MB.

    @DEBUG_FLAG_OUTPUT_TO_SCREEN@ - Write debug information to TTY.

    @DEBUG_FLAG_ENCODER@ - Turn on debugging for encoder.

    @DEBUG_FLAG_DECODER@ - Turn on debugging for decoder.

    @DEBUG_FLAG_VIDEO_ALARM@ - Turn on debugging for video alarm.

    @DEBUG_FLAG_SENSORS_RELAYS@ - Turn on debugging for sensors and relays.

    @DEBUG_FLAG_AUDIO_OPERATIONS@ - Turn on debugging for audio operations.

    @DEBUG_FLAG_DISPLAY_OPERATIONS@ - Turn on debugging for video operations.

    @DEBUG_FLAG_BOARD@ - Turn on debugging for board configuration.

    @DEBUG_FLAG_GENERAL_SDK@ - Turn on debugging for SDK configuration.

    @DEBUG_FLAG_SMO@ - Turn on debugging for SMO configuration.

    @DEBUG_FLAG_OSD@ - Turn on debugging for OSD configuration.

    @DEBUG_FLAG_CHANNEL@ - Turn on debugging for channel configuration.

    @DEBUG_FLAG_VIDEO_FRAME@ - Turn on debugging of video frames.

    @DEBUG_FLAG_FW_WRITE_TO_FILE@ - Turn on firmware only messages.

    @DEBUG_FLAG_RECORD_TO_FILE@ - Turn on debugging of recording the
    audio and video frame to a file.

    @DEBUG_FLAG_EMO@ - Turn on debugging for Encoded Monitor Out stream.

    @DEBUG_FLAG_MANAGE_FRAMES@ - Turn on debugging of low level frame management

    @DEBUG_FLAG_AUDIO_FRAME@ - Turn on debugging of audio frames.

    @DEBUG_FLAG_WATCHDOG@ - Turn on debugging of watchdog message exchanges.
 
    @DEBUG_FLAG_FOSD@ - Turn on debugging for FOSD configuration.
    
****************************************************************************/
#define DEBUG_FLAG_DEBUGGING_ON       0x1
#define DEBUG_FLAG_ALL                0xFFFFFFFE
#define DEBUG_FLAG_WRITE_TO_FILE      (1 << 1)  /*0x2*/
#define DEBUG_FLAG_OUTPUT_TO_SCREEN   (1 << 2)  /*0x4*/
#define DEBUG_FLAG_ENCODER            (1 << 3)  /*0x8*/
#define DEBUG_FLAG_DECODER            (1 << 4)  /*0x10*/
#define DEBUG_FLAG_VIDEO_ALARM        (1 << 5)  /*0x20*/
#define DEBUG_FLAG_SENSORS_RELAYS     (1 << 6)  /*0x40*/
#define DEBUG_FLAG_AUDIO_OPERATIONS   (1 << 7)  /*0x80*/
#define DEBUG_FLAG_DISPLAY_OPERATIONS (1 << 8)  /*0x100*/
#define DEBUG_FLAG_BOARD              (1 << 9)  /*0x200*/
#define DEBUG_FLAG_GENERAL_SDK        (1 << 10) /*0x400*/
#define DEBUG_FLAG_SMO                (1 << 11) /*0x800*/
#define DEBUG_FLAG_OSD                (1 << 12) /*0x1000*/
#define DEBUG_FLAG_CHANNEL            (1 << 13) /*0x2000*/
#define DEBUG_FLAG_VIDEO_FRAME        (1 << 14) /*0x4000*/
#define DEBUG_FLAG_FW_WRITE_TO_FILE   (1 << 15) /*0x8000*/
#define DEBUG_FLAG_RECORD_TO_FILE     (1 << 16) /*0x00010000*/
#define DEBUG_FLAG_EMO                (1 << 17) /*0x00020000*/
#define DEBUG_FLAG_MANAGE_FRAMES      (1 << 18) /*0x00040000*/
#define DEBUG_FLAG_AUDIO_FRAME        (1 << 19) /*0x00080000*/
#define DEBUG_FLAG_LIMIT_LOGFILE_SIZE (1 << 20) /*0x00100000*/
#define DEBUG_FLAG_WATCHDOG           (1 << 21) /*0x00200000*/
#define DEBUG_FLAG_FOSD               (1 << 22) /*0x00400000*/

/****************************************************************************
  VISIBLE: This data structure is used to hold SDK configuration
  parameters.

  To exchange encoded and raw video with the board, the SDK needs to
  allocate buffers to hold the data on its way to and from the DVR
  Application and the board. The number of buffers to be allocated on
  each of these paths and the sizes of these buffers are set using this
  data structure.

  We recommend that you use the default values for each frame buffer as if they
  are optimized for streaming at 30 fps for NTSC, and 25fps for PAL video standards.

    @enc_buf_num@ - The number of buffers to be allocated for each encoder
    channel. Each encoder channel will have the same number of
    buffers allocated.
    It is important that you allocate enough buffers for encoded frames
    to be held between the times you can process them. The maximum allowed encoded
    audio and video frame buffer size is 22.

    @raw_buf_num@ - The number of buffers to be allocated for each channel that
    will be sending raw video to the Application. Each channel sending
    raw video will have the same number of buffers.
    Typically, you will need to display raw frames 30 times per second.
    Therefore, you only need between 2 and 4 buffers to hold raw video.
    The maximum allowed raw audio and video frame buffer size is 5.

    @dec_buf_num@ - The number of buffers to be allocated for each decoder
    channel. Each decoder channel will have the same number of buffers
    allocated. The maximum number of decoder buffers allowed is 5.

    @dec_buf_size@ - The size of each buffer used to hold encoded frames on
    the way to the decoder hardware. Typically, this is the size of the
    largest encoded frame that needs to be decoded.
    The same buffer size is used across all decoder channels.
    The default value is 256K.

    The following table shows the required buffer size for various full
    size video frames:
    {
        +------------------------------------------+------------------------+
        |            Encoded video frames          |                        |
        |------------------------------------------|  4-2-0 raw video frame |
        |     SD     |     HD-720     |   HD-1080  |                        |
        |------------+----------------+------------|------------------------|
        |    256K    |   3 * 256K     |  6 * 256K  | width * height * 1.5   |
        +------------+----------------+------------+------------------------+

    Note: The width for all Y, U, V components must be rounded up to
          be multiple of 16.
    }

    @timeout@ - For a variety of reasons, it is possible for the board to
    hang. By setting the timeout value, you specify when the SDK will
    give up on a response from the board and inform the Application
    that the board is hung. The value of this parameter is in seconds.
    A value of 0 indicates that there is no timeout and the SDK will
    wait indefinitely for the board to respond.
    Do not set the value of timeout too low, or during times of heavy
    traffic on the bus, you might get a false warning that the board
    has hung. Setting the value to 0 (no timeout), may cause the
    PC to hang if the firmware on the board dies.
    The default value is 10 seconds except in ARM Linux  it is set to 20 seconds.

    @debug_flag@ - This is a bit field of flags that can be set to enable
    various levels of debugging as defined by the debug flags.
    See the defines for DEBUG_FLAG_xxx for the definition of each field.
    Setting debugging flags has a noticeable effect on
    system performance. The default value is zero.

    @debug_file_name@ - The name of the file where the debug information
    is stored. This string should include the full path name to the
    file, or a file in the current working directory (depends on the OS
    as to how this is defined) is created.
    If the file does not exist, it is created. If it already exists,
    it will be truncated. You also must enable the DEBUG_FLAG_WRITE_TO_FILE
    bit in the @debug_flag@ field to save the debugging information
    into the file. In addition to the given specified @debug_file_name@, which
    includes all the SDK tracing, a new file will be created with @_fw@ appended
    to the @debug_file_name@, which contains all the low level commands sent to the
    DVR firmware.

    Remarks:

    The size of each buffer is determined by the firmware. Additionally, the
    default value of each SDK buffer depends on the OS where the host application
    is running. Call sdvr_get_sdk_params() to get these default values.

****************************************************************************/
typedef struct _sdvr_sdk_params_t {
    sx_uint32 enc_buf_num;
    sx_uint32 raw_buf_num;
    sx_uint32 dec_buf_num;
    sx_uint32 dec_buf_size;
    sx_uint32 timeout;
    sx_uint32 debug_flag;
    char *debug_file_name;
} sdvr_sdk_params_t;

/****************************************************************************
  VISIBLE: This data structure defines the capabilities of the SDVR boards.

    @num_cameras_supported@ - Number of cameras supported by the board, i.e.,
    the number of physical camera connectors on the board.

    @num_microphones_supported@ - Number of microphones supported by the
    board, i.e., the number of physical microphone connectors on the board.

    @has_smo@ - If true, this board has a spot monitor output.

    @num_sensors@ - Number of sensors on this board.

    @num_relays@ - Number of relays on this board.

    @camera_type@ - The maximum HD and SD resolution that was specified by
    the Application when connecting to the board. Typically, all
    SD or HD cameras connected to the DVR have this resolution, hence the
    name of the field.

    NOTE: Prior to DVR firmware version 6.0.0.0, all the video inputs assumed to be
    of the same video standard. As a result, this field returns either
    the SD or HD video standard that was specified at the time of
    sdvr_board_connect(). Starting DVR firmware version 6.0.0.0 and higher,
    none-SD only DVR boards, may have mixed SD and HD video standard input
    connected to a board. In such cases, the camera_type has the two
    standards ORed together.
    Additionally, you can get the specific detected video standard of each
    video input port by calling sdvr_get_chan_vstd().

    @num_decoders_supported@ - The number of decoding channel supported
    by the board. This value is zero if the board does not support decoding.

    @num_host_encoders_supported@ - The number of host video encoder channels
    supported by the board. This value is zero if the board does not support 
    host video encoding.

    @num_smos@ - The number of spot monitor output on the DVR board. Check this
    field only if @has_smo@ is true.

    @num_encoders_supported@ - The number of encoder channel supported
    by the board. This value is zero if the board does not support encoding.

    @num_audio_out_ports@ - The number of audio-output ports supported by
    the board.

    @num_emos@ - Number of EMO (Encoded Monitor Out) instances supported
    by the board. This field is either set to zero(0) meaning the DVR firmware
    does not support this feature or one (1) that means there can only be one
    instance of EMO created on the firmware.

    @num_video_encoders_per_camera@ - Maximum number of video CODECs that can be
    assigned to a camera. The camera is HMO only and no video encoding is allowed
    on it.

    @num_raw_video_stream_per_camera@ - Maximum number of raw video stream allowed
    per camera.

    @num_host_encoders_supported@ - The number of host video encoder channels
    supported by the board. This value is zero if the board does not support 
    host video encoding.
****************************************************************************/
typedef struct _sdvr_board_config_t {
    sx_uint32   num_cameras_supported;
    sx_uint32   num_microphones_supported;
    sx_bool     has_smo;
    sx_uint32   num_sensors;
    sx_uint32   num_relays;
    sdvr_video_std_e camera_type;
    sx_uint32   num_decoders_supported;
    sx_uint32   num_smos;
    sx_uint32   num_encoders_supported;
    sx_uint32   num_audio_out_ports;
    sx_uint32   num_emos;
    sx_uint32   num_video_encoders_per_camera;
    sx_uint32   num_raw_video_stream_per_camera;
    sx_uint32   num_host_encoders_supported;
} sdvr_board_config_t;

/************************************************************************
    VISIBLE: Enumerated type describing geometry and refresh for custom
    video.
************************************************************************/
typedef struct _sdvr_vstd_custom_t {
    sx_uint16 w;
    sx_uint16 h;
    sx_uint16 frame_rate;
} sdvr_vstd_custom_t;

/****************************************************************************
  VISIBLE: This data structure defines the capabilities of the SDVR boards.

    @video_std@ - The video standard and maximum system-wide resolution
    of all the SD cameras connected to the board.
    Set this field to zero If connecting to an HD DVR board with firmware
    version prior to 6.0.0.0 or no SD video input source is going to be
    connected.

    @is_h264_SCE@ - This field specifies whether the SDK or the H.264
    encoder should perform  start code emulation (SCE) for h.264 video
    frames. For performance reasons,
    it is highly recommended to always set this field to "true" so that the
    SDK performs SCE, except for the embedded Applications it
    must be set to "false" which means the encoder to perform this task.
    This field is ignored if connecting to any S7 chip type.

    @audio_rate@ - This field specifies the audio sampling rate.
    (See @sdvr_audio_rate_e@ for definition of supported audio
    sampling rates.)

    @hd_video_std@ - The default HD video standard and maximum system-wide
    resolution of all the HD cameras connected to the board. Set this field
    to zero if board does not support HD video inputs or no HD video input
    is going to be connected.

    @enc_audio_mode@ - This field specifies whether the encoded audio should
    be 8 bit mono or 16 bit stereo. (See @sdvr_enc_audio_mode_e@ for
    definition of supported audio modes.) NOTE: This field is ignored if
    audio_rate is set to SDVR_AUDIO_RATE_NONE;

    @omit_blank_frames@ - This field specifies whether the encoded streams,
    HMO stream, and SMO streams should automatically drop video frames when
    signal loss is detected.  No further notification of these frame drops
    will be made beyond the existing video loss signal.  This setting affects
    all streams on this board and may cause the board to buffer a couple of
    additional frames.

    @custom_vstd@ - If specified "video_std" is custom, width, height and
    frame rate are indicated here.

    @reserved1 and reserved2@ - These fields must be initialized to zero.

  Remarks:

    Both the SD and HD video standards defaults must be of the
    same standard. (i,e D1_NTSC and 1080P60).

****************************************************************************/
typedef struct _sdvr_board_settings_t {
    sdvr_video_std_e  video_std;
    sx_bool           is_h264_SCE;
    sdvr_audio_rate_e audio_rate;
    sdvr_video_std_e  hd_video_std;
    sx_uint8          enc_audio_mode;
    sx_uint8          omit_blank_frames;
    sdvr_vstd_custom_t custom_vstd;
    sx_uint8          reserved1[3];
    sx_uint32         reserved2[8];
} sdvr_board_settings_t;

/****************************************************************************
  VISIBLE: This data structure defines the firmware, boot loader and
  BSP version, and build information.

  Stretch follows the convention of using four numbers for version
  control. A change in the major number indicates major changes to
  functionality, a change in the minor number indicates minor
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality.
  A change to the build number indicates only bug fixes that do not
  change functionality.

    @fw_major@ - The firmware major version number. A change in this field
    indicates major changes to functionality.

    @fw_minor@ - The firmware minor version number. A change in this field
    indicates minor changes to functionality.

    @fw_revision@ - The firmware revision version number. A change in this
    field indicates significant bug fixes that were introduced in the minor
    change functionality.

    @fw_build@ - The firmware build version number. A change in this field
    indicates only bug fixes that do not change functionality.

    @fw_build_year@ - The year of firmware build.

    @fw_build_month@ - The month of firmware build.

    @fw_build_day@ - The day of firmware build.

    @bootloader_major@ - The major version number of boot loader.

    @bootloader_minor@ - The minor version number of boot loader.

    @bsp_major@ - The major version number of BSP.

    @bsp_minor@ - The minor version number of BSP.
****************************************************************************/
typedef struct _sdvr_firmware_ver_t {
    sx_uint8 fw_major;
    sx_uint8 fw_minor;
    sx_uint8 fw_revision;
    sx_uint8 fw_build;
    sx_uint16 fw_build_year;
    sx_uint8  fw_build_month;
    sx_uint8  fw_build_day;
    sx_uint8  bootloader_major;
    sx_uint8  bootloader_minor;
    sx_uint8  bsp_major;
    sx_uint8  bsp_minor;
} sdvr_firmware_ver_t;

/****************************************************************************
  VISIBLE: These defines specify the different maximum regions. These limits
  apply to regions that defined based on pixels and not Macro Blocks.

    @SDVR_MAX_MD_REGIONS@ - Maximum number of motion detection regions.

    @SDVR_MAX_BD_REGIONS@ - Maximum number of blind detection regions.

    @SDVR_MAX_PR_REGIONS@ - Maximum number of privacy regions.
****************************************************************************/
#define SDVR_MAX_MD_REGIONS          4
#define SDVR_MAX_BD_REGIONS          1
#define SDVR_MAX_PR_REGIONS          1

/**************************************************************************
  VISIBLE: This defines the maximum number of OSD text characters that can be display as an
  OSD text string.

    @SDVR_MAX_OSD_TEXT@ - The maximum size of OSD text for single byte
    OSD APIs is 10 characters.

    @SDVR_MAX_OSD_EX_TEXT@ - The maximum length of OSD text in OSD APIs
    supporting double byte is 100 unsigned short.
    Available only in firmware version 3.2.0.0 or later.
***************************************************************************/
#define SDVR_MAX_OSD_TEXT          10
#define SDVR_MAX_OSD_EX_TEXT       100

/**************************************************************************
    VISIBLE: This defines the maximum number of OSD item that can be configure
    per each channel. Available only in firmware version 3.2.0.0 or later.
***************************************************************************/
#define SDVR_MAX_OSD                5

/************************************************************************
    VISIBLE: OSD blink flags. These flags can be ORed together to request
    the firmware to blink OSD text item under different conditions.
    Note that each alarm must be enabled for the corresponding flags
    to take effect except for video loss detection which is always enabled.

    If no flag is specified, then blinking is controlled by the show
    state of the OSD, i.e. the text will blink as long as it is shown.

    @SDVR_OSD_BLINK_ON_MD_ALARM@ - Make the OSD blink when the motion
    detection alarm is triggered.

    @SDVR_OSD_BLINK_ON_ND_ALARM@ - Make the OSD blink when the night
    detection alarm is triggered.

    @SDVR_OSD_BLINK_ON_BD_ALARM@ - Make the OSD blink when the blind
    detection alarm is triggered.

    @SDVR_OSD_BLINK_ON_VIDEO_LOSS@ - Make the OSD blink when video
    input is lost.

    @SDVR_OSD_BLINK_ALWAYS@ - make the OSD blink always

************************************************************************/
typedef enum _sdvr_osd_blink_e {
    SDVR_OSD_BLINK_ON_MD_ALARM    = 0x1,
    SDVR_OSD_BLINK_ON_ND_ALARM    = 0x2,
    SDVR_OSD_BLINK_ON_BD_ALARM    = 0x4,
    SDVR_OSD_BLINK_ON_VIDEO_LOSS  = 0x8,
    SDVR_OSD_BLINK_ALWAYS         = 0x10
} sdvr_osd_blink_e;

/**************************************************************************
    VISIBLE: This defines the invalid channel handle.
***************************************************************************/
#define INVALID_CHAN_HANDLE (sx_int32)0xFFFFFFFF


/****************************************************************************
  VISIBLE: The DVR firmware sends asynchronous signal messages to the DVR
  Application as it encounters errors not related to any direct function
  call.
  This data structure defines parameters associated with the asynchronous
  signals sent from the DVR firmware to the host Application.

    "sig_type" - The type identifying the signal cause.

    "chan_type" -  The type of channel causing the signal.

    "chan_num" - The ID of channel causing the signal.

    "data" - The error code associated with the sig_type.

            For @SDVR_SIGNAL_RUNTIME_ERROR@,    it is the error code.

            For @SDVR_SIGNAL_FATAL_ERROR@,      it is the error code.

            For @SDVR_SIGNAL_WATCHDOG_EXPIRED@, there is no data.

            For @SDVR_SIGNAL_PCI_CONGESTION@,   there is no data.

            For @SDVR_SIGNAL_TEMPERATURE@,      it is the temperature in degrees centigrade (C).

            For @SDVR_SIGNAL_FRAME_TOO_LARGE@,  it is the stream ID.

            For @SDVR_SIGNAL_MEMORY_USAGE@,  it is the DDR memory usage value of PE

    "extra_data" - Optional data information associated with the
    signal_data.

            For @SDVR_SIGNAL_RUNTIME_ERROR@,   it is the extended error code.

            For @SDVR_SIGNAL_FATAL_ERROR@,     it is the extended error code.

            For @SDVR_SIGNAL_TEMPERATURE@,     it is the fractional part of the
            temperature in tenths of a degrees centigrade (C).

            For @SDVR_SIGNAL_FRAME_TOO_LARGE@, it is the frame size in bytes.

            For @SDVR_SIGNAL_MEMORY_USAGE@,  it is the PE id.

    @NOTE@: For the user-defined signals (i.e. @SDVR_SIGNAL_USER_1@ and
    @SDVR_SIGNAL_USER_2@) all fields other than "sig_type" can be used
    for any purpose.

***************************************************************************/

typedef struct sdvr_signal_info {
    sdvr_signals_type_e sig_type;
    sdvr_chan_type_e    chan_type;
    sx_uint8            chan_num;
    sx_uint8            reserved1;
    sx_uint32           data;
    sx_uint32           extra_data;
    sx_uint32           reserved3;
} sdvr_signal_info_t;

/****************************************************************************
  VISIBLE: Enumerated type describing various camera input impedance
  termination.

    "SDVR_TERM_75OHM" - 75 ohm impedance termination.

    "SDVR_TERM_HIGH_IMPEDANCE" - High impedance termination.

***************************************************************************/
typedef enum _sdvr_term_e {
    SDVR_TERM_75OHM,
    SDVR_TERM_HIGH_IMPEDANCE
} sdvr_term_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various LED types.

    "SDVR_LED_TYPE_RECORD" - A group of LEDs indicating the current state of
    recording.

    "SDVR_LED_TYPE_ALARM" - A group of LEDs indicating the current state
    of the alarms.

***************************************************************************/

typedef enum _sdvr_led_type_e {
    SDVR_LED_TYPE_RECORD,
    SDVR_LED_TYPE_ALARM
} sdvr_led_type_e;


/****************************************************************************
  VISIBLE: This data structure is used to hold A/V frame queue size and the
  total buffer size allocated by the driver to receive A/V frames.

  To exchange encoded and raw video with the board, the SDK needs to
  allocate buffers to hold the data on its way to and from the DVR
  Application and the board. The number of buffers to be allocated on
  each of these paths and the sizes of these buffers are set using this
  data structure per channel.

  @NOTE: The size of all the A/V frames are fixed and determined by
  the DVR firmware.@

  If you don't allocate large enough frame queues or allocate too large of a
  queue, you may encounter A/V frame drops or cause the DVR frame to hang.

  We recommend that you use the suggested default values for each frame buffer
  size as if they are optimized for streaming at 30 fps for NTSC, and 25fps for
  PAL video standards.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_chan_buf_def_t.

    @max_buf_count@ -- Total number of buffers to be allocated by the driver
    to receive A/V frames from the hardware. This is generally the sum of
    all the buffer counts plus 1 per buffer type. (NOTE: You must
    take into account the secondary video encoder if one is going to exist.)
    The extra size is needed to allow each A/V frame queue recycle frames
    that are not being picked up quickly by the Application.
    This ensures the continuous streaming of A/V frame regardless how fast
    the Application processes these buffers.
    If you are confident that your application is recycling the A/V
    buffers fast enough, you can use a lot less buffer than the above
    suggested formula.
    A value of zero in this field indicates the SDK should calculate the best
    possible maximum buffer count.
    The maximum value is 60.

    @cmd_response_buf_count@ - The number of buffer to allocate to receive
    command response sent as a result of raw generic command sent by a
    sub-system within the DVR board. Default value is 0.

    NOTE: The application must consume
    these buffers and release them as soon as possible. Otherwise it is
    possible for the system to hang.

    @reserved1@ - This field must to be initialized to zero.

    -------- Encoder channel specific A/V frame queue allocation ---------

    @WARNING: The sum of all the buffer sizes should not exceed 50. Doing so, may
    cause the hardware not to respond to any command and stop streaming
    A/V frames.@

    @max_buf_size@ - The maximum size of an A/V sent by the DVR firmware.
    This field is currently ignored.

    @video_buf_count@ - The encoded video frames queue size for each
    video encoder associated with a encoder channel. Set this field to zero
    if this channel does not support encoding of video frames.
    The suggested value is 8 for non-VRM boards.

    @audio_buf_count@ - The encoded audio frames queue size.
    Set this field to zero if this channel does not
    support encoding of audio frames.
    The suggested value is 8 for non-VRM boards

    @raw_vbuf_count@ - The raw video frames queue size for each
    raw video stream associated with this channel.
    Set this field to zero if this channel is not streaming any
    live raw video.
    Typically, you will need to display raw frames 30 times per second.
    Therefore, you only need between 2 and 4 buffers to hold raw video.
    The suggested value is 2 for non-VRM boards. For VRM boards it
    is suggested to set this field to zero.

    @raw_abuf_count@ - The raw audio frames queue size.
    Set this field to zero if this channel is not expected to play raw audio.
    The suggested value is 2 for non-VRM boards. For VRM boards it
    is suggested to set this field to zero.

    @motion_buf_count@ - The number of buffers to be allocated to receive
    motion value buffers. Set this field to zero if you don't intend to
    process motion value buffers that are generated as result of motion
    alarms. In this case, the SDK drops all such frames.
    The suggested value is 5 for non-VRM boards. For VRM boards it
    is suggested to set this field to zero.

    "data_port_buf_count" - The number of dataport input buffers to allocate
    for this channel. A larger number of buffers will help maintain
    performance (i.e. reduce frame drops) under instantaneous high
    loading, but will show worse latency for live view (SMO).
    A smaller number of buffers will deliver lower latency but may
    cause frame drops under heavy loading. The valid range is from
    3 to 8. The recommended values are:
    - 3 for best latency
    - 7 for best performance

    The default value or any value out side valid range is set to 7.

    @NOTE:@ This field is ignored for boards that perform video
    stream demuxing in firmware. The number of dataport buffers is
    determined by the firmware.

    @reserved1@ - This field must be initialized to zero.

    ------- Decoder channel specific A/V frame queue allocation ---------

    @raw_vbuf_count@ - The raw video frames queue size for each
    raw video stream generated as a result of the hardware decoding.
    Set this field to zero if this channel is not going to
    receive the decoded raw video frames.
    Typically, you will need to display raw frames 30 times per second.
    Therefore, you only need between 2 and 4 buffers to hold raw video.
    The suggested value is 2.

    @raw_abuf_count@ - Set this field to zero since hardware audio decoding
    is currently not supported.

    @buf_count@ - The number of buffers to be allocated to hold each encoded
    frame that is going to be sent to the decoder hardware.
    The maximum number of decoder buffers allowed is 5.
    The suggested value is 5.

    @buf_size@ - The size of each buffer used to hold encoded frames on
    the way to the decoder hardware. Typically, this is the size of the
    largest encoded frame that needs to be decoded. See sdvr_sdk_params_t
    regarding suggestions for various buffer sizes.

    @reserved1@ - This field is required to be initialized to zero.

    ------- Host encoder channel specific A/V frame queue allocation ---------

    @raw_vbuf_count@ - The raw video frames queue size for each
    raw video stream required for hardware encoding of host video.
    Typically, you will need to receive raw frames 30 times per second.
    Therefore, you only need between 2 and 4 buffers to hold raw video.
    The suggested value is 2.

    @raw_abuf_count@ - Set this field to zero since hardware host audio
    encoding is currently not supported.

    @buf_count@ - The number of buffers to be allocated to hold each encoded
    frame that is going to be sent from the encoder hardware.
    The maximum number of decoder buffers allowed is 5.
    The suggested value is 5.

    @buf_size@ - The size of each buffer used to hold encoded frames on
    the way to the encoder hardware. Typically, this is the size of the
    largest encoded frame that needs to be decoded. See sdvr_sdk_params_t
    regarding suggestions for various buffer sizes.

    @reserved1@ - This field is required to be initialized to zero.

    Remarks:

    Call sdvr_get_board_attributes() to determine what is the maximum
    supported receive buffer count for each board. This will help you to
    choose what how to assigned receive buffers (i,e, video_buf_count).
****************************************************************************/
typedef struct _sdvr_chan_buf_def_t {
    sx_uint8  max_buf_count;
    sx_uint8  cmd_response_buf_count;
    sx_uint8  reserved1[18];

    union {
        struct {
            sx_uint32 max_buf_size;
            sx_uint8  video_buf_count;
            sx_uint8  audio_buf_count;
            sx_uint8  raw_vbuf_count;
            sx_uint8  raw_abuf_count;
            sx_uint8  motion_buf_count;
            sx_uint8  data_port_buf_count;
            sx_uint8  reserved1[14];
        } encoder;
        struct {
            sx_uint32 buf_size;
            sx_uint8  raw_vbuf_count;
            sx_uint8  raw_abuf_count;
            sx_uint8  buf_count;
            sx_uint8  reserved1[17];
        } decoder;
        struct {
            sx_uint32 buf_size;
            sx_uint8  raw_vbuf_count;
            sx_uint8  raw_abuf_count;
            sx_uint8  buf_count;
            sx_uint8  reserved1[17];
        } host_encoder;
    }u1;
} sdvr_chan_buf_def_t;


/****************************************************************************
  VISIBLE: This enumerated type defines the different video standards for
  each decoder.

    SDVR_DECODE_VSTD_NONE - Undefined video standard

    SDVR_DECODE_VSTD_SD - The SD video standard as it is set at the time
    of board connect.

    SDVR_DECODE_VSTD_HD_1080 - The 1080-HD video size

    SDVR_DECODE_VSTD_HD_720 -  The 720-HD video size

  Remarks:

    Using HD video standard on a SD only board can affect the performance.

****************************************************************************/
typedef enum _sdvr_decode_vstd_e {
    SDVR_DECODE_VSTD_NONE = 0,
    SDVR_DECODE_VSTD_SD,
    SDVR_DECODE_VSTD_HD_1080,
    SDVR_DECODE_VSTD_HD_720
} sdvr_decode_vstd_e;

/****************************************************************************
  VISIBLE: This data structure defines parameters that are needed to create
  a new encoder, decoder, or HMO-only channel.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_chan_def_t.

    "board_index" - The zero based index of the board where this channel resides.

    "chan_num" - The channel number identifier.
    In the case of encoding, the range is 0 to M-1, where M is the number of cameras
    supported by the board that are designated encoding channels.
    In the case of decoding, the range is 0 to N-1, where N is the number of decoders
    supported by the board that are designated decoding channels.
    NOTE: The channel number specifies on which Stretch chip the channel is going to be
    created. This assignment of channel number to physical port address varies from
    board to board.

    "chan_type" -  The type channel to create as specified in sdvr_chan_type_e,
    i.e., encoder or decoder.
    NOTE: To create an HMO- or SMO-only channel, set the chan_type to SDVR_CHAN_TYPE_ENCODER
    and the video_format_primary to SDVR_VIDEO_ENC_NONE. To create a video-out only
    channel that sends raw video to be displayed on a SMO grid, set the chan_type to
    SDVR_CHAN_TYPE_DECODER and the video_format_primary to SDVR_VIDEO_ENC_RAW_YUV_420.

    "video_format_primary" - The primary encode or decode video format, e.g., H.264.

    "audio_format" - The encode or decode audio format, e.g., G.711. If no audio
    is associated with this channel, you can specify SDVR_AUDIO_ENC_NONE.
    This field is ignored in version 1.0.

    "video_format_secondary" - The secondary encode video format, e.g., H.264
    This field is ignored when creating a decoder channel, or if the primary
    video format is SDVR_VIDEO_ENC_NONE.

    "vpp_mode" - This field is deprecated and should not be used.

    "raw_video_stream_count" - The number of different raw video streams
    with possibly different resolutions that are associated with this channel.
    This field must be set to 1 or 2. Otherwise, it is interpreted as 1.
    This field is ignored for decoding channels.
    To disable raw video streaming on this channel and not allocate any
    buffers to hold raw video frames, you must set raw_vbuf_count field in
    the sdvr_chan_buf_def_t structure to zero when calling sdvr_create_chan_ex().

    "decode_vstd" - The decoder video standard. This field only needs to be
    set for channel type of SDVR_CHAN_TYPE_DECODER.

    @NOTE: Specifying a HD decode video standard on a SD only board could
    affect the performance and must be used with caution.@

    "set_video_encoders_count" - A flag to indicate whether you are going to
    specify the number of video encoder streams that are going to be assigned for
    this channel. If set to non-zero, enough memory will be allocated
    in advance on this channel in order to be able to specify a maximum
    video_encoders_count of video encoder streams. In this case video_format_primary
    and video_format_secondary fields will not determine the number of encoder buffers.
    If this field is set to zero, video_encoders_count field is ignored and you can
    only specify a maximum of two encoders per channel if allowed by the board
    and depending on the value of video_format_primary or video_format_primary.

    "video_encoders_count" - The number of video encoders that can be assigned
    to the encoder channel.
    This field is ignored if connected to a firmware prior to 7.2 or it is set to less than
    2 when either of the video_format_primary or video_format_secondary is
    set to a video encoder. This field is only valid while creating an
    encoder channel.  Valid range is 0 - maximum number of sub-encoders supported
    by an encoder channel on your board. (See sdvr_get_board_config() to
    get the maximum number of sub-encoders supported.)

    "host_encoder_size" - This structure specifies the width and height of a
    host encoding channel.  Only used if chan_type is set to SDVR_CHAN_TYPE_HOST_ENCODER.

    @reserved1@ - This field must be initialized to zero.

  Remarks:

    The decoder channel video format should be set to SDVR_VIDEO_ENC_H264
    when decoding either H.264 AVC or SVC.

    Specifying multiple encoders on a camera takes away processing
    power from other channels. As result you may not be able to create some
    encoder channels if multiple encoding is used in all the encoder
    channels.

    You may set either or both the video_format_primary and video_format_secondary
    fields to SDVR_VIDEO_ENC_NONE and set their corresponding video CODECs at run time if
    connected to a firmware version 7.2 or higher and video_encoder_count field
    is set properly. This is true for both creating an encoder or decoder channel.

    If set_video_encoders_count field is set to zero, you can only change the codec
    type of the primary or secondary video encoders to another valid video codec.
    You can not set them if their original value is SDVR_VIDEO_ENC_NONE. Nor you
    can change them to SDVR_VIDEO_ENC_NONE.
***************************************************************************/

typedef struct _sdvr_chan_def_t {
    sx_uint8            board_index;
    sx_uint8            chan_num;
    sdvr_chan_type_e    chan_type;
    sdvr_venc_e         video_format_primary;
    sdvr_aenc_e         audio_format;
    sdvr_venc_e         video_format_secondary;
    sdvr_vpp_mode_e     vpp_mode;
    sx_uint8            raw_video_stream_count;
    sdvr_decode_vstd_e  decode_vstd;
    sx_uint8            set_video_encoders_count;
    sx_uint8            video_encoders_count;
    struct {
        sx_uint16       width;
        sx_uint16       height;
    } host_encoder_size;
    sx_uint8            reserved1[17];
} sdvr_chan_def_t;

/************************************************************************
  VISIBLE: These defines specify the flags that can be used
  to change one or more of image control parameters associated with each
  video-in data port by ORing each defines when calling
  sdvr_set_video_in_params().

    @SDVR_ICFLAG_ALL@ - Use this define to change all the image
    control parameters.

    @SDVR_ICFLAG_HUE@ - - Change the hue image value.

    @SDVR_ICFLAG_SATURATION@ - Change the saturation image value.

    @SDVR_ICFLAG_BRIGHTNESS@ - Change the brightness image value.

    @SDVR_ICFLAG_CONTRAST@ - Change the contrast image value.

    @SDVR_ICFLAG_SHARPNESS@ - Change the sharpness image value.
************************************************************************/
#define SDVR_ICFLAG_ALL                  0xFFFF
#define SDVR_ICFLAG_HUE                  0x1
#define SDVR_ICFLAG_SATURATION           0x2
#define SDVR_ICFLAG_BRIGHTNESS           0x4
#define SDVR_ICFLAG_CONTRAST             0x8
#define SDVR_ICFLAG_SHARPNESS            0x10

/****************************************************************************
  VISIBLE: This data structure defines various parameters that
  can be changed for each video-in data port. These parameters affect both the
  raw and encoded video frames.

    @hue@ - Value of hue control in the range of 0 to 255.

    @saturation@ - Value of saturation control in the range of 0 to 255.

    @brightness@ - Value of brightness control in the range of 0 to 255.

    @contrast@ - Value of contrast control in the range of 0 to 255.

    @sharpness@ - Value of sharpness control in the range of 1 to 15.

  Remarks:

    Default values for these fields are hardware specific.
    These settings may affect the blind detection alarm.
***************************************************************************/
typedef struct _sdvr_image_ctrl_t {
    sx_uint8 hue;
    sx_uint8 saturation;
    sx_uint8 brightness;
    sx_uint8 contrast;
    sx_uint8 sharpness;
} sdvr_image_ctrl_t;

/************************************************************************
    VISIBLE: Enumerated constants for H.264 decoder operating modes.

    @SDVR_H264_DEC_MODE_STRETCH@ - Configures the H.264 decoder to operate
    in Stretch compatibility mode. This is usually more efficient in CPU
    usage, but may not correctly decode inputs from non-Stretch encoders.
    This is the default decoder mode.

    @SDVR_H264_DEC_MODE_STANDARD@ - Configure the H.264 decoder to operate
    in Standard mode. This usually requires more CPU resources, and will
    be able to better handle input from non-Stretch encoders.

************************************************************************/
typedef enum _sdvr_h264_decoder_mode_e {
    /* NOTE: values must always numerically match the decoder config enums. */
    SDVR_H264_DEC_MODE_STRETCH  = 0,
    SDVR_H264_DEC_MODE_STANDARD = 1,
} sdvr_h264_decoder_mode_e;

/************************************************************************
  VISIBLE: Enumerated type describing various methods to control the
  encoding frame rate. These methods affect the meaning of the frame_rate
  field in sdvr_video_enc_chan_params_t and sdvr_alarm_video_enc_params_t
  data structures. The actual value of the frame_rate field should be
  set by the return value from the call to sdvr_set_frame_rate().

    @SDVR_FRS_METHOD_NONE@ - An absolute frame rate.
    For example, 30 FPS

    @SDVR_FRS_METHOD_FRAMES@  - Skip 'n-1' frames between each video
    encoding. For example, if 'n' is set to 4, video frame number 4, 8,
    12, etc will be encoded

    @SDVR_FRS_METHOD_SECONDS@ - Encode a video frame every n seconds

    @SDVR_FTS_METHOD_MINUTES@ - Encode a video frame every n minutes.
************************************************************************/
typedef enum _sdvr_frame_rate_skip_method_e {
    SDVR_FRS_METHOD_NONE,
    SDVR_FRS_METHOD_FRAMES,
    SDVR_FRS_METHOD_SECONDS,
    SDVR_FTS_METHOD_MINUTES
} sdvr_frame_rate_skip_method_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various H.264 encoding mode settings.
  These settings control the target quality level of the encoder per
  encoder channel.
  @SDVR_H264_ENC_MODE_BASE_LINE@ is the default.

  For S7 firmware, the mode setting controls the encoding profile. The
  choices are:

    @SDVR_H264_ENC_PROFILE_BASE@ - Use H.264 baseline profile.

    @SDVR_H264_ENC_PROFILE_MAIN@ - Use H.264 main profile with default (CABAC).

    @SDVR_H264_ENC_PROFILE_HIGH@ - Use H.264 high profile with default (CABAC).

    @SDVR_H264_ENC_PROFILE_MAIN_CABAC@ - Use H.264 main profile with CABAC.

    @SDVR_H264_ENC_PROFILE_HIGH_CABAC@ - Use H.264 high profile with CABAC.

    @SDVR_H264_ENC_PROFILE_MAIN_CAVLC@ - Use H.264 main profile with CAVLC.

    @SDVR_H264_ENC_PROFILE_HIGH_CAVLC@ - Use H.264 high profile with CAVLC.

    Not all profiles may be supported on all boards. If this field is set
    to @SDVR_H264_ENC_MODE_BASE_LINE@, the default profile setting for the board is
    used.


  Remarks:

    SDVR_H264_ENC_MODE_4 is not supported by H.264-SVC encoder.

    SDVR_H264_ENC_MODE_6 - If this mode is used, it must be applied to all
    the cameras within a DVR board. Make sure no video encoder is enabled
    while setting this encoder mode.
****************************************************************************/
typedef enum _sdvr_h264_encoder_mode_enum {
    SDVR_H264_ENC_MODE_BASE_LINE = 0,
    SDVR_H264_ENC_MODE_1,
    SDVR_H264_ENC_MODE_2,
    SDVR_H264_ENC_MODE_3,
    SDVR_H264_ENC_MODE_4,
    SDVR_H264_ENC_MODE_5,
    SDVR_H264_ENC_MODE_6,
    SDVR_H264_ENC_PROFILE_BASE = 50,
    SDVR_H264_ENC_PROFILE_MAIN,
    SDVR_H264_ENC_PROFILE_HIGH,
    SDVR_H264_ENC_PROFILE_MAIN_CABAC,
    SDVR_H264_ENC_PROFILE_HIGH_CABAC,
    SDVR_H264_ENC_PROFILE_MAIN_CAVLC,
    SDVR_H264_ENC_PROFILE_HIGH_CAVLC,
} sdvr_h264_encoder_mode_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various video encoding modes to
  be applied to all the encoder channels within a DVR board.

  @SDVR_ENC_MODE_DEFAULT@ - The default video encoder behavior.

  @SDVR_ENC_MODE_FRAME_RATE@ - Instruct the H.264 video encoder to favor
  frame rate over the video quality if necessary.

  Remarks:

    Not all modes are supported by all DVR boards.

****************************************************************************/
typedef enum _sdvr_encoder_mode_enum {
    SDVR_ENC_MODE_DEFAULT    = 0,
    SDVR_ENC_MODE_FRAME_RATE = 0x1
} sdvr_encoder_mode_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various SVC gop styles

    SDVR_SVC_GOP_DEFAULT - same as SDVR_SVC_GOP_2X_4X.

    SDVR_SVC_GOP_2X_4X - I P2 P1 P2 P0 P2 P1 P2 P0 ...
    where P0 references previous P0 or I frame (and not intermediate P1 and P2)
    P1 reference previous P1 or P0 or I frame (and not intermediate P2)

    SDVR_SVC_GOP_IREF - only ref to the closest I frame.

    Remarks:

        When using SDVR_SVC_GOP_2X_4X, the gop size has to be multiple of 4

****************************************************************************/
typedef enum _sdvr_svc_gop_style_e {
    SDVR_SVC_GOP_DEFAULT = 0,
    SDVR_SVC_GOP_2X_4X = 1,
    SDVR_SVC_GOP_IREF = 2
} sdvr_svc_gop_style_e;


/****************************************************************************
  VISIBLE: This data structure defines video encoder parameters.
  For each channel used for encoding, use this data structure to set its
  parameters.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_video_enc_chan_params_t.

    @frame_rate@ - The frequency of recording. The value of this field should
    be assigned from the return value of the function sdvr_set_frame_rate().
    The default is 30 fps for NTSC and 25 fps for PAL.

    @res_decimation@ - Resolution decimation of the channel. You can specify
    the resolution of the encoded channel to be the same as, 1/4, or
    1/16 of the system-wide maximum resolution. As well as defining an arbitrary
    output size based on a source video frame of equal, 1/4, or 1/16 of the
    maximum resolution for the camera.
    The default for the first video encoding of the camera is
    SDVR_VIDEO_RES_DECIMATION_EQUAL whereas
    for all other streams is SDVR_VIDEO_RES_DECIMATION_QCIF.

    @flush_buf@ - A flag to indicate whether the encoder must flush its buffer
    immediately. Setting this flag to true causes the encoder buffer to be
    flushed immediately. Otherwise, there is a 1 frame latency before the encoder
    buffer is flushed. It is not recommended to set this flag to true for
    frame rates higher than 10. @NOTE: Enabling the encoder flush buffer could affect
    the system performance.@

  The parameters for each encoder are set in the following
  "union".

  H.264 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR, VBR, or CBR_S is selected.

     @max_bitrate@ - The maximum target bit rate in Kbits per second
     if VBR or constant quality is selected.

     @bitrate_control@ - The encoder bit rate control. Refer to
     sdvr_br_control_e for description of each bit rate control.

     @gop@ - Number of pictures in a GOP. It is recommended
     that this value be set to have one GOP per second of video.

     @quality@ - The quality level to be maintained when bitrate_control
     is set to constant quality. The valid range is 1 - 100.

     @enc_mode@ - The h.264 encoder mode sets the target encoding quality level.
     Different modes have different encoder effort associated with them.
     See sdvr_h264_encoder_mode_e for various options.

     @i_frame_bitrate@ - If none-zero, the bit rate for I-Frames will be iTh percent
     of the @max_bitrate@. Valid range is 0 - 99; Recommended values for
     @i_frame_bitrate@ are 15 - 45. For a value of  0 (default) the size of the I frames
     is chosen dynamically and is controlled by @bitrate-control@ algorithm.


  H.264 SVC Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR, VBR, or CBR_S is selected.

     @max_bitrate@ - The maximum target bit rate in Kbits per second
     if VBR or constant quality is selected.

     @bitrate_control@ - The encoder bit rate control. Refer to
     sdvr_br_control_e for description of each bit rate control.

     @gop@ -  Number of pictures in a GOP. It is recommended
     that this value be set to have one GOP per second of video.
     GOP size must be in the range of 1 to 128 and be multiple of 4.

     @gop_style@ -  The encoder gop style. Refer to
     sdvr_svc_gop_style_e for description of each style.

     @quality@ - The quality level to be maintained when bitrate_control
     is set to constant quality. The valid range is 1 - 100.

     @i_frame_bitrate@ - If none-zero, the bit rate for I-Frames will be iTh percent
     of the @max_bitrate@. Valid range is 0 - 99; Recommended values for
     @i_frame_bitrate@ are 15 - 45. For a value of  0 (default) the size of the I frames
     is chosen dynamically and is controlled by @bitrate-control@ algorithm.

     @enc_mode@ - The h.264 encoder mode sets the target encoding quality level.
     Different modes have different encoder effort associated with them.
     See sdvr_h264_encoder_mode_e for various options.

  JPEG Parameters:

     @quality@ - A number in the range of 1 - 100 to control the quality of the
     compression. A higher number implies better quality.

     @is_image_style@ - The JPEG encoder generates a
     video-style JPEG (Motion JPEG) frame header when this value is set to zero. This is
     suitable in AVI/MOV MJPEG or still JPEG image files. This is the style that
     most DVR applications should use.
     The JPEG encoder generates an image-style JPEG frame
     header to be used for RTP when this value is set to one (1).
     Set this field to two (2) if you need JPEG encoder to not add any JPEG frame header.
     Image style 2 maybe needed for some RTSP/RTP streaming. Image style 2 is not
     available prior to version 7 of the firmware. This is the recommended style for
     IP Camera applications that are going to stream over RTSP/RTP.

     @frame_bundle_count@ - For performance reason, you can set this field to
     2 in order for the MJPEG encoder encode two frames at a time.
     But for low frame rate which could cause latency in timestamps,
     you may turn off this feature by set this field to 1.
     Set to zero (0) to use the default settings for the current board.

  MPEG4 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR, VBR, or CBR_S is selected.

     @max_bitrate@ - The maximum target bit rate in Kbits per second
     if VBR or constant quality is selected.

     @bitrate_control@ - The encoder bit rate control. Refer to
     sdvr_br_control_e for description of each bit rate control.
     Note: the SDVR_BITRATE_CONTROL_CONSTANT_QUALITY is not currently
     available for MPEG4.

     @gop@ - Number of pictures in a GOP. It is recommended
     that this value be set to have one GOP per second of video.

     @quality@ - The quality level is not currently used.

  MPEG2 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR, VBR, or CBR_S is selected.

     @max_bitrate@ - The maximum target bit rate in Kbits per second
     if VBR or constant quality is selected.

     @bitrate_control@ - The encoder bit rate control. Refer to
     sdvr_br_control_e for description of each bit rate control.

     @gop@ - Number of pictures in a GOP. It is recommended
     that this value be set to have one GOP per second of video.
     The valid range is from 1 to 30.

  Custom Crop Parameters: This parameters are only required if
  'res_decimation' is set to SDVR_VIDEO_RES_CUSTOM.

     @src@ - The source picture to use. Must be one of the valid @sdvr_crop_src_e@
     values.

     @cropped_width@ - Cropped width. Valid width must be a multiple of 16
     and at least 64. If this is set to zero, cropping will be skipped.

     @cropped_height@ - Cropped height. Valid height must be an even number
     and at least 64. If this is set to zero, cropping will be skipped.

     @crop_x_offset@ - X offset of the top left hand corner of the crop
     region within the input picture.

     @crop_y_offset@ - Y offset of the top left hand corner of the crop
     region within the source picture.

     @scaled_width@ - Scaled picture width. Valid width must be a multiple
     of 16 and at least 64. If this is zero then scaling is skipped.

     @scaled_height@ - Scaled picture height. Valid height must be an even
     number and at least 64. If this is zero then scaling is skipped.

  Remarks:

    Default values for each CODEC parameter are hardware specific.

****************************************************************************/
typedef struct _sdvr_video_enc_chan_params_t {
    sx_uint8                    frame_rate;
    sx_uint8                    res_decimation;
    sx_bool                     flush_buf;
    sx_uint8                    reserved1;

    // Video encoder specific channel parameters
    union {
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            enc_mode;
            sx_uint8            i_frame_bitrate;
            sx_uint8            reserved1[3];
            sx_uint32           reserved2[9];
        } h264;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            enc_mode;
            sx_uint8            i_frame_bitrate;
            sx_uint8            gop_style;
            sx_uint8            reserved1[2];
            sx_uint32           reserved2[9];
        } h264_svc;
        struct {
            sx_uint16           quality;
            sx_uint8            is_image_style;
            sx_uint8            frame_bundle_count;
            sx_uint32           reserved2[11];
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
            sx_uint32           reserved2[10];
        } mpeg4;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint16           reserved1;
            sx_uint32           reserved2[10];
        } mpeg2;

    } encoder;
    struct {
        sdvr_crop_src_e     src;
        sx_uint16           cropped_width;
        sx_uint16           cropped_height;
        sx_uint16           crop_x_offset;
        sx_uint16           crop_y_offset;
        sx_uint16           scaled_width;
        sx_uint16           scaled_height;
    } custom_res;

} sdvr_video_enc_chan_params_t;

/****************************************************************************
  VISIBLE: This data structure defines video encode channel parameters for
  alarm video streaming. After any of the alarms reaches its specified
  threshold, the video encoded frame is streamed using these new
  parameters for the given minimum duration.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_alarm_video_enc_params_t.

  NOTE: Currently these parameters are used for all triggered alarms.

    @frame_rate@ - The frequency of recording on alarm. The value of this field should
    be assigned from the return value of the function sdvr_set_frame_rate().
    The default is 30 fps for NTSC and 25 fps for PAL.

    @min_on_seconds@ - Minimum number of seconds to stream using the
    new encoder parameter after the alarm is triggered.

    @min_off_seconds@ - Minimum number of quiet periods between each
    alarm streaming condition.

    @enable@ - A flag to enable or disable on-alarm video streaming.
    0 turns off the on-alarm streaming. 1 turns on the on-alarm
    streaming.

    @encoder@ - The encoder-specific parameters. See the encoder union in
    sdvr_video_enc_chan_params_t for a detailed description.
****************************************************************************/
typedef struct _sdvr_alarm_video_enc_params_t
{
    sx_uint8    frame_rate;
    sx_uint8    min_on_seconds;
    sx_uint8    min_off_seconds;
    sx_uint8    enable;

    // Video encoder specific channel parameters
    union {
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            i_frame_bitrate;
            sx_uint32           reserved1[10];
        } h264;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            i_frame_bitrate;
            sx_uint8            gop_style;
            sx_uint8            reserved[3];
            sx_uint32           reserved1[9];
        } h264_svc;
        struct {
            sx_uint16           quality;
            sx_uint8            is_image_style;
            sx_uint8            frame_bundle_count;
            sx_uint32           reserved2[11];
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
            sx_uint32           reserved2[10];
        } mpeg4;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint16           reserved1;
            sx_uint32           reserved2[10];
        } mpeg2;

    } encoder;

} sdvr_alarm_video_enc_params_t;

/****************************************************************************
  VISIBLE: Structure defining audio encoder channel parameters.

    @audio_enc_type@ - The type of audio encoder to use.
****************************************************************************/
typedef  struct _sdvr_audio_enc_chan_params_t {
    sdvr_aenc_e   audio_enc_type;
} sdvr_audio_enc_chan_params_t;

/****************************************************************************
  VISIBLE: Data structure for a Macro Block (MB) rectangular region of interest.
  A MB region is specified by upper left row and column macro block
  and lower right macro block within a map of ROI buffers. See
  sdvr_set_regions_map()for detailed description.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_mb_region_t.

    @alarm_flag@    - A flag indicating whether the motion value in this
    Macro Block rectangle exceeds the specified threshold. This
    field is set by sdvr_motion_value_analyzer() upon successful
    return.

    @upper_left_row@ - MB row number of the upper left corner.

    @upper_left_col@ - MB column number of the upper left corner.

    @lower_right_row@ - MB row number of the lower right corner.

    @lower_right_col@ - MB column number of the lower right corner.
****************************************************************************/
typedef struct _sdvr_mb_region_t {
    sx_bool   alarm_flag;
    sx_uint8  reserved1;
    sx_uint16 reserved2;
    sx_uint16 upper_left_x;
    sx_uint16 upper_left_y;
    sx_uint16 lower_right_x;
    sx_uint16 lower_right_y;
} sdvr_mb_region_t;

/****************************************************************************
  VISIBLE: Data structure for a region.
  A region is specified by its upper left and lower right coordinates
  in pixels. The upper left corner of an image is the origin (0,0).

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_region_t.

    @region_id@    - The region identifier. It is needed to change or
    remove a region. When adding a new region, this field will be set
    by the system.

    @upper_left_x@ - X-coordinate of the upper left corner.

    @upper_left_y@ - Y-coordinate of the upper left corner.

    @lower_right_x@ - X-coordinate of the lower right corner.

    @lower_right_y@ - Y-coordinate of the lower right corner.
****************************************************************************/
typedef struct _sdvr_region_t {
    sx_uint8  region_id;
    sx_uint16 upper_left_x;
    sx_uint16 upper_left_y;
    sx_uint16 lower_right_x;
    sx_uint16 lower_right_y;
} sdvr_region_t;

/****************************************************************************
  VISIBLE: Data structure for motion detection alarm.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_motion_detection_t.

    @threshold@ - The threshold value for motion detection.
    Motion above the threshold is reported.
    A threshold of zero means the motion detection alarm is
    triggered constantly. The valid range is 0 - 99.
    The default is 20.

    @enable@ - A value of 1 enables motion detection for all the specified
    regions. If no region is defined, the entire picture will be used for
    motion detection. A value of 0 disables the motion detection.

    @num_regions@ - This field specifies the number of motion detection
    regions added to the current video channel. A value of zero means
    motion detection is applied to the entire picture. This is a read-only
    field.

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to identify the region.
    This is a read-only field.
****************************************************************************/
typedef struct _sdvr_motion_detection {
    sx_uint8        threshold;
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_MD_REGIONS];
} sdvr_motion_detection_t;

/****************************************************************************
  VISIBLE: Data structure for blind detection alarm.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_blind_detection_t.

    @threshold@ - The threshold value for blind detection. Blinding
    above the threshold is reported.  The valid range is 0 - 99.
    The default is 60.

    @enable@ - A value of 1 enables blind detection for all the specified
    regions. If no region is defined, the entire picture will be used for
    blind detection. A value of 0 disables the blind detection.

    @num_regions@ - This field specifies the number of blind detection
    regions added to the current video channel. A value of 0 means
    blind detection is applied to the entire picture. This is a read-only
    field.

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to identify the region.
    This is a read-only field.
****************************************************************************/
typedef struct _sdvr_blind_detection {
    sx_uint8        threshold;
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_BD_REGIONS];
} sdvr_blind_detection_t;

/****************************************************************************
  VISIBLE: Data structure for night detection alarm.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_night_detection_t.

    @threshold@ - The threshold value for night detection. Values
    below the threshold are reported. Setting this value to 255
    disables night detection. The valid range is 0 - 255.
    The default is 40.

    @enable@ - A value of 1 enables night detection. A value of 0 disables
    the night detection.

****************************************************************************/
typedef struct _sdvr_night_detection {
    sx_uint8        threshold;
    sx_bool         enable;
} sdvr_night_detection_t;

/****************************************************************************
  VISIBLE: Data structure for privacy regions.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_privacy_region_t.

    @enable@ - A value of 1 enables block out for all the specified
    regions. If no region is defined, the entire picture will be blocked
    out. A value of zero turns off privacy.

    @num_regions@ - This field specifies the number of blocked out
    regions added to the current video channel. A value of 0 means
    the entire picture will be blocked out. This is a read-only field.

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to identify the region.
    This is a read-only field.


****************************************************************************/
typedef struct _sdvr_privacy_region {
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_PR_REGIONS];
} sdvr_privacy_region_t;

/****************************************************************************
  VISIBLE: The AV buffer structure used in the SDK for receiving various
  buffer types.

  @NOTE: The fields in this data structure should not be accessed directly.
  Instead, call functions in the "Frame Buffer Field Access API" group.
  It is possible in the future that the fields in this structure be
  changed or removed.@
****************************************************************************/
typedef struct _sdvr_av_buffer_t {
    sx_uint32       signature;          /* Internal use field. */
    sx_uint16       hdr_version;        /* Internal use field. */
    sx_uint16       hdr_size;           /* Internal use field. */
    sx_uint8        board_id;           /* Internal use field. */
    sdvr_chan_type_e  channel_type;     /* Internal use field. */
    sx_uint8        channel_id;         /* Internal use field. */
    sdvr_frame_type_e frame_type;       /* Internal use field. */
    sx_uint8        motion_detected;    /* Internal use field. */
    sx_uint8        blind_detected;     /* Internal use field. */
    sx_uint8        night_detected;     /* Internal use field. */
    sx_uint8        av_state_flags;     /* Internal use field. */
    sx_uint8        stream_id;          /* Internal use field. */
    sx_uint8        stream_count;       /* Internal use field. */
    sx_uint16       reserved3;          /* Internal use field. */
    sx_uint32       payload_size;       /* Internal use field. */
    sx_uint32       timestamp;          /* Internal use field. */
    sx_uint32       timestamp_high;     /* Internal use field. */
    sx_uint8        motion_value[4];    /* Internal use field. */
    sx_uint8        blind_value[4];     /* Internal use field. */
    sx_uint8        night_value[4];     /* Internal use field. */
    sx_uint16       active_width;       /* Internal use field. */
    sx_uint16       padded_width;       /* Internal use field. */
    sx_uint16       active_height;      /* Internal use field. */
    sx_uint16       padded_height;      /* Internal use field. */
    sx_uint32       seq_number;         /* Internal use field. */
    sx_uint32       frame_number;       /* Internal use field. */
    sx_uint32       frame_drop_count;   /* Internal use field. */
    sx_uint8        reserved4[2];       /* Internal use field. */
    sx_uint8        enc_audio_mode;     /* Internal use field. */
    sx_uint8        yuv_format;         /* Internal use field. */
    sx_uint32       y_data_size;        /* Internal use field. */
    sx_uint32       u_data_size;        /* Internal use field. */
    sx_uint32       v_data_size;        /* Internal use field. */
    sx_uint32       y_data_offset;      /* Internal use field. */
    sx_uint32       u_data_offset;      /* Internal use field. */
    sx_uint32       v_data_offset;      /* Internal use field. */
    sx_uint16       uv_active_width;    /* Internal use field. */
    sx_uint16       uv_padded_width;    /* Internal use field. */
    sx_uint16       uv_active_height;   /* Internal use field. */
    sx_uint16       uv_padded_height;   /* Internal use field. */
    sx_uint8        overlay_id;         /* Internal use field. */
    sx_uint8        reserved5[3];       /* Internal use field. */
    sx_uint16       layer_width;        /* Internal use field. */
    sx_uint16       layer_height;       /* Internal use field. */
    sx_int16        layer_top_left_x;   /* Internal use field. */
    sx_int16        layer_top_left_y;   /* Internal use field. */
    sx_uint32       num_packets;        /* Internal use field. */
    sx_uint32       packet_size[7];     /* Internal use field. */
    sx_uint32       hmo_local_data;     /* Internal use field. */
    sx_uint8        is_test_frame;      /* Internal use field. */
    sx_uint8        test_pattern;       /* Internal use field. */
    sx_uint16       reserved1;          /* Internal use field. */
    sx_uint32       reserved[18];       /* Internal use field. */
    sx_uint32       internal_dbg[8];    /* Internal use field. */
    sx_uint8        payload[1];         /* Internal use field. */
} sdvr_av_buffer_t;

/************************************************************************
    VISIBLE: This data structure defines the display characteristics
    of each raw video frame that is sent from the Application to a
    video overlay object on a SMO display with SDVR_SMO_CAP_OUTPUT capability.

    Before setting any of the fields in this data structure, you must memset it to
    zero with the size of the sdvr_overlay_frame_attrib_t.

        @overlay_id@ - The overlay_id that was returned from the call to
        sdvr_smo_add_overlay()

        @video_format@ - The overlay raw video formats.
        see @sdvr_rawv_formats_e@. If this field is set to zero, it is
        assumed that this overlay is using the video format that
        was specified at the call to sdvr_start_video_overlay().

        @reserved1@ - The field must be set to zero.

        @width@ - The width of raw video frame that is being sent
        to the SMO display. This number should not exceed the width of
        the SMO port that is retrieved from sdvr_smo_attribute_t
        minus the top_left_x coordinates field that the video should be
        displayed. A value of zero in this field causes the last video frame
        corresponding to the given layer number to be cleared from the
        SMO display.

        @height@ - The number of line of raw video frame that is being displayed
        on the SMO display. This number should not exceed the height of
        the SMO port that is retrieved by sdvr_smo_attribute_t
        minus the top_left_y coordinates field that the video should be
        displayed. A value of zero in this field causes the last video frame
        corresponding to the given layer number to be cleared from the
        SMO display.

        @top_left_x@, @top_left_y@ - The top left coordinates position
        where the raw video frame should be placed within the SMO video overlay.
        The upper left corner of the overlay is the origin (0,0). NOTE:
        This coordinate is relative to the top left corner of the video overlay
        object and not the SMO display.

        @reserved2@ - The field must be set to zero.
************************************************************************/
typedef struct _sdvr_overlay_frame_attrib_t {
    sx_uint8    overlay_id;
    sx_uint8    video_format;
    sx_uint8    reserved1[2];
    sx_uint16   width;
    sx_uint16   height;
    sx_int16    top_left_x;
    sx_int16    top_left_y;
    sx_uint8    reserved2[8];
} sdvr_overlay_frame_attrib_t;

/************************************************************************
    VISIBLE: This data structure defines the characteristics of a
    video overlay object on a SMO display with SDVR_SMO_CAP_OUTPUT capability.

        @overlay_id@ - The overlay_id that was returned from the call to
        sdvr_smo_add_overlay() Or sdvr_smo_add_direct_mem_overlay().

        @video_format@ - The overlay raw video formats.
        see @sdvr_rawv_formats_e@. If this field is set to zero, it is
        assumed that this overlay is using the video format that
        was specified at the call to sdvr_start_video_overlay().

        @layer_order@ - The relative ordering layer (Z-order) in comparison to other
        overlays. An overlay with the lower order number is
        placed beneath the one with a higher layer order number.
        The layer order of zero is a special layer. It is placed below all
        the video frames on  the specified SMO display and then can only be one
        such layer defined. This layer can be used to specify a
        wall paper for the SMO display.

        @reserved1@ - Must be set to zero.

        @width@ - The width of the overlay within the SMO display.
        This number should not exceed the width of
        the SMO port that is retrieved from sdvr_smo_attribute_t.
        This field is ignored if layer_order is set to zero. The width is
        assumed to be the full width of the SMO display.

        @height@ - The number of line of the overlay within the SMO display.
        This number should not exceed the height of
        the SMO port that is retrieved by sdvr_smo_attribute_t.
        This field is ignored if layer_order
        is set to zero. The height is assumed to be the full height of the
        SMO display.

        @reserved2@ - Must be set to zero.
************************************************************************/
typedef struct _sdvr_overlay_t {
    sx_uint8    overlay_id;
    sx_uint8    video_format;
    sx_uint8    layer_order;
    sx_uint8    reserved1;
    sx_uint16   width;
    sx_uint16   height;
    sx_int32    reserved2[3];
} sdvr_overlay_t;
/****************************************************************************
    VISIBLE: The following constants describe various raw video formats.

    SDVR_RAWV_FORMAT_YUV_4_2_0 - 4:2:0 YUV format.

    SDVR_RAWV_FORMAT_YUV_4_2_2 - 4:2:2 YUV format. Not supported by 7.x releases.

    SDVR_RAWV_FORMAT_YVU_4_2_0 - 4:2:0 YVU (YV12) format.

    SDVR_RAWV_FORMAT_YUYV_4_2_2i - 4:2:2 YUYV pixel interleaved format. Not supported by 7.x releases.

    SDVR_RAWV_FORMAT_RGB_8 -- One byte raw RGB format - Not supported.

    SDVR_RAWV_FORMAT_RGB_655 -- 16-bit raw RGB format memory pixel
    representation is: GGRR RRRR BBBB BGGG.
    Strong Red is represented by (0x3F 0x00), strong Green is (0xC0 0x07),
    and strong Blue is (0x00 0xF8).

    SDVR_RAWV_FORMAT_RGB_565_LE -- 16 bit unsigned short little endian raw
    RGB 565.

    SDVR_RAWV_FORMAT_RGB_565_BE - 16 bit unsigned short big endian raw
    RGB 565.

    SDVR_RAWV_FORMAT_RGB_565Q_LE - 16 bit unsigned short little endian raw
    RGB 565. Red is the low order 5 bits. Blue is the high order 5 bits.

    SDVR_RAWV_FORMAT_RGB_565Q_BE - 16 bit unsigned short big endian raw
    RGB 565. Red is the low order 5 bits. Blue is the high order 5 bits.

    SDVR_RAWV_FORMAT_RGB_565X_LE - same as SDVR_RAWV_FORMAT_RGB_565Q_LE.

    SDVR_RAWV_FORMAT_RGB_565X_BE - same as SDVR_RAWV_FORMAT_RGB_565Q_BE.

    SDVR_RAWV_FORMAT_BMP - BMP file format. Supported BMP file formats are:

       24 bit true color

       8 bit grey scale or color palette

       4 bit grey scale or color palette

       1 bit black and white

    Remarks:

       RGB raw video is not supported by any firmware before version 7. Additionally,
       not all the video formats are supported by all the DVR boards.

       RGB_565Q and RGB_565X are not reported in the capabilities info from the board,
       and are assumed to be supported if RGB_565 is supported.

****************************************************************************/
typedef enum _sdvr_rawv_formats_e {
    SDVR_RAWV_FORMAT_YUV_4_2_0 = 1,
    SDVR_RAWV_FORMAT_YUV_4_2_2 = 2,
    SDVR_RAWV_FORMAT_YVU_4_2_0 = 4,
    SDVR_RAWV_FORMAT_YUYV_4_2_2i = 8,
    SDVR_RAWV_FORMAT_RGB_8 = 16,
    SDVR_RAWV_FORMAT_RGB_655 = 32,
    SDVR_RAWV_FORMAT_RGB_565 = 64,
    SDVR_RAWV_FORMAT_RGB_565_LE = SDVR_RAWV_FORMAT_RGB_565,
    SDVR_RAWV_FORMAT_RGB_565_BE = 128,
    SDVR_RAWV_FORMAT_RGB_565Q = 0xc1,
    SDVR_RAWV_FORMAT_RGB_565Q_LE = 0xc1,
    SDVR_RAWV_FORMAT_RGB_565Q_BE = 0xc2,
    SDVR_RAWV_FORMAT_RGB_565X = 0xc3,
    SDVR_RAWV_FORMAT_RGB_565X_LE = 0xc3,
    SDVR_RAWV_FORMAT_RGB_565X_BE = 0xc4,

    SDVR_RAWV_FORMAT_BMP = 3
}sdvr_rawv_formats_e;

/************************************************************************
    VISIBLE: This data structure is used to configure each one of the OSD items
    associated with video frames of any camera or player. Each OSD
    item can be up to a 100 double-byte string.
    See "Important Restrictions" for number of OSD texts allowed per channel.


    After an OSD item is configured, it can be shown or hidden at any time.

    Before setting any of the fields in this data structure, you must memset it to
    zero with the size of the sdvr_osd_config_ex_t.

        @translucent@ - This field specifies the intensity of
        translucence when overlay OSD text is on the active video.
        0 means least translucent, 255 means most translucent.

        @location_ctrl@ - The position of OSD text.
        It can be any of the predefined locations in
        @sdvr_location_e@ or a custom defined location.

        @top_left_x@, @top_left_y@ - The top left coordinates of the OSD
        text when the custom @location_ctrl@ is specified, otherwise
        these fields are ignored. These coordinates are based on full
        size video standard frame and will be decimated accordingly when a different
        video decimation is selected at the time of video streaming.
        The upper left corner of the video frame is the origin (0,0).

        @dts_format@ - The format of date and time to optionally
        be appended to the end of the OSD text.

        @text_len@ - The number of unsigned double byte characters
        in the text field.

        @text@ - Up to 100 unsigned double byte Unicode text to be displayed.

************************************************************************/
typedef struct _sdvr_osd_config_ex_t {
    sx_uint8    translucent;
    sdvr_location_e    location_ctrl;
    sx_uint16   top_left_x;
    sx_uint16   top_left_y;
    sdvr_dts_style_e  dts_format;
    sx_uint8    text_len;
    sx_uint16   text[SDVR_MAX_OSD_EX_TEXT];
} sdvr_osd_config_ex_t;

/************************************************************************
    VISIBLE: This structure is associated with the functions
    @sdvr_fosd_get_cap@ and @sdvr_fosd_spec@.

        @num_enc@ - the number of streams for an encoder job.
        It is zero for a decoder job.

        @num_enc_osd@ - the number of OSD lines for a encoder stream.
        It is zero for a decoder job.

        @num_hmo@ - the number of HMO.

        @num_hmo_osd@ - the number of OSD lines for an HMO.

        @num_smo@ - the number of SMO.

        @num_smo_osd@ - the number of OSD lines for an SMO.

        @num_emo@ - the number of EMO.

        @num_emo_osd@ - the number of OSD lines for an EMO.

        @max_width_cap@ - the maximum width of an OSD line in pixels.

        @max_height_cap@ -the maximum height of an OSD line in pixels.

    Remarks:

        @num_emo@ is not supported yet.

************************************************************************/
typedef struct {
    sx_uint8        num_enc;
    sx_uint8        num_enc_osd;
    sx_uint8        num_hmo;
    sx_uint8        num_hmo_osd;
    sx_uint8        num_smo;
    sx_uint8        num_smo_osd;
    sx_uint8        num_emo;
    sx_uint8        num_emo_osd;
    sx_uint16       max_width_cap;
    sx_uint16       max_height_cap;
} sdvr_fosd_msg_cap_t;

/************************************************************************
    VISIBLE: This data structure is used to configure each one of the OSD items
    associated with video frames of any camera or player. Each OSD
    item can be up to a 100 double-byte string.
    See "Important Restrictions" for number of OSD texts allowed per channel.

    After an OSD item is configured, it can be shown or hidden at any time.

    Before setting any of the fields in this data structure, you must memset it to
    zero with the size of the sdvr_fosd_config_t.

        @dts_format@ - The format of date and time to optionally
        be appended to the end of the OSD text.

        @position_ctrl@ - The position of OSD text.
        It can be any of the predefined locations in
        @sdvr_location_e@ or a custom defined location.

        @pos_x@, @pos_y@ - The top left coordinates of the OSD line
        if the custom @position_ctrl@ is specified;
        otherwise these fields are ignored.

        @max_width@, @max_height@ - the maximum width and height of
        this OSD line in pixel.
        They can not be larger than those defined at @sdvr_fosd_msg_cap_t@;

        @font_table_id@ - the font table ID of this OSD text line.
        Its valid range is 0 to 15. The default value is 0.
        If a font table of this ID is not available, no OSD is shown.
        User defined font table IDs are from 8 to 15, IDs 0 to 7 are
        reserved for system fonts.

        @text_len@ - The number of unsigned double byte characters
        in the text field.

        @text@ - Up to 100 unsigned double byte Unicode text per resolution (see Remark below)
        to be displayed. Some of texts are chopped if it is beyond the @max_width_cap@ specified
        by @sdvr_fosd_spec@.

    Remarks:

        For each video stream having standard resolution, it can only specify one text
        for such resolution and all the other streams that have that same resolution
        will have the same text with it. However, if the a stream has customed (non standard)
        resolution, it can have independent fosd, i.e., have different osd texts on
        different streams of the same resolution.

        @max_width@ and @max_height@ are not supported yet.
        They will be used to reduce the memory for the bitmap of an OSD line.

************************************************************************/
typedef struct {
    sdvr_dts_style_e    dts_format;
    sdvr_location_e     position_ctrl;
    sx_uint16           pos_x;
    sx_uint16           pos_y;
    sx_uint16           max_width;
    sx_uint16           max_height;

    sx_uint8            font_table_id;
    sx_uint8            text_len;
    sx_uint16           text[SDVR_MAX_OSD_EX_TEXT];
} sdvr_fosd_config_t;

/************************************************************************
    VISIBLE: FOSD module id of an osd id

    @SDVR_FOSD_MODULE_ID_ENC@ - this osd applies to one of encoder streams

    @SDVR_FOSD_MODULE_ID_HMO@ - this osd applies to HMO

    @SDVR_FOSD_MODULE_ID_SMO@ - this osd applies to SMO

    @SDVR_FOSD_MODULE_ID_EMO@ - this osd applies to EMO

    @SDVR_FOSD_MODULE_ID_NUM@ - the number of sdvr_fosd_module_id_t values

************************************************************************/
typedef enum {
    SDVR_FOSD_MODULE_ID_ENC,
    SDVR_FOSD_MODULE_ID_HMO,
    SDVR_FOSD_MODULE_ID_SMO,
    SDVR_FOSD_MODULE_ID_EMO,
    SDVR_FOSD_MODULE_ID_NUM,
} sdvr_fosd_module_id_t;

/************************************************************************
    Get module id from an OSD id
************************************************************************/
#define SDVR_FOSD_MODULE_ID(osd_id)      (((osd_id) >> 12) & 0x0F)

/************************************************************************
    Get stream id from an OSD id
************************************************************************/
#define SDVR_FOSD_STREAM_ID(osd_id)      (((osd_id) >> 8) & 0x0F)

/************************************************************************
    Get OSD text id from an OSD id
************************************************************************/
#define SDVR_FOSD_TEXT_ID(osd_id)        ((osd_id) & 0xFF)

/************************************************************************
    Set an OSD id by module id, stream id, and osd text id

    @mid@ - module ID specifies the place to apply OSD.
    Its valid values are defined at @sdvr_fosd_module_id_t@.

    @sid@ - stream ID specifies the stream to apply OSD.
    Its valid values are specified by @sdvr_fosd_spec@.
    For example, a camera encoder job supports 2 stream encodings,
    2 HMO outputs, and 2 SMO outputs (ports).
    The ranges of the stream IDs for encoder, HMO output, and SMO output are 0 to 1.
    For example, an IP camera job supports 4 stream encodings,
    0 HMO output, and 1 SMO output (port).
    The range of the stream IDs for encoder is 0 to 3.
    The valid value of the stream IDs for SMO output is 0.

    @tid@ - text ID specifies the OSD line to apply OSD.
    Its valid values are specified by @sdvr_fosd_spec@.
    For example, the valid range is 0 to 4, if the number of OSDs is 5.

************************************************************************/
#define SDVR_FOSD_OSD_ID(mid, sid, tid)  ((((mid)&0x0F) << 12) | (((sid)&0x0F) << 8) | (((tid)&0xFF)))

/************************************************************************
    VISIBLE: Stretch DVR pre-loaded font tables.

    @SDVR_FT_FONT_ENGLISH@ - Fonts for English character sets.

************************************************************************/
#define SDVR_FT_FONT_ENGLISH         0

/************************************************************************
    VISIBLE: The format of the font file.

    @SDVR_FT_FORMAT_BDF@ - The Glyph Bitmap Distribution Format (BDF) by
    Adobe is a file format for storing bitmap fonts. BDF is the most commonly
    used font file within the Linux operating system.
    Currently, this is the only supported format.

************************************************************************/
#define SDVR_FT_FORMAT_BDF     1

/***********************************************************************
    VISIBLE: This data structure is used to specify a new OSD font table.
    You can either use all the characters or a sub-set of the characters within
    the font file. Additionally, you can choose a color to be used for all
    the characters. The same OSD font is used for all the DVR boards that
    are connected when sdvr_osd_set_font_table() is called.

    Before setting any of the fields in this data structure, you must memset it to
    zero with the size of the sdvr_font_table_t.

    @font_file@ - Full path to the font file. Currently this must be
    a .bdf file.

    @font_table_id@ - The font table ID. User defined font table IDs are
    from 8 to 15, IDs 0 to 7 are reserved for system fonts.
    @This field is ignored in this release.@

    @font_table_format@ - The format of the font file. Currently the only
    supported font format is SDVR_FT_FORMAT_BDF. @This field is ignored
    in this release.@

    @start_font_code@ - The first font character to use within the font
    file. Use 0 for the lowest character code.

    @end_font_code@ - The last font character to use within the font file. Use
    65535 for the highest character code. Set both start_font_code and
    end_font_code to zero if you only are interested to send the characters
    used in the OSD text. See sdvr_osd_set_font_table() for more details.

    @color_y@ - Y component color of the character in YUV space. Use 255 for
    white.

    @color_u@ - U component color of the character in YUV space. Use 128 for
    white.

    @color_v@ - V component color of the character in YUV space. Use 128 for
    white.
******************************************************************************/
typedef struct _sdvr_font_table_t {
    char *font_file;
    sx_uint8 font_table_id;
    sx_uint8 font_table_format;
    sx_uint32 start_font_code;
    sx_uint32 end_font_code;
    sx_uint8 color_y;
    sx_uint8 color_u;
    sx_uint8 color_v;
} sdvr_font_table_t;

/****************************************************************************
    VISIBLE: The following enum describes various options that you can use
    to force SMO re-sizing to take place.

    SDVR_RESIZE_LOC_DEFAULT - The firmware chooses where to re-size the
    decoder output. This location may differ  depending on the board's hardware
    configuration.

    SDVR_RESIZE_LOC_SOURCE - Force the re-sizing of the camera or the
    decoder output to be on the same PE as the camera or decoder.

    SDVR_RESIZE_LOC_SMO - Force he re-sizing of the decoder output to be
    on the same PE as the SMO port.
****************************************************************************/
typedef enum _sdvr_resize_location_e {
    SDVR_RESIZE_LOC_DEFAULT = 0,
    SDVR_RESIZE_LOC_SOURCE,
    SDVR_RESIZE_LOC_SMO
} sdvr_resize_location_e;

/****************************************************************************
  VISIBLE: This data structure is used to specify the spot monitor output
  grid pattern. The SMO display is divided into different grids specified
  by a top left location and a resolution decimation of the original video
  camera assigned to that grid. Grids can be overlaid  on top of each other
  to produce a picture-in-picture effect.

  Each encode or decode video channel can display its raw image at a particular
  pixel position on the SMO display.

  Each grid on the SMO screen consists of one or more encode or decode channel
  outputs with a specific resolution decimation.

  After a grid is defined, you can temporarily enable or disable its output.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_smo_grid_t.

    @top_left_mb_x@, @top_left_mb_y@ - These two numbers specify the top-left
    coordinates of the display position based on screen resolution pixels.
    These coordinates must be even number values.
    The coordinate of the top-left corner of SMO is (0,0).

    @dwell_time@ - Deprecated field on SDK version 7.7.0.0.

    @enable@ - To disable and not display this grid on the SMO, set the value
    of this field to zero. Otherwise, the channel assigned to this grid
    is displayed on the SMO.

    @width@ - Width dimension based on screen resolution pixels
    to resize this grid. The width must be multiple of 2 with a
    minimum value of 8.  A value
    of zero will omit resizing and uses the maximum video standard width.

    @height@ - Height dimension based on screen resolution pixels
    to resize this grid. The height must be multiple of 2 with a
    minimum value of 8. A value
    of zero will omit resizing and uses the maximum video standard height.

    @layer@ - Layer number for this image. Use this field to implement
    picture-in-picture affect on the SMO display. Grids with layer number
    0 are placed first, grids with layer number 1 are place on top of the
    grids with layer number 0, and so on. This field is only available  on
    firmware version 5.2 or higher.

    @resize_location@ - This field allows you to have more control as where
    to perform re-sizing of the camera or decoder output video frames.
    See sdvr_resize_location_e for details.
    NOTE: Re-sizing takes place on the default location
    for the board for any illegal location.

    @reserved1@ - Set the field to zeros.

  Remarks:

    It is assumed that both @width@ and @height@ fields are zero if either
    are set to zero.

****************************************************************************/
typedef struct _sdvr_smo_grid_t {
    sx_uint16           top_left_mb_x;
    sx_uint16           top_left_mb_y;
    sx_uint8            dwell_time;
    sx_bool             enable;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint8            layer;
    sx_uint8            resize_location;
    sx_uint8            reserved1[17];
} sdvr_smo_grid_t;

/****************************************************************************
    VISIBLE: The following enum describes various supported SMO capabilities.
    Each feature is supported by the SMO port if the corresponding bit is set.

    SDVR_SMO_CAP_OUTPUT - Supports outputting DVR Host Application
    generated raw video.

    SDVR_SMO_CAP_OSD - Supports OSD text that is displayed only on this SMO port.

    SDVR_SMO_CAP_ALPHA_BLENDING - Supports alpha blending of OSD.

    SDVR_SMO_CAP_TILING - Supports tiling of video, streaming from different
    cameras or host output on this SMO display.

    SDVR_SMO_CAP_ANALOG - Supports full analog video, streaming from one
    camera at a time in full resolution on this SMO display. OSD text per camera
    or tiling of multiple cameras is not supported in this mode.

    SDVR_SMO_CAP_ANALOG_CASCADE - Supports cascading the analog SMO outputs
    across multiple boards in a single system. The boards will have to be
    physically wired together for this to work. Only one board's SMO output
    must be enabled at any time. It is the host application's responsibility
    to ensure this.

    SDVR_SMO_CAP_AUDIO - Supports decoding of G711 audio frame and playing
    the audio.

    SDVR_SMO_CAP_EMO - Supports encoded video output of the monitor rather
    than raw video output to a display device. The encoded video is sent
    to the host.  Legacy (S6) EMOs only.

    SDVR_SMO_CAP_EMO_MIRROR - Supports encoded video output mirroring of the
    monitor in addition to the display device.  The encoded video is sent
    to the host.  Not supported on legacy (S6) EMOs.

****************************************************************************/
typedef enum _sdvr_smo_capabilities_e {
    SDVR_SMO_CAP_OUTPUT = 1,
    SDVR_SMO_CAP_OSD = 2,
    SDVR_SMO_CAP_ALPHA_BLENDING = 4,
    SDVR_SMO_CAP_TILING = 8,
    SDVR_SMO_CAP_ANALOG = 16,
    SDVR_SMO_CAP_ANALOG_CASCADE = 32,
    SDVR_SMO_CAP_AUDIO = 64,
    SDVR_SMO_CAP_EMO = 128,
    SDVR_SMO_CAP_EMO_MIRROR = 256,
} sdvr_smo_capabilities_e;

/****************************************************************************
    VISIBLE: The following enum describes various actions that can take place
    while the SMO port is being disabled.


    SDVR_SMO_DISABLE_MODE_BLANK - Clear the SMO display.

    SDVR_SMO_DISABLE_MODE_COLORBAR - Display color-bar while the SMO display is
    disabled.
****************************************************************************/
typedef enum _sdvr_smo_disable_mode_e {
    SDVR_SMO_DISABLE_MODE_BLANK = 0,
    SDVR_SMO_DISABLE_MODE_COLORBAR
} sdvr_smo_disable_mode_e;

/****************************************************************************
  VISIBLE: This data structure holds attributes for a specified Spot Monitor
  Out (SMO).

    @video_formats@ - A bit map to indicate supported raw video formats.
    see @sdvr_rawv_formats_e@.

    @width@ - The width of the SMO display.

    @height@ - The number of lines of the SMO display

    @cap_flags@ - A bit map specifying all the capabilities of the
    requested SMO port. See @sdvr_smo_capabilities_e@.

    @supported_vstds@ -
    It is a bit OR of sdvr_video_std_e that indicates all the supported
    video standards on
    a SMO port. This field is only valid when connecting to a DVR firmware
    version of 7.0.0.0 or higher. Otherwise, its value is undefined.

    @current_standard@ - The video standard that is currently set for
    this SMO port. This field is only valid when connecting to a DVR firmware
    version of 7.0.0.0 or higher. Otherwise, its value is undefined.

****************************************************************************/
typedef struct _sdvr_smo_attribute_t {
    sx_uint16           video_formats;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint16           cap_flags;
    sx_uint16           supported_vstds;
    sx_uint16           current_standard;
} sdvr_smo_attribute_t;

/****************************************************************************
  VISIBLE: This data structure is used to specify the encoded monitor output
  grid pattern. The EMO display is divided into different grids specified
  by a top-left location and a resolution decimation of the original video
  camera assigned to that grid. Grids can be overlaid  on top of each other
  to produce a picture-in-picture effect.

  Each encode or decode video channel can display its raw image at a particular
  pixel position on the EMO display.

  Each grid on the EMO consists of one encode or decode channel
  output with a specific resolution decimation.

  After a grid is defined, you can temporarily enable or disable its output.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_smo_grid_t.

    @top_left_x@, @top_left_y@ - These two numbers specify the top-left
    coordinates of the display position based on screen resolution pixels.
    These coordinates must be even number values.
    The coordinate of the top-left corner of EMO is (0,0).

    @enable@ - To disable and not display this grid on the EMO, set the value
    of this field to zero. Otherwise, the channel assigned to this grid
    is displayed on the EMO.

    @width@ - Width dimension based on screen resolution pixels
    to resize this grid. The width must be a multiple of 2.  A value
    of zero omits resizing and uses the maximum video standard width.

    @height@ - Height dimension based on screen resolution pixels
    to resize this grid. The height must be a multiple of 2. A value
    of zero omits resizing and uses the maximum video standard height.

    @layer@ - Layer number for this image. Use this field to implement a
    picture-in-picture effect on the SMO display. Grids with layer number
    0 are placed first, grids with layer number 1 are place on top of the
    grids with layer number 0, and so on.

    @reserved2@ - Set this field to zeros.

  Remarks:

    This data structure is only valid on DVR firmware versions 5.3 or
    higher.
****************************************************************************/
typedef struct _sdvr_emo_grid_t {
    sx_uint16           top_left_x;
    sx_uint16           top_left_y;
    sx_uint8            reserved1;
    sx_bool             enable;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint8            layer;
    sx_uint8            reserved2[18];
} sdvr_emo_grid_t;

/****************************************************************************
  VISIBLE: This data structure holds attributes for a specified Encoded Monitor
  Out (EMO).

    @width@ - The width of the SMO display.

    @height@ - The number of lines of the SMO display

    @cap_flags@ - A bit map specifying all the capabilities of the
    requested EMO port. See @sdvr_smo_capabilities_e@.

  Remarks:

    This data structure is only valid on DVR firmware versions 5.3 or
    higher.

****************************************************************************/
typedef struct _sdvr_emo_attribute_t {
    sx_uint16           reserved1;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint16           cap_flags;
    sx_uint32           reserved2[8];
} sdvr_emo_attribute_t;

/****************************************************************************
  VISIBLE: This data structure defines video encoder parameters for
  Encoded Monitor Out (EMO) channels.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of _sdvr_emo_video_enc_params_t.

    @frame_rate@ - The frequency of recording. The value of this field should
    be assigned from the return value of the function sdvr_set_frame_rate().
    The default is 30 fps for NTSC and 25 fps for PAL.

    @flush_buf@ - A flag to indicate whether the encoder must flush its buffer
    immediately. Setting this flag to true causes the encoder buffer to be
    flushed immediately. Otherwise, there is a 1 frame latency before the encoder
    buffer is flushed. It is not recommended to set this flag to true for
    frame rates of higher than 10. @NOTE: Enabling the encoder flush buffer could affect
    the system performance.@

  The parameters for each encoder are set in the following
  "union".

  H.264 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR, VBR, or CBR_S is selected.
     The default is 2000.

     @max_bitrate@ - The maximum target bit rate in Kbits per second
     if VBR or constant quality is selected.
     The default is 4000.

     @bitrate_control@ - The encoder bit rate control. Refer to
     sdvr_br_control_e for description of each bit rate control.
     The default is constant bit rate.

     @gop_size@ - GOP size for the H.264 encoder. GOP size must be
     greater than zero. The default is 15.

     @quality@ - The quality level to be maintained when bitrate_control
     is set to constant quality. The valid range is 1 - 100.
     The default is 50.

****************************************************************************/
typedef struct _sdvr_emo_video_enc_params_t {
    sx_uint8                    frame_rate;
    sx_uint8                    reserved1;
    sx_bool                     flush_buf;
    sx_uint8                    reserved2;

    // Video encoder specific channel parameters
    union {
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } h264;
    } encoder;

} sdvr_emo_video_enc_params_t;

/****************************************************************************
  VISIBLE: Typedef for video alarm callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK will alert the DVR
  Application when video alarms happen above the specified threshold for
  the specific alarm.

  The video alarm callback function is called whenever the encoder detects
  motion, blinding, nighttime light conditions, or video loss or detection..
  The callback function takes as its arguments the channel handle,
  the type of alarm, and alarm data. The meaning of alarm data varies
  depending on the type of alarm (i.e., for motion alarm, the alarm data
  is the actual amount of motion over the given threshold).
  These arguments are set by the SDK so that the callback function can
  determine which board and which video channel the alarm is coming from,
  and the type of alarm.
****************************************************************************/
typedef void (*sdvr_video_alarm_callback)(sdvr_chan_handle_t handle,
                                          sdvr_video_alarm_e alarm_type,
                                          sx_uint32 data);

/****************************************************************************
  VISIBLE: Typedef for sensor callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application when sensors trigger.

  The sensor callback is called whenever one or more sensors on each
  board are triggered. The callback function takes as its arguments the
  board index and the sensor map. These arguments are set by the SDK
  so that the callback function can determine which board and which sensors
  have triggered. The sensors that have triggered are in
  sensor_map, with bit 0 corresponding to sensor 0, bit 1 to sensor 1,
  and so on.

  Remarks:

     @NOTE@: This is a deprecated type and should not be used.
     This type is replaced with sdvr_sensor_64_callback.
****************************************************************************/
typedef void (*sdvr_sensor_callback)(sx_uint32 board_index,
                                     sx_uint32 sensor_map);

/****************************************************************************
  VISIBLE: Typedef for sensor callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application when sensors trigger.

  The sensor callback is called whenever one or more sensors on each
  board are triggered. The callback function takes as its arguments the
  board index and the sensor map. These arguments are set by the SDK
  so that the callback function can determine which board and which sensors
  have triggered. The sensors that have triggered are in
  sensor_map, with bit 0 corresponding to sensor 0, bit 1 to sensor 1,
  and so on.
****************************************************************************/
typedef void (*sdvr_sensor_64_callback)(sx_uint32 board_index,
                                        sx_uint64 sensor_map);

/****************************************************************************
  VISIBLE: Typedef for stream callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application whenever encoded or raw AV frames as well as other stream
  such as motion map buffer are available.

  The stream callback function is called whenever a new
  encoded AV frame is available, or when a raw video or
  audio frame is available. The callback function takes as its arguments
  the channel handle and the frame type to
  determine what the frame is and where it came from. The last argument is
  a zero based id to distinguish between different streams within a
  particular frame category. (i,e. There
  can be multiple encoder stream associated with an encoder channel. In this
  case the stream_id indicates the encoder stream).
  The stream_id parameter is not valid for all the frame types.
****************************************************************************/
typedef void (*sdvr_stream_callback)(sdvr_chan_handle_t handle,
                                     sdvr_frame_type_e frame_type,
                                     sx_uint32 stream_id);

/****************************************************************************
  VISIBLE: Typedef for AV frame callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application whenever encoded or raw AV frames are available.

  The AV frame callback function is called whenever a new
  encoded AV frame is available, or when a raw video or
  audio frame is available. The callback function takes as its arguments
  the channel handle and the frame type to
  determine what the frame is and where it came from. The last argument is
  to distinguish between primary and secondary subchannels in a
  particular encoding channel. This parameter is only valid for
  encoded video frames and has no meaning for audio or raw video frames.

  @NOTE: This is a deprecated callback. You should use sdvr_stream_callback and
  sdvr_set_stream_callback().@
****************************************************************************/
typedef void (*sdvr_av_frame_callback)(sdvr_chan_handle_t handle,
                                       sdvr_frame_type_e frame_type,
                                       sx_bool primary_frame);

/****************************************************************************
  VISIBLE: Typedef for displaying debug callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application whenever tracing error messages need to be displayed on the
  screen.

  The callback function takes as its argument the string buffer to display.
****************************************************************************/
typedef void (*sdvr_display_debug_callback)(char *message);


/****************************************************************************
  VISIBLE: Typedef for firmware asynchronous send message callback function.
  This function has to be written by the Application writer and be
  registered as a callback. Through this callback mechanism, the SDK will
  signal the Application when firmware needs to send messages to the DVR
  Application without the application initiating a request. These messages
  are currently only error conditions generated on the firmware side, but in
  the future it could include other types of messages.

  The callback function takes as its argument the signal type and a pointer
  to the signal information data structure.
****************************************************************************/
typedef void (*sdvr_signals_callback)(sx_uint32 board_index,
                                      sdvr_signal_info_t *signal_info);

/****************************************************************************
  VISIBLE: Typedef for send frame confirmation callback function. This function has
  to be written by the Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR
  Application that the frame was received by the DVR firmware.

  The send frame confirmation callback function is called whenever
  the frame that was sent to the DVR firmware is
  processed and released. Encode frames to be decoded or DVR host
  provided raw video frames are examples for such confirmation.
  The callback function takes as its argument
  the channel handle that sent the frame.
****************************************************************************/
typedef void (*sdvr_send_confirmation_callback)(sdvr_chan_handle_t handle);


/****************************************************************************
  VISIBLE: Typedef for the firmware ROM file data callback function.
  This function has to be written by the Application writer and be passed to the
  sdvr_flash_firmware_buffer() to burn the Flash on the board using the
  Application provided firmware file. Through this callback mechanism, the SDK requests
  the next chunk of firmware ROM file data that will be burned on to the
  board. The Application should fill the next chunk of firmware ROMfile
  data in the specified buffer and return the actual buffer size.
  After all the firmware ROM file data has been transmitted, the
  Application should return the buffer size of zero to signal end of
  transmission.

  Parameters:

    rom_file_chunk - A pointer to a buffer that should hold the next
    chunk of the ROM file data to be burned. This buffer is allocated by the
    SDK. The Application should copy the next ROM file data chunk up to
    the specified chunk size.  This is an output parameter.

    rom_file_chunk_size - The maximum size of
    each ROM file chunk buffer.

  Return:

    Return zero if no more data is available to be sent. Otherwise,
    the actual number of bytes copied in the rom_file_chunk buffer.

****************************************************************************/
typedef sx_uint32(*sdvr_flash_fw_callback)(sx_uint8 *rom_file_chunk,
                                           sx_uint32 rom_file_chunk_size);



/****************************************************************************
  VISIBLE: Enumerated type describing the various A/V file container types supported
  by the SDK in order to save A/V frames to a file or extract A/V frames from
  the specified file.

    SDVR_FILE_TYPE_NONE - Invalid file type.

    SDVR_FILE_TYPE_MOV - This is a MP4 container as define by QuickTime player.
    This container supports saving of both audio and video frames. This A/V
    container supports SDVR_VIDEO_ENC_H264, SDVR_VIDEO_ENC_JPEG, and
    SDVR_VIDEO_ENC_MPEG4 video encoders. This file type has .mov file extension.

    SDVR_FILE_TYPE_STRETCH - This is a Stretch proprietary A/V container format.
    This container supports saving of both audio and video frames. It
    supports SDVR_VIDEO_ENC_H264, SDVR_VIDEO_ENC_H264_SVC, SDVR_VIDEO_ENC_JPEG ,and
    SDVR_VIDEO_ENC_MPEG4 video encoders.
    The Stetch Multi-media File container has the extension of .smf.

    SDVR_FILE_TYPE_ELEMENTARY - This file format stores the encoded video bit streams
    as they are sent by the DVR board. It does not have any file indexing and
    only supports saving of video frames. It supports any video frame type supported by
    stretch video encoders. There is no video frame extractions supported by this file
    format except for h.264-svc elementary stream. The file extension for this file
    type varies depending on the video CODEC type:

        H.264-AVC CODEC - The file extension is .h264
        H.264-SVC CODEC - The file extension is .svc
        JPEG CODEC      - The file extension is .mjpeg
        MPEG4 CODEC     - The file extension is .mpv

    SDVR_FILE_TYPE_AVI - AVI file format is not currently supported.

    SDVR_FILE_TYPE_THIRDPARTY - It supports various file formats from 3rd party.
    Since its file extension can be varied, a simple file type detector is
    implemented to decide its file type and find a right parser to parse it.
    It returns SDVR_FILE_TYPE_NONE if a right parser is not found.

****************************************************************************/
typedef enum _sdvr_file_type_e {
    SDVR_FILE_TYPE_NONE = 0,
    SDVR_FILE_TYPE_MOV,
    SDVR_FILE_TYPE_STRETCH,
    SDVR_FILE_TYPE_ELEMENTARY,
    SDVR_FILE_TYPE_AVI,
    SDVR_FILE_TYPE_THIRDPARTY
} sdvr_file_type_e;

/****************************************************************************
  VISIBLE: Enumerated type describing the different A/V frames recording mode
  as is supported by sdvr_file_start_recording().

    SDVR_FILE_REC_MODE_NORMAL - In this mode, the SDK saves A/V frames into
    the specified A/V container between the time sdvr_file_start_recording()
    and  sdvr_file_stop_recording() is called.
    If the append_date_time field of sdvr_rec_specification_t structure is set
    to true, the current date/time stamp will be appended to the specified
    file prefix before the file is created.

    SDVR_FILE_REC_MODE_FRAMES_LIMIT - The same as SDVR_FILE_REC_MODE_NORMAL except
    the recording file re-opens once the specified number of video frames based
    on GOP size is recorded. The file re-opening is based on the rule
    specified by the append_date_time field of sdvr_rec_specification_t structure.

    SDVR_FILE_REC_MODE_TIME_LIMIT - Not supported.

    SDVR_FILE_REC_MODE_SIZE_LIMIT - Not supported.

    NOTE: All of the limiting modes are approximate. The new file will not be
    re-opened until the current GOP finishes.
****************************************************************************/
typedef enum _sdvr_file_rec_mode_e {
    SDVR_FILE_REC_MODE_NORMAL,
    SDVR_FILE_REC_MODE_FRAMES_LIMIT,
    SDVR_FILE_REC_MODE_TIME_LIMIT,
    SDVR_FILE_REC_MODE_SIZE_LIMIT
} sdvr_file_rec_mode_e;

/****************************************************************************
  VISIBLE: These defines specify the maximum size path of a folder to record
  the A/V frames as well as the size of prefix for name of a A/V file container.

    SDVR_FILE_MAX_PATH - The maximum name of the folder to save the A/V
    file containers.

    SDVR_FILE_MAX_PREFIX - The maximum length of a prefix to be used for
    naming of a A/V file container.
****************************************************************************/
#define SDVR_FILE_MAX_PATH    200
#define SDVR_FILE_MAX_PREFIX  20

/****************************************************************************
  VISIBLE: This data structure allows you to specify the A/V frames recording
  specification when requesting the SDK to start saving of the A/V frames to
  a file by calling sdvr_file_start_recording(). You can choose the A/V file
  container type, where the file should be stored and a prefix for the file
  name, as well as other recording policies. Additionally, you need specify
  information regarding the A/V frames that will be stored in this file.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_rec_specification_t.

    @file_type@    - The A/V file container type. See sdvr_file_type_e for
    more description.

    @file_path@ - The path to the folder to save the recorded files. If the
    append_date_time is set to true, there could be multiple files created in this
    folder. It is recommended to choose different folder for different
    cameras and their sub-encoders. For example, choose c:\recorded_av_files\camera_1\primary.

    @file_prefix@ - The prefix to use to identify the recorded file name.
    Note that if the append_date_time field is set to true, the file name will be
    appended with the current date and time. The file extension is set by
    using the file_type field and the video CODEC. See sdvr_file_type_e for
    more information.

    @rec_mode@ - The file recording mode. See sdvr_file_rec_mode_e for more
    information.

    @append_date_time@ - If set to true, the current date/time is appended to the
    file_prefix before opening the file for recording.

    @is_skip_audio@ - If set to true, no audio frames will be recorded in the
    file even if the encoded audio frames are being streamed.

    @rec_limit_data@ - The data associated to the rec_mode. Set to zero for
    no recording limit. Otherwise, the field holds the data associated with the
    value of rec_mode. (i,e. if rec_mode is set to SDVR_FILE_REC_MODE_FRAMES_LIMIT, this
    field holds the maximum number of video frame based on GOP size to record.
    Assuming a GOP size of 15 if this field is set to 100, the file includes a
    maximum of 1500 video frames.)

    @reserved@ - Reserved field must be set to zeros.
****************************************************************************/
typedef struct _sdvr_rec_specification
{
    sdvr_file_type_e      file_type;
    char                  file_path[SDVR_FILE_MAX_PATH];
    char                  file_prefix[SDVR_FILE_MAX_PREFIX];
    sdvr_file_rec_mode_e  rec_mode;
    sx_bool               append_date_time;
    sx_bool               is_skip_audio;
    sx_uint64             rec_limit_data;
    sx_uint8              reserved[30];
} sdvr_rec_specification_t;

/**********************************************************************************
    Maximum number of h.264-svc spatial layers
**********************************************************************************/
#define SDVR_MAX_SVC_SPATIAL_LAYERS  3

/**********************************************************************************
    Maximum number of h.264-svc temporal levels
**********************************************************************************/
#define SDVR_MAX_SVC_TEMPORAL_LEVELS 3

/**********************************************************************************
  VISIBLE: This data structure describes information per svc
  spatial/temporal layer combination.

    @spatial_layer@ - Spatial layer id of the layer. The valid
    ranges are zero (0) to SDVR_MAX_SVC_SPATIAL_LAYERS minus one (1).

    @temporal_level@ - Temporal level id of the layer. The valid
    ranges are zero (0) to SDVR_MAX_SVC_TEMPORAL_LEVELS minus one (1).

    @frame_width@ - Frame width represented by the layer.

    @frame_height@ - Frame height represented by the layer.

    @frame_rate@ - Frame rate represented by the layer.
    Divide this field by 256.0 in order to get number of frames
    per seconds.

**********************************************************************************/
typedef struct _sdvr_svc_layer_info {
    sx_uint8  spatial_layer;
    sx_uint8  temporal_level;
    sx_uint16  frame_width;
    sx_uint16  frame_height;
    sx_uint16  frame_rate;
} sdvr_svc_layer_info_t;


/**********************************************************************************
  VISIBLE: This data structure describes information regarding the svc
  stream.

    @num_layers@ - Total number of spatial/temporal layer combinations.

    @num_spatial_layers@ - Total number of spatial layers.

    @num_temporal_levels@ - Total number of temporal levels.

    @layer_info@ - An array of @st_svcext_layer_info_t@.

**********************************************************************************/
typedef struct _sdvr_svc_info {
    sx_uint32  num_layers;
    sx_uint32  num_spatial_layers;
    sx_uint32  num_temporal_levels;
    sdvr_svc_layer_info_t layer_info[SDVR_MAX_SVC_SPATIAL_LAYERS * SDVR_MAX_SVC_TEMPORAL_LEVELS];
} sdvr_svc_info_t;

/****************************************************************************
  VISIBLE: This data structure holds various information regarding video and
  audio tracks in an A/V file container.

  Before setting any of the fields in this data structure, you must memset it to
  zero with the size of the sdvr_file_avtrack_info_t.

  @video@ - This structure holds properties of the video track inside the
  A/V container.

    @codec@    - The video CODEC of the track. See sdvr_venc_e for
    more description.

    @frame_width@ - The width of the input raw video frame prior to encoding.
    In case of h.264-svc this field correspond to frame width represented by the highest
    spatial layer at the highest temporal level.

    @frame_height@ - The height of the input raw video frame prior to encoding.
    In case of h.264-svc this field correspond to frame height represented by the highest
    spatial layer at the highest temporal level.

    h264-avc specific video information


    h264-svc specific video information

        @svc_info@ - This field describes information regarding the svc
        stream. See sdvr_svc_layer_info_t for details. This field is ignored
        when calling sdvr_file_open_write().

  @audio@ - This structure holds properties of the audio track inside the
  A/V container.

    @codec@    - The audio CODEC of the track. See sdvr_aenc_e for
    more description.

    @sampling_rate@ - The sampling audio rate. It must be set
    to either SDVR_AUDIO_RATE_32KHZ, SDVR_AUDIO_RATE_16KHZ, or
    SDVR_AUDIO_RATE_8KHZ. It defaults
    to SDVR_AUDIO_RATE_8KHZ if the sampling rate has any other value.

    @num_channels@ - This field holds the number of audio channel within
    this audio track. One (1) means mono recording whereas two (2) indicates
    stereo recording. For .mov file this field must be set to 2.

  @reserved@ - Reserved field must be set to zeros.
****************************************************************************/
typedef struct _sdvr_file_avtrack_info
{
    struct {
        sdvr_venc_e codec;
        sx_uint32   frame_width;
        sx_uint32   frame_height;

        union {
            struct {
                sx_uint32   reserved1[200];
            } mpeg4;

            struct {
                sx_uint32   reserved1[200];
            } mpeg2;

            struct {
                sx_uint32   reserved1[200];
            } h264_avc;
            struct {
                sdvr_svc_info_t svc_info;
                sx_uint32   reserved2[3];
            } h264_svc;
        } u;
        sx_uint32   reserved3[9];
    } video;

    struct {
        sdvr_aenc_e codec;
        sx_uint16    sampling_rate;
        sx_uint16    num_channels;
        sx_uint32   reserved4[8];
    } audio;
    sx_uint32   reserved0[10];
} sdvr_file_avtrack_info_t;

/****************************************************************************
  VISIBLE: This data structure holds the A/V file container description to
  be opened for read back. It is used as the parameter passed to
  sdvr_file_open_read().

    @file_type@  - The A/V file container type. See sdvr_file_type_e for
    more description. NOTE: File type of SDVR_FILE_TYPE_ELEMENTARY
    is only supported for h.264-svc elementary files.

    @file_path@ - The full path to the A/V file container to open for reading.

    File read extraction information specific to .mov A/V file container:

    @disable_audio_tracks@ - If set to true, the audio tracks in the file is skipped.
    Otherwise, the audio frames in the file will be retrieved if they exist.


    File read extraction information specific to .smf A/V file container:

    @disable_audio_tracks@ - If set to true, the audio tracks in the file is skipped.
    Otherwise, the audio frames in the file will be retrieved if they exist.

        Following information must be specified if extracting h.264-svc frames:

        @extract_avc@ - The AVC/SVC frame extraction flag. If set to true,
        the extracted frame correspond to a h.264-avc video frame for the specified
        spatial layer and temporal level. If set to false, the extracted frame correspond
        to a h.264-svc starting at the specified spatial layer and temporal level and
        below.

        @spatial_layer@ - Targeted spatial layer of the operating point. For
        SVC, that means it contains all the data for the spatial layer specified
        and below. For AVC, that means it contains all the data for that spatial layer.

        @temporal_level@ - Targeted temporal level of the operating point. For
        SVC, that means it contains all the data for the temporal level specified and
        below. For AVC, that means it contains all the data for that temporal level.

        @insert_dummy_avc_frame@ - If the output is AVC mode, setting this flag to true forces
        the extractor to insert skip frames for frames taken out in temporal sublayer
        streams. This allows the frame number in frames to be contiguous. This
        is needed for some players to playback the sequence.

        @Note: Frame number gap is allowed in H.264 specification but some players may not have
        handled it properly. The side effect of inserting dummy/skip frame is a little
        higher bit rate and more processing cycles for the decoder. Another
        impact is that the skip frames cannot be authenticated. The authentication
        software should ignore those skip frames or the extractor should setup
        to disable insert_dummy_avc_frame for authentication to work].@

    File read extraction information specific to elementary  video file.
    Currently only elementary svc file format is supported.

        @video_encoder@ - Type of video frames stored in the file.
        Currently only SDVR_VIDEO_ENC_H264_SVC is supported.

        Following information must be specified if extracting h.264-svc frames
        (.svc file):

        @frame_width@ - The video width of the largest spatial layer.

        @frame_height@ - The video height of the largest spatial layer.

        @extract_avc@ - The AVC/SVC frame extraction flag. If set to true,
        the extracted frame correspond to a h.264-avc video frame for the specified
        spatial layer and temporal level. If set to false, the extracted frame correspond
        to a h.264-svc starting at the specified spatial layer and temporal level and
        below.

        @spatial_layer@ - Targeted spatial layer of the operating point. For
        SVC, that means it contains all the data for the spatial layer specified
        and below. For AVC, that means it contains all the data for that spatial layer.

        @temporal_level@ - Targeted temporal level of the operating point. For
        SVC, that means it contains all the data for the temporal level specified and
        below. For AVC, that means it contains all the data for that temporal level.

        @insert_dummy_avc_frame@ - If the output is AVC mode, setting this flag to true forces
        the extractor to insert skip frames for frames taken out in temporal sublayer
        streams. This allows the frame number in frames to be contiguous. This
        is needed for some players to playback the sequence.


   Remarks:

    Before setting any of the fields in this data structure, you must memset it to
    zero with the size of the sdvr_file_read_desc_t.

****************************************************************************/

typedef struct _sdvr_file_read_desc_t
{
    sdvr_file_type_e file_type;
    char *file_path;
    union {
        struct {
            sx_bool     disable_audio_track;
            sx_uint8    reserved1[3];
            sx_uint32   reserved[3];
        } mov;
        struct {
            sx_bool     disable_audio_track;
            sx_uint8    reserved1[3];

            struct {
                sx_bool     extract_avc;
                sx_uint8    spatial_layer;
                sx_uint8    temporal_level;
                sx_bool     insert_dummy_avc_frame;
            } svc;
            sx_uint32   reserved[2];

        } smf;
        struct {
            sdvr_venc_e video_encoder;
            struct {

                sx_uint32   frame_width;
                sx_uint32   frame_height;
                sx_bool     extract_avc;
                sx_uint8    spatial_layer;
                sx_uint8    temporal_level;
                sx_bool     insert_dummy_avc_frame;
            } svc;

        } elementary;
    } u;
    sx_uint32   reserved[10];
} sdvr_file_read_desc_t;
/**************************************************************************
   VISIBLE: A handle to an A/V file container.
***************************************************************************/
typedef sx_size_t      sdvr_file_handle_t;


/****************************************************************************
  VISIBLE: Image Sensor Pipeline video component definition.
  Valid values for all the fields is a floating number in range of [0.0, 1.0].
  A strength of 0.0 disables the corresponding temporal filter.

     Y - The luma component.

     Cb - The blue-difference chroma component.

     Cr - The red-difference chroma component.

***************************************************************************/
typedef struct _sdvr_YCbCr_t {
    float Y;
    float Cb;
    float Cr;
} sdvr_YCbCr_t;

/****************************************************************************
  VISIBLE: Enumerated modes for all camera's iris control parameters.

    SDVR_IRIS_NONE: invalid mode
 
    SDVR_START_IRIS_MODE: don't use it, just for range checking
 
    SDVR_IRIS_MANUAL: manually set iris

    SDVR_IRIS_AUTO: automatically set iris

    SDVR_IRIS_OVERRIDE: use manual mode until a scene change that set it to auto
 
    SDVR_END_IRIS_MODE: don't use it, just for range checking

***************************************************************************/
typedef enum _sdvr_iris_mode_e {
    SDVR_IRIS_NONE = 255,
    SDVR_START_IRIS_MODE = 0,

    SDVR_IRIS_MANUAL = 1,
    SDVR_IRIS_AUTO = 0,
    SDVR_IRIS_OVERRIDE = 2,
    SDVR_END_IRIS_MODE = SDVR_IRIS_OVERRIDE

} sdvr_iris_mode_e;

/****************************************************************************
  VISIBLE: Enumerated camera's exposure control priority modes.

    SDVR_APERTURE: set exposure control to aperture priority

    SDVR_SHUTTER:  set exposure control to shutter priority

***************************************************************************/
typedef enum _sdvr_exposure_priority_e {

    SDVR_APERTURE = 0,
    SDVR_SHUTTER = 1
} sdvr_exposure_priority_e;

/****************************************************************************
  VISIBLE: this struct consists the four corner's coordinates of a ROI rectangle window.

    tl: top-left, br: bottom-right corners

    _x: the horizontal axis from left to right

    _y: the vertical axis from top to bottom

    tl_x, tl_y : These two numbers specify the top left coordinates of the

    rectangle window. The coordinate of the top left corner of the image frame is (0,0)

    br_x, br_y : These two numbers specify the bottom right coordinates of the

    rectangle window.

    The ranges of these coordinates are bounded by the imaging sensor's width and height

***************************************************************************/
typedef struct _sdvr_rectangle {

    sx_uint16 tl_x, br_x;
    sx_uint16 tl_y, br_y;
    sx_uint8              reserved[32];

} sdvr_roi_rectangle_t;

/****************************************************************************
  VISIBLE: this struct consists all camera's exposure control modes.

    SDVR_EXPOSURE_NONE: invalid mode
 
    SDVR_START_EXPOSURE_MODE: don't use it, just for range checking
 
    SDVR_EXPOSURE_MANUAL: manually set exposure

    SDVR_EXPOSURE_AUTO_LOW_NOISE: automatically set exposure but bias toward longer
    exposure if the input noises are high.

    SDVR_EXPOSURE_AUTO: automatically set exposure
 
    SDVR_END_EXPOSURE_MODE: don't use it, just for range checking


***************************************************************************/
typedef enum _sdvr_exposure_mode_e {
    SDVR_EXPOSURE_NONE = 255,
    SDVR_START_EXPOSURE_MODE = 0,
    SDVR_EXPOSURE_MANUAL = 2,
    SDVR_EXPOSURE_AUTO_LOW_NOISE = 0,
    SDVR_EXPOSURE_AUTO = 1,
    SDVR_END_EXPOSURE_MODE = SDVR_EXPOSURE_MANUAL
} sdvr_exposure_mode_e;

/****************************************************************************
  VISIBLE: this struct consists all camera's exposure control parameters.

    exposure_mode: set the mode of exposure control

    rect: the roi rectangle's coordinates of the incoming light window for
    exposure measurement.
 
    setpoint: the auto exposure target [0 100]; default set to 20.

***************************************************************************/
typedef struct _sdvr_exposure_t {

    sdvr_exposure_mode_e exposure_mode;
    sdvr_roi_rectangle_t rect;
    sx_uint8    setpoint;
    sx_uint8 reserved[31];

} sdvr_exposure_t;

/****************************************************************************
  VISIBLE: this struct consists all camera's shutter control parameters.

    shutter_target: the target shutter time in the unit of micro second
    range in [0, 1e6] micro sec.

    shutter_auto_min, shutter_auto_max: set the min and max shutter
    time (micro second) for the auto exposure shutter control algorithm
    range in [0, 1e6] micro second.

***************************************************************************/
typedef struct _sdvr_shutter_t {

    sx_uint32 shutter_target;
    sx_uint32 shutter_auto_min;
    sx_uint32 shutter_auto_max;
    sx_uint8 reserved[32];

} sdvr_shutter_t;

/****************************************************************************
  VISIBLE: this struct consists all camera's iris control parameters.

    iris_target: floating-point value for the target iris aperture
    The unit is in f-numbers. For example, typical full-stop f-number scale are
    the following:
    0.7 1.0 1.4 2 2.8 4 5.6 8 11 16 22 32 45 64, Max is 64.
    The larger the f numbers, the samller the aperture. Default is 1.0

    iris_auto_min, iris_auto_max: floating-point values of f-number that set
    the min and max iris aperture for the auto exposure iris control algorithm
    min and max are by face values, not by aperture.
    iris_auto_min default is 0.7, iris_auto_max default is 16

    iris_mode: mode of iris control, see sdvr_iris_mode_e for each mode

***************************************************************************/
typedef struct _sdvr_iris_t {

    float iris_target;
    float iris_auto_min;
    float iris_auto_max;
    sdvr_iris_mode_e iris_mode;
    sx_uint8 reserved[32];

} sdvr_iris_t;

/****************************************************************************
  VISIBLE: this struct consists all camera's gain control parameters.

    gain_target: floating-point value for the target camera gain in the range of 1.0 to 64.0

    gain_auto_min, gain_auto_max: set the min and max camera gains in the range of 1.0 to 64.0
    for the auto exposure gain control algorithm

***************************************************************************/
typedef struct _sdvr_gain_t {

    float gain_target;
    float gain_auto_min;
    float gain_auto_max;
    sx_uint8 reserved[32];

} sdvr_gain_t;

/****************************************************************************
  VISIBLE: Enumerated all camera's ircut control modes.

    SDVR_IRCUT_NONE: invalid mode
 
    SDVR_START_IRCUT_MODE: don't use it, just for range checking
 
    SDVR_IRCUT_DAY: ircut control mode used in day light

    SDVR_IRCUT_NIGHT: ircut control mode used in night light

    SDVR_IRCUT_AUTO: automatically decides what light conditions is

    SDVR_IRCUT_SCHEDULED: use the scheduled begin and end time to control ircut filter.
 
    SDVR_END_IRCUT_MODE: don't use it, just for range checking
 
***************************************************************************/
typedef enum _sdvr_ircut_mode_e {
    SDVR_IRCUT_NONE = 255,
    SDVR_START_IRCUT_MODE = 0,
    SDVR_IRCUT_DAY = 1,
    SDVR_IRCUT_NIGHT = 2,
    SDVR_IRCUT_AUTO = 0,
    SDVR_IRCUT_SCHEDULED = 3,
    SDVR_END_IRCUT_MODE = SDVR_IRCUT_SCHEDULED
} sdvr_ircut_mode_e;

/****************************************************************************
  VISIBLE: this struct consists the time parameters for scheduling ircut control.
***************************************************************************/
typedef struct _sdvr_ircut_time_t {

    // in 24 hour format
    sx_uint8 hr, min, sec;
    sx_uint8 reserved[32];

} sdvr_ircut_time_t;

/****************************************************************************
  VISIBLE: this struct consists ircut filter control parameters.

    mode: modes from sdvr_ircut_mode_e

***************************************************************************/
typedef struct _sdvr_ircut_t {

    sdvr_ircut_mode_e ircut_mode;
    sx_uint8 reserved[32];

} sdvr_ircut_t;

/****************************************************************************
  VISIBLE: Enumerated modes for camera's focus control.
 
    SDVR_FOCUS_NONE: invalid mode
 
    SDVR_START_FOCUS_MODE: don't use it, just for range checking
 
    SDVR_FOCUS_MANUAL: set to use manual focus

    SDVR_FOCUS_AUTO: set to use auto focus

    SDVR_FOCUS_AUTO_BACK_FOCUS: to change focus when field of view (zoom) has changed

    SDVR_FOCUS_OVERRIDE: use manual mode until a scene change that set it to auto
 
    SDVR_END_FOCUS_MODE: don't use it, just for range checking

***************************************************************************/
typedef enum _sdvr_focus_mode_e {
    SDVR_FOCUS_NONE = 255,

    SDVR_START_FOCUS_MODE = 0,
    SDVR_FOCUS_MANUAL = 1,
    SDVR_FOCUS_AUTO = 0,
    SDVR_FOCUS_AUTO_BACK_FOCUS = 3,
    SDVR_FOCUS_OVERRIDE = 2,
    SDVR_END_FOCUS_MODE = SDVR_FOCUS_AUTO_BACK_FOCUS
} sdvr_focus_mode_e;

/****************************************************************************
  VISIBLE: this struct consists of all the camera's focus control parameters.

    mode: mode for control camera's focus

    rect: the rectangle window where the focus measurement is taken

    focus_target: is in the range from 0 to 1000 for manual mode only.
    in which 0 is the closest focus the lens can get and 1000 is the farest, usually infinity.

***************************************************************************/
typedef struct _sdvr_focus_t {

    sdvr_focus_mode_e focus_mode;
    sdvr_roi_rectangle_t rect;
    sx_uint16 focus_target;
    sx_uint8 reserved[32];

} sdvr_focus_t;

/****************************************************************************
  VISIBLE: this struct consists of all the camera's wdr control parameters.

    level: strength of wdr improvement, integers in the range of 0 to 100,
    0 turns this feature off (i.e., disable it)

***************************************************************************/
typedef struct _sdvr_wdr_t {

    sx_uint8 level;
    sx_uint8 reserved[32];

} sdvr_wdr_t;

/****************************************************************************
  VISIBLE: Enumerated all camera's white balance modes.

    SDVR_WB_NONE: invalid WB mode
 
    SDVR_WB_START: don't use it, just for range checking

    SDVR_WB_MANUAL: set WB manually

    SDVR_WB_AUTO: set WB automatically
 
    SDVR_WB_END: don't use it, just for range checking
 
***************************************************************************/
typedef enum _sdvr_wb_mode_e {
    SDVR_WB_NONE = 255,
    SDVR_WB_START = 0,
    SDVR_WB_MANUAL = 1,
    SDVR_WB_AUTO = 0,
    SDVR_WB_END = SDVR_WB_MANUAL
} sdvr_wb_mode_e;

/****************************************************************************
  VISIBLE: Enumerated all pre-defined illuminants for white balance.

    SDVR_IL_NONE: invalid illuminant
 
    SDVR_IL_START: don't use it, just for range checking
    
    SDVR_IL_INCANDESCENT: indoors Tungsten lamp

    SDVR_IL_FLUORESCENT_WHITE: Mercury-vapor lamp

    SDVR_IL_FLUORESCENT_YELLOW: Sodium-vapor lamp

    SDVR_IL_OUTDOOR: outdoors day light

    SDVR_IL_BLACK_WHITE: monochromatic light
 
    SDVR_IL_END: don't use it, just for range checking
 
***************************************************************************/
typedef enum _sdvr_wb_illuminant_e {
    SDVR_IL_NONE = 255,

    SDVR_IL_START = 0,
    SDVR_IL_INCANDESCENT = 0,
    SDVR_IL_FLUORESCENT_WHITE = 1,
    SDVR_IL_FLUORESCENT_YELLOW = 2,
    SDVR_IL_OUTDOOR = 3,
    SDVR_IL_BLACK_WHITE = 4,
    SDVR_IL_END = SDVR_IL_BLACK_WHITE
} sdvr_wb_illuminant_e;

/****************************************************************************
  VISIBLE: this struct consists all camera's white balance control parameters.

    wb_mode: white balance modes

    light_source: illuminant

    level: integer percentage of fully saturated WB correction, 0 has no WB correction
    and 100 is to the fullest

    R2G_gain, B2G_gain: manual control the ratios of R/G and B/G for manual
    WB mode. They are floating-point values in the range of [0.1, 10.0].
    These two gains have no effect in release 7.7, will be implemented in later releases.

***************************************************************************/
typedef struct _sdvr_wb_t {

    sdvr_wb_mode_e wb_mode;
    sdvr_wb_illuminant_e light_source;
    sx_uint8 level;
    float R2G_gain;
    float B2G_gain;
    sx_uint8 reserved[32];

} sdvr_wb_t;

/****************************************************************************
  VISIBLE: Enumerated all pre-defined flip modes for displaying images.

    SDVR_FLIP_MODE_NONE: invalid MODE
 
    SDVR_FLIP_MODE_START: don't use it, just for range checking
    
    SDVR_FLIP_MODE_NORMAL: no flipping of images

    SDVR_FLIP_MODE_VERTICAL: flipping up-side-down

    SDVR_FLIP_MODE_HORIZONTAL: flipping left-right (mirror)

    SDVR_FLIP_MODE_BOTH: flipping both up-side-down and left-right
 
    SDVR_FLIP_MODE_END: don't use it, just for range checking
 
***************************************************************************/
typedef enum _sdvr_flip_mode_e {
    SDVR_FLIP_MODE_NONE = 255,
    SDVR_FLIP_MODE_START = 0,
    SDVR_FLIP_MODE_NORMAL = 0,
    SDVR_FLIP_MODE_HORIZONTAL = 1,
    SDVR_FLIP_MODE_VERTICAL = 2,
    SDVR_FLIP_MODE_BOTH = 3,
    SDVR_FLIP_MODE_END = SDVR_FLIP_MODE_BOTH

} sdvr_flip_mode_e;

/****************************************************************************
  VISIBLE: this struct consists all camera's flip mode control parameters.

    flip_mode: flip modes

***************************************************************************/
typedef struct _sdvr_flip_t {

    sdvr_flip_mode_e flip_mode;
    sx_uint8 reserved[32];

} sdvr_flip_t;

/****************************************************************************
  VISIBLE: This data structure defines various parameters that control IPCAM's ISP

    @hue@ - Value of hue control in the range of 0 to 100. The default is 50.

    0 maps to -90 degree in hue, and 100 maps to +90 degree in hue, 50 is zero degree neutral.

    @saturation@ - Value of saturation control in the range of 0 to 100. The default is 50.

    @brightness@ - Value of brightness control in the range of 0 to 100. The default is 50.

    @contrast@ - Value of contrast control in the range of 0 to 100. The default is 50.

    @sharpness@ - Value of sharpness control in the range of 0 to 100. The default is 50.
    The value 50 is neutral, less is softer, more is sharper.
 
  Note: The effectiveness of sharpness depends on the input image noise level.

    If the noise is too high, then sharpness may lessen its strength automatically or
    refuse to work at all.

  Remarks:

    The actual meanings of 0 and 100 values for these fields are hardware specific.

***************************************************************************/
typedef struct _sdvr_img_t {
    sx_uint8 hue;
    sx_uint8 saturation;
    sx_uint8 brightness;
    sx_uint8 contrast;
    sx_uint8 sharpness;
    sx_uint8 reserved[32];
} sdvr_img_t;

/************************************************************************
    VISIBLE: Zone parameter values for use with the Cryptographic eeprom APIs.
    These defines indicate which zone of the device to access.

    @SDVR_CRYPTO_EEPROM_ZONE_2@ - User-accessible zone #2.

    @SDVR_CRYPTO_EEPROM_ZONE_3@ - User-accessible zone #3.

    @SDVR_CRYPTO_EEPROM_ZONE_CFG@ - Configuration zone for modifying
    internal crypto EEPROM device state. Note that verifying write
    password on this zone gives the write access to any zone
    (i.e, supper user password).
************************************************************************/
typedef enum _sdvr_crypto_eeprom_zones_e {
    SDVR_CRYPTO_EEPROM_ZONE_2   = 2,
    SDVR_CRYPTO_EEPROM_ZONE_3   = 3,
    SDVR_CRYPTO_EEPROM_ZONE_CFG = 7
} sdvr_crypto_eeprom_zones_e;

/************************************************************************
    VISIBLE: Security type parameter values for use with the EEPROM APIs
    These defines indicate possible access types for user-accessible zones.

    @SDVR_EEPROM_NO_PASS@ - No password required for read nor write.

    @SDVR_EEPROM_W_PASS@ - Password required for write; read free.

    @SDVR_EEPROM_RW_PASS@ - Password required for read or write
    (write password grants read access).
************************************************************************/
typedef enum _sdvr_eeprom_security_e {
    SDVR_EEPROM_NO_PASS = 1,
    SDVR_EEPROM_W_PASS,
    SDVR_EEPROM_RW_PASS
} sdvr_eeprom_security_e;

/************************************************************************
    VISIBLE: Password type parameter values for use with the eeprom APIs.
    These defines indicate possible access types of passwords for
    user-accessible zones.

    @SDVR_EEPROM_PASS_TYPE_READ@ - Read password (no write access).

    @SDVR_EEPROM_PASS_TYPE_WRITE@ - Write password (+ read access).
************************************************************************/
typedef enum _sdvr_eeprom_password_type_e {
    SDVR_EEPROM_PASS_TYPE_READ = 1,
    SDVR_EEPROM_PASS_TYPE_WRITE,
} sdvr_eeprom_password_type_e;

/************************************************************************
    VISIBLE: Fuse ID parameter values for use with the with the Cryptographic
    eeprom APIs.
    These fuses must be blown in sequence: FAB first, then CMA, then PER.

    @SDVR_CRYPTO_EEPROM_FUSE_FAB@ - Answer-to-reset and Fab code fuse.

    @SDVR_CRYPTO_EEPROM_FUSE_CMA@ - Card manufacturer code fuse.

    @SDVR_CRYPTO_EEPROM_FUSE_PER@ - Remainder of configuration space fuse.
************************************************************************/
typedef enum _sdvr_crypt_eeprom_fuse_e {
    SDVR_CRYPTO_EEPROM_FUSE_FAB = 2,
    SDVR_CRYPTO_EEPROM_FUSE_CMA = 3,
    SDVR_CRYPTO_EEPROM_FUSE_PER = 4
}sdvr_crypt_eeprom_fuse_e;

/************************************************************************
    VISIBLE: Default password for Atmel cryptographic EEPROM devices.
    This is the default secure code (configuration zone write password)
    as shipped from the factory.  It is strongly recommended this password
    be changed before blowing the fuses.
************************************************************************/
#define SDVR_CRYPTO_EEPROM_DEFAULT_ATMEL_RW_PASS    0x605734

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

/**************************************************************************
    VISIBLE: Different sub-systems within the firmware application
    that can redirect raw commands.

    Remarks:

       Currently only SDVR_FIRMWARE_SUB_SYSTEM_IPP is supported.
***************************************************************************/
typedef enum _sdvr_firmware_sub_system_e {
    SDVR_FIRMWARE_SUB_SYSTEM_NONE, 
    SDVR_FIRMWARE_SUB_SYSTEM_IPP,
    SDVR_FIRMWARE_SUB_SYSTEM_VPP,
    SDVR_FIRMWARE_SUB_SYSTEM_FIRMWARE,
    SDVR_FIRMWARE_SUB_SYSTEM_SMO,
    SDVR_FIRMWARE_SUB_SYSTEM_H264_ENC,
    SDVR_FIRMWARE_SUB_SYSTEM_H264_SVC_ENC,
    SDVR_FIRMWARE_SUB_SYSTEM_MJPEG_ENC,
    SDVR_FIRMWARE_SUB_SYSTEM_MPEG4_ENC,
    SDVR_FIRMWARE_SUB_SYSTEM_H264_DEC,
    SDVR_FIRMWARE_SUB_SYSTEM_MJPEG_DEC,
    SDVR_FIRMWARE_SUB_SYSTEM_MPEG4_DEC
} sdvr_firmware_sub_system_e;

/**************************************************************************
    VISIBLE: This defines the maximum raw command response buffer size.
***************************************************************************/
#define SDVR_MAX_RAW_CMD_RESPONSE_SIZE 8

/**************************************************************************
    VISIBLE: This structure defines the format that is used to send
    command to any particular sub-system within the DVR firmware application
    that is only known to that sub-system and the Host Application.

    "response" - The returned response. This buffer needs to
    be mapped to the return response data structure for this command.
    It is recommended that first byte to contain the status.

    "sub_system" - The sub-system within the firmware application
    to redirect this message.

    "response_size" - The size of the response buffer that is going to be
    returned via SDVR_FRAME_CMD_RESPONSE frame buffer. This size is needed
    by the firmware to allocate a buffer large enough to hold the response
    returned by the sub-system. This size should be provided by the sub-system
    for each supported command. Set this field to zero if you don't expect
    the sub-system to return response data using SDVR_FRAME_CMD_RESPONSE
    frame buffer. This buffer will be received by the host application through the
    buffer callback.

    "cmd_data_size" - Number of bytes in the cmd_data buffer

    "cmd_data" - The data structure for this command.
    It is recommended that the first byte of the buffer specify the command
    op-code. The format of this buffer is specified by each sub-system.
***************************************************************************/
typedef struct _sdvr_raw_command_t {
    sx_uint8 response[SDVR_MAX_RAW_CMD_RESPONSE_SIZE];
    sdvr_firmware_sub_system_e sub_system;
    sx_uint32 response_size;
    sx_uint32 cmd_data_size;
    sx_uint8 *cmd_data;
} sdvr_raw_command_t;

#define sdvr_yuv_buffer_t sdvr_av_buffer_t

#define sdvr_start_video_output( board_index, port_num, video_format, handle) \
    sdvr_start_video_overlay( board_index, port_num,  video_format, handle)
#define sdvr_stop_video_output(handle) sdvr_stop_video_overlay(handle)
#define sdvr_get_video_output_buffer(handle, frame_buffer) sdvr_get_video_overlay_buffer(handle, NULL, frame_buffer, 3)
#define sdvr_send_video_output(handle, buf) sdvr_send_video_overlay(handle, buf)
#define sdvr_overaly_frame_attrib_t sdvr_overlay_frame_attrib_t
#define sdvr_clear_video_overlay sdvr_smo_clear_overlay
#define sdvr_set_smo_grid(handle, smo_grid) sdvr_smo_set_grid(handle, 0, 1, smo_grid);
#define sdvr_get_smo_grid(handle, smo_grid) sdvr_smo_get_grid(handle, 0, 1, smo_grid);
#define sdvr_get_firmware_version_ex(board_index, version_info) sdvr_get_firmware_version(board_index, version_info)

EXTERN sdvr_err_e sdvr_set_frame_skip_params(sdvr_chan_handle_t handle,
                                             sx_uint32 frame_skip_count,
                                             sx_uint32 frame_skip_sample_size);
EXTERN sdvr_err_e sdvr_raw_command_channel(sdvr_chan_handle_t handle,
                                           sdvr_raw_command_t *cmd_buf);
EXTERN int               sutil_vres_height(sdvr_video_std_e vs, sdvr_video_res_decimation_e res, sx_bool bTrueSize);
EXTERN int               sutil_vres_width(sdvr_video_std_e vs, sdvr_video_res_decimation_e res);

EXTERN sx_uint32         sutil_vstd_frame_rate(sdvr_video_std_e video_std);
EXTERN sdvr_video_size_e sutil_width_height_to_vsize(sx_uint32 width, sx_uint32 height);
EXTERN sdvr_video_std_e  sutil_width_height_to_vstd(sx_uint32 width, sx_uint32 height);

/****************************************************************************
  GROUP: System Set Up, SDK and Board Initialization API
****************************************************************************/
EXTERN char* sdvr_get_error_text(sdvr_err_e error_no);

EXTERN sdvr_err_e sdvr_sdk_init();
EXTERN sdvr_err_e sdvr_sdk_close();

EXTERN sx_uint32 sdvr_get_board_count();
EXTERN sdvr_err_e sdvr_get_board_attributes(sx_uint32 board_index,
                                            sdvr_board_attrib_t *board_attrib);
EXTERN sdvr_err_e sdvr_get_pci_attrib(sx_uint32 board_index,
                                      sdvr_pci_attrib_t *pci_attrib);

EXTERN sdvr_err_e sdvr_board_reset(sx_uint32 board_index);
EXTERN sdvr_err_e sdvr_board_connect_ex(sx_uint32 board_index,
                                        sdvr_board_settings_t *board_settings);
EXTERN sdvr_err_e sdvr_board_connect(sx_uint32 board_index,
                                     sdvr_video_std_e video_std,
                                     sx_bool is_h264_SCE);
EXTERN sdvr_err_e sdvr_board_disconnect(sx_uint32 board_index);

EXTERN sdvr_err_e sdvr_upgrade_firmware(sx_uint32 board_index,
                                        char *firmware_file_name);
EXTERN sdvr_err_e sdvr_flash_firmware(sx_uint32 board_index,
                                      char *firmware_file_name);
EXTERN sdvr_err_e sdvr_flash_firmware_buffer(sx_uint32 board_index, sx_uint32 fw_rom_size,
                                             sdvr_flash_fw_callback get_fw_data_callback);

EXTERN sdvr_err_e sdvr_set_dma_burst_size(sx_uint32 board_index, sx_uint32 size);

EXTERN sdvr_err_e sdvr_get_sdk_params(sdvr_sdk_params_t *sdk_params);
EXTERN sdvr_err_e sdvr_set_sdk_params(sdvr_sdk_params_t *sdk_params);

EXTERN void sdvr_get_sdk_version(sx_uint8 *major,
                                 sx_uint8 *minor,
                                 sx_uint8 *revision,
                                 sx_uint8 *build);
EXTERN sdvr_err_e sdvr_get_driver_version(sx_uint32 board_index,
                                          sx_uint8 *major,
                                          sx_uint8 *minor,
                                          sx_uint8 *revision,
                                          sx_uint8 *build);
EXTERN sdvr_err_e sdvr_get_firmware_version(sx_uint32 board_index,
                                            sdvr_firmware_ver_t *version_info);

EXTERN sdvr_sensor_callback sdvr_set_sensor_callback(sdvr_sensor_callback sensor_callback);
EXTERN sdvr_sensor_64_callback sdvr_set_sensor_64_callback(sdvr_sensor_64_callback sensor_callback);
EXTERN sdvr_video_alarm_callback sdvr_set_video_alarm_callback(sdvr_video_alarm_callback video_alarm_callback);
EXTERN sdvr_stream_callback sdvr_set_stream_callback(sdvr_stream_callback stream_callback);
EXTERN sdvr_av_frame_callback sdvr_set_av_frame_callback(sdvr_av_frame_callback av_frame_callback);
EXTERN sdvr_display_debug_callback sdvr_set_display_debug(sdvr_display_debug_callback display_debug_callback);
EXTERN sdvr_signals_callback sdvr_set_signals_callback(sdvr_signals_callback signals_callback);
EXTERN sdvr_send_confirmation_callback sdvr_set_confirmation_callback(sdvr_send_confirmation_callback conf_callback);

EXTERN sdvr_err_e sdvr_get_board_config(sx_uint32 board_index,
                                        sdvr_board_config_t *board_config);
EXTERN sdvr_err_e sdvr_get_supported_vstd(sx_uint32 board_index,
                                          sx_uint32 *vstd);
EXTERN sdvr_err_e sdvr_get_video_standard(sx_uint32 board_index,
                                          sdvr_video_std_e *video_std_type);
EXTERN sdvr_err_e sdvr_set_watchdog_state_ex(sx_uint32 board_index,
                                             sdvr_watchdog_control_e enable,
                                             sx_uint32 msec);
EXTERN sdvr_err_e sdvr_get_watchdog_state(sx_uint32 board_index,
                                          sdvr_watchdog_control_e *enable);
EXTERN sx_uint8 sdvr_set_frame_rate(sdvr_frame_rate_skip_method_e skip_method, sx_uint8 count);
EXTERN sdvr_err_e sdvr_set_date_time(sx_uint32 board_index,
                                     time_t unix_time);
EXTERN sdvr_err_e sdvr_get_date_time(sx_uint32 board_index,
                                     time_t *unix_time);
EXTERN sdvr_err_e sdvr_run_diagnostics(sx_uint32 board_index,
                                       char *diag_file_name,
                                       sdvr_diag_code_e *diag_code,
                                       int *diag_code_size);
EXTERN sx_uint8 sdvr_get_board_index(sdvr_chan_handle_t handle);
EXTERN sx_uint8 sdvr_get_chan_num(sdvr_chan_handle_t handle);
EXTERN sdvr_chan_type_e sdvr_get_chan_type(sdvr_chan_handle_t handle);

EXTERN sdvr_err_e sdvr_enable_temperature_signal(sx_uint8 board_index, sx_uint8 interval, sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_disable_temperature_signal(sx_uint8 board_index);
EXTERN sdvr_err_e sdvr_measure_temperature(sx_uint8 board_index, float *temp);

EXTERN sdvr_err_e sdvr_write_ioctl(sdvr_chan_handle_t handle,
                                   sx_uint8  device_id,
                                   sx_uint8  reg_num,
                                   sx_uint16 value);
EXTERN sdvr_err_e sdvr_read_ioctl(sdvr_chan_handle_t handle,
                                  sx_uint8  device_id,
                                  sx_uint8  reg_num,
                                  sx_uint16 *value);

EXTERN sdvr_err_e sdvr_enable_memory_usage_signal(sx_uint8 board_index,
                                                  sx_uint8 interval,
                                                  sx_uint8 pe_id,
                                                  sx_uint16 threshold);
EXTERN sdvr_err_e sdvr_disable_memory_usage_signal(sx_uint8 board_index, sx_uint8 pe_id);
EXTERN sdvr_err_e sdvr_reset_signal(sx_uint8 board_index,
                                    __sdvr_signals_type_e signal_id,
                                    sx_uint8 interval);

/****************************************************************************
  GROUP: MAC Address API
****************************************************************************/
EXTERN sdvr_err_e sdvr_get_macaddr(sx_uint8 board_index, sx_uint8 *macaddr);
EXTERN sdvr_err_e sdvr_set_macaddr(sx_uint8 board_index, sx_uint8 *macaddr);

/****************************************************************************
  GROUP: INTERNAL
****************************************************************************/
EXTERN sdvr_err_e sdvr_set_macaddr_copy_protect(sx_uint8 board_index, sx_bool enable);

/****************************************************************************
  GROUP: Test Frames API
****************************************************************************/
EXTERN sdvr_err_e sdvr_set_test_frames(sx_bool enable);

/****************************************************************************
  GROUP: EEPROM API
****************************************************************************/
EXTERN sdvr_err_e sdvr_eeprom_blow_fuse(sx_uint8 board_index, sdvr_crypt_eeprom_fuse_e fuse);
EXTERN sdvr_err_e sdvr_eeprom_get_security(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                           sdvr_eeprom_security_e *security);
EXTERN sdvr_err_e sdvr_eeprom_read(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                   sx_uint8 zone_addr,
                                   sx_uint8 *num_bytes,
                                   sx_uint8 *data);
EXTERN sdvr_err_e sdvr_eeprom_set_password(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                           sdvr_eeprom_password_type_e password_type,
                                           sx_uint8 *password);
EXTERN sdvr_err_e sdvr_eeprom_set_security(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                           sdvr_eeprom_security_e security);
EXTERN sdvr_err_e sdvr_eeprom_verify_password(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                              sdvr_eeprom_password_type_e password_type,
                                              sx_uint8 *password);
EXTERN sdvr_err_e sdvr_eeprom_write(sx_uint8 board_index, sdvr_crypto_eeprom_zones_e zone,
                                    sx_uint8 zone_addr,
                                    sx_uint8 num_bytes,
                                    sx_uint8 *data);
/****************************************************************************
  GROUP: Channel Set Up API
****************************************************************************/
EXTERN sdvr_err_e sdvr_create_chan(sdvr_chan_def_t *chan_def,
                                   sdvr_chan_handle_t *handle_ptr);
EXTERN sdvr_err_e sdvr_create_chan_ex(sdvr_chan_def_t *chan_def,
                                      sdvr_chan_buf_def_t *buf_def,
                                      sdvr_chan_handle_t *handle_ptr);
EXTERN sdvr_err_e sdvr_set_chan_codec_pe(sx_uint8 board_index, sx_uint8 chan_num,
                                         sx_uint8 chan_type, sdvr_codec_action_e codec_action,
                                         sx_int8 pe_num);
EXTERN sdvr_err_e sdvr_set_channel_default(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_destroy_chan(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_set_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 user_data);
EXTERN sdvr_err_e sdvr_get_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 *user_data);
EXTERN sdvr_err_e sdvr_set_chan_video_codec(sdvr_chan_handle_t handle,
                                            sx_uint32 stream_id,
                                            sdvr_venc_e video_codec);
EXTERN sdvr_err_e sdvr_get_chan_vstd(sdvr_chan_handle_t handle,
                                     sdvr_video_std_e *video_std_type);

EXTERN sdvr_err_e sdvr_set_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sx_uint32 enc_stream_id,
                                                        sdvr_video_enc_chan_params_t *video_enc_params);
EXTERN sdvr_err_e sdvr_get_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sx_int32 enc_stream_id,
                                                        sdvr_video_enc_chan_params_t *video_enc_params);
EXTERN sdvr_err_e sdvr_set_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                                      sx_uint32 enc_stream_id,
                                                      sdvr_alarm_video_enc_params_t *alarm_video_enc_params);
EXTERN sdvr_err_e sdvr_get_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                                      sx_int32 enc_stream_id,
                                                      sdvr_alarm_video_enc_params_t *alarm_video_enc_params);


EXTERN sdvr_err_e sdvr_set_audio_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sdvr_audio_enc_chan_params_t *audio_enc_params);
EXTERN sdvr_err_e sdvr_get_audio_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sdvr_audio_enc_chan_params_t *audio_enc_params);
/****************************************************************************
  GROUP: Video Analytics and Privacy Blocking API
****************************************************************************/

EXTERN sdvr_err_e sdvr_set_regions_map(sdvr_chan_handle_t handle,
                                       sdvr_regions_type_e region_type,
                                       sx_uint8 *regions_map);
EXTERN sdvr_err_e sdvr_set_regions(sdvr_chan_handle_t handle,
                                   sdvr_regions_type_e region_type,
                                   sx_uint8 overlay_num,
                                   sdvr_mb_region_t *regions_list,
                                   sx_uint8 num_of_regions);
EXTERN sdvr_err_e sdvr_motion_value_analyzer(
   sx_uint8 *motion_values,
   sdvr_video_std_e motion_values_vstd,
   sdvr_mb_region_t *regions_list,
   sx_uint8 num_of_regions,
   sx_uint32 threshold);
EXTERN sdvr_err_e sdvr_set_motion_value_frequency(sdvr_chan_handle_t handle, sx_uint8 every_n_frames);
EXTERN sdvr_err_e sdvr_get_alarm_motion_value(sx_uint32 data, sx_uint8 overlay_num, sx_uint8 *motion_value);

EXTERN sdvr_err_e sdvr_add_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                  sdvr_region_t *region);
EXTERN sdvr_err_e sdvr_change_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sdvr_region_t *region);
EXTERN sdvr_err_e sdvr_remove_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sx_uint8 region_id);

EXTERN sdvr_err_e sdvr_get_motion_detection(sdvr_chan_handle_t handle,
                                            sdvr_motion_detection_t *motion_detection);

EXTERN sdvr_err_e sdvr_get_blind_detection(sdvr_chan_handle_t handle,
                                           sdvr_blind_detection_t *blind_detection);
EXTERN sdvr_err_e sdvr_get_privacy_regions(sdvr_chan_handle_t handle,
                                           sdvr_privacy_region_t *privacy_regions);
EXTERN sdvr_err_e sdvr_get_night_detection(sdvr_chan_handle_t handle,
                                           sdvr_night_detection_t *night_detection);

EXTERN sdvr_err_e sdvr_enable_privacy_regions(sdvr_chan_handle_t handle,
                                              sx_bool enable);
EXTERN sdvr_err_e sdvr_enable_motion_detection(sdvr_chan_handle_t handle,
                                               sx_bool enable,
                                               sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_motion_detection_ex(sdvr_chan_handle_t handle,
                                                  sx_bool enable,
                                                  sx_uint8 threshold1,
                                                  sx_uint8 threshold2,
                                                  sx_uint8 threshold3,
                                                  sx_uint8 threshold4);

EXTERN sdvr_err_e sdvr_enable_blind_detection(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_night_detection(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_analytics(sdvr_chan_handle_t handle,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_set_va_data(sdvr_chan_handle_t handle, sx_uint8 *data);
EXTERN sdvr_err_e sdvr_get_va_data(sdvr_chan_handle_t handle, sx_uint8 *data);

/****************************************************************************
  GROUP: Encoding and Raw Audio/Video API
****************************************************************************/
EXTERN sdvr_err_e sdvr_enable_auth_key(sx_bool enable);
EXTERN sdvr_err_e sdvr_enable_encoder(sdvr_chan_handle_t handle,
                                      sx_uint32 enc_stream_id,
                                      sx_bool enable);

EXTERN sdvr_err_e sdvr_force_key_vframe(sdvr_chan_handle_t handle,
                                        sx_int32 enc_stream_id);

EXTERN sdvr_err_e sdvr_get_av_buffer(sdvr_chan_handle_t handle,
                                     sdvr_frame_type_e frame_type,
                                     sdvr_av_buffer_t **frame_buffer);
EXTERN sdvr_err_e sdvr_get_stream_buffer(sdvr_chan_handle_t handle,
                                         sdvr_frame_type_e frame_type,
                                         sx_uint32 stream_id,
                                         sdvr_av_buffer_t **frame_buffer);

EXTERN sdvr_err_e sdvr_get_yuv_buffer(sdvr_chan_handle_t handle,
                                      sdvr_yuv_buffer_t **frame_buffer);

EXTERN sdvr_err_e sdvr_get_host_encode_input_buffer(sdvr_chan_handle_t handle,
                                                    sdvr_av_buffer_t **frame_buffer,
                                                    sx_uint32 wait_time);

EXTERN sdvr_err_e sdvr_release_av_buffer(sdvr_av_buffer_t *frame_buffer);
EXTERN sdvr_err_e sdvr_release_yuv_buffer(sdvr_yuv_buffer_t *frame_buffer);


EXTERN sdvr_err_e sdvr_set_encoder_mode(sx_int32 board_index,
                                        sdvr_encoder_mode_e  modes);
EXTERN sdvr_err_e sdvr_set_encoder_skip_frame(sdvr_chan_handle_t handle,
                                              sx_uint32 enc_stream_id,
                                              sx_uint8 enable);

/****************************************************************************
  GROUP: Frame Buffer Field Access API
****************************************************************************/
EXTERN sdvr_chan_handle_t sdvr_get_buffer_channel(void *frame_buffer);
EXTERN sdvr_err_e sdvr_get_buffer_timestamp(void *frame_buffer, sx_uint64 *timestamp64);
EXTERN sdvr_err_e sdvr_set_buffer_timestamp(sdvr_av_buffer_t *frame_buffer, sx_uint64 timestamp64);
EXTERN sdvr_err_e sdvr_get_buffer_frame_type(void *frame_buffer, sx_uint8 *frame_type);
EXTERN sdvr_err_e sdvr_set_buffer_frame_type(void *frame_buffer, sx_uint8 frame_type);
EXTERN sdvr_err_e sdvr_get_buffer_test_frame(void *frame_buffer, sx_bool *test_frame);
EXTERN sdvr_err_e sdvr_get_buffer_test_pattern(void *frame_buffer, sx_uint8 *test_pattern);
EXTERN sdvr_err_e sdvr_get_buffer_yuv_format(sdvr_av_buffer_t *frame_buffer, sx_uint8 *format);
EXTERN sdvr_err_e sdvr_get_buffer_sub_encoder(sdvr_av_buffer_t *frame_buffer, sx_uint32 *enc_stream_id);
EXTERN sdvr_err_e sdvr_get_buffer_alarm_value(void *frame_buffer,
                                              sdvr_video_alarm_e  alarm_type,
                                              sx_uint8 overlay_num,
                                              sx_uint8 *alarm_value);
EXTERN sdvr_err_e  sdvr_av_buf_video_dimensions(void *frame_buffer,
                                                sx_uint16 *width,
                                                sx_uint16 *lines);
EXTERN sdvr_err_e  sdvr_av_buf_sequence(void *frame_buffer,
                                        sx_uint32 *seq_number,
                                        sx_uint32 *frame_number,
                                        sx_uint32 *frame_drop_count);
EXTERN sdvr_err_e  sdvr_av_buf_payload(sdvr_av_buffer_t *frame_buffer,
                                       sx_uint8 **payload,
                                       sx_uint32 *payload_size);
EXTERN sdvr_err_e  sdvr_av_buf_yuv(sdvr_av_buffer_t *frame_buffer,
                                   sx_uint8 **y_data,
                                   sx_uint8 **u_data,
                                   sx_uint8 **v_data,
                                   sx_uint32 *y_data_size,
                                   sx_uint32 *u_data_size,
                                   sx_uint32 *v_data_size);
EXTERN sdvr_err_e  sdvr_av_buf_yuv_payload(sdvr_yuv_buffer_t *yuv_buffer,
                                           sx_uint8 **y_data,
                                           sx_uint8 **u_data,
                                           sx_uint8 **v_data,
                                           sx_uint32 *y_data_size,
                                           sx_uint32 *u_data_size,
                                           sx_uint32 *v_data_size);
EXTERN sdvr_err_e  sdvr_get_buffer_yuv_payloads(sdvr_av_buffer_t *frame_buffer,
                                                sx_uint8 **y_data,
                                                sx_uint8 **u_data,
                                                sx_uint8 **v_data);
EXTERN sdvr_err_e  sdvr_set_buffer_yuv_data_size(sdvr_av_buffer_t *frame_buffer,
                                                 sx_uint32 y_data_size,
                                                 sx_uint32 u_data_size,
                                                 sx_uint32 v_data_size);
EXTERN sdvr_err_e  sdvr_set_buffer_payload_size(sdvr_av_buffer_t *frame_buffer,
                                                sx_uint32 payload_size);
EXTERN sx_uint8* sdvr_get_buffer_payload_ptr(sdvr_av_buffer_t *frame_buffer);
EXTERN sx_int32   sdvr_av_buf_packet_count(sdvr_av_buffer_t *frame_buffer);
EXTERN sdvr_err_e  sdvr_av_buf_packet(sdvr_av_buffer_t *frame_buffer,
                                      sx_int32 packet_num,
                                      sx_uint8 **packet_data,
                                      sx_uint32 *packet_size);


/****************************************************************************
  GROUP: Decoding API
****************************************************************************/
EXTERN sdvr_err_e sdvr_enable_decoder(sdvr_chan_handle_t handle,
                                      sx_bool enable);
EXTERN sdvr_err_e sdvr_set_decoder_size(sdvr_chan_handle_t handle,
                                        sx_uint16 width, sx_uint16 height);
EXTERN sdvr_err_e sdvr_disable_decoder_pipeline(sdvr_chan_handle_t handle,
                                                sx_bool disable);
EXTERN sdvr_err_e sdvr_set_decoder_audio_mode(sdvr_chan_handle_t handle,
                                              sdvr_enc_audio_mode_e enc_audio_mode);
EXTERN sdvr_err_e sdvr_set_decoder_mode(sdvr_chan_handle_t handle,
                                        sdvr_h264_decoder_mode_e mode);

EXTERN sdvr_err_e sdvr_alloc_av_buffer_wait(sdvr_chan_handle_t handle,
                                            sdvr_av_buffer_t **frame_buffer,
                                            sx_uint32 timeout);

EXTERN sdvr_err_e sdvr_alloc_av_buffer(sdvr_chan_handle_t handle,
                                       sdvr_av_buffer_t **frame_buffer);

EXTERN sdvr_err_e sdvr_send_av_frame(sdvr_av_buffer_t *frame_buffer);
/****************************************************************************
  GROUP: Display and Sound API
****************************************************************************/
EXTERN sdvr_err_e sdvr_stream_raw_video(sdvr_chan_handle_t handle,
                                        sdvr_video_res_decimation_e res_decimation,
                                        sx_uint8 frame_rate,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_stream_raw_video_secondary(sdvr_chan_handle_t handle,
                                                  sdvr_video_res_decimation_e res_decimation,
                                                  sx_uint8 frame_rate,
                                                  sx_bool enable);
EXTERN sdvr_err_e sdvr_stream_raw_audio(sdvr_chan_handle_t handle,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_get_video_in_params(sdvr_chan_handle_t handle,
                                           sdvr_image_ctrl_t *image_ctrl);
EXTERN sdvr_err_e sdvr_set_video_in_params(sdvr_chan_handle_t handle,
                                           sx_uint16 image_ctrl_flag,
                                           sdvr_image_ctrl_t *image_ctrl);
EXTERN sdvr_err_e sdvr_set_yuv_format(sx_uint32 board_index, sx_uint8 format);

EXTERN sdvr_err_e sdvr_enable_deinterlacing(sdvr_chan_handle_t handle,
                                            sx_bool enable);

EXTERN sdvr_err_e sdvr_enable_pixel_adaptive_deinterlacing(sdvr_chan_handle_t handle, 
                                                            sx_bool enable);

EXTERN sdvr_err_e sdvr_enable_motion_based_deinterlacing(sdvr_chan_handle_t handle,
                                                         sx_bool enable,
                                                         sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_median_filter(sdvr_chan_handle_t handle,
                                            sx_bool enable);
EXTERN sdvr_err_e sdvr_enable_noise_reduction(sdvr_chan_handle_t handle,
                                              sx_bool enable);
EXTERN sdvr_err_e sdvr_set_noise_reduction_strength(sdvr_chan_handle_t handle,
                                              sx_uint8 luma_strength,
                                              sx_uint8 chroma_strength);
EXTERN sdvr_err_e sdvr_get_noise_reduction_strength(sdvr_chan_handle_t handle,
                                              sx_uint8 *luma_strength,
                                              sx_uint8 *chroma_strength);
EXTERN sdvr_err_e sdvr_set_gain_mode(sdvr_chan_handle_t handle,
                                     sx_uint8 value);
EXTERN sdvr_err_e sdvr_set_camera_termination(sdvr_chan_handle_t handle,
                                              sdvr_term_e term);
EXTERN sdvr_err_e sdvr_enable_audio_out(sdvr_chan_handle_t  handle,
                                        sx_uint8 aout_port_num,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_start_audio_out(sx_uint32 board_index,
                                       sx_uint8 aout_port_num,
                                       sdvr_chan_handle_t *handle);
EXTERN sdvr_err_e sdvr_stop_audio_out(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_enable_ir_cut_filter(sdvr_chan_handle_t handle,
                                            sx_bool enable);
EXTERN sdvr_err_e sdvr_snapshot(sdvr_chan_handle_t handle,
                                sdvr_video_res_decimation_e resolution);

EXTERN sdvr_err_e sdvr_snapshot_ex(sdvr_chan_handle_t handle,
                                    sdvr_video_res_decimation_e resolution,
                                    sx_uint16  width,
                                    sx_uint16  height);


EXTERN sdvr_err_e sdvr_set_frame_skip_params(sdvr_chan_handle_t handle,
                                             sx_uint32 frame_skip_count,
                                             sx_uint32 frame_skip_sample_size);

/****************************************************************************
  GROUP: On-Screen Display API
****************************************************************************/
EXTERN sdvr_err_e sdvr_osd_text_config_ex(sdvr_chan_handle_t handle,
                                          sx_uint8 osd_id,
                                          sdvr_osd_config_ex_t *osd_text_config);
EXTERN sdvr_err_e sdvr_osd_set_text(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                                    sx_uint8    text_len, sx_uint16   *text);
EXTERN sdvr_err_e sdvr_osd_text_show(sdvr_chan_handle_t handle,
                                     sx_uint8 osd_id,
                                     sx_bool show);
EXTERN sdvr_err_e sdvr_osd_text_show_all(sdvr_chan_handle_t handle,
                                         sx_bool show);
EXTERN sdvr_err_e sdvr_osd_blinking_on(sdvr_chan_handle_t handle,
                                       sx_uint8 osd_id,
                                       sx_uint8 on_frame_counts,
                                       sx_uint8 off_frame_counts,
                                       sx_uint32 flags);
EXTERN sdvr_err_e sdvr_osd_blinking_off(sdvr_chan_handle_t handle,
                                        sx_uint8 osd_id);

EXTERN sdvr_err_e sdvr_osd_set_font_table(sdvr_font_table_t *font_desc);
EXTERN sdvr_err_e sdvr_osd_use_font_table(sx_uint8 font_id);

/****************************************************************************
  GROUP: Flexible On-Screen Display API
****************************************************************************/
EXTERN sdvr_err_e sdvr_fosd_get_cap(const sdvr_chan_handle_t handle,
                            sdvr_fosd_msg_cap_t *fosd_cap);

EXTERN sdvr_err_e sdvr_fosd_spec(const sdvr_chan_handle_t handle,
                            const sdvr_fosd_msg_cap_t *fosd_spec);

EXTERN sdvr_err_e sdvr_fosd_config(const sdvr_chan_handle_t handle,
                            const sx_uint16           osd_id,
                            const sdvr_fosd_config_t *fosd_config);

EXTERN sdvr_err_e sdvr_fosd_set_text(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id,
                            const sx_uint8   font_table_id,
                            const sx_uint8   text_len,
                            const sx_uint16 *text);

EXTERN sdvr_err_e sdvr_fosd_set_graphic(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id,
                            const sx_uint16  width,
                            const sx_uint16  height,
                            const sx_uint8  *bitmap);

EXTERN sdvr_err_e sdvr_fosd_clear_text(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id);

EXTERN sdvr_err_e sdvr_fosd_show(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id,
                            const sx_bool    show);

EXTERN sdvr_err_e sdvr_fosd_show_all(const sdvr_chan_handle_t handle,
                            const sx_bool    show);

EXTERN sdvr_err_e sdvr_fosd_blinking_on(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id,
                            const sx_uint8   on_frames,
                            const sx_uint8   off_frames,
                            const sx_uint32  flags);

EXTERN sdvr_err_e sdvr_fosd_blinking_off(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id);

EXTERN sdvr_err_e sdvr_fosd_set_color(const sdvr_chan_handle_t handle,
                            const sx_uint16  osd_id,
                            const sx_uint8   color_y,
                            const sx_uint8   color_u,
                            const sx_uint8   color_v,
                            const sx_uint8   color_y_high,
                            const sx_uint8   color_u_high,
                            const sx_uint8   color_v_high,
                            const sx_uint8   color_y_low,
                            const sx_uint8   color_u_low,
                            const sx_uint8   color_v_low);

EXTERN sdvr_err_e sdvr_fosd_set_position(const sdvr_chan_handle_t handle,
                            const sx_uint16         osd_id,
                            const sdvr_location_e   position_ctrl,
                            const sx_uint16         pos_x,
                            const sx_uint16         pos_y);

/****************************************************************************
  GROUP: Spot Monitor Output API
****************************************************************************/
EXTERN sdvr_err_e sdvr_smo_set_disable_mode(sx_uint32 board_index,
                                            sx_uint8 port_num,
                                            sdvr_smo_disable_mode_e mode);
EXTERN sdvr_err_e sdvr_set_smo_video_vstd(sx_uint32 board_index,
                                          sx_uint8 port_num,
                                          sdvr_video_std_e video_std_type);
EXTERN sdvr_err_e sdvr_get_smo_attributes(sx_uint32 board_index,
                                          sx_uint8 port_num,
                                          sdvr_smo_attribute_t *smo_attrib);

EXTERN sdvr_err_e sdvr_set_smo_bkg_color(sx_uint32 board_index,
                                         sx_uint8 port_num,
                                         sx_uint8 y_color, sx_uint8 u_color,
                                         sx_uint8 v_color);

EXTERN sdvr_err_e sdvr_set_smo_grid_ex(sdvr_chan_handle_t  handle,
                                       sx_uint8 port_num,
                                       sdvr_smo_grid_t *smo_grid);
EXTERN sdvr_err_e sdvr_smo_set_grid(sdvr_chan_handle_t  handle,
                                    sx_uint8 instance_id,
                                    sx_uint8 port_num,
                                    const sdvr_smo_grid_t *smo_grid);

EXTERN sdvr_err_e sdvr_get_smo_grid_ex(sdvr_chan_handle_t  handle,
                                       sx_uint8 port_num,
                                       sdvr_smo_grid_t *smo_grid);
EXTERN sdvr_err_e sdvr_smo_get_grid(sdvr_chan_handle_t  handle,
                                    sx_uint8 instance_id,
                                    sx_uint8 port_num,
                                    sdvr_smo_grid_t *smo_grid);

EXTERN sdvr_err_e sdvr_smo_flush_queue(sdvr_chan_handle_t handle,
                                       sx_uint8 port_num);
EXTERN sdvr_err_e sdvr_smo_zoom_grid(sdvr_chan_handle_t  handle,
                                     sx_uint8   instance_id,
                                     sx_uint8   smo_port_num,
                                     sx_uint16  top_left_x,
                                     sx_uint16  top_left_y,
                                     sx_uint16  width,
                                     sx_uint16  height);
EXTERN sdvr_err_e sdvr_zoom_smo_grid(sdvr_chan_handle_t  handle,
                                     sx_uint8   smo_port_num,
                                     sx_uint16  top_left_x,
                                     sx_uint16  top_left_y,
                                     sx_uint16  width,
                                     sx_uint16  height);
EXTERN sdvr_err_e sdvr_smo_pause_grid(sdvr_chan_handle_t handle,
                                      sx_uint8 instance_id,
                                      sx_uint8 smo_port_num,
                                      sx_bool pause);
EXTERN sdvr_err_e sdvr_pause_smo_grid(sdvr_chan_handle_t handle,
                                      sx_uint8 smo_port_num,
                                      sx_bool pause);
EXTERN sdvr_err_e sdvr_smo_grid_mirror_image(sdvr_chan_handle_t handle,
                                             sx_uint8           instance_id,
                                             sx_uint8           port_num);
EXTERN sdvr_err_e sdvr_start_video_overlay(sx_uint32 board_index,
                                           sx_uint8 port_num,
                                           sdvr_rawv_formats_e video_format,
                                           sdvr_chan_handle_t *handle);
EXTERN sdvr_err_e sdvr_stop_video_overlay(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_smo_add_overlay(sdvr_chan_handle_t handle,
                                       sx_uint16 width, sx_uint16 height,
                                       sx_uint8 layer_order,
                                       sx_uint8 *overlay_id);
EXTERN sdvr_err_e sdvr_smo_add_overlay_ex(sdvr_chan_handle_t handle,
                                          sx_uint16 width, sx_uint16 height,
                                          sx_uint8 layer_order,
                                          sx_uint8 *overlay_id,
                                          void **vmem);
EXTERN sdvr_err_e sdvr_smo_add_direct_mem_overlay(sdvr_chan_handle_t handle,
                                                  sdvr_overlay_t *overlay_info,
                                                  void **vmem);
EXTERN sdvr_err_e sdvr_smo_update_overlay(sdvr_chan_handle_t handle, sx_uint8 overlay_id);
EXTERN sdvr_err_e sdvr_smo_duplicate_overlay(sdvr_chan_handle_t handle,
                                             sx_uint8 layer_order,
                                             sx_uint8 dup_overlay_id,
                                             sx_uint8 *overlay_id);
EXTERN sdvr_err_e sdvr_smo_delete_overlay(sdvr_chan_handle_t handle,
                                          sx_uint8 overlay_id);
EXTERN sdvr_err_e sdvr_smo_show_overlay(sdvr_chan_handle_t handle,
                                        sx_uint8 overlay_id, sx_uint8 alpha_value);
EXTERN sdvr_err_e sdvr_smo_set_overlay_pos(sdvr_chan_handle_t handle,
                                           sx_uint8 overlay_id,
                                           sx_int16 top_left_x, sx_int16 top_left_y);
EXTERN sdvr_err_e sdvr_smo_clear_overlay(sdvr_chan_handle_t handle,
                                         sx_uint8 overlay_id);
EXTERN sdvr_err_e sdvr_smo_set_overlay_from_file(sdvr_chan_handle_t handle,
                                                 sx_uint8 overlay_id, sx_uint8 file_type,
                                                 char *file_name,
                                                 sx_int16 top_left_x, sx_int16 top_left_y,
                                                 sx_uint16 width, sx_uint16 height);
EXTERN sdvr_err_e sdvr_get_video_overlay_buffer(sdvr_chan_handle_t handle,
                                                sdvr_overlay_frame_attrib_t *frame_attrib,
                                                sdvr_av_buffer_t **frame_buffer,
                                                sx_uint32 wait_time);
EXTERN sdvr_err_e sdvr_send_video_overlay(sdvr_chan_handle_t handle,
                                          sdvr_av_buffer_t *buf);
EXTERN sdvr_err_e sdvr_freeze_smo_output(sx_uint32 board_index, sx_uint8 port_num, sx_bool freeze);

EXTERN sdvr_err_e sdvr_smo_osd_text_config(sx_uint32 board_index,
                                           sx_uint8 smo_port,
                                           sx_uint8 osd_id,
                                           sdvr_osd_config_ex_t *osd_text_config);
EXTERN sdvr_err_e sdvr_smo_osd_text_show(sx_uint32 board_index,
                                         sx_uint8 smo_port,
                                         sx_uint8 osd_id,
                                         sx_bool show);
EXTERN sdvr_err_e sdvr_smo_osd_text_show_all(sx_uint32 board_index,
                                             sx_uint8 smo_port,
                                             sx_bool show);

EXTERN sdvr_err_e sdvr_open_hmo_mirror(sx_uint32 board_index, sx_uint32 smo_port);
EXTERN sdvr_err_e sdvr_close_hmo_mirror();

/****************************************************************************
  GROUP: Encode Monitor Output API
****************************************************************************/
EXTERN sdvr_err_e sdvr_create_emo(sx_uint32 board_index,
                                  sdvr_chan_handle_t *emo_handle);
EXTERN sdvr_err_e sdvr_create_emo_port(sx_uint32 board_index,
                                       sx_uint32 smo_port,
                                       sdvr_chan_handle_t *emo_handle);

EXTERN sdvr_err_e sdvr_get_emo_attributes(sdvr_chan_handle_t emo_handle,
                                          sdvr_emo_attribute_t *emo_attrib);

EXTERN sdvr_err_e sdvr_set_emo_video_enc_params(sdvr_chan_handle_t emo_handle,
                                                sdvr_emo_video_enc_params_t *enc_params);

EXTERN sdvr_err_e sdvr_set_emo_grid(sdvr_chan_handle_t emo_handle,
                                    sdvr_chan_handle_t  handle,
                                    sdvr_emo_grid_t *emo_grid);

EXTERN sdvr_err_e sdvr_destroy_emo(sdvr_chan_handle_t emo_handle);

/****************************************************************************
  GROUP: Image Sensor Pipeline API for IP Camera
****************************************************************************/

EXTERN sdvr_err_e sdvr_set_gamma_value(sdvr_chan_handle_t handle, float  value);
EXTERN sdvr_err_e sdvr_get_gamma_value(sdvr_chan_handle_t handle, float *value);

#if 0
EXTERN sdvr_err_e sdvr_get_exposure_properties(sdvr_chan_handle_t handle, sdvr_exposure_t *exposure);
EXTERN sdvr_err_e sdvr_set_exposure_properties(sdvr_chan_handle_t handle, sdvr_exposure_t *exposure);
#endif

EXTERN sdvr_err_e sdvr_get_ircut_properties(sdvr_chan_handle_t handle, sdvr_ircut_t *ircut);
EXTERN sdvr_err_e sdvr_set_ircut_properties(sdvr_chan_handle_t handle, sdvr_ircut_t *ircut);

EXTERN sdvr_err_e sdvr_get_shutter_properties (sdvr_chan_handle_t handle, sdvr_shutter_t *shutter);
EXTERN sdvr_err_e sdvr_set_shutter_properties (sdvr_chan_handle_t handle, sdvr_shutter_t *shutter);
EXTERN sdvr_err_e sdvr_get_iris_properties (sdvr_chan_handle_t handle, sdvr_iris_t *iris)         ;
EXTERN sdvr_err_e sdvr_set_iris_properties (sdvr_chan_handle_t handle, sdvr_iris_t *iris)         ;
EXTERN sdvr_err_e sdvr_get_gain_properties (sdvr_chan_handle_t handle, sdvr_gain_t *gain)         ;
EXTERN sdvr_err_e sdvr_set_gain_properties (sdvr_chan_handle_t handle, sdvr_gain_t *gain)         ;
EXTERN sdvr_err_e sdvr_get_exposure_properties(sdvr_chan_handle_t handle, sdvr_exposure_t *exposure)   ;
EXTERN sdvr_err_e sdvr_set_exposure_properties(sdvr_chan_handle_t handle, sdvr_exposure_t *exposure)   ;

EXTERN sdvr_err_e sdvr_get_focus_properties(sdvr_chan_handle_t handle, sdvr_focus_t *focus);
EXTERN sdvr_err_e sdvr_set_focus_properties(sdvr_chan_handle_t handle, sdvr_focus_t *focus);

EXTERN sdvr_err_e sdvr_get_wdr_properties(sdvr_chan_handle_t handle, sdvr_wdr_t *wdr);
EXTERN sdvr_err_e sdvr_set_wdr_properties(sdvr_chan_handle_t handle, sdvr_wdr_t *wdr);

EXTERN sdvr_err_e sdvr_get_wb_properties(sdvr_chan_handle_t handle, sdvr_wb_t *wb);
EXTERN sdvr_err_e sdvr_set_wb_properties(sdvr_chan_handle_t handle, sdvr_wb_t *wb);

EXTERN sdvr_err_e sdvr_get_img_properties(sdvr_chan_handle_t handle, sdvr_img_t *img);
EXTERN sdvr_err_e sdvr_set_img_properties(sdvr_chan_handle_t handle, sdvr_img_t *img);

EXTERN sdvr_err_e sdvr_enable_motion_map (sdvr_chan_handle_t handle, sx_bool enable);

EXTERN sdvr_err_e sdvr_get_flip_properties(sdvr_chan_handle_t handle, sdvr_flip_t *flip);
EXTERN sdvr_err_e sdvr_set_flip_properties(sdvr_chan_handle_t handle, sdvr_flip_t *flip);

/****************************************************************************
  GROUP: Audio/Video File Container API
****************************************************************************/

EXTERN sdvr_err_e sdvr_file_start_recording(sdvr_chan_handle_t handle, sx_uint8 sub_enc,
                                            sdvr_rec_specification_t *rec_spec,
                                            sx_bool  enable_encoder);

EXTERN sdvr_err_e sdvr_file_stop_recording(sdvr_chan_handle_t handle, sx_uint8 sub_enc,
                                           sx_bool  disable_encoder);

EXTERN sdvr_err_e sdvr_file_open_read(sdvr_file_read_desc_t *file_info,
                                      sdvr_file_handle_t *file_handle);
EXTERN sdvr_err_e sdvr_file_open_write(sdvr_rec_specification_t *rec_spec,
                                       sdvr_file_avtrack_info_t *avtrack_info,
                                       sdvr_file_handle_t *file_handle);

EXTERN sdvr_err_e sdvr_file_close(sdvr_file_handle_t file_handle);

EXTERN sdvr_err_e sdvr_file_read_frame(sdvr_file_handle_t file_handle,
                                       sx_uint8 *av_frame_data, sx_uint32 *av_frame_data_size,
                                       sx_uint8 *av_frame_type, sx_uint64 *av_frame_timestamp);

EXTERN sdvr_err_e sdvr_file_write_frame(sdvr_file_handle_t file_handle, sdvr_av_buffer_t *av_buffer);

EXTERN sdvr_err_e sdvr_file_get_avtrack_from_file(char *file_path, sdvr_file_type_e file_type,
                                                  sdvr_file_avtrack_info_t *avtrack_info);
EXTERN sdvr_av_buffer_t* sdvr_file_alloc_av_buffer(sx_uint32 payload_size);
EXTERN sdvr_err_e sdvr_file_free_av_buffer(sdvr_av_buffer_t *av_buffer);

/****************************************************************************
  GROUP: RS485, GPIO, and TWI Communication API
****************************************************************************/
EXTERN sdvr_err_e sdvr_init_uart(sx_uint32 board_index,
                                 sx_uint32 baud_rate,
                                 sx_uint8 data_bits,
                                 sx_uint8 stop_bits,
                                 sx_uint8 parity_enable,
                                 sx_uint8 parity_even);
EXTERN sdvr_err_e sdvr_read_uart(sx_uint32 board_index,
                                 sx_uint8 *data_count_read,
                                 sx_uint8 max_data_size,
                                 sx_uint8 *data);
EXTERN sdvr_err_e sdvr_write_uart(sx_uint32 board_index,
                                  sx_uint8 count,
                                  sx_uint8 *data);

EXTERN sdvr_err_e sdvr_read_gpio(sx_uint32 board_index,
                                 sx_uint8 pe_id,
                                 sx_uint8 pin_num,
                                 sx_uint8 *value,
                                 sx_uint8 *direction);
EXTERN sdvr_err_e sdvr_write_gpio(sx_uint32 board_index,
                                  sx_uint8 pe_id,
                                  sx_uint8 pin_num,
                                  sx_uint8 value,
                                  sx_uint8 direction);

EXTERN sdvr_err_e sdvr_read_twi_regval(sx_uint32 board_index,
                                       sx_uint8 pe_id,
                                       sx_uint8 ra_16bit,
                                       sx_uint16 twi_addr,
                                       sx_uint16 reg_addr,
                                       sx_uint16 *reg_data);
EXTERN sdvr_err_e sdvr_write_twi_regval(sx_uint32 board_index,
                                        sx_uint8 pe_id,
                                        sx_uint8 ra_16bit,
                                        sx_uint16 twi_addr,
                                        sx_uint16 reg_addr,
                                        sx_uint16 reg_data);

/****************************************************************************
  GROUP: Sensors and Relays API
****************************************************************************/
EXTERN sdvr_err_e sdvr_config_sensors(sx_uint32 board_index,
                                      sx_uint32 sensor_enable_map,
                                      sx_uint32 edge_triggered_map);
EXTERN sdvr_err_e sdvr_config_sensors_64(sx_uint32 board_index,
                                         sx_uint64 sensor_enable_map);
EXTERN sdvr_err_e sdvr_enable_sensor(sx_uint32 board_index,
                                     sx_uint32 sensor_num,
                                     sx_bool enable);
EXTERN sdvr_err_e sdvr_get_sensors(sx_uint32 board_index,
                                   sx_uint32 *sensor_enable_map,
                                   sx_uint32 *edge_triggered_map);
EXTERN sdvr_err_e sdvr_get_sensors_64(sx_uint32 board_index,
                                      sx_uint64 *sensor_enable_map);
EXTERN sdvr_err_e sdvr_get_sensors_state(sx_uint32 board_index,
                                         sx_uint32 *sensor_state_map);
EXTERN sdvr_err_e sdvr_get_sensors_state_64(sx_uint32 board_index,
                                            sx_uint64 *sensor_state_map);

EXTERN sdvr_err_e sdvr_get_relays(sx_uint32 board_index,
                                  sx_uint32 *relay_status);
EXTERN sdvr_err_e sdvr_get_relays_64(sx_uint32 board_index,
                                     sx_uint64 *relay_status);
EXTERN sdvr_err_e sdvr_trigger_relay(sx_uint32 board_index,
                                     sx_uint32 relay_num,
                                     sx_bool is_triggered);

EXTERN sdvr_err_e sdvr_set_led(sx_uint32 board_index,
                               sdvr_led_type_e led_type,
                               sx_uint8 led_num,
                               sx_bool enable);
#endif /* STRETCH_SDVR_SDK_H */
