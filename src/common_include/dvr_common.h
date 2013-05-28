/**************************************************************************
Copyright (c) 2008 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/

#ifndef STRETCH_DVR_COMMON_H
#define STRETCH_DVR_COMMON_H

/************************************************************************
    PACKAGE: dvr_common -- Common Definitions Shared Between Board and PC

    DESCRIPTION:

    SECTION: Include
    {
    #include "dvr_common.h"
    }


    SECTION: Introduction

    @dvr_common@ is a collection of common declarations to be shared
    between the Stretch PCIe DVR card software and the Stretch DVR SDK
    running on the host PC.

    SECTION: Basic Communication Method Between the PCIe Card and the Host PC

    The Stretch PCIe card communicates with the host PC via layered APIs.
    Sitting at the bottom level is the basic PCIe device drivers running
    on the board and on the host PC.  It provides basic PCIe hardware
    abstraction.  For details on the PCIe driver, refer to @pci@
    in Stretch SBIOS.

    The next level up is a basic communication layer @sct@.  @sct@
    provides basic communication channels and messages between the
    application code running on the PCIe card and on the host PC.
    Refer to @sct@ for more details about the communication layer API.

    The next level up is the application code.  Specific to the DVR,
    the application code on the PCIe is generally referred to as "DVR
    Firmware" and the application code on the host PC as "DVR SDK".
    Using @sct@, the DVR firmware and DVR SDK exchange all required data
    using agreed-upon @sct@ messages and @sct@ channels.  The purpose
    of this document is to detail the common message types, semantics of
    the message data, and the @sct@ channel creation process.  This file must
    be included by the DVR firmware and DVR SDK to ensure consistency.


    SECTION: Usage Model

    This interface file is designed with a particular usage model in mind.
    Using a PCIe card is defined as creating and configuring various
    jobs running on the board.  The following is a list of basic
    jobs that the board supports:

        Encoding of a real-time camera input.  In this job, the input
        video is encoded and the bit stream is sent to the host.
        Optionally, a decimated raw input image can be sent to the Spot
        Monitor Out (SMO) or the host (HMO) for display.

        Decoding a bit stream supplied by the host.  In this job, the
        bit stream supplied by the host is decoded.  The decoded
        image can be sent to SMO or host for display.

        Encoding of raw images supplied by the host.  In this job, the
        raw images supplied by the host are encoded.  The resulting
        bit stream is sent back to the host.  Optionally, a decimated
        raw input image can be sent to the SMO for display.

    A job can be created, destroyed, configured, enabled, or disabled.
    The following are the rules must be obeyed when managing the jobs:

        A job must be created before it can be configured.

        A job can only be configured or reconfigured when it is disabled.
        (When a job is first created, it is disabled.)

    After a basic job is properly configured, it can be activated
    or deactivated.  These basic jobs are described in more
    detail in the following subsections.

    SUBSECTION: Command and Response Messages

    The host queries board state by sending "GET" messages. Information
    about a specific module is queried by sending a "DVR_GET_xxx" message.
    The board replies with the corresponding "DVR_REP_xxx" message.
    Job settings and state are modified by sending "DVR_SET_xxx" messages.
    The board replies to these using "DVR_REP_xxx" messages as well.
    The GET/SET/REP messages use the same data structure as payload.

    SUBSECTION: Creating, Destroying, and Configuring a Job

    A specific handshake between the host application and the firmware
    must be followed when creating a job. A new job is created by the host
    application initiating the request. The following scenario describes
    the sequence to be followed in setting up a camera encode job.

        1) The host sends a DVR_SET_JOB  message with the control field set to
        DVR_JOB_CREATE. In response, the firmware opens a PCI port connection
        for sending data, initializes the dataport for its camera input, and
        pre-allocates all the necessary resources for the job.

        2) The host calls the @sct_channel_accept@ function to accept the
        data connection from the board.
        (The host DVR application must call the macro DVR_JOB_NUM to get
        the PCI port number.)

        3) The host waits for the DVR_REP_JOB message from the firmware.

    To destroy the job:

        1) The host sends a DVR_SET_JOB message with control field set
        to DVR_JOB_DESTROY. In response, the firmware closes the PCI port
        connection and frees up all resources allocated for this job.

        2) The host calls the @sct_channel_close@ function to shut down
        the data connection to the job.

        3) The host waits for DVR_REP_JOB message from the firmware.

    SUBSECTION: Dynamic Control of Basic Jobs

    One special group of messages are used to activate or deactivate jobs
    on the board.  The messages are @DVR_GET_JOB@, @DVR_SET_JOB@,
    and @DVR_REP_JOB@.

    SECTION: A Convention

    We use the following convention to uniquely associate a message
    with a particular instance of a job:

        A job type, which must be one of
        @DVR_JOB_CAMERA_ENCODE@, @DVR_JOB_HOST_DECODE@,
        or @DVR_JOB_HOST_ENCODE@.

        A job id, which is a 8-bit number.  The IDs do not have
        to start at 0 and do not need to be sequential.

    SECTION: Data Communication Between Firmware and SDK

    All data exchanged between board and host are between specific
    job on the board and SDK on the host.  A job on the board
    is uniquely identified by its type and id.  By combining
    the board id, the job type and the job ID into a single 16 number
    <board_id, job_type, job_id>,
    we have a unique job number (JN).  This JN will be used
    as the port number for exchange all the data between
    the job and the SDK.  The macro @DVR_JOB_NUM@ should
    be used to construct the job number to ensure
    compatability between SDK and firmware.

    The actual data exchanged is either raw video, raw audio,
    or bitstream.  In addition, other ancillary data might
    be needed.  So, the following convention will be used
    to exchange the data.

    Data buffers sent to or received from the board are of
    variable size. When the system is initialized, buffers
    are allocated to handle the maximum possible size. The
    maximum is currently defined by one frame of raw YUV
    data for a D1 PAL image, the buffer size being determined
    by the Y plane size which is the largest component.
    For decimated images, encoded frames, and ancillary data,
    the actual transfer sizes could be a lot smaller.

    All data buffers must begin with a header of type
    @dvr_data_header_t@. This allows identification of the job
    associated with the data, specifies the transfer size, and
    provides other ancillary information. The rest of the data
    follows the header. The total number of bytes transferred is
    equal to the size of the header structure plus the actual
    buffer size.

    SECTION: Asynchronous Events

    The board can generate the following asynchronous events.
    These are all signalled to the host via the @DVR_SIG_HOST@
    message.

        - Sensor activated.

        - Motion detection alarm triggered.

        - Blind detection alarm triggered.

        - Night detection alarm triggered.

        - Video signal from camera lost.

        - Video signal from camera detected.

        - Runtime error in the firmware.

        - Fatal error in the firmware.

        - Board heartbeat. (This is currently not implemented).

    SECTION: Restrictions

    The resolutions at which a video channel (incoming or decoded)
    can be displayed on the spot monitor (SMO) or the host monitor
    (HMO) are restricted to its original resolution, 2x-decimated, or
    4x-decimated resolutions. Enlargement is not supported.
************************************************************************/

#include <sx-types.h>

/************************************************************************
    Defines the convention for mapping a job to a job number
************************************************************************/
#define DVR_JOB_NUM(board_id, job_type, job_id) \
    (((board_id & 0xf) << 12) | ((job_type & 0xf) << 8) | (job_id & 0xff))


/************************************************************************
    Status for all message exchanges. All reply messages must set the
    status field to let the originator know how the message was handled.
    @DVR_STATUS_OK@ means a message was successfully processed.
************************************************************************/
enum dvr_status_enum {
    DVR_STATUS_OK = 0,
    DVR_STATUS_WRONG_CAMERA_NUMBER,
    DVR_STATUS_WRONG_CAMERA_TYPE,
    DVR_STATUS_WRONG_CODEC_FORMAT,
    DVR_STATUS_WRONG_CODEC_RESOLUTION,
    DVR_STATUS_WRONG_JOB_TYPE,
    DVR_STATUS_WRONG_JOB_ID,
    DVR_STATUS_WRONG_VIDEO_FORMAT,
    DVR_STATUS_WRONG_AUDIO_FORMAT,
    DVR_STATUS_EXCEED_CPU_LIMIT,
    DVR_STATUS_JOB_NOT_CREATED,
    DVR_STATUS_JOB_ALREADY_CREATED,
    DVR_STATUS_JOB_NOT_ENABLED,
    DVR_STATUS_JOB_NOT_DISABLED,
    DVR_STATUS_SMO_NOT_CREATED,
    DVR_STATUS_INVALID_TIME,
    DVR_STATUS_ILLEGAL_SMO_PARAMS,
    DVR_STATUS_SMO_NOT_SUPPORTED,
    DVR_STATUS_VDET_ERROR,
    DVR_STATUS_RUNTIME_ERROR,
    DVR_STATUS_VPP_RUNTIME_ERROR,
    DVR_STATUS_ENCODER_RUNTIME_ERROR,
    DVR_STATUS_DECODER_RUNTIME_ERROR,
    DVR_STATUS_ILLEGAL_PARAMETER,
    DVR_STATUS_INTERNAL_ERROR,
    DVR_STATUS_ILLEGAL_COMMAND,
    DVR_STATUS_SMO_NOT_DISABLED,
    DVR_STATUS_OUT_OF_MEMORY,
    DVR_STATUS_NO_IO_BOARD,
    DVR_STATUS_AUDIO_RUNTIME_ERROR,
    DVR_STATUS_UNSUPPORTED_COMMAND,
    DVR_STATUS_SMO_CHAN_FAILED,
    DVR_STATUS_RES_LIMIT_EXCEEDED,
    DVR_STATUS_UNSUPPORTED_VIDEO_RES,
    DVR_STATUS_OUTPUT_BUFFER_OVERRUN,
    DVR_STATUS_WATCHDOG_EXPIRED,
    DVR_STATUS_ASSERT_FAIL,
    DVR_STATUS_AUDIOOUT_NOT_SUPPORTED,
    DVR_STATUS_AUDIOOUT_NOT_DISABLED,
    DVR_STATUS_AUDIOOUT_CHAN_FAILED,
    DVR_STATUS_EMO_NOT_SUPPORTED,
    DVR_STATUS_INVALID_DEFAULT_VSTD,
    DVR_STATUS_REMOTE_ENC_NOT_SUPPORTED,
    DVR_STATUS_SMO_ALREADY_CREATED,
    DVR_STATUS_IPP_NOT_PRESENT,
    DVR_STATUS_NO_CODEC_CHANGE_WHEN_ACTIVE,
    DVR_STATUS_UNSUPPORTED_VIDEO_FORMAT,
    DVR_STATUS_UNSUPPORTED_AUDIO_FORMAT,
    DVR_STATUS_ILLEGAL_OPCODE,
    DVR_STATUS_ILLEGAL_STREAM_ID,
    DVR_STATUS_STREAM_NOT_DISABLED,
    DVR_STATUS_ILLEGAL_PE_ID,
    DVR_STATUS_TWI_ERROR,
    DVR_STATUS_ILLEGAL_GPIO_NUM,
    DVR_STATUS_EMO_NOT_CREATED,
    DVR_STATUS_INVALID_IMAGE_W_H,
    DVR_STATUS_BUFFER_TOO_SMALL,
    DVR_STATUS_INVALID_ENC_AUDIO_MODE,
    DVR_STATUS_BUFFER_TOO_LARGE,
    DVR_STATUS_INVALID_ENC_MODE,
    DVR_STATUS_INVALID_PORT_NUM,
    DVR_STATUS_INVALID_LAYER,
    DVR_STATUS_INVALID_SIZE,
    DVR_STATUS_INVALID_INDEX,
    DVR_STATUS_INVALID_RATE,
    DVR_STATUS_INVALID_EEPROM_ZONE,
    DVR_STATUS_INVALID_EEPROM_ACCESS_TYPE,
    DVR_STATUS_INVALID_EEPROM_PASS_TYPE,
    DVR_STATUS_INVALID_EEPROM_FUSE_ID,
    DVR_STATUS_INVALID_TEMP_THRESHOLD,
    DVR_STATUS_INVALID_TEMP_INTERVAL,
    DVR_STATUS_IOCHAN_ALREADY_CREATED,
    DVR_STATUS_IOCHAN_NOT_CREATED,
    DVR_STATUS_UNSUPPORTED_BITSTREAM,
    DVR_STATUS_INVALID_MEM_INTERVAL,
    DVR_STATUS_INVALID_SMO_INSTANCE,
    DVR_STATUS_INVALID_SIGNAL,
    DVR_STATUS_UNSUPPORT_FLIP,
    DVR_STATUS_INVALID_EEPROM_FMT,
    DVR_STATUS_INVALID_VALUE_IN_SPEC,
    DVR_STATUS_FOSD_ERROR,
    DVR_STATUS_FONT_NOT_AVAILABLE,
    DVR_STATUS_PROTECTION_ERROR,
    DVR_STATUS_SNAPSHOT_NOT_READY,
};

/************************************************************************
    Chip revision definitions.
************************************************************************/
enum dvr_chip_rev_enum {
    CHIP_S6100_3_REV_C = 0,
    CHIP_S6105_3_REV_C,
    CHIP_S6106_3_REV_C,
    CHIP_S6100_3_REV_D = 16,
    CHIP_S6105_3_REV_D,
    CHIP_S6106_3_REV_D,
    CHIP_S6100_3_REV_F = 32,
    CHIP_S6105_3_REV_F,
    CHIP_S6106_3_REV_F,
    CHIP_S6100_3_UNKNOWN = 48,
    CHIP_S6105_3_UNKNOWN,
    CHIP_S6106_3_UNKNOWN,
    CHIP_S7100_UNKNOWN = 64,
    CHIP_S7100_REV_A,
    CHIP_S7100_REV_B,
    CHIP_S7100_REV_C,
    CHIP_UNKNOWN = 255,
};

/************************************************************************
    Camera types:

        @DVR_VSTD_D1_PAL@    is defined as 704x576 at 25fps.

        @DVR_VSTD_D1_NTSC@   is defined as 704x480 at 30fps.

        @DVR_VSTD_CIF_PAL@   is defined as 352x288 at 25fps.

        @DVR_VSTD_CIF_NTSC@  is defined as 352x240 at 30fps.

        @DVR_VSTD_2CIF_PAL@  is defined as 704x288 at 25fps.

        @DVR_VSTD_2CIF_NTSC@ is defined as 704x240 at 30fps.

        @DVR_VSTD_4CIF_PAL@  is defined as 704x576 at 25fps.

        @DVR_VSTD_4CIF_NTSC@ is defined as 704x480 at 30fps.

        @DVR_VSTD_1080P30@ is defined as 1920x1080 at 30 frames per second.

        @DVR_VSTD_1080P25@ is defined as 1920x1080 at 25 frames per second.

        @DVR_VSTD_720P60@ is defined as 1280x720 at 60 frames per second.

        @DVR_VSTD_720P50@ is defined as 1280x720 at 50 frames per second.

        @DVR_VSTD_1080I60@ is defined as 1920x540 at 60 fields per second.

        @DVR_VSTD_1080I50@ is defined as 1920x540 at 50 fields per second.

        @DVR_VSTD_1080P60@ is defined as 1920x1080 at 60 frames per second.

        @DVR_VSTD_1080P50@ is defined as 1920x1080 at 50 frames per second.

        @DVR_VSTD_720P30@ is defined as 1280x720 at 30 frames per second.

        @DVR_VSTD_720P25@ is defined as 1280x720 at 25 frames per second.

        @DVR_VSTD_CUSTOM@ is a custom geometry and framerate defined elsewhere.

    Camera type masks:

        @DVR_VSTD_PAL_MASK@  is defined as all PAL types.

        @DVR_VSTD_NTSC_MASK@ is defined as all NTSC types.

        @DVR_VSTD_SD_MASK@  is defined as all SD types.

        @DVR_VSTD_HD_MASK@  is defined as all HD types.

        @DVR_VSTD_N_P_MASK@ is defined as the NTSC/PAL standard mask.

        @DVR_VSTD_SIZE_MASK@ is defined as the standard-less size mask.
************************************************************************/
#define DVR_VSTD_UNKNOWN     0
#define DVR_VSTD_D1          (1 << 0)
#define DVR_VSTD_CIF         (1 << 1)
#define DVR_VSTD_2CIF        (1 << 2)
#define DVR_VSTD_4CIF        (1 << 3)
#define DVR_VSTD_720P        (1 << 4)
#define DVR_VSTD_1080I       (1 << 5)
#define DVR_VSTD_1080P       (1 << 6)
#define DVR_VSTD_720P_H      (1 << 7)
#define DVR_VSTD_1080P_H     (1 << 8)
#define DVR_VSTD_CUSTOM      (1 << 9)
#define DVR_VSTD_LAST        (DVR_VSTD_CUSTOM)
#define DVR_VSTD_SIZE_MASK   ((DVR_VSTD_LAST << 1) - 1)

#define DVR_VSTD_NTSC_MASK   (1 << 14)
#define DVR_VSTD_PAL_MASK    (1 << 15)
#define DVR_VSTD_N_P_MASK    (DVR_VSTD_NTSC_MASK | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_SD_MASK     (DVR_VSTD_D1 | DVR_VSTD_CIF | \
                              DVR_VSTD_2CIF | DVR_VSTD_4CIF | \
                              DVR_VSTD_N_P_MASK)
#define DVR_VSTD_HD_MASK     (DVR_VSTD_720P | DVR_VSTD_720P_H | \
                              DVR_VSTD_1080P | DVR_VSTD_1080P_H | \
                              DVR_VSTD_1080I | \
                              DVR_VSTD_N_P_MASK)

#define DVR_VSTD_D1_PAL      (DVR_VSTD_D1 | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_D1_NTSC     (DVR_VSTD_D1 | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_CIF_PAL     (DVR_VSTD_CIF | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_CIF_NTSC    (DVR_VSTD_CIF | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_2CIF_PAL    (DVR_VSTD_2CIF | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_2CIF_NTSC   (DVR_VSTD_2CIF | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_4CIF_PAL    (DVR_VSTD_4CIF | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_4CIF_NTSC   (DVR_VSTD_4CIF | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_1080P25     (DVR_VSTD_1080P_H | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_1080P30     (DVR_VSTD_1080P_H | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_720P50      (DVR_VSTD_720P | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_720P60      (DVR_VSTD_720P | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_1080I50     (DVR_VSTD_1080I | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_1080I60     (DVR_VSTD_1080I | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_1080P50     (DVR_VSTD_1080P | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_1080P60     (DVR_VSTD_1080P | DVR_VSTD_NTSC_MASK)
#define DVR_VSTD_720P25      (DVR_VSTD_720P_H | DVR_VSTD_PAL_MASK)
#define DVR_VSTD_720P30      (DVR_VSTD_720P_H | DVR_VSTD_NTSC_MASK)


/************************************************************************
    Supported interlacing formats.

    @DVR_VSTD_FMT_UNKNOWN@      Interlaced/Progressive unknown.

    @DVR_VSTD_FMT_INTERLACED@   Interlaced video.

    @DVR_VSTD_FMT_PROGRESSIVE@  Progressive video.
************************************************************************/
enum dvr_vstd_fmt_enum {
    DVR_VSTD_FMT_UNKNOWN        = 0,
    DVR_VSTD_FMT_INTERLACED     = 1,
    DVR_VSTD_FMT_PROGRESSIVE    = 2
};

/************************************************************************
    Supported job types.

    @DVR_JOB_CAMERA_ENCODE@ encodes real-time video captured by a camera.

    @DVR_JOB_HOST_ENCODE@   encodes raw images supplied by the host.

    @DVR_JOB_HOST_DECODE@   decodes bitstreams supplied by the host.

    @DVR_JOB_RSVD_REM_CE@   Reserved for firmware.  Do not use.

    @DVR_JOB_RSVD_REM_HE@   Reserved for firmware.  Do not use.

    @DVR_JOB_RSVD_REM_HD@   Reserved for firmware.  Do not use.

    @DVR_JOB_EMO_ENCODE@    encodes monitor (tiled) video derived from
    multiple cameras or decoders.

    @DVR_JOB_AUDIO_OUTPUT@  manages audio output from the board.

    @DVR_JOB_VIDEO_OUTPUT@  manages video output from the board.
************************************************************************/
enum dvr_job_type_enum {
    DVR_JOB_CAMERA_ENCODE   = 0,
    DVR_JOB_HOST_ENCODE     = 1,
    DVR_JOB_HOST_DECODE     = 2,
    DVR_JOB_RSVD_REM_CE     = 3,
    DVR_JOB_RSVD_REM_HE     = 4,
    DVR_JOB_RSVD_REM_HD     = 5,
    DVR_JOB_EMO_ENCODE      = 0xD,
    DVR_JOB_AUDIO_OUTPUT    = 0xE,
    DVR_JOB_VIDEO_OUTPUT    = 0xF
};

/************************************************************************
    Video output resolution formats.

    @DVR_VIDEO_RES_INVALID@ - Invalid.

    @DVR_VIDEO_RES_FULL@    - Full size picture, size depends on the video
    standard selected. For D1, it is 720x480 (NTSC) or 720x576 (PAL). For
    4CIF, it is 704x480 (NTSC) or 704x576 (PAL).

    @DVR_VIDEO_RES_CIF@     - Absolute CIF size.  352x240 (NTSC) or 352x288 (PAL).

    @DVR_VIDEO_RES_QCIF@    - Absolute QCIF size. 176x112 (NTSC) or 176x144 (PAL).

    @DVR_VIDEO_RES_HALF@    - Half size. Same width as @DVR_VIDEO_RES_FULL@
    but half the height.

    @DVR_VIDEO_RES_DCIF@    - DCIF size. 528x320 (NTSC) or 528x384 (PAL).
    This size is supported only for encoding and decoding.

    @DVR_VIDEO_RES_CLASSIC_CIF@  - 320x240 - Not supported for
    encoding or decoding.

    @DVR_VIDEO_RES_CLASSIC_2CIF@ - 640x240  - Not supported for
    encoding or decoding.

    @DVR_VIDEO_RES_CLASSIC_4CIF@ - 640x480  - Not supported for
    encoding or decoding.

    @DVR_VIDEO_RES_D1@   - Absolute D1 size.   720x480 (NTSC) or 720x576 (PAL).

    @DVR_VIDEO_RES_4CIF@ - Absolute 4CIF size. 704x480 (NTSC) or 704x576 (PAL).

    @DVR_VIDEO_RES_2CIF@ - Absolute 2CIF size. 704x249 (NTSC) or 704x288 (PAL).

    @DVR_VIDEO_RES_720P@ - 1280x720 resolution for both NTSC (30, 60fps) and PAL (25, 50fps).

    @DVR_VIDEO_RES_1080I@ - 1920x1080 resolution for both NTSC (30, 60fps) and PAL (25, 50fps).

    @DVR_VIDEO_RES_1080P@ - 1920x1080 resolution for both NTSC (30, 60fps) and PAL (25, 50fps).

    @DVR_VIDEO_RES_CUSTOM_1@ - Arbitrary output size that is larger than
    fourth/CIF decimation. Actual size will be defined by sending IOCTL
    commands - see opcodes @DVR_IOCTL_CODE_IMG_CROP@ and
    @DVR_IOCTL_CODE_IMG_SCALE@ for details. Restrictions are (1) the final
    output size cannot be larger than the video standard (2) final width and
    height must be even numbers. Either or both of cropping and scaling can
    be specified. If both are specified, cropping always occurs before scaling.

    @DVR_VIDEO_RES_CUSTOM_4@ - Arbitrary output size that is equal to or
    smaller than fourth/CIF decimation, but larger than sixteenth/QCIF
    decimation. Actual size will be defined by sending IOCTL commands - see
    opcodes @DVR_IOCTL_CODE_IMG_CROP@ and @DVR_IOCTL_CODE_IMG_SCALE@ for
    details. Restriction is the final width and height must be even numbers.
    Either or both of cropping and scaling can be specified. If both are
    specified, cropping always occurs before scaling.

    @DVR_VIDEO_RES_CUSTOM_16@ - Arbitrary output size that is equal to or
    smaller than sixteenth/QCIF decimation. Actual size will be defined by
    sending IOCTL commands - see opcodes @DVR_IOCTL_CODE_IMG_CROP@ and
    @DVR_IOCTL_CODE_IMG_SCALE@ for details. Restriction is the final width and
    height must be even numbers. Either or both of cropping and scaling can
    be specified. If both are specified, cropping always occurs before scaling.

    @NOTE@: Not all boards may support custom output size.
************************************************************************/
enum dvr_video_res_enum {
    DVR_VIDEO_RES_INVALID = 0,
    DVR_VIDEO_RES_FULL    = 1,
    DVR_VIDEO_RES_CIF     = 2,
    DVR_VIDEO_RES_QCIF    = 4,
    DVR_VIDEO_RES_HALF    = 5,
    DVR_VIDEO_RES_DCIF    = 6,
    DVR_VIDEO_RES_CLASSIC_CIF  = 7,
    DVR_VIDEO_RES_CLASSIC_2CIF = 8,
    DVR_VIDEO_RES_CLASSIC_4CIF = 9,
    DVR_VIDEO_RES_D1 = 10,
    DVR_VIDEO_RES_4CIF = 11,
    DVR_VIDEO_RES_2CIF = 12,
    DVR_VIDEO_RES_720P = 13,
    DVR_VIDEO_RES_1080I = 14,
    DVR_VIDEO_RES_1080P = 15,
    DVR_VIDEO_RES_CUSTOM = 128,
    DVR_VIDEO_RES_CUSTOM_1 = 129,
    DVR_VIDEO_RES_CUSTOM_4 = 130,
    DVR_VIDEO_RES_CUSTOM_16 = 131,
};

/************************************************************************
    Video encoding and decoding formats.

    @DVR_VC_FORMAT_NONE@         no video codec.

    @DVR_VC_FORMAT_H264_AVC@     H.264 advanced video codec baseline.

    @DVR_VC_FORMAT_H264@         same as H.264 AVC kept for backward compatibility.

    @DVR_VC_FORMAT_JPEG@         Motion JPEG.

    @DVR_VC_FORMAT_MPEG4@        MPEG4 simple profile.

    @DVR_VC_FORMAT_H264_SVC@     H264 scalable video codec.

    @DVR_VC_FORMAT_MPEG2@        MPEG2 video codec.

    @DVR_VC_FORMAT_RAW_YUV_420@  Raw YUV 4:2:0 video; decoder only.
************************************************************************/
enum dvr_vc_format_enum {
    DVR_VC_FORMAT_NONE          = 0,
    DVR_VC_FORMAT_H264_AVC      = 1,
    DVR_VC_FORMAT_H264          = DVR_VC_FORMAT_H264_AVC,
    DVR_VC_FORMAT_JPEG          = 2,
    DVR_VC_FORMAT_MPEG4         = 3,
    DVR_VC_FORMAT_H264_SVC      = 4,
    DVR_VC_FORMAT_MPEG2         = 5,
    DVR_VC_FORMAT_RAW_YUV_420   = 6
};

/************************************************************************
    Audio encoding and decoding formats.

    @DVR_AC_FORMAT_NONE@        no audio codec.

    @DVR_AC_FORMAT_G711@        G.711 audio

    @DVR_AC_FORMAT_G726_16K@    G.726 audio at 16 Kbits/sec.

    @DVR_AC_FORMAT_G726_32K@    G.726 audio at 32 Kbits/sec.

    @DVR_AC_FORMAT_G726_48K@    G.726 audio at 48 Kbits/sec - Not Supported

    The other formats defined are not currently supported.
************************************************************************/
enum dvr_ac_format_enum {
    DVR_AC_FORMAT_NONE      = 0,
    DVR_AC_FORMAT_G711      = 1,
    DVR_AC_FORMAT_G726_16K  = 2,
    DVR_AC_FORMAT_G726_24K  = 3,
    DVR_AC_FORMAT_G726_32K  = 4,
    DVR_AC_FORMAT_G726_48K  = 5,
};

/************************************************************************
    Audio sampling rates.

    @DVR_AUDIO_RATE_NONE@       Audio is disabled.

    @DVR_AUDIO_RATE_8KHZ@       8KHz audio sampling.

    @DVR_AUDIO_RATE_16KHZ@      16KHz audio sampling.

    @DVR_AUDIO_RATE_32KHZ@      32KHz audio sampling.
************************************************************************/
enum dvr_audio_rate_enum {
    DVR_AUDIO_RATE_NONE     = 0,
    DVR_AUDIO_RATE_8KHZ     = 1,
    DVR_AUDIO_RATE_16KHZ    = 2,
    DVR_AUDIO_RATE_32KHZ    = 3
};

/************************************************************************
    Video preprocessing actions supported. Each action is independent of
    the others. Action flags must be or-ed together to specify a combined
    set of actions.

    @NOTE@: Some actions may impact the frame rate when enabled.

    @DVR_VPP_ACTION_ANALYTIC@       Enables VPP analytics. If this action
    is disabled then motion detection, night detection, blind detection
    and privacy blocking will all be disabled.

    @DVR_VPP_ACTION_DEINTERLACE@    Enables deinterlacing. If this action
    is disabled then the picture will remain in interlaced format, with 2
    separate fields.

    @DVR_VPP_ACTION_MEDIAN_FILTER@  Enables median filtering as a part of
    deinterlacing. If this action is disabled then the two fields will be
    weaved together but no filtering will be performed. If deinterlacing
    is disabled, this action will have no effect.

    @DVR_VPP_ACTION_SLATERAL@       Enables Stretch-lateral noise filtering.

    @DVR_VPP_ACTION_MBD_THRESHOLD@  Enables motion threshold based deinterlacing.
    If this flag is set, then VPP will perform weave deinterlacing as long as
    the detected motion value is less than or equal to the "md_threshold" value.
    Once the detected motion exceeds this value, VPP will switch to median
    filtering for deinterlacing. When this action is enabled, VPP will ignore
    the setting of @DVR_VPP_ACTION_MEDIAN_FILTER@. Setting the threshold to zero
    has the same result as disabling this action.

    @NOTE@: The default preprocessing configuration at startup is set up
    as -
    for encoder boards - ( DVR_VPP_ACTION_ANALYTIC | DVR_VPP_ACTION_DEINTERLACE | DVR_VPP_ACTION_MEDIAN_FILTER | DVR_VPP_ACTION_SLATERAL ).
    for capture boards - ( DVR_VPP_ACTION_DEINTERLACE | DVR_VPP_ACTION_MEDIAN_FILTER | DVR_VPP_ACTION_SLATERAL ).
************************************************************************/
enum dvr_vpp_action_enum {
    DVR_VPP_ACTION_ANALYTIC             = 1,
    DVR_VPP_ACTION_DEINTERLACE          = 2,
    DVR_VPP_ACTION_MEDIAN_FILTER        = 4,
    DVR_VPP_ACTION_SLATERAL             = 8,
    DVR_VPP_ACTION_MBD_THRESHOLD        = 16,
    DVR_VPP_ACTION_SLATERAL_STRENGTH    = 32,
    DVR_VPP_ACTION_PIXEL_ADAPTIVE_DEINTERLACE = 64
};

/************************************************************************
    Encoder rate-control algorithm selectors.

    @DVR_RC_VBR@ generates a variable bit rate stream. The encoder attempts
    to not exceed the average bit rate over the long term. It also attempts
    to not exceed the maximum specified bit rate at all times.

    @DVR_RC_CBR@ generates a constant bit rate stream. However, short-term
    variations in bit rate are allowed to maintain picture auality.

    @DVR_RC_CBR_S@ also generates a constant bit rate stream. However,
    this mode strictly limits short-term variations in bit rate so that
    deviations from the specified bit rate are minimized. This can cause
    a short-term degradation in picture quality during sudden motion or
    scene changes.

    @DVR_RC_CQP@ generates a constant QP stream.

    @DVR_RC_CQ@  generates a constant quality stream. This mode attempts
    to maintain the specified picture quality level while minimizing the
    bit rate.

    @NOTE@: @DVR_RC_CBR_S@ is supported only for H.264 AVC and SVC at this
    time.

    @NOTE@: @DVR_RC_CQP@ is not supported at this time.
************************************************************************/
enum dvr_rc_enum {
    DVR_RC_NONE = -1,
    DVR_RC_VBR  = 0,
    DVR_RC_CBR,
    DVR_RC_CQP,
    DVR_RC_CQ,
    DVR_RC_CBR_S,
};

/************************************************************************
    Defines job control constants.

    @DVR_JOB_CREATE@    creates a new job.

    @DVR_JOB_DESTROY@   destroys an existing job.

    @DVR_JOB_RESET@     resets a job to its default configuration.

    @NOTE@: @DVR_JOB_RESET@ is deprecated. It is currently a no-op and
    will be removed in a future release.
************************************************************************/
enum dvr_job_action_enum {
    DVR_JOB_CREATE = 0,
    DVR_JOB_DESTROY,
    DVR_JOB_RESET,
};

/************************************************************************
    Defines the on-screen display style.

    @DVR_OSD_POS_TL@     puts OSD at top-left corner.

    @DVR_OSD_POS_BL@     puts OSD at bottom-left corner.

    @DVR_OSD_POS_TR@     puts OSD at top-right corner.

    @DVR_OSD_POS_BR@     puts OSD at bottom-right corner.

    @DVR_OSD_POS_CUSTOM@ puts OSD at the specified location on the screen.
    This item can only be used in conjunction with @DVR_SET_OSD_EX@ message.
************************************************************************/
enum dvr_osd_pos_enum {
    DVR_OSD_POS_TL = 0,
    DVR_OSD_POS_BL,
    DVR_OSD_POS_TR,
    DVR_OSD_POS_BR,
    DVR_OSD_POS_CUSTOM
};

/************************************************************************
    Format of the date on OSD.

    @DVR_OSD_DTS_NONE@     disables DTS.

    @DVR_OSD_DTS_DEBUG@      enables a special debug display mode.

    @DVR_OSD_DTS_DEBUG_2@    enables a special debug display mode.

    @DVR_OSD_DTS_DEBUG_3@    enables a special debug display mode.

    The following options all display the month name as text.

    @DVR_OSD_DTS_MDY_12H@  is "MMM-DD-YY HH:MM:SS am/pm".

    @DVR_OSD_DTS_DMY_12H@  is "DD-MMM-YY HH:MM:SS am/pm".

    @DVR_OSD_DTS_YMD_12H@  is "YY-MMM-DD HH:MM:SS am/pm".

    @DVR_OSD_DTS_MDY_24H@  is "MMM-DD-YY HH:MM:SS".

    @DVR_OSD_DTS_DMY_24H@  is "DD-MMM-YY HH:MM:SS".

    @DVR_OSD_DTS_YMD_24H@  is "YY-MMM-DD HH:MM:SS".

    The following options all display the month as a numeric value.

    @DVR_OSD_DTS_MDY_12H_NUM@  is "MM-DD-YYYY HH:MM:SS am/pm".

    @DVR_OSD_DTS_DMY_12H_NUM@  is "DD-MM-YYYY HH:MM:SS am/pm".

    @DVR_OSD_DTS_YMD_12H_NUM@  is "YYYY-MM-DD HH:MM:SS am/pm".

    @DVR_OSD_DTS_MDY_24H_NUM@  is "MM-DD-YYYY HH:MM:SS".

    @DVR_OSD_DTS_DMY_24H_NUM@  is "DD-MM-YYYY HH:MM:SS".

    @DVR_OSD_DTS_YMD_24H_NUM@  is "YYYY-MM-DD HH:MM:SS".

************************************************************************/
enum dvr_osd_dts_enum {
    DVR_OSD_DTS_NONE = 0,
    DVR_OSD_DTS_DEBUG,
    DVR_OSD_DTS_MDY_12H,
    DVR_OSD_DTS_DMY_12H,
    DVR_OSD_DTS_YMD_12H,
    DVR_OSD_DTS_MDY_24H,
    DVR_OSD_DTS_DMY_24H,
    DVR_OSD_DTS_YMD_24H,
    DVR_OSD_DTS_DEBUG_2,
    DVR_OSD_DTS_MDY_12H_NUM,
    DVR_OSD_DTS_DMY_12H_NUM,
    DVR_OSD_DTS_YMD_12H_NUM,
    DVR_OSD_DTS_MDY_24H_NUM,
    DVR_OSD_DTS_DMY_24H_NUM,
    DVR_OSD_DTS_YMD_24H_NUM,
    DVR_OSD_DTS_DEBUG_3,
};

/************************************************************************
    Type of data transfered between firmware and SDK.

    @DVR_DATA_RAW_VIDEO_xxx@   Y,U, and V component of a raw video frame.
    For interlaced video, a frame must contain the top field followed by
    the bottom field.

    @DVR_DATA_RAW_AUDIO@       Raw audio PCM data.

    @DVR_DATA_H264_xxx@        NAL unit of H.264 bitstream.

    @DVR_DATA_MPEG4_x@         NAL unit of MPEG4 encoded bitstream.

    @DVR_DATA_JPEG@            JPEG-compressed image.

    @DVR_DATA_G711@            G.711-compressed audio data, but in raw data format.

    @DVR_DATA_G726_16K@        G.726-compressed audio data at 16 Kbits/sec.

    @DVR_DATA_G726_32K@        G.726-compressed audio data at 32 Kbits/sec.

    @DVR_DATA_MOTION_VALUES@   Macro block motion values per raw video frames.

    @DVR_DATA_RAW_VIDEO@

    @DVR_FRAME_H264_SVC_SEI@

    @DVR_FRAME_H264_SVC_PREFIX@

    @DVR_FRAME_H264_SVC_SUBSET_SPS@

    @DVR_FRAME_H264_SVC_SLICE_SCALABLE@

    @DVR_DATA_CMD_RESPONSE@ - A buffer containing the response to the
    raw command sent from the Host Application to a sub-system
    within the firmware. The format of the payload is only known
    by the sub-system for the specific command.

    @DVR_DATA_JPEG_SNAPSHOT@            JPEG-compressed single-frame snapshot image.

    @DVR_DATA_VIDEO_ANALYTICS@

    following data types are not presented to the Host DVR Application

    data_type of 100-254 is reserved for the DVR SDK.
************************************************************************/
enum dvr_data_type_enum {
    DVR_DATA_RAW_VIDEO_Y = 0,
    DVR_DATA_RAW_VIDEO_U,
    DVR_DATA_RAW_VIDEO_V,
    DVR_DATA_RAW_AUDIO,
    DVR_DATA_H264_IDR,
    DVR_DATA_H264_I,
    DVR_DATA_H264_P,
    DVR_DATA_H264_B,
    DVR_DATA_H264_SPS,
    DVR_DATA_H264_PPS,
    DVR_DATA_JPEG,
    DVR_DATA_G711,
    DVR_DATA_MPEG4_I,
    DVR_DATA_MPEG4_P,
    DVR_DATA_MPEG4_B,
    DVR_DATA_MPEG4_VOL,
    DVR_DATA_G726_16K,
    DVR_DATA_G726_32K,
    DVR_DATA_MOTION_VALUES,
    DVR_DATA_RAW_VIDEO,
    DVR_DATA_H264_SVC_SEI,
    DVR_DATA_H264_SVC_PREFIX,
    DVR_DATA_H264_SVC_SUBSET_SPS,
    DVR_DATA_H264_SVC_SLICE_SCALABLE,
    DVR_DATA_MPEG2_I,
    DVR_DATA_MPEG2_P,
    DVR_DATA_MPEG2_B,
    DVR_DATA_CMD_RESPONSE,
    DVR_DATA_JPEG_SNAPSHOT,
    DVR_DATA_MOTION_MAP,

    DVR_DATA_VIDEO_ANALYTICS = 90,

    DVR_DATA_UNKNOWN = 255,
};

/************************************************************************
    Defines signal types from board to host.

    @DVR_SIGNAL_SENSOR_ACTIVATED@  - sent when a sensor is activated or
    deactivated.

    @DVR_SIGNAL_MOTION_DETECTED@   - sent when motion is detected.

    @DVR_SIGNAL_BLIND_DETECTED@    - sent when the camera is blind.

    @DVR_SIGNAL_NIGHT_DETECTED@    - sent when the image is too dark.

    @DVR_SIGNAL_VIDEO_LOST@        - sent when the video signal from a
    camera is lost.

    @DVR_SIGNAL_VIDEO_DETECTED@    - sent when a video signal is detected
    on an input channel, where there was no signal before.

    @DVR_SIGNAL_RUNTIME_ERROR@     - indicates that a non-fatal runtime
    error has occurred on the board.

    @DVR_SIGNAL_FATAL_ERROR@       - indicates that a fatal error has
    occurred on the board. If this signal is received, the board must
    be reset.

    @DVR_SIGNAL_HEARTBEAT@         - if the heartbeat function is enabled
    by the host, then this message will be sent once a second by the board
    as long as the firmware is operational.

    @DVR_SIGNAL_WATCHDOG_EXPIRED@  - indicates that the watchdog timer has
    expired and the board is about to be reset. If the PC reset function
    is enabled then the host PC will also be reset.

    @DVR_SIGNAL_LAST_ERROR@ - indicates that there was an error the last time
    the board shut down or crashed, and provides the error code. Only errors
    that can be trapped by the firmware will be recorded and reported.

    @DVR_SIGNAL_PCI_CONGESTION@ - indicates that the firmware is dropping
    data due to congestion on the PCI bus. This usually happens only if the
    host application is not picking up the arriving data fast enough.

    @DVR_SIGNAL_TEMPERATURE@ - reports the current core temperature of PE 0,
    as measured from the silicon. This signal is enabled by default.
    The default measurement interval is 15 seconds and the default threshold
    is 85 deg C. The signal is sent only if the threshold is exceeded.
    Use the @DVR_SET_IOCTL@ command with the @DVR_IOCTL_CODE_TEMPERATURE@
    control code to change the defaults.

    @DVR_SIGNAL_FRAME_TOO_LARGE@ - indicates that the firmware had to drop
    an encoded frame because it was too large to fit in the SCT buffer.
    This usually happens when the encoder quality for MJPEG is set too high
    or if the combination of frame rate / bit rate / GOP size for H.264 or
    MPEG4 results in very large I frames.

    @DVR_SIGNAL_MEMORY_USAGE@ - reports the current PE wise memory usage,
    value in MB. This is disabled by default. The default measurement
    interval is 30 seconds

    @DVR_SIGNAL_COUNT@ - This is for internal use to keep the number of
    signals. NOTE: Add any new signal before this.


    @DVR_SIGNAL_USER_1@ - this is for general use by anyone writing their
    own firmware that wishes to communicate with the host SDK. The message
    can contain any user-defined information.

    @DVR_SIGNAL_USER_2@ - this is for general use by anyone writing their
    own firmware that wishes to communicate with the host SDK. The message
    can contain any user-defined information.

************************************************************************/
enum dvr_signal_type_enum {
    DVR_SIGNAL_SENSOR_ACTIVATED = 0,
    DVR_SIGNAL_MOTION_DETECTED,
    DVR_SIGNAL_BLIND_DETECTED,
    DVR_SIGNAL_NIGHT_DETECTED,
    DVR_SIGNAL_VIDEO_LOST,
    DVR_SIGNAL_VIDEO_DETECTED,
    DVR_SIGNAL_RUNTIME_ERROR,
    DVR_SIGNAL_FATAL_ERROR,
    DVR_SIGNAL_HEARTBEAT,
    DVR_SIGNAL_WATCHDOG_EXPIRED,
    DVR_SIGNAL_LAST_ERROR,
    DVR_SIGNAL_PCI_CONGESTION,
    DVR_SIGNAL_TEMPERATURE,
    DVR_SIGNAL_FRAME_TOO_LARGE,
    DVR_SIGNAL_MEMORY_USAGE,
    DVR_SIGNAL_COUNT,

    DVR_SIGNAL_USER_1 = 50,
    DVR_SIGNAL_USER_2,
};

/****************************************************************************
    VISIBLE: The following enum describe various raw video formats.
    Each enum value must correspond to a unique bit to be used in
    a bitmap field.

    NOTE: all RGB/YUV formats supported by overlays must be in the lowest
    8 bits of this structure, as the width of "rgb_yuv_format" is limited.

    DVR_RAWV_FORMAT_YUV_4_2_0 - 4:2:0 YUV format.

    DVR_RAWV_FORMAT_YUV_4_2_2 - 4:2:2 YUV format.

    DVR_RAWV_FORMAT_YVU_4_2_0 - 4:2:0 YVU (YV12) format.

    DVR_RAWV_FORMAT_YUYV_4_2_2i - 4:2:2 YUYV pixel interleaved format

    DVR_RAWV_FORMAT_RGB_8 - 1 byte raw RGB format
    DVR_RAWV_FORMAT_RGB_655 - 2 bytes RGB format
    DVR_RAWV_FORMAT_RGB_565 - 2 bytes RGB format
    DVR_RAWV_FORMAT_RGB_565be - 2 bytes RGB format (big endian)
****************************************************************************/
enum dvr_rawv_format_enum {
    DVR_RAWV_FORMAT_YUV_4_2_0 = 1,
    DVR_RAWV_FORMAT_YUV_4_2_2 = 2,
    DVR_RAWV_FORMAT_YVU_4_2_0 = 4,
    DVR_RAWV_FORMAT_YUYV_4_2_2i = 8,
    DVR_RAWV_FORMAT_RGB_8 = 16,
    DVR_RAWV_FORMAT_RGB_655 = 32,
    DVR_RAWV_FORMAT_RGB_565 = 64,
    DVR_RAWV_FORMAT_RGB_565be = 128,
    DVR_RAWV_FORMAT_RGB_565Q = 0xc1,
    DVR_RAWV_FORMAT_RGB_565Qbe = 0xc2,
    DVR_RAWV_FORMAT_RGB_565X = 0xc3,
    DVR_RAWV_FORMAT_RGB_565Xbe = 0xc4,
};

/************************************************************************
    DVR job state flags. The flags are OR-ed together and sent to the
    host in every data buffer header.

    @DVR_JOB_STATE_VIDEO_LOST@ is set when the video signal is lost.

    @DVR_JOB_STATE_AUDIO_LOST@ is set when the audio signal is lost.
************************************************************************/
#define DVR_JOB_STATE_VIDEO_LOST    0x01
#define DVR_JOB_STATE_AUDIO_LOST    0x02

/************************************************************************
    Message class shared between the DVR firmware and DVR SDK.

    The communication between firmware and DVR is done in a transaction
    style with agreed upon conventions.  Both firmware and SDK can
    originate a message transaction.  The receiver of the message must
    reply with requested data.  The first byte of the data
    contains the return status.  If the return status is not DVR_STATUS_OK,
    it is used as an error code.  This error code is used to
    look up a string message for the user.

    The messages fall into two categories:

        Information messages for the host to find out specific attributes
        of the board.  These messages have the form DVR_GET_xxx and
        DVR_REP_xxx where the GET message is the request from the
        host, and the REP message is the reply from the board.

        Parameter-setting messages for the host to control the board
        functions.  These messages have the form DVR_SET_xxx and DVR_REP_xxx
        where the SET message is originated from the host, and REP is the
        reply from the board.

    The following set of messages are used to exchange information
    about the board.  A message from the SDK to the firmware represents a
    request, and a message from the firmware to the SDK is the response.

        @DVR_GET_VER_INFO@, @DVR_REP_VER_INFO@ - Firmware version.
        The data associated with this message must
        be of type @dvr_ver_info_t@.

    The following set of messages is used to configure the board
    for various jobs.

        @DVR_GET_BOARD@, @DVR_SET_BOARD@, @DVR_REP_BOARD@ - Get
        information about the board and configure the board.
        The data for this message is of type @dvr_board_t@.

        @DVR_GET_BOARD_2@ and @DVR_REP_BOARD_2@ - Get extended board
        information. These messages are used to retrieve information
        about the board and firmware. Note that there is no SET message.

        @DVR_GET_JOB@, @DVR_SET_JOB@, @DVR_REP_JOB@ - Configure a
        job on the board using the information in the message data of
        type @dvr_job_t@.  The firmware cannot migrate a job from one
        chip to another once it is created.  So, it is important that
        real-time encoding jobs be created before other jobs.

        @DVR_GET_SMO@, @DVR_SET_SMO@, @DVR_REP_SMO@ - Configure the PCIe
        card spot monitor for a particular encoding or decoding channel.
        The configuration data is of type @dvr_smo_t@.

        @DVR_GET_HMO@, @DVR_SET_HMO@, @DVR_REP_HMO@ - Configure a
        particular video channel (incoming or decoded) for display on
        the host PC.  This raw video image will be transmitted to the
        host side.  The configuration data is of type @dvr_hmo_t@.

        @DVR_GET_EMO@, @DVR_SET_EMO@, @DVR_REP_EMO@ - Used to control the
        Encoded Monitor Ouput (EMO) if present. An EMO is like an SMO but
        returns an encoded picture to the host instead of displaying it.
        The configuration data is of type @dvr_emo_t@.

        @DVR_GET_PR@, @DVR_SET_PR@, @DVR_REP_PR@ - Defines private regions
        to be blocked.  The configuration data is of type @dvr_pr_t@.

        @DVR_GET_MD@, @DVR_SET_MD@, @DVR_REP_MD@ - Configure the motion
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_md_t@.

        @DVR_GET_BD@, @DVR_SET_BD@, @DVR_REP_BD@ - Configure the blind
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_bd_t@.

        @DVR_GET_ND@, @DVR_SET_ND@, @DVR_REP_ND@ - Configure the night
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_nd_t@.

        @DVR_GET_ENCODE@, @DVR_SET_ENCODE@,
        @DVR_REP_ENCODE@ - Configure the encoder for a particular encoding
        channel.  The configuration data is of type @dvr_encode_info_t@.

        @DVR_GET_ENC_ALARM@, @DVR_SET_ENC_ALARM@,
        @DVR_REP_ENC_ALARM@ - Configure the encoder alarm settings for a
        particular encoding channel.  The configuration data is of type
        @dvr_enc_alarm_info_t@.

        @DVR_GET_DECODE@, @DVR_SET_DECODE@,
        @DVR_REP_DECODE@ - Configure the decoder for a particular decode
        channel.  The configuration data is of type @dvr_decode_info_t@.

        @DVR_GET_ENABLE@, @DVR_SET_ENABLE@, @DVR_REP_ENABLE@ - Configure
        the enable state of a channel. Used to enable or disable encoded
        and raw streams, and read out enable status.

        @DVR_SET_FONT_TABLE@, @DVR_REP_FONT_TABLE@ - These messages are
        used to sent a new font table from host DVR to the DVR frimare
        or select an existing pre-defined font_table. There can only be
        one user defined font table at a time and wants it is sent any
        previously user defined font table will be erased and the new one
        is selected.
        NOTE: The font table messages are not in 3.2 release.

        @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, @DVR_REP_OSD_EX@ - These
        messages are used to set/get OSD text, positions, and show
        state for each OSD definitions. Currently up to 5 separate lines of
        OSD can be defined per channel. DVR_xxx_OSD_EX and DVR_xxx_OSD
        message can not be called at the same time for the same channel.
        Doing so, invalidates the previous call.

        @DVR_GET_RELAYS@, @DVR_SET_RELAYS@, @DVR_REP_RELAYS@ - Configure
        relay outputs.  The configuration data is of type @dvr_relay_t@.

        @DVR_GET_SENSORS@, @DVR_SET_SENSORS@, @DVR_REP_SENSORS@ - Configure
        sensor inputs.  The configuration data is of type @dvr_sensor_t@.

        @DVR_GET_WATCHDOG@, @DVR_SET_WATCHDOG@, @DVR_REP_WATCHDOG@ -
        Configure the watchdog timer.  The configuration data is of type
        @dvr_watchdog_t@.  The host is responsible for sending this
        message before the previous watchdog timer expires.

        @DVR_GET_CONTROL@, @DVR_SET_CONTROL@, @DVR_REP_CONTROL@ - These messages
        are used to enable, disable, or get the status of a job.
        The configuration data is of type @dvr_control_t@.

        @DVR_GET_TIME@, @DVR_SET_TIME@, @DVR_REP_TIME@ - These messages
        are used to get the time of day on the board and to set the time of
        the day on the board based on PC host time.
        The configuration data is of type @dvr_time_t@.

        @DVR_GET_UART@, @DVR_SET_UART@ and @DVR_REP_UART@ - These messages
        are used to configure the RS-485 port and to send data out through
        the port.

        @DVR_GET_IOCTL@, @DVR_SET_IOCTL@, and @DVR_REP_IOCTL@ - These are
        used to get and set device-specific parameters for various onboard
        devices.

        @DVR_GET_AUTH_KEY@, @DVR_REP_AUTH_KEY@ - These are used to get the
        security authentication key from the board. The data structure associated
        with these messages is @dvr_auth_key_t@;

        @DVR_SET_AUTH_KEY@ - Setting the authentication key is not
        supported at this time.

        @DVR_SET_REGIONS_MAP@, @DVR_REP_REGIONS_MAP@ -
        Configure the regions map for motion/blind/night detection as well
        as privacy blocking, for an encoding or decoding channel.
        The configuration data is of type @dvr_regions_map_t@.

        @DVR_SET_AV_OUTPUT@, @DVR_REP_AV_OUTPUT@ -
        These are used to configure the firmware to start accepting raw
        video/audio frame to either display video or play audio out of
        the specified SMO/audio out port.

        @DVR_GET_SMO_ATTRIB@, @DVR_SET_SMO_ATTRIB@, @DVR_REP_SMO_ATTRIB@ -
        Get the current attributes setting of the requested Spot Monitor
        display.

        @DVR_GET_VA@, @DVR_SET_VA@, @DVR_REP_VA@ -
        This set of messages is used to communicate with the video analytics
        module, if any. The firmware does not interpret these messages in any
        form. They are simply passed on to the VA module or the host as needed.

        @DVR_SET_AOUT@, @DVR_REP_AOUT@ -
        Specify an audio output for receiving data either from the host or a
        live audio source.

        @DVR_SET_DISCONNECT@, @DVR_REP_DISCONNECT@ -
        Indicate to the firmware that the host has diconnected, and any
        necessary clean-up can be performed.  After receiving this message from
        the host, the board will not respond to any future messages.

    The following message is for the board to signal the host about an event.

        @DVR_SIG_HOST@ signals the host that an event just happened and
        needs the host's attention.  The data associated with this
        message must be of type @dvr_sig_host_t@.

************************************************************************/
enum dvr_message_enum {
    DVR_GET_VER_INFO,                               DVR_REP_VER_INFO,
    DVR_GET_BOARD_2,                                DVR_REP_BOARD_2,
    DVR_GET_BOARD,          DVR_SET_BOARD,          DVR_REP_BOARD,
    DVR_GET_JOB,            DVR_SET_JOB,            DVR_REP_JOB,
    DVR_GET_SMO,            DVR_SET_SMO,            DVR_REP_SMO,
    DVR_GET_HMO,            DVR_SET_HMO,            DVR_REP_HMO,
    DVR_GET_EMO,            DVR_SET_EMO,            DVR_REP_EMO,
    DVR_GET_PR,             DVR_SET_PR,             DVR_REP_PR,
    DVR_GET_MD,             DVR_SET_MD,             DVR_REP_MD,
    DVR_GET_BD,             DVR_SET_BD,             DVR_REP_BD,
    DVR_GET_ND,             DVR_SET_ND,             DVR_REP_ND,
    DVR_GET_RELAYS,         DVR_SET_RELAYS,         DVR_REP_RELAYS,
    DVR_GET_SENSORS,        DVR_SET_SENSORS,        DVR_REP_SENSORS,
    DVR_GET_WATCHDOG,       DVR_SET_WATCHDOG,       DVR_REP_WATCHDOG,
    DVR_GET_CONTROL,        DVR_SET_CONTROL,        DVR_REP_CONTROL,
    DVR_GET_TIME,           DVR_SET_TIME,           DVR_REP_TIME,
    DVR_GET_ENCODE,         DVR_SET_ENCODE,         DVR_REP_ENCODE,
    DVR_GET_ENC_ALARM,      DVR_SET_ENC_ALARM,      DVR_REP_ENC_ALARM,
    DVR_GET_DECODE,         DVR_SET_DECODE,         DVR_REP_DECODE,
                            DVR_SET_FONT_TABLE,     DVR_REP_FONT_TABLE,
    DVR_GET_OSD_EX,         DVR_SET_OSD_EX,         DVR_REP_OSD_EX,
    DVR_GET_UART,           DVR_SET_UART,           DVR_REP_UART,
    DVR_GET_IOCTL,          DVR_SET_IOCTL,          DVR_REP_IOCTL,
    DVR_GET_AUTH_KEY,       DVR_SET_AUTH_KEY,       DVR_REP_AUTH_KEY,
                            DVR_SET_REGIONS_MAP,    DVR_REP_REGIONS_MAP,
                            DVR_SET_AV_OUTPUT,      DVR_REP_AV_OUTPUT,
    DVR_GET_SMO_ATTRIB,                             DVR_REP_SMO_ATTRIB,
    DVR_GET_VA,             DVR_SET_VA,             DVR_REP_VA,
                            DVR_SET_AOUT,           DVR_REP_AOUT,
                            DVR_SET_DISCONNECT,     DVR_REP_DISCONNECT,
                            DVR_SET_SMO_ATTRIB,     /* Here for backward-compatibility */
    DVR_GET_IPP,            DVR_SET_IPP,            DVR_REP_IPP,
    DVR_GET_ENABLE,         DVR_SET_ENABLE,         DVR_REP_ENABLE,

    /* The last legal message ID is 95 (SCPA_CLASS_LAST) */
    DVR_SIG_HOST            = 90,
    DVR_PREPARE_FOR_RESET   = 94,
    DVR_FW_INTERNAL         = 95,
};

/****************************************************************************
    VISIBLE: The following enum describes various encoded audio modes

    @DVR_ENC_AUDIO_MODE_NONE@   - Audio mode is undefined.
    @DVR_ENC_AUDIO_MODE_MONO@   - The encoded audio is always 8 bit mono.
    @DVR_ENC_AUDIO_MODE_STEREO@ - The encoded audio is always 16 bit stereo.
****************************************************************************/
enum dvr_enc_audio_mode_enum {
    DVR_ENC_AUDIO_MODE_NONE    = 0,
    DVR_ENC_AUDIO_MODE_MONO    = 1,
    DVR_ENC_AUDIO_MODE_STEREO  = 2
};

/****************************************************************************
    VISIBLE: The following enum describes various audio data types

    DVR_AUDIO_DATA_TYPE_RAW     - The firmware always sends raw audio.

    DVR_AUDIO_DATA_TYPE_ENCODED - the firmware always sends encoded audio.
****************************************************************************/
enum dvr_audio_data_type_enum {
    DVR_AUDIO_DATA_TYPE_INVALID = 0,
    DVR_AUDIO_DATA_TYPE_RAW     = 1,
    DVR_AUDIO_DATA_TYPE_ENCODED = 2
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_status_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_status_enum        dvr_status_e;
#else
typedef unsigned char dvr_status_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_chip_rev_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_chip_rev_enum        dvr_chip_rev_e;
#else
typedef unsigned char dvr_chip_rev_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_job_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vstd_fmt_enum      dvr_vstd_fmt_e;
#else
typedef unsigned char dvr_vstd_fmt_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_job_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_job_type_enum      dvr_job_type_e;
#else
typedef unsigned char dvr_job_type_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_video_res_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_video_res_enum     dvr_video_res_e;
#else
typedef unsigned char dvr_video_res_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_vc_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vc_format_enum     dvr_vc_format_e;
#else
typedef unsigned char dvr_vc_format_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_ac_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_ac_format_enum     dvr_ac_format_e;
#else
typedef unsigned char dvr_ac_format_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_audio_rate_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_audio_rate_enum     dvr_audio_rate_e;
#else
typedef unsigned char dvr_audio_rate_e;
#endif

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_vpp_action_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vpp_action_enum     dvr_vpp_action_e;
#else
typedef unsigned char dvr_vpp_action_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_rc_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_rc_enum       dvr_rc_e;
#else
typedef unsigned char dvr_rc_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_job_action_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_job_action_enum    dvr_job_action_e;
#else
typedef unsigned char dvr_job_action_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_osd_pos_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_osd_pos_enum       dvr_osd_pos_e;
#else
typedef unsigned char dvr_osd_pos_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_osd_dts_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_osd_dts_enum       dvr_osd_dts_e;
#else
typedef unsigned char dvr_osd_dts_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_data_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_data_type_enum     dvr_data_type_e;
#else
typedef unsigned char dvr_data_type_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_signal_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_signal_type_enum   dvr_signal_type_e;
#else
typedef unsigned char dvr_signal_type_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_message_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_message_enum       dvr_message_e;
#else
typedef unsigned char dvr_message_e;
#endif

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_message_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_enc_audio_mode_enum       dvr_enc_audio_mode_e;
#else
typedef unsigned char dvr_enc_audio_mode_e;
#endif

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_audio_data_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_audio_data_type_enum  dvr_audio_data_type_e;
#else
typedef unsigned char dvr_audio_data_type_e;
#endif

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_rawv_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_rawv_format_enum  dvr_rawv_format_e;
#else
typedef unsigned char dvr_rawv_format_e;
#endif

/************************************************************************
    VISIBLE: This structure defines custom video specification.
************************************************************************/
typedef struct dvr_vstd_custom_struct {
    sx_uint16       w;
    sx_uint16       h;
    sx_uint16       frame_rate;
} dvr_vstd_custom_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_VER_INFO@ and @DVR_REP_VER_INFO@.

    "status" should always be @DVR_STATUS_OK@.

    "version_major" is the major release version number.

    "version_minor" is the minor release version number.

    "version_bug" is the bug release version number.

    "version_build" is the build version number.

    The complete firmware version is "<major>.<minor>.<build>.<bug>".

    "board_revision" is the revision  number of Stretch DVR PCIe board.

    "chip_revision" is the S6 chip revision used in the board_revision.
    See @dvr_chip_rev_e@ for all the different chip revisions.

    "board_sub_rev" is the sub-revision number of Stretch DVR PCIe board.

    "bootloader_ver_major" is the major version number of bootloader.

    "bootloader_ver_minor" is the minor version number of bootloader.

    "bsp_ver_major" is the major version number of BSP.

    "bsp_ver_minor" is the minor version number of BSP.
************************************************************************/
typedef struct dvr_ver_info_struct {
    dvr_status_e    status:8;
    sx_uint8        version_major;
    sx_uint8        version_minor;
    sx_uint8        version_bug;
    sx_uint16       build_year;
    sx_uint8        build_month;
    sx_uint8        build_day;
    sx_uint8        board_revision;
    dvr_chip_rev_e  chip_revision:8;
    sx_uint8        board_sub_rev;
    sx_uint8        version_build;
    sx_uint8        bootloader_ver_major;
    sx_uint8        bootloader_ver_minor;
    sx_uint8        bsp_ver_major;
    sx_uint8        bsp_ver_minor;
} dvr_ver_info_t;
sx_static_assert(sizeof(dvr_ver_info_t) == 16, dvr_ver_info_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_BOARD@, @DVR_SET_BOARD@, and @DVR_REP_BOARD@.

        "status" - status of the reply.

        "board_id" - This field is specified by the DVR SDK
        to give the board a unique ID number.

        "default_camera_types" - This field is specified by the DVR
        application on DVR_SET_BOARD and DVR firmware on DVR_GET_BOARD.
        It must be one or two of the camera types specified
        in "camera_info".  At least one SD type must be defined, along with
        one optional HD type.  All SD cameras connected to a board must be
        of the same SD type specified here.  The HD camera type specified
        here is used as the default but may be overridden.  All the cameras
        (SD and HD) connected to a board must be either NTSC or PAL.


    Following information needs to be set as response to DVR_GET_BOARD in the
    union get struct:

        "num_cameras" - Number of cameras supported by the board.

        "num_sensors" - Number of sensors available on the board.

        "num_relays" - Number of relays available on the board.

        "num_smos" - Number of spot monitors supported by the board.

        "camera_info" - Specifies what cameras are supported by the board.
        It is a bit-wise or of camera types, such as @DVR_VSTD_D1_PAL@
        or @DVR_VSTD_D1_NTSC@.

        "num_host_encoders" - Number of host video encoders supported by the board.

        "num_audio_out_ports" - Number of audio-ouput ports available
        on the board.

        "num_emos" - Number of EMO instances supported by the board.
        Currently this will always be either 0 or 1.

        "num_encoders" - Number of encoders supported by the board.

        "num_decoders" - Number of decoders supported by the board.

        "audio_supported" - Number of audio-input channels supported by the
        board. Zero indicates no audio support.

    Following information needs to be set with DVR_SET_BOARD in the
    union 'set' struct which will be provided by the DVR application:

        "audio_rate" - Audio sampling rate. Must be one of the valid
        @dvr_audio_rate_enum@ values.

        "h264_skip_SCE" - Setting this field to true causes the firmware to skip
        start code emulation (SCR) for H.264 encoding. In such case, the host SDK
        must pefrom SCE. Setting to false means the firmware should perform
        SCR for H.264 encoders.

        "enc_audio_mode" - This field specifies whether the encoded audio should
        be 8 bit mono or 16 bit stereo. (See @dvr_enc_audio_mode_e@ for
        definition of supported audio modes.) NOTE: This field is ignored if
        audio_rate is set to DVR_AUDIO_RATE_NONE;

        "omit_blank_frames" - Setting this field specifies that the firmware
        should omit all blank frames from all encoded streams and HMO/SMO streams
        for this board.  Done at the dataport level by checking video status from
        the video decoder.

************************************************************************/
typedef struct dvr_board_struct {
      dvr_status_e        status:8;
      sx_uint8            board_id;
      sx_uint16           default_camera_types;
      union {
        struct {
            sx_uint8            num_cameras;
            sx_uint8            num_sensors;
            sx_uint8            num_relays;
            sx_uint8            num_smos;
            sx_uint16           camera_info;
            sx_uint8            num_host_encoders;
            sx_uint8            num_audio_out_ports;
            sx_uint8            num_emos;
            sx_uint8            num_encoders;
            sx_uint8            num_decoders;
            sx_uint8            audio_supported;

        } get;
        struct {
            dvr_audio_rate_e          audio_rate:8;
            sx_uint8                  h264_skip_SCE;
            dvr_enc_audio_mode_e      enc_audio_mode:8;
            sx_uint8                  omit_blank_frames;
            struct {
                sx_uint16             w;
                sx_uint16             h;
                sx_uint16             frame_rate;
            } custom_vstd;
            sx_uint16                 reserved;
        } set;
      } u1;
} dvr_board_t;
sx_static_assert(sizeof(dvr_board_t) == 16, dvr_board_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_BOARD_2@ and @DVR_REP_BOARD_2@.

        "status" - status of the reply.

        "num_enc_streams_per_chan" - Max number of encoded video streams
        per input channel (camera). Each stream can be independently
        configured.

        "num_hmo_streams_per_chan" - Max number of raw video streams per
        input channel. Each stream can be independently configured.

        "audio_data_type"          - Indicates whether the firmware sends
        raw or encoded audio data when the host requests audio. This is the
        audio type that will be send whether the host request for encoded
        or raw audio. The SDK may or may not need to encode or decode the
        received audio data.  See @dvr_audio_data_type_e@ for definition
        of supported audio data types.

************************************************************************/
typedef struct dvr_board_2_struct {
    dvr_status_e        status:8;
    sx_uint8            num_enc_streams_per_chan;
    sx_uint8            num_hmo_streams_per_chan;
    dvr_audio_data_type_e audio_data_type:8;
    sx_uint32           reserved2[3];
} dvr_board_2_t;
sx_static_assert(sizeof(dvr_board_2_t) == 16, dvr_board_2_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_JOB@, @DVR_SET_JOB@ and @DVR_REP_JOB@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.  The default is the camera number
        for the encoder job, and a sequentially assigned number for all
        other jobs.

        "control" - Specifies what to do with the job, for example,
        @DVR_JOB_CREATE@.

        "camera_type" - For job type @DVR_JOB_CAMERA_ENCODE@, this
        specifies how to configure the input camera, and must be
        consistent with the information returned by @DVR_GET_BOARD@.
        For job type @DVR_JOB_HOST_DECODE@, this specifies the video
        resolution of the stream to be decoded.

        "audio_format" - Audio CODEC format. Must be one of the valid
        @dvr_ac_format_enum@ values.

        "num_dp_buf" - The number of dataport input buffers to allocate
        for this channel. A larger number of buffers will help maintain
        performance (i.e. reduce frame drops) under instanteneous high
        loading, but will show worse latency for live view (SMO).
        A smaller number of buffers will deliver lower latency but may
        cause frame drops under heavy loading. The valid range is from
        3 to 8. The recommended values are:
        - 3 for best latency
        - 7 for best performance

        @NOTE:@ The default value of "num_dp_buf" is 7. If this field is
        set to zero, the firmware will use 7 buffers.

        @NOTE:@ For boards that perform video stream demuxing in firmware,
        the number of dataport buffers is determined by the firmware. The
        value specified in "num_dp_buf" will be ignored.

        "video_format" - Video CODEC format for primary encoded stream.
        Must be one of the valid @dvr_vc_format_enum@ values.

        "video_format_2" - Video CODEC format for secondary encoded stream.
        Must be one of the valid @dvr_vc_format_enum@ values.

        "encoder_2_pe" - This field specifies the PE on which the codec
        work is to be done, where the first PE is 1.  A value of 0 means
        to use the default as defined by "job_id".
        @NOTE:@ currently, only secondary encoder uses this field.

        "emo_smo_mirror_port" - Only used for job type "DVR_JOB_EMO_ENCODE",
        this field specifies the SMO port to mirror onto an EMO.  Not used
        for legacy (S6) EMOs.  Port number is 1-based as are all SMO ports.

        "width" - Only used for job type "DVR_JOB_HOST_ENCODE".  This field
        specifies the width of a host encoder job.  Eventually this may be
        used to replace camera_type.

        "height" - Only used for job type "DVR_JOB_HOST_ENCODE".  This field
        specifies the height of a host encoder job.  Eventually this may be
        used to replace camera_type.
************************************************************************/
typedef struct dvr_job_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_job_action_e    control:8;
    sx_uint16           camera_type;
    dvr_ac_format_e     audio_format:8;
    sx_uint8            num_dp_buf;
    dvr_vc_format_e     video_format:8;         /* Primary stream   */
    dvr_vc_format_e     video_format_2:8;       /* Secondary stream */
    sx_int8             encoder_2_pe;           /* PE for 2nd encoder */
    sx_uint8            emo_smo_mirror_port;    /* type DVR_JOB_EMO_ENCODE */
    struct {
        sx_uint16       width;
        sx_uint16       height;
    } host_encode_size;
} dvr_job_t;
sx_static_assert(sizeof(dvr_job_t) == 16, dvr_job_t_size);

/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_SMO@ command. These flags
    indicate which action should be taken by the firmware when enabling
    the SMO.

    For backward compatibility reasons, SMO opcodes must not set the
    lower two bits; thus, all SMO opcodes must be multiples of four and
    are shifted left by two.

    @DVR_SMO_OPCODE_ENABLE_VIDEO@ - Enable the SMO channel with the given
    port number to ouput video data.

    @DVR_SMO_OPCODE_DISABLE@ - Disable the specified SMO channel.  Both
    audio and video are disabled, as applicable.  Must be zero for
    backward-compatibility.

    @DVR_SMO_OPCODE_ZOOM@ - Zoom the specified SMO channel according to
    the given position (position_x/y) and size (width/height)
    parameters.  Other parameters are ignored. This opcode will not send
    any response message to the host.

    @DVR_SMO_OPCODE_SIZE_POSITION@ - This message specifies the SMO grid's
    postion according to the given postion (position_x/y),
    size (width/height), and layer (for picture-in-picture)
    parameters. Other parameters are ignored.
    This opcode will not send any response message to the host.

    @DVR_SMO_OPCODE_PAUSE@ - Pause the video for a channel on the SMO.
    This opcode will not send any response message to the host.

    @DVR_SMO_OPCODE_UNPAUSE@ - Unpause the video for a channel on the SMO.
    This opcode will not send any response message to the host.

    @DVR_SMO_OPCODE_FLUSH_QUEUE@ - Clears the video frame queue for the given
    SMO port and the given job type/ID. This is mostly needed for the
    trick playback. This opcode does not return any response back to the host.

    @DVR_SMO_OPCODE_MIRROR_IMG@ - Flip the SMO channel's video horizontally.
************************************************************************/
#define DVR_SMO_OPCODE_ENABLE_VIDEO         (1 << 2)
#define DVR_SMO_OPCODE_ZOOM                 (2 << 2)
#define DVR_SMO_OPCODE_SIZE_POSITION        (3 << 2)
#define DVR_SMO_OPCODE_DISABLE              (0)
#define DVR_SMO_OPCODE_PAUSE                (4 << 2)
#define DVR_SMO_OPCODE_UNPAUSE              (5 << 2)
#define DVR_SMO_OPCODE_FLUSH_QUEUE          (6 << 2)
#define DVR_SMO_OPCODE_MIRROR_IMG           (7 << 2)


/************************************************************************
    VISIBLE: Maximum number of instantiations of a single video tile on
    a specific SMO port.  Must be a power-of-two.

    NOTE: Currently this value limits the maximum number of jobs per
    board in a system to 64 (8-bit field in DVRFW_PORTNUM becomes 6 bits).
************************************************************************/
#define DVR_SMO_MAX_NUM_INSTANCES           (4)


/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SMO@, @DVR_SET_SMO@ and @DVR_REP_SMO@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "port_num" - SMO port number (starting from 1).

        "position_x","position_y" - X and Y coordinates for the top left
        corner of the display image. The display image for this channel
        will be tiled into the SMO display at this position. The top left
        corner of the SMO display is (0,0).
        @NOTE@: X and Y coordinates must be even numbers. If not, the set
        command will fail.

        "resize_at_source" -- Set to (0) if the resizing of encoder/decoder
        output should be done at the PE which performs SMO.
        Set to non-zero if the output of the encoder/decoder should be
        resized on the same PE as where the encoder/decoder job is located.

        "instance_id" - The instantiation ID for this SMO tile.  Non-zero
        only if an camera/playback tile is displayed multiple times on
        the same SMO.  Used in conjunction with "job_type" and "job_id"
        to identify an SMO channel, so this field is required for all
        SMO messages (even disable messages).  Must be between 0 and
        (DVR_SMO_MAX_NUM_INSTANCES - 1), inclusive.

        "opcode" - The SMO command code.  Bits zero and one are reserved
        for backward compatibility and must not be used.

        "layer" - Layer number for this image.  Channels with this field set to
        0 will be placed first, then, channels with this field set to 1 or higher
        will be placed next.

        "width" - Width dimension to resize this grid to.  A value
        of zero will omit resizing.

        "height" - Height dimension to resize this grid to.  A value
        of zero will omit resizing.
************************************************************************/
typedef struct dvr_smo_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            port_num;
    sx_uint16           position_x;
    sx_uint16           position_y;
    sx_uint8            resize_at_source;
    sx_uint8            instance_id;
    sx_uint8            opcode;
    sx_uint8            layer;
    sx_uint16           width;
    sx_uint16           height;
} dvr_smo_t;
sx_static_assert(sizeof(dvr_smo_t) == 16, dvr_smo_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_SET_AOUT@ and @DVR_REP_AOUT@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "port_num" - audio-out port number (starting from 1).

        "enable" - Controls whether to play the raw audio frames received
        from the sct audio-out channel on the specified audio-out port.
        1 enables playing of the audio. 0 disables it.
        If the SMO is already enabled while the audio-out is being enabled
        for the same job ID and type, then the A/V will be synched. Otherwise,
        the audio and video for the same channels will not be synched.

        "check_only" - For internal firmware use only.
************************************************************************/
typedef struct dvr_audioout_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            port_num;
    sx_uint8            enable;
    sx_uint8            check_only;
    sx_uint16           reserved2;
    sx_uint32           reserved3[2];
} dvr_audioout_t;
sx_static_assert(sizeof(dvr_audioout_t) == 16, dvr_audioout_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_HMO@, @DVR_SET_HMO@, and @DVR_REP_HMO@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "output_res" - Must be a valid enum of type @dvr_video_res_e@.
        The image is resized according to this enum before being sent
        to the host. The default is @DVR_VIDEO_RES_QCIF@.

        "frame_rate" - Output frame rate, must be between 1 and the maximum
        allowed by the video standard.

        "stream_id" - The raw video stream identifier. It specifies
        which raw video stream is being queried or enabled.
        Currently, it must be 0 (primary) or 1 (secondary).
************************************************************************/
typedef struct dvr_hmo_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_video_res_e     output_res:8;
    sx_uint8            frame_rate;
    sx_uint8            reserved0;
    sx_uint8            reserved1;
    sx_uint8            stream_id;
    sx_uint32           reserved2;
    sx_uint32           reserved3;
} dvr_hmo_t;
sx_static_assert(sizeof(dvr_hmo_t) == 16, dvr_hmo_t_size);

/************************************************************************
    VISIBLE: Command codes for use with @DVR_SET_EMO@ and @DVR_GET_EMO@.
    The opcodes indicate what action should be taken by the firmware.

    @DVR_EMO_OPCODE_GET_ATTRIB@ - Returns the EMO attributes and its
    capabilities.
************************************************************************/
#define DVR_EMO_OPCODE_GET_ATTRIB   0x1

/************************************************************************
    VISIBLE: This structure is associated with the messages @DVR_GET_EMO@,
    @DVR_SET_EMO@, and @DVR_REP_EMO@.

        "status" - Status of the reply.

        "job_type" - Must be @DVR_JOB_EMO_ENCODE@.

        "job_id" - Zero-based index of the EMO instance.

        "port_num" - The EMO port number. EMO port numbers are shared
        with the SMO port numbers, and start after the SMOs. For example
        if there are two SMOs with port numbers 1 and 2, the first EMO
        port number will be 3.

        "opcode" - The EMO command code.

        "smo_port_mirrored" - The SMO port to be mirrored by this EMO, if
        mirroring is supported.

        "width" - Width of the EMO picture in pixels. Returned in response
        to the @DVR_EMO_OPCODE_GET_ATTRIB@ command.

        "height" - Height of the EMO picture in pixels. Returned in response
        to the @DVR_EMO_OPCODE_GET_ATTRIB@ command.

        "cap_flags" - Bit field indicating the EMO capabilities. Returned
        in response to the @DVR_EMO_OPCODE_GET_ATTRIB@ command.
************************************************************************/
typedef struct dvr_emo_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            port_num;
    sx_uint8            opcode;
    sx_uint8            smo_port_mirrored;
    sx_uint16           width;
    sx_uint16           height;
    sx_uint16           cap_flags;
    sx_uint32           reserved3;
} dvr_emo_t;
sx_static_assert(sizeof(dvr_emo_t) == 16, dvr_emo_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_OSD@, @DVR_SET_OSD@, and @DVR_REP_OSD@.

    NOTE: These messages and the data structure will be obseleted and
    replaces with @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, and @DVR_REP_OSD_EX@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "position" - OSD position, for example, @DVR_OSD_POS_TL@.

        "format" - DTS format, for example, @DVR_OSD_DTS_MDY_12H@.

        "enable" - Set to 1 to turn on and to 0 to turn off OSD.
        OSD configuration is maintained while it is off.
************************************************************************/
typedef struct dvr_osd_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_osd_pos_e       position:8;
    dvr_osd_dts_e       format:8;
    sx_uint8            enable;
    sx_uint8            title[10];
} dvr_osd_t;
sx_static_assert(sizeof(dvr_osd_t) == 16, dvr_osd_t_size);

/************************************************************************
    VISIBLE: Font table related Opcodes.

    @DVR_FONT_OPCODE_START@ - Indicates that the host application is ready
    to start downloading font information for a new font.

    @DVR_FONT_OPCODE_FINISH@ - Indicates that the download is complete
    and the connection should be closed.

    @DVR_FONT_OPCODE_SELECT@ - Indicates that a new font index is to be
    selected.

    @DVR_FONT_OPCODE_BUF_YY@ - Indicates that the download for the font
    bitmap Y buffers is about to start.

    @DVR_FONT_OPCODE_BUF_UU@ - Indicates that the download for the font
    bitmap U buffers is about to start.

    @DVR_FONT_OPCODE_BUF_VV@ - Indicates that the download for the font
    bitmap V buffers is about to start.

    @DVR_FONT_OPCODE_BUF_UTF16@ - Indicates that the download for the
    UTF16 character codes is about to start.

    @DVR_FONT_OPCODE_BUF_INDEX@ - Indicates that the download for the
    character indexes is about to start.

    @DVR_FONT_OPCODE_BUF_WIDTH@ - Indicates that the download for the
    character width values is about to start.

    @DVR_FONT_OPCODE_COPY@ - This opcode is used internally by the
    firmware.

    @DVR_FONT_OPCODE_COPY_DONE@ - This opcode is used internally by
    the firmware.

    @DVR_FONT_OPCODE_YUV@ - The Y, U, and V color settings are being
    passed in.
************************************************************************/
#define DVR_FONT_OPCODE_START           1
#define DVR_FONT_OPCODE_FINISH          2
#define DVR_FONT_OPCODE_SELECT          3
#define DVR_FONT_OPCODE_BUF_YY          4
#define DVR_FONT_OPCODE_BUF_UU          5
#define DVR_FONT_OPCODE_BUF_VV          6
#define DVR_FONT_OPCODE_BUF_UTF16       7
#define DVR_FONT_OPCODE_BUF_INDEX       8
#define DVR_FONT_OPCODE_BUF_WIDTH       9
#define DVR_FONT_OPCODE_COPY            10
#define DVR_FONT_OPCODE_COPY_DONE       11
#define DVR_FONT_OPCODE_YUV             12

/************************************************************************
    VISIBLE: Stretch DVR pre-loaded font tables.

    @DVR_FONT_ENGLISH@ - Default font for English (ASCII) character set.
************************************************************************/
#define DVR_FONT_ENGLISH                0

/************************************************************************
    VISIBLE: The format of the font table (i.e. bmp 4-2-0).

    @DVR_FONT_FORMAT_YUV_4_2_0@ - The font table is constructed as YUV
    4-2-0 format. Currently, this is the only supported format.
************************************************************************/
#define DVR_FONT_FORMAT_YUV_4_2_0       1

/************************************************************************
    VISIBLE: This structure is associated with the @DVR_SET_FONT_TABLE@
    and @DVR_REP_FONT_TABLE@ messages.

        "status" - Status of the reply.

        "opcode" - The code defining the operation to be performed.
        Always set by the host.

        "font_index" - Index of the font being downloaded / selected.
        Index 0 is the default font that is built into the firmware.
        Values 0-7 are reserved by Stretch. User fonts must have index
        values between 8 and 15.

        "font_format" - The format of the font table that is being set.
        Currently @DVR_FT_FORMAT_BMP_4_2_0@ is the only supported format.
        This field is assumed to be set by the host only when the opcode
        is @DVR_FONT_OPCODE_START@.

        "port_id" - The SCT port ID that should be used by the firmware
        to download the font data. This field is assumed to be set by the
        host only when the opcode is @DVR_FONT_OPCODE_START@.

        "osdheight" - The height of all the characters in the font table.
        Set only when the op_code is @DVR_FONT_OPCODE_START@.

        "osdnglyph" - The number of characters (glyphs) that are to be
        downloaded. Set only when the opcode is @DVR_FONT_OPCODE_START@.

        "osdyysize" - The size of the buffer required to store all the
        Y fields of the YUV character bitmaps. Set only when the opcode
        is @DVR_FONT_OPCODE_START@.

        "bufsize" - The size of the buffer being sent. Set only if the
        opcode is one of the "DVR_FONT_OPCODE_BUF_XXX" codes.

        "color_y" - The Y component of the OSD text color value.

        "color_u" - The U component of the OSD text color value.

        "color_v" - The V component of the OSD text color value.

************************************************************************/
typedef struct dvr_font_table_struct {
    dvr_status_e        status:8;
    sx_uint8            opcode;
    sx_uint8            font_index;
    sx_uint8            font_format;
    sx_uint16           port_id;
    sx_uint16           osdheight;
    sx_uint16           osdnglyph;
    sx_uint16           reserved1;
    union {
        sx_uint32       osdyysize;
        sx_uint32       bufsize;
        struct dvr_font_yuv_struct {
            sx_uint8    color_y;
            sx_uint8    color_u;
            sx_uint8    color_v;
            sx_uint8    pad;
        } font_yuv;
    } u1;
} dvr_font_table_t;
sx_static_assert(sizeof(dvr_font_table_t) == 16, dvr_font_table_t_size);

/************************************************************************
    VISIBLE: OSD commands Opcodes.

    @DVR_OSD_OPCODE_CONFIG@ - OSD opcode to configure displaying of
    each OSD line.

    @DVR_OSD_OPCODE_SHOW@   - OSD opcode to set the show status of
    each of the OSD line.

    @DVR_OSD_OPCODE_TEXT@   - OSD opcode to set the text for each
    one of the OSD line. This opcode is invalid in @DVR_GET_OSD_EX@.
    NOTE: It is possible to receive multiple @DVR_OSD_OPCODE_TEXT@
    messages for large texts.

    @DVR_OSD_OPCODE_CLEAR_TEXT@ - OSD opcode to remove the OSD text.

    @DVR_OSD_OPCODE_SHOW_ALL@ - OSD opcode to show all lines of OSD.

    @DVR_OSD_OPCODE_BLINK@ - OSD opcode to enable/disable blink mode.
************************************************************************/
#define DVR_OSD_OPCODE_CONFIG     1
#define DVR_OSD_OPCODE_SHOW       2
#define DVR_OSD_OPCODE_TEXT       3
#define DVR_OSD_OPCODE_CLEAR_TEXT 4
#define DVR_OSD_OPCODE_SHOW_ALL   5
#define DVR_OSD_OPCODE_BLINK      6

/************************************************************************
    VISIBLE: OSD blink flags.

    @DVR_OSD_BLINK_ON_MD_ALARM@ - make the OSD blink only when the motion
    detection alarm is triggered.

    @DVR_OSD_BLINK_ON_ND_ALARM@ - make the OSD blink only when the night
    detection alarm is triggered.

    @DVR_OSD_BLINK_ON_BD_ALARM@ - make the OSD blink only when the blind
    detection alarm is triggered.

    @DVR_OSD_BLINK_ON_VIDEO_LOSS@ - make the OSD blink only when video
    input is lost.

    @DVR_OSD_BLINK_ALWAYS@ - make the OSD blink always

    @DVR_OSD_BLINK_FORCE@ - internal use only.

    The above flags can be ORed together. Note that motion, night and
    blind detection alarms must be enabled for the corresponding flags
    to take effect. Video loss detection is always enabled.

    If no flags are specified, then blinking is controlled by the show
    state of the OSD, i.e. the text will blink as long as it is shown.
************************************************************************/
#define DVR_OSD_BLINK_ON_MD_ALARM      0x01
#define DVR_OSD_BLINK_ON_ND_ALARM      0x02
#define DVR_OSD_BLINK_ON_BD_ALARM      0x04
#define DVR_OSD_BLINK_ON_VIDEO_LOSS    0x08
#define DVR_OSD_BLINK_ALWAYS           0x10
#define DVR_OSD_BLINK_FORCE            0x10000000


/************************************************************************
    VISIBLE: Maximum OSD text length.
************************************************************************/
#define DVRFW_OSD_LEN             100 /* does not include DTS text */

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, and @DVR_REP_OSD_EX@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "op_code" - The currently selected operation on the
        given OSD line ID.

        "config" - This union is associated to opcode
        @DVR_OSD_OPCODE_CONFIG@ in order to configure one
        OSD item.

            "osd_id"  - The OSD identifier to be configured.
            id zero correspond to the 1st OSD, 1 to the 2nd, etc.
            Currently up to 5 OSDs can be defined.

            "translucent_high" - This field specifies the high value
            of the background threshold for Y.

            "translucent_low" - This field will be a part of a future
            enhancement.  It specifies the low value of the background
            threshold for Y.

            "postion_ctrl" - You can select the location of OSD
            text to be displayed using of the pre-define location in
            @dvr_osd_pos_enum@ or specify a custom define location.

            "x_TL", "y_TL" - The top left coordinates of the OSD
            test if the custom @postion_ctrl@ is specified, otherwise
            these fields are ignored.

            "dts_format" - The format of date/time to optionally
            be appended to the end end of OSD text.

            "smo_port" - Setting this field to 0xFF causes the OSD settings to
            apply to HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD setting to apply to HMO and encoder
            but not any of the sport monitor displays.
            Otherwise, the setting applies to the specific spot monitor displays.

        "show" - This union is associated with opcodes @DVR_OSD_OPCODE_SHOW@ and
        @DVR_OSD_OPCODE_SHOW_ALL@. These control the visibility of the OSD lines
        in the picture.

            "osd_id"  - The OSD identifier to set its show state. This field is
            ignored by @DVR_OSD_OPCODE_SHOW_ALL@.

            "state"  - Show state of the currently selected OSD id.
            zero means to hide. Otherwise, show the osd text.

            "smo_port" - Setting this field to 0xFF causes the OSD text to
            be shown or hidden on HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD text to be shown or hidden on HMO and encoder
            but not any of the sport monitor displays.
            Otherwise, the OSD text will be shown or hidden on the specific spot monitor
            displays.

        "text" - This union is associated to opcode
        @DVR_OSD_OPCODE_TEXT@ in order to set the OSD text of the given OSD
        ID for the current job. The OSD text is a unicode short string.
        It can be divided into multiple of 4 text and sent to the
        firmware.

            "osd_id"  - The OSD identifier to set its display text.

            "length" - The total length of OSD display text.
            The maximum OSD display text size is 256.

            "data[4]" - Array of unsigned short unicode containing the
            nth 4 string of the OSD text. The maximum size of an OSD
            text is 256 unsigned short.

            "smo_port" - Setting this field to 0xFF causes the OSD text to
            be changes on HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD text to be changed on HMO and encoder
            but not any of the spot monitor displays.
            Otherwise, the OSD text will be changed on the specific spot monitor
            displays.

        "blink" - This union is used by the @DVR_OSD_OPCODE_BLINK@ opcode. This
        opcode is used to enable/disable blinking display and to set the blink
        interval.

            "osd_id" - The OSD identifier to select the line affected.

            "on_frames" - OSD on period specified in frames.

            "off_frames" - OSD off period specified in frames.

            "flags" - Blink control flags. Currently only @DVR_OSD_BLINK_ON_ALARM@
            is supported.

        @NOTE:@ The actual display is controlled by the "show" and "show_all" commands.
        If the show state is disabled, setting the blink parameters will have no effect
        until the show state is enabled. Blinking is disabled by setting both the on
        and off times to zero.

        @NOTE:@ Blinking OSD is supported only on camera OSD text. It is not supported
        on SMO OSD text or on decode channels.
************************************************************************/
typedef struct dvr_osd_ex_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;

    union {
        struct {
            sx_uint8    osd_id;
            sx_uint8    translucent_high;
            sx_uint8    position_ctrl;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint8    dts_format;
            sx_uint8    smo_port;
            sx_uint8    translucent_low;
            sx_uint8    reserved1;
        } config;
        struct {
            sx_uint8    osd_id;
            sx_uint8    state;
            sx_uint8    smo_port;
            sx_uint8    reserved1;
            sx_uint32   reserved2;
            sx_uint32   reserved3;
        } show;
        struct {
            sx_uint8    osd_id;
            sx_uint8    length;
            sx_uint16   data[4];
            sx_uint8    smo_port;
            sx_uint8    reserved1;
        } text;
        /* The layout of text2 and text common fields must match exactly */
        struct {
            sx_uint8    osd_id;
            sx_uint8    length;
            sx_uint16   reserved1;
            sx_uint32   ptr;
            sx_uint16   reserved2;
            sx_uint8    smo_port;
            sx_uint8    reserved3;
        } text2;
        struct {
            sx_uint8    osd_id;
            sx_uint8    on_frames;
            sx_uint8    off_frames;
            sx_uint8    reserved1;
            sx_uint32   flags;
            sx_uint32   reserved2;
        } blink;
    } u1;
} dvr_osd_ex_t;
sx_static_assert(sizeof(dvr_osd_ex_t) == 16, dvr_osd_ex_t_size);

/************************************************************************
    VISIBLE: FOSD commands Opcodes.

    @DVR_FOSD_OPCODE_CAP@ -  FOSD opcode to query the capability of the FOSD of a job.
    This opcode is invalid in @DVR_SET_OSD_EX@.

    @DVR_FOSD_OPCODE_SPEC@ - FOSD opcode to specify the FOSD of a job.

    @DVR_FOSD_OPCODE_DESTROY@ - FOSD opcode to destroy FOSD in firmware.
    This opcode is invalid in @DVR_GET_OSD_EX@.

    @DVR_FOSD_OPCODE_SHOW_ALL@ - FOSD opcode to show all lines of OSD of a stream.
    This opcode is invalid in @DVR_GET_OSD_EX@.

    @DVR_FOSD_OPCODE_CONFIG@ - FOSD opcode to configure displaying of an OSD.

    @DVR_FOSD_OPCODE_POSITION@ - FOSD opcode to set the position of an OSD.

    @DVR_FOSD_OPCODE_TEXT@ - FOSD opcode to set the text of an OSD.
    This opcode is invalid in @DVR_GET_OSD_EX@.

    @DVR_FOSD_OPCODE_GRAPHIC@ - FOSD opcode to set the graphic of an OSD.
    This opcode is not available at the current release.
    This opcode is invalid in @DVR_GET_OSD_EX@.

    @DVR_FOSD_OPCODE_CLEAR_TEXT@ - FOSD opcode to remove the text of an OSD.
    This opcode is invalid in @DVR_GET_OSD_EX@.

    @DVR_FOSD_OPCODE_SHOW@ - FOSD opcode to set the show status of an OSD.

    @DVR_FOSD_OPCODE_BLINK@ - FOSD opcode to enable/disable blink mode of an OSD.

    @DVR_FOSD_OPCODE_COLOR@ - FOSD opcode to set the color of an OSD.

************************************************************************/
typedef enum dvr_fosd_opcode_num {
    DVR_FOSD_OPCODE_BASE        = 0x80,
    DVR_FOSD_OPCODE_CAP         = DVR_FOSD_OPCODE_BASE,
    DVR_FOSD_OPCODE_SPEC,
    DVR_FOSD_OPCODE_DESTROY,
    DVR_FOSD_OPCODE_SHOW_ALL,
    DVR_FOSD_OPCODE_CONFIG,
    DVR_FOSD_OPCODE_POSITION,
    DVR_FOSD_OPCODE_TEXT,
    DVR_FOSD_OPCODE_GRAPHIC,
    DVR_FOSD_OPCODE_CLEAR_TEXT,
    DVR_FOSD_OPCODE_SHOW,
    DVR_FOSD_OPCODE_BLINK,
    DVR_FOSD_OPCODE_COLOR,
} dvr_fosd_opcode_e;

/************************************************************************
    VISIBLE: FOSD module id of an osd id

    @DVR_FOSD_MODULE_ID_ENC@ - this osd applies to one of encoder streams

    @DVR_FOSD_MODULE_ID_HMO@ - this osd applies to HMO

    @DVR_FOSD_MODULE_ID_SMO@ - this osd applies to SMO

    @DVR_FOSD_MODULE_ID_EMO@ - this osd applies to EMO

************************************************************************/
typedef enum {
    DVR_FOSD_MODULE_ID_ENC,
    DVR_FOSD_MODULE_ID_HMO,
    DVR_FOSD_MODULE_ID_SMO,
    DVR_FOSD_MODULE_ID_EMO,
    DVR_FOSD_MODULE_ID_NUM,
} dvr_fosd_module_id_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode
    @DVR_OSD_OPCODE_CAP@ and @DVR_OSD_OPCODE_SPEC@.

        @num_enc@ - the number of streams for an encoder job.
        It is zero for a decoder job.

        @num_enc_osd@ - the number of OSD lines for a encoder stream.

        @num_hmo@ - the number of HMO.

        @num_hmo_osd@ - the number of OSD lines for an HMO.

        @num_smo@ - the number of SMO.

        @num_smo_osd@ - the number of OSD lines for an SMO.

        @num_emo@ - the number of EMO.

        @num_emo_osd@ - the number of OSD lines for an EMO.

        @max_width@ - the maximum width of an OSD line in pixel

        @max_height@ -the maximum height of an OSD line in pixel

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
    sx_uint16       max_width;
    sx_uint16       max_height;
} dvr_fosd_msg_cap_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "dts_format" - The format of date/time to optionally be appended
        to the end end of OSD text.

        "position_ctrl" - You can select the location of OSD
        text to be displayed using of the pre-define location in
        @dvr_osd_pos_enum@ or specify a custom define location.

        "pos_x", "pos_y" - The top left coordinates of the OSD line
        if the custom @position_ctrl@ is specified;
        otherwise these fields are ignored.

        "max_width", "max_height" - the maximum width and height of
        this OSD line in pixel.
        They can not be larger than those defined at @dvr_fosd_msg_cap_t@;

    Remarks:

        @max_width@ and @max_height@ are not supported yet.
        They will be used to reduce the memory for the bitmap of an OSD line.

************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    dvr_osd_dts_e   dts_format:8;
    dvr_osd_pos_e   position_ctrl:8;
    sx_uint16       pos_x;
    sx_uint16       pos_y;
    sx_uint16       max_width;
    sx_uint16       max_height;
} dvr_fosd_msg_config_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "font_table_id" - the font table id of this OSD text line

        "length" - The total length of OSD display text.
        The maximum OSD display text size is 256.

        @NOTE:@ The text data is sent via data buffer.
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint8        font_table_id;
    sx_uint8        length;
    sx_uint32       data[2];
} dvr_fosd_msg_text_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "width", "height" - The width and height of the graphic bitmap in pixel.

        @NOTE:@ The bitmap is sent via data buffer.
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint16       width;
    sx_uint16       height;
    sx_uint16       data[3];
} dvr_fosd_msg_graphic_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "enable" - show (enable) or hid (disable) of an OSD.
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint8        enable; /*  state */
    sx_uint8        reserved1;
    sx_uint32       reserved2;
    sx_uint32       reserved3;
} dvr_fosd_msg_show_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "on_frames" - OSD on period specified in frames.

        "off_frames" - OSD off period specified in frames.

        "flags" - Blink control flags.
        Currently only @DVR_OSD_BLINK_ON_ALARM@ is supported.
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint8        on_frames;
    sx_uint8        off_frames;
    sx_uint32       flags;
    sx_uint32       reserved1;
} dvr_fosd_msg_blink_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "color_y", "color_u", "color_v" -
        The Y, U, and V components of an OSD color

        "color_high_y", "color_high_u", "color_high_v" -
        The Y, U, and V components of an OSD transluent color

        "color_low_y", "color_low_u", "color_low_v" -
        The Y, U, and V components of an OSD transluent color
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint8        color_y;
    sx_uint8        color_u;
    sx_uint8        color_v;
    sx_uint8        color_high_y;
    sx_uint8        color_high_u;
    sx_uint8        color_high_v;
    sx_uint8        color_low_y;
    sx_uint8        color_low_u;
    sx_uint8        color_low_v;
    sx_uint8        reserved1;
} dvr_fosd_msg_color_t;

/************************************************************************
    VISIBLE: This structure is associated with the opcode @DVR_OSD_OPCODE_CONFIG@.

        "osd_id" - The OSD identifier of this message.

        "position_ctrl" - You can select the location of OSD
        text to be displayed using of the pre-define location in
        @dvr_osd_pos_enum@ or specify a custom define location.

        "pos_x", "pos_y" - The top left coordinates of the OSD line
        if the custom @position_ctrl@ is specified;
        otherwise these fields are ignored.
************************************************************************/
typedef struct {
    sx_uint16       osd_id;
    sx_uint8        reserved1;
    dvr_osd_pos_e   position_ctrl:8;
    sx_uint16       pos_x;
    sx_uint16       pos_y;
    sx_uint16       reserved2;
    sx_uint16       reserved3;
} dvr_fosd_msg_position_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, and @DVR_REP_OSD_EX@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "op_code" - The currently selected operation on the
        given OSD line ID.

        "cap" - This union is associated to opcode @DVR_OSD_OPCODE_CAP@
        and is of type @dvr_fosd_msg_cap_t@.

        "spec" - This union is associated to opcode @DVR_OSD_OPCODE_SPEC@
        and is of type @dvr_fosd_msg_cap_t@.

        "config" - This union is associated to opcode @DVR_OSD_OPCODE_CONFIG@
        and is of type @dvr_fosd_msg_config_t@.

        "text" - This union is associated to opcode @DVR_OSD_OPCODE_TEXT@
        and is of type @dvr_fosd_msg_text_t@.

        "graphic" - This union is associated to opcode @DVR_OSD_OPCODE_GRAPHIC@
        and is of type @dvr_fosd_msg_graphic_t@.

        "show" - This union is associated to opcode @DVR_OSD_OPCODE_SHOW@
        and is of type @dvr_fosd_msg_show_t@.

        "blink" - This union is associated to opcode @DVR_OSD_OPCODE_BLINK@
        and is of type @dvr_fosd_msg_blink_t@.

        "color" - This union is associated to opcode @DVR_OSD_OPCODE_COLOR@
        and is of type @dvr_fosd_msg_color_t@.

        "position" - This union is associated to opcode @DVR_OSD_OPCODE_POSITION@
        and is of type @dvr_fosd_msg_position_t@.

        "config" - This union is associated to opcode
        @DVR_OSD_OPCODE_CONFIG@ in order to configure one
        OSD item.

        @NOTE:@ The actual display is controlled by the "show" and "show_all" commands.
        If the show state is disabled, setting the blink parameters will have no effect
        until the show state is enabled. Blinking is disabled by setting both the on
        and off times to zero.

        @NOTE:@ Blinking OSD is supported only on camera OSD text. It is not supported
        on SMO OSD text or on decode channels.
************************************************************************/
typedef struct dvr_fosd_ex_struct {
    dvr_status_e    status:8;
    dvr_job_type_e  job_type:8;
    sx_uint8        job_id;
    sx_uint8        op_code;

    union {
        dvr_fosd_msg_cap_t      cap;
        dvr_fosd_msg_cap_t      spec;
        dvr_fosd_msg_config_t   config;
        dvr_fosd_msg_text_t     text;
        dvr_fosd_msg_graphic_t  graphic;
        dvr_fosd_msg_show_t     show;
        dvr_fosd_msg_blink_t    blink;
        dvr_fosd_msg_color_t    color;
        dvr_fosd_msg_position_t position;
    } u1;
} dvr_fosd_msg_t;
sx_static_assert(sizeof(dvr_fosd_msg_t) == 16, dvr_fosd_msg_t_size);

/************************************************************************
    Get module id from an osd id
************************************************************************/
#define DVR_FOSD_MODULE_ID(osd_id)      (((osd_id) >> 12) & 0x0F)

/************************************************************************
    Get stream id from an osd id
************************************************************************/
#define DVR_FOSD_STREAM_ID(osd_id)      (((osd_id) >> 8) & 0x0F)

/************************************************************************
    Get osd text id from an osd id
************************************************************************/
#define DVR_FOSD_TEXT_ID(osd_id)        ((osd_id) & 0xFF)

/************************************************************************
    Set an osd id by moduel id, stream id, and osd text id

    Set an OSD id by module id, stream id, and osd text id

    @mid@ - module ID specifies the place to apply OSD.
    Its valid values are defined at @dvr_fosd_module_id_t@.

    @sid@ - stream ID specifies the stream to apply OSD.
    Its valid values are specified by the @spec@ of dvr_fosd_msg_t.
    For example, a camera encoder job supports 2 stream encodings,
    2 HMO outputs, and 2 SMO outputs (ports).
    The ranges of the stream IDs for encoder, HMO output, and SMO output are 0 to 1.
    For example, an IP camera job supports 4 stream encodings,
    0 HMO output, and 1 SMO output (port).
    The range of the stream IDs for encoder is 0 to 3.
    The valid value of the stream IDs for SMO output is 0.

    @tid@ - text ID specifies the OSD line to apply OSD.
    Its valid values are specified by the @spec@ of dvr_fosd_msg_t.
    For example, the valid range is 0 to 4, if the number of OSDs is 5.
************************************************************************/
#define DVR_FOSD_OSD_ID(mid, sid, tid)  ((((mid)&0x0F) << 12) | (((sid)&0x0F) << 8) | (((tid)&0xFF)))

/************************************************************************
    Defines the length of the authentication key in bytes.
************************************************************************/
#define DVR_AUTH_KEY_LEN    (32)

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_AUTH_KEY@, @DVR_REP_AUTH_KEY@.

        "status" - Status of the reply.

        "board_id" - The board identifier.

        "read_offset" - For the GET command, this is offset of the key
        to read from. For example, setting this to zero is a command to
        read the first 12 bytes of the key.

        "read_length" - For the REP command, this specifies the number
        of bytes actually copied into the data field. This value will
        always be less than or equal to 12.

        "total_length" - For the REP command, this specifies the total
        length of the key in bytes. Currently the maximum key length
        is 256 bits, or 32 bytes. For the GET command, this field is
        unused.

        "data" - Array of unsigned chars in which the key fragments
        are returned.
************************************************************************/
typedef struct dvr_auth_key_struct {
    dvr_status_e    status:8;
    sx_uint8        board_id;

    union {
        sx_uint8    read_offset;
        sx_uint8    read_length;
    } u1;

    sx_uint8        total_length;
    sx_uint8        data[12];
} dvr_auth_key_t;
sx_static_assert(sizeof(dvr_auth_key_t) == 16, dvr_auth_key_t_size);

/************************************************************************
    VISIBLE: Region map related opcodes.

    @DVR_REGION_OPCODE_START@ - Indicates that the host application is
    ready to start downloading the region map for the given region type.
    The data is downloaded through the control data channel (opened via
    an IOCTL command). Each buffer size N bytes where N is the number of
    16x16 macroblocks in the current D1 picture frame. When transferring
    the regions map, each bit in the region map index corresponding to a
    region layer for the region type. The layer bit map start from the
    right to left, where the most rightmost bit indicates the first layer.

    @DVR_REGION_OPCODE_FINISH@ - Indicates that the download is complete.

    @DVR_REGION_OPCODE_CONFIG@ - Indicates the we are configuring the
    region by setting one or more alarm thresholds for the region type.

    @DVR_REGION_OPCODE_ENABLE@ - Used to enable or disable the specified
    alarm, or privacy blocking.
************************************************************************/
#define DVR_REGION_OPCODE_START           1
#define DVR_REGION_OPCODE_FINISH          2
#define DVR_REGION_OPCODE_CONFIG          3
#define DVR_REGION_OPCODE_ENABLE          4

/************************************************************************
    VISIBLE: Region type related Opcodes.

    @DVR_REGION_TYPE_MD@ - Indicates motion detection regions.

    @DVR_REGION_TYPE_BD@ - Indicates blind detection regions.

    @DVR_REGION_TYPE_ND@ - Indicates night detection regions.

    @DVR_REGION_TYPE_PR@ - Indicates privacy blocking regions.

************************************************************************/
#define DVR_REGION_TYPE_MD           1
#define DVR_REGION_TYPE_BD           2
#define DVR_REGION_TYPE_ND           3
#define DVR_REGION_TYPE_PR           4

/************************************************************************
    VISIBLE: Maximum size of a region map in bytes for a 1080i or 1080p picture.
************************************************************************/
#define DVR_REGION_MAP_MAX_SIZE      8192

/************************************************************************
    VISIBLE: This structure is associated with the @DVR_SET_REGIONS_MAP@
    and @DVR_REP_REGIONS_MAP@ messages. It is used to send a different
    regions map for alarm detection and privacy blocking, as well as,
    configuring them.

    Common Fields:

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "opcode" - The code defining the operation to be performed.
        Always set by the host.

        "region_type" - The region map type to be affected by the opcode.

    Config Command Fields:

        "threshold1" - Alarm threshold for overlay number 1 of the specified
        region type.

        "threshold2" - Alarm threshold for overlay number 1 of the specified
        region type.

        "threshold3" - Alarm threshold for overlay number 1 of the specified
        region type.

        "threshold4" - Alarm threshold for overlay number 1 of the specified
        region type.

        @NOTE@: Currently, overlay numbers 2-4 are valid only for region type
        @DVR_REGION_TYPE_MD@. For all others, these are ignored.

    Enable Command Fields:

        "enable_flag" - Set to 1 to enable alarm detection, set to zero to
        disable alarm detection.

    Map Command Fields:

        "map_size" - The size of the regions map. This size is always the
        same as the D1 size video frame for the current video standard.
        This field must be set by the host for @DVR_REGION_OPCODE_START@
        and @DVR_REGION_OPCODE_FINISH@.

        "overlay_num" - The region overlay number to set its ROI map.
        Value of zero (0) in this field means the ROI map which is being
        sent includes all the overlay ROI for the current region type.
        Otherwise, it is the nth overlay number.
        For region_type of DVR_REGION_TYPE_MD: the valid
        range is 0 - 4. For all other region_type the valid value is 0 or 1.
        This field must be set by the host for @DVR_REGION_OPCODE_START@
        and @DVR_REGION_OPCODE_FINISH@.
************************************************************************/
typedef struct dvr_regions_map_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            opcode;
    sx_uint8            region_type;
    sx_uint8            reserved1;
    sx_uint16           reserved2;

    union {
        struct {
            sx_uint8    threshold1;
            sx_uint8    threshold2;
            sx_uint8    threshold3;
            sx_uint8    threshold4;
            sx_uint32   reserved3;
        } config;
        struct {
            sx_uint8    enable_flag;
            sx_uint8    reserved3;
            sx_uint16   reserved4;
            sx_uint32   reserved5;
        } enable;
        struct {
            sx_uint16   map_size;
            sx_uint8    overlay_num;
            sx_uint8    reserved3;
            sx_uint32   reserved4;
        } map;
    } u1;
} dvr_regions_map_t;
sx_static_assert(sizeof(dvr_regions_map_t) == 16, dvr_regions_map_t_size);

/************************************************************************
    VISIBLE: Opcodes for commands that support regions.

    @DVR_VPP_OPCODE_CONTROL@   Opcode for control messages.

    @DVR_VPP_OPCODE_REGIONS@   Opcode for region messages.
************************************************************************/
#define DVR_VPP_OPCODE_CONTROL  0
#define DVR_VPP_OPCODE_REGIONS  1

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_PR@, @DVR_SET_PR@, and @DVR_REP_PR@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to 1 to enable and to 0 to disable motion detection.
        The default is 0.

        "num_region" - Number of private regions. A value of 1 means only
        blank out the first region. A value of 2 means blank out
        both regions.

        "x00,y00,x01,y01" - First rectangular region to be blanked
        for privacy.

        "x10,y10,x11,y11" - Second rectangular region to be blanked
        for privacy.
************************************************************************/
typedef struct dvr_pr_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;

    union {
        struct {
            sx_uint8    enable;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_pr_t;
sx_static_assert(sizeof(dvr_pr_t) == 16, dvr_pr_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_MD@, @DVR_SET_MD@, and @DVR_REP_MD@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "threshold" - Motion detection threshold.  The valid range is
        0 - 99. Default is 15.

        "enable" - Set to 1 to enable and to 0 to disable motion detection.

        "num_region" - Number of regions to be used.  0
        implies the whole image, 1 uses the first region,
        and 2 uses both regions.

        "x00,y00,x01,y01" - First rectangular region to be used
        for detecting motion.

        "x10,y10,x11,y11" - Second rectangular region to be used
        for detecting motion.
************************************************************************/
typedef struct dvr_md_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;

    union {
        struct {
            sx_uint8    enable;
            sx_uint8    threshold;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_md_t;
sx_static_assert(sizeof(dvr_md_t) == 16, dvr_md_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_BD@, @DVR_SET_BD@, and @DVR_REP_BD@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "threshold" - Blind detection threshold.  The valid range is
        0 - 99. Default is 50.

        "enable" - Set to 1 to enable and to 0 to disable blind detection.

        "num_region" - Number of regions to be used.  0
        implies the whole image, 1 uses the first region,
        and 2 uses both regions.

        "x00,y00,x01,y01" - First rectangular region to be used
        for blind detection.

        "x10,y10,x11,y11" - Second rectangular region to be used
        for blind detection.
************************************************************************/
typedef struct dvr_bd_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;

    union {
        struct {
            sx_uint8    enable;
            sx_uint8    threshold;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_bd_t;
sx_static_assert(sizeof(dvr_bd_t) == 16, dvr_bd_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_ND@, @DVR_SET_ND@, and @DVR_REP_ND@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to 1 to enable and to 0 to disable night detection.
        The default is 0.

        "threshold" - Night detection threshold. The valid range is
        0 - 255. Default is 40.
************************************************************************/
typedef struct dvr_nd_struct {
    dvr_status_e   status:8;
    sx_uint8  job_type;
    sx_uint8  job_id;
    sx_uint8  enable;
    sx_uint8  threshold;
    sx_uint8  reserved0;
    sx_uint16 reserved1;
    sx_uint32 reserved2;
    sx_uint32 reserved3;
} dvr_nd_t;
sx_static_assert(sizeof(dvr_nd_t) == 16, dvr_nd_t_size);

/************************************************************************
    VISIBLE: Opcodes for the @DVR_SET_ENABLE@ and @DVR_GET_ENABLE@ commands.

    @DVR_ENABLE_HMO@ - Set this flag to enable HMO.

    @DVR_ENABLE_ENC@ - Set this flag to enable encoding.

    @DVR_ENABLE_DEC@ - Set this flag to enable decoding.

    @DVR_ENABLE_AUDIO@ - Set this flag to enable audio.
************************************************************************/
#define DVR_ENABLE_HMO        0x01
#define DVR_ENABLE_ENC        0x02
#define DVR_ENABLE_DEC        0x04
#define DVR_ENABLE_AUDIO      0x08

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_ENABLE@, @DVR_SET_ENABLE@, and @DVR_REP_ENABLE@.

    "status" - Status of the reply. This field is only valid for
    a @DVR_REP_ENABLE@ message.

    "job_type" - Must be one of the supported job types.

    "job_id" - A unique job ID.

    "opcode" - A command code specifying the action to take.

    "enable" - If non-zero, enable the stream. If zero, disable the stream.

    "hmo_sid" - The stream identifier for HMO. Specifies which HMO
    stream is being queried or configured. The number of supported
    streams varies by product. For decode channels, this field is
    currently ignored.

    "enc_sid" - The stream identifier for encode. Specifies which encode
    stream is being queried or configured. The number of supported
    streams varies by product. For decode channels, this field is
    ignored.

    "dec_sid" - The stream identifier for decode. Specifies which decode
    stream is being queried or configured. The number of supported
    streams varies by product. For encode channels, this field is
    ignored.

    @NOTE@: If the opcode is @DVR_ENABLE_AUDIO@ then the stream IDs are
    ignored. This opcode is used only for raw audio streaming (HMO audio)
    so no stream ID is necessary.
************************************************************************/
typedef struct dvr_enable_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            opcode;
    sx_uint8            enable;
    sx_uint8            hmo_sid;
    sx_uint8            enc_sid;
    sx_uint8            dec_sid;
    sx_uint32           reserved1;
    sx_uint32           reserved2;
} dvr_enable_t;
sx_static_assert(sizeof(dvr_enable_t) == 16, dvr_enable_t_size);

/****************************************************************************
    VISIBLE: The following enum describes bit definitions for the encoder
    flags field.

    @DVR_ENC_FLAGS_FLUSH_BUF@ - If bit 0 is set, the encoder should flush
    its buffers immediately.

    @DVR_ENC_FLAGS_ENABLE_SKIP_FRAMES@ - If bit 1 is set, the encoder will
    generate skip frames.
****************************************************************************/
enum dvr_enc_flags_e {
    DVR_ENC_FLAGS_FLUSH_BUF             = 1,
    DVR_ENC_FLAGS_ENABLE_SKIP_FRAMES    = 2,
};

/****************************************************************************
    VISIBLE: H.264 encoding mode types.

    For S6 firmware, the mode setting controls the target quality level.
    @DVR_H264_ENC_MODE_0@ is the default. See additional documentation for
    the meaning of the various custom modes.

    For S7 firmware, the mode setting controls the encoding profile. The
    choices are

    @DVR_H264_ENC_PROFILE_BASE@ - Use H.264 baseline profile.

    @DVR_H264_ENC_PROFILE_MAIN@ - Use H.264 main profile with default (CABAC).

    @DVR_H264_ENC_PROFILE_HIGH@ - Use H.264 high profile with default (CABAC).

    @DVR_H264_ENC_PROFILE_MAIN_CABAC@ - Use H.264 main profile with CABAC.

    @DVR_H264_ENC_PROFILE_HIGH_CABAC@ - Use H.264 high profile with CABAC.

    @DVR_H264_ENC_PROFILE_MAIN_CAVLC@ - Use H.264 main profile with CAVLC.

    @DVR_H264_ENC_PROFILE_HIGH_CAVLC@ - Use H.264 high profile with CAVLC.

    Not all profiles may be supported on all boards. If this field is set
    to @DVR_H264_ENC_MODE_0@, the default profile setting for the board is
    used.
****************************************************************************/
enum dvr_h264_encoder_mode_enum {
    DVR_H264_ENC_MODE_0 = 0,
    DVR_H264_ENC_MODE_1,
    DVR_H264_ENC_MODE_2,
    DVR_H264_ENC_MODE_3,
    DVR_H264_ENC_MODE_4,
    DVR_H264_ENC_MODE_5,
    DVR_H264_ENC_MODE_6,
    /* Following applicable to S7 only */
    DVR_H264_ENC_PROFILE_BASE = 50,
    DVR_H264_ENC_PROFILE_MAIN,
    DVR_H264_ENC_PROFILE_HIGH,
    DVR_H264_ENC_PROFILE_MAIN_CABAC,
    DVR_H264_ENC_PROFILE_HIGH_CABAC,
    DVR_H264_ENC_PROFILE_MAIN_CAVLC,
    DVR_H264_ENC_PROFILE_HIGH_CAVLC,
};

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_h264_encoder_mode_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_h264_encoder_mode_enum   dvr_h264_encoder_mode_e;
#else
typedef unsigned char dvr_h264_encoder_mode_e;
#endif

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_ENCODE@, @DVR_SET_ENCODE@, and @DVR_REP_ENCODE@.

    Common fields:

        "status" - For a @DVR_REP_ENCODE@ message, this is the command
        status. For @DVR_SET_ENCODE@ messages, this is a flag field and
        can have the following values -

            0 - video encoder configuration
            1 - video codec type change only

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@.

        "job_id" - A unique job ID.

        "stream_id" - The stream identifier, specifies which encode
        stream is being queried or configured. The number of supported
        streams varies by product.

        "record_frame_rate" - Recording frame rate, must be between 1
        and the maximum allowed by the video standard.

        "record_res" - The video resolution to record at. Must be a
        valid enum of type @dvr_video_res_e@. Not all values of
        @dvr_video_res_e@ may be supported by all boards. Selecting
        any value other than @DVR_VIDEO_RES_FULL@, @DVR_VIDEO_RES_CIF@,
        or @DVR_VIDEO_RES_QCIF@ can have an impact on performance due
        to additional rescaling performed prior to encoding.

        "video_format" - Video CODEC format for the encoded stream.
        Must be one of the valid @dvr_vc_format_enum@ values.
        IMPORTANT: The video format for a stream can be changed only
        when the stream is disabled.

        "flags" - Bit field to specify a set of common
        actions/attributes applied to the encoder of the specified
        job/stream. See dvr_enc_flags_e for different encoder flags.

    H.264-AVC-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Applicable to @DVR_RC_VBR@, @DVR_RC_CBR@, and @DVR_RC_CBR_S@.
        Ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@ and @DVR_RC_CBR_S@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@, @DVR_RC_CBR@, @DVR_RC_CBR_S@ or @DVR_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 128. Default is 15.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_RC_CQ@ mode, and is ignored for
        other RC modes.

        "mode" - Sets the target quality level. Different modes have
        different encoder effort associated with them. Mode 0 is the default.

    H.264-SVC-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Applicable to @DVR_RC_VBR@, @DVR_RC_CBR@, and @DVR_RC_CBR_S@.
        Ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@ and @DVR_RC_CBR_S@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@, @DVR_RC_CBR@, @DVR_RC_CBR_S@ or @DVR_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 128. Default is 16.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_RC_CQ@ mode, and is ignored for
        other RC modes.

        "mode" - Sets the target quality level. Different modes have
        different encoder effort associated with them. Mode 0 is the default.

    MJPEG-specific fields:

        "quality" - Specifies the quality of compression. A higher number
        implies better quality. Must be between 1 and 99. Default is 50.

        "jpeg_style" - The JPEG encoder generates a
        video-style JPEG (Motion JPEG) frame header when this value is set to zero. This is
        suitable in AVI/MOV MJPEG or still JPEG image files. This is the style that
        most DVR applications should use.
        The JPEG encoder generates an image-style JPEG frame
        header to be used for RTP when this value is set to one (1).
        Set this field to two (2) if you need JPEG encoder to not add any JPEG frame header.
        Image style 2 maybe needed for some RTSP/RTP streaming. Image sytel 2 is not
        available prior to version 7 of the firmware. This is the recommanded style for
        IP Camera applications that are going to stream over RTSP/RTP.
        The default is 0.

        "frame_bundle_count" - For performance reason, you can set this field to
        2 in order for the MJPEG encoder encode two frames at a time.
        But for low frame rate which could cause latency in timestamps,
        you may turn off this feature by set this field to 1.
        Setting to 0 means to use the default behavior for the current board.

    MPEG4-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@ or @DVR_RC_CBR@. Default is @DVR_RC_VBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 30. Default is 15.

        "quality" - Quality level, must be between 1 (worst) and 100 (best).
        The recommended (and default) value is 50. Only applicable to
        @DVR_RC_VBR@, ignored for other modes.

    MPEG2-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@ or @DVR_RC_CBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 30. Default is 15.

************************************************************************/
typedef struct dvr_encode_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            stream_id;
    sx_uint8            record_frame_rate;
    dvr_video_res_e     record_res:8;
    dvr_vc_format_e     video_format:8;
    sx_uint8            flags;

    union {
        struct {
            sx_uint32           reserved2;
            sx_uint32           reserved3;
        } none;
        struct {
            sx_uint16                   avg_bitrate;
            sx_uint16                   max_bitrate;
            dvr_rc_e                    rate_control:8;
            sx_uint8                    gop_size;
            sx_uint8                    quality;
            dvr_h264_encoder_mode_e     mode:8;
        } h264;
        struct {
            sx_uint16                   avg_bitrate;
            sx_uint16                   max_bitrate;
            dvr_rc_e                    rate_control:8;
            sx_uint8                    gop_size;
            sx_uint8                    quality;
            dvr_h264_encoder_mode_e     mode:8;
        } h264_svc;
        struct {
            sx_uint16           quality;
            sx_uint8            jpeg_style;
            sx_uint8            frame_bundle_count;
            sx_uint32           reserved3;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved2;
        } mpeg4;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            reserved1;
            sx_uint8            reserved2;
        } mpeg2;
    } u1;
} dvr_encode_info_t;
sx_static_assert(sizeof(dvr_encode_info_t) == 16, dvr_encode_info_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_ALARM@, @DVR_SET_ALARM@, and @DVR_REP_ALARM@.

    Common fields:

        "status" - Status of the reply. This field is only valid for
        a @DVR_REP_ENC_ALARM@ message.

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@.

        "job_id" - A unique job ID.

        "stream_id" - The stream identifier, specifies which encode
        stream is being queried or configured. Currently, must be
        0 (primary) or 1 (secondary).

        "record_frame_rate" - Recording frame rate, must be between 1
        and the maximum allowed by the video standard.

        "min_on_seconds","min_off_seconds" - Minimum number of seconds
        to remain in alarm mode (normal mode) before returning to normal
        mode (alarm mode).  This is to prevent the alarm being on and off
        too frequently.

        "enable" - Set to 1 to enable or to 0 to disable the encode
        stream.

    H.264-AVC-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Applicable to @DVR_RC_VBR@, @DVR_RC_CBR@, and @DVR_RC_CBR_S@.
        Ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@ and @DVR_RC_CBR_S@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@, @DVR_RC_CBR@, @DVR_RC_CBR_S@ or @DVR_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 128. Default is 15.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_RC_CQ@ mode, and is ignored for
        other RC modes.

        "mode" - Sets the target quality level. Different modes have
        different encoder effort associated with them. Mode 0 is the default.

    H.264-SVC-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Applicable to @DVR_RC_VBR@, @DVR_RC_CBR@, and @DVR_RC_CBR_S@.
        Ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@ and @DVR_RC_CBR_S@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@, @DVR_RC_CBR@, @DVR_RC_CBR_S@ or @DVR_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 128. Default is 16.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_RC_CQ@ mode, and is ignored for
        other RC modes.

        "mode" - Sets the target quality level. Different modes have
        different encoder effort associated with them. Mode 0 is the default.

    MJPEG-specific fields:

        "quality" - Specifies the quality of compression. A higher number
        implies better quality. Must be between 1 and 99. Default is 50.

        "jpeg_style" - Controls frame header generation. Must be set to
        either 0 (for motion JPEG frame header) or 1 (for image style
        frame header).

        "frame_bundle_count" - For performance reason, you can set this field to
        2 in order for the MJPEG encoder encode two frames at a time.
        But for low frame rate which could cause latency in timestamps,
        you may turn off this feature by set this field to 1.
        Setting to 0 means to use the default behavior for the current board.

    MPEG4-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@ or @DVR_RC_CBR@. Default is @DVR_RC_VBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 30. Default is 15.

        "quality" - Quality level, must be between 1 (worst) and 100 (best).
        The recommended (and default) value is 50. Only applicable to
        @DVR_RC_VBR@, ignored for other modes.

    MPEG2-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_RC_VBR@ or @DVR_RC_CBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per 1/2 sec of video.
        The allowable range of values is 1 - 30. Default is 15.

************************************************************************/
typedef struct dvr_enc_alarm_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            stream_id;
    sx_uint8            record_frame_rate;
    sx_uint8            min_on_seconds;
    sx_uint8            min_off_seconds;
    sx_uint8            enable;

    union {
        struct {
            sx_uint32           reserved1;
            sx_uint32           reserved2;
        } none;
        struct {
            sx_uint16                   avg_bitrate;
            sx_uint16                   max_bitrate;
            dvr_rc_e                    rate_control:8;
            sx_uint8                    gop_size;
            sx_uint8                    quality;
            dvr_h264_encoder_mode_e     mode:8;
        } h264;
        struct {
            sx_uint16                   avg_bitrate;
            sx_uint16                   max_bitrate;
            dvr_rc_e                    rate_control:8;
            sx_uint8                    gop_size;
            sx_uint8                    quality;
            dvr_h264_encoder_mode_e     mode:8;
        } h264_svc;
        struct {
            sx_uint16           quality;
            sx_uint8            jpeg_style;
            sx_uint8            frame_bundle_count;
            sx_uint32           reserved2;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } mpeg4;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            reserved1;
            sx_uint8            reserved2;
        } mpeg2;
    } u1;
} dvr_enc_alarm_info_t;
sx_static_assert(sizeof(dvr_enc_alarm_info_t) == 16, dvr_enc_alarm_info_t_size);


/************************************************************************
    VISIBLE: Enumerated constants for H.264 decoder operating modes.

    @DVR_H264_DEC_MODE_STRETCH@ - Configures the H.264 decoder to operate
    in Stretch compatibility mode. This is usually more efficient in CPU
    usage, but may not correctly decode inputs from non-Stretch encoders.

    @DVR_H264_DEC_MODE_STANDARD@ - Configure the H.264 decoder to operate
    in Standard mode. This usually requires more CPU resources, and will
    be able to better handle input from non-Stretch encoders.

************************************************************************/
enum dvr_h264_decoder_mode_enum {
    /* NOTE: values must always numerically match the decoder config enums. */
    DVR_H264_DEC_MODE_STRETCH  = 0,
    DVR_H264_DEC_MODE_STANDARD = 1,
};


/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_h264_decoder_mode_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_h264_decoder_mode_enum   dvr_h264_decoder_mode_e;
#else
typedef unsigned char dvr_h264_decoder_mode_e;
#endif

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_DECODE@, @DVR_SET_DECODE@, and @DVR_REP_DECODE@.

    Common fields:

        "status" - Status of the reply. This field is only valid for
        a @DVR_REP_DECODE@ message.

        "job_type" - Must be @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "video_format" - The video format to be decoded. Must be one of
        the supported video formats.

        "width" - Width in pixels of the encoded frames.

        "height" - Height in pixels of the encoded frames.

    @NOTE@: Width and height combinations must match one of the supported
    video resolutions. These parameters can be changed while the job is
    disabled.

    H.264-specific fields:

        "disable_pipeline" - Setting this field to a nonzero value will
        disable the decoder's internal pipeline. This will reduce the
        output latency, but will also adversely impact performance due
        to the loss of efficiency. It is recommended that this flag not
        be set for high frame rates, unless the loss in performance is
        acceptable. Setting this field to zero enables the pipeline.

    MJPEG-specific fields:

        None.

************************************************************************/
typedef struct dvr_decode_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_vc_format_e     video_format:8;
    sx_uint16           width;
    sx_uint16           height;

    union {
        struct {
            sx_uint8                   disable_pipeline;
            dvr_h264_decoder_mode_e    decoder_mode:8;
            sx_uint16                  reserved3;
            sx_uint32                  reserved4;
        } h264;
        struct {
            sx_uint32       reserved3;
            sx_uint32       reserved4;
        } jpeg;
        struct {
            sx_uint32       reserved3;
            sx_uint32       reserved4;
        } mpeg4;
    } u1;
} dvr_decode_info_t;
sx_static_assert(sizeof(dvr_decode_info_t) == 16, dvr_decode_info_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_RELAYS@, @DVR_SET_RELAYS@, and @DVR_REP_RELAYS@.

        "status" - Status of the reply.

        "relay_status" - This is a bit field, one bit per relay output:

            For @DVR_GET_RELAYS@, this field is unused.

            For @DVR_REP_RELAYS@, this field reflects the status of the
            relay outputs. If a bit is 1, the corresponding relay is on;
            if the bit is 0, the relay is off.

            For @DVR_SET_RELAYS@, this field specifies the new states of
            the relay outputs.
************************************************************************/
typedef struct dvr_relay_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           relay_status;
    sx_uint32           reserved3;
    sx_uint32           reserved4;
} dvr_relay_t;
sx_static_assert(sizeof(dvr_relay_t) == 16, dvr_relay_t_size);


/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_RELAYS@, @DVR_SET_RELAYS@, and @DVR_REP_RELAYS@.

        "status" - Status of the reply.

        "relay_status" - This is a bit field, one bit per relay output:

            For @DVR_GET_RELAYS@, this field is unused.

            For @DVR_REP_RELAYS@, this field reflects the status of the
            relay outputs. If a bit is 1, the corresponding relay is on;
            if the bit is 0, the relay is off.

            For @DVR_SET_RELAYS@, this field specifies the new states of
            the relay outputs.
************************************************************************/
typedef struct dvr_relay_64_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           reserved2;
    sx_uint64           relay_status;
} dvr_relay_64_t;
sx_static_assert(sizeof(dvr_relay_64_t) == 16, dvr_relay_64_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SENSORS@, @DVR_SET_SENSORS@, and @DVR_REP_SENSORS@.

        "status" - Status of the reply.

        "sensor_status" - For @DVR_REP_SENSORS@, this reports the states
        of the sensors, one bit per sensor input.

        "sensor_enable" - For @DVR_SET_SENSORS@, this specifies the set of
        sensors to enable. If a bit is set to 1, the corresponding sensor
        is enabled.

************************************************************************/
typedef struct dvr_sensor_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           sensor_status;
    sx_uint32           sensor_enable;
    sx_uint32           reserved2;
} dvr_sensor_t;
sx_static_assert(sizeof(dvr_sensor_t) == 16, dvr_sensor_t_size);


/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_SENSOR@ and @DVR_GET_SENSOR@
    command. These flags
    indicate which fields of the data structure are valid.

    @DVR_SENSOR_OPCODE_STATUS_MAP@ - Only valid with @DVR_GET_SENSOR@
    command. It is a bit map corresponding to each
    sensor input to indicate whether its input has been grounded.
    The least significant bit (LSB) of this map corresponds to the first
    sensor, the next bit to the second sensor, and so on.
    The value of 1 for a bit means that the corresponding input has been
    grounded, and 0 means it is at 5 Volts.

    @DVR_SENSOR_OPCODE_ENABLE_MAP@ - It can be used with either
    @DVR_GET_SENSOR@ or @DVR_SET_SENSOR@ command. In case of @DVR_SET_SENSOR@
    it contains the set of sensors to enable. In case of @DVR_GET_SENSOR@
    it returns the set of sensors that are enabled. If a bit is set to 1,
    the corresponding sensor is enabled.
************************************************************************/
#define DVR_SENSOR_OPCODE_STATUS_MAP             0x1
#define DVR_SENSOR_OPCODE_ENABLE_MAP             0x2

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SENSORS@, @DVR_SET_SENSORS@, and @DVR_REP_SENSORS@. This
    data structure must be used in firmware version 7.5.2.x or higher.
    It is capable of supporting upto 64 sensors.

        "status" - Status of the reply.

        "opcode" - Indicates the meaning of "map" field.

        "map" - A bitmap of enable or status of all the sensors depending
        on the value of "opcode",

************************************************************************/
typedef struct dvr_sensor_64_struct {
    dvr_status_e        status:8;
    sx_int8             opcode;
    sx_int16            reserved1;
    sx_uint32           reserved2;
    sx_uint64           map;
} dvr_sensor_64_t;
sx_static_assert(sizeof(dvr_sensor_64_t) == 16, dvr_sensor_64_t_size);

/************************************************************************
    VISIBLE: Enumerated constants for watchdog control.

    @DVR_WATCHDOG_DISABLE@ - Disable WDT.

    @DVR_WATCHDOG_SCP@ - Enable WDT with SCP controlling refresh.

    @DVR_WATCHDOG_HOST@ - Enable WDT with host controlling refresh.

    @DVR_WATCHDOG_REFRESH@ - Refresh host-controlled WDT.

    NOTE: DISABLE and SCP enums must line up with previous API's false/true
    defines; to preserve backward-compatibility, do not change these values.
************************************************************************/
typedef enum dvr_watchdog_control_enum {
    DVR_WATCHDOG_DISABLE = 0,
    DVR_WATCHDOG_SCP     = 1,
    DVR_WATCHDOG_HOST    = 2,
    DVR_WATCHDOG_REFRESH = 3
} dvr_watchdog_control_e;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_WATCHDOG@, @DVR_SET_WATCHDOG@, and @DVR_REP_WATCHDOG@.

        "status" - status of the reply.

        "enable" - Enables the watchdog timer if set to a nonzero value,
        disables the timer if set to zero. Once enabled, the timer must
        be refreshed at intervals not exceeding the specified timeout.
        For @DVR_REP_WATCHDOG@ messages, this field indicates the current
        enable state of the watchdog timer.

        "msec" - Specifies the watchdog timeout period. The valid range is
        1 to 10,000 msec. If this field is zero and the enable flag is set
        then the timeout period defaults to 10,000 msec (10 sec). If the
        enable flag is not set, this field is ignored.
        For @DVR_REP_WATCHDOG@ messages, this field is always zero.

        @NOTE@: Once the watchdog function is enabled, successive
        @DVR_SET_WATCHDOG@ commands must be sent to the board before
        the previous timeout expires.
************************************************************************/
typedef struct dvr_watchdog_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    dvr_watchdog_control_e enable;
    sx_uint32           msec;
    sx_uint32           reserved3;
} dvr_watchdog_t;
sx_static_assert(sizeof(dvr_watchdog_t) == 16, dvr_watchdog_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_CONTROL@, @DVR_SET_CONTROL@, and @DVR_REP_CONTROL@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to none-zero value to enable and to 0 to
        disables the job.
************************************************************************/
typedef struct dvr_control_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            enable;
    sx_uint32           reserved1;
    sx_uint32           reserved2;
    sx_uint32           reserved3;
} dvr_control_t;
sx_static_assert(sizeof(dvr_control_t) == 16, dvr_control_t_size);

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_TIME@, @DVR_SET_TIME@, and @DVR_REP_TIME@.

        "status" - Status of the reply.

        "tv_sec", "tv_usec" - Number of seconds and microseconds
        since Epoch.
************************************************************************/
typedef struct dvr_time_struct {
    dvr_status_e    status:8;
    sx_uint8        reserved0;
    sx_uint16       reserved1;
    sx_int32        tv_sec;
    sx_int32        tv_usec;
    sx_uint32       reserved3;
} dvr_time_t;
sx_static_assert(sizeof(dvr_time_t) == 16, dvr_time_t_size);


/************************************************************************
    Maximum number of sub packets per DVR packet.
************************************************************************/
#define DVR_MAX_SUB_PACKETS     (7)

/************************************************************************
    VISIBLE: This structure describes the contents of every data buffer
    exchanged between the host and the firmware. Every data buffer must
    begin with this structure at its head.

        "signature" - The header signature. This is used to detect a
        valid header. Must always be set to @DVR_DATA_HDR_SIG@.

        "version" - Indicates the header version. Must be set to the
        current header version @DVR_DATA_HDR_VER@.

        "hdr_size" - Size of the header in bytes. Must be equal to
        @DVR_DATA_HDR_LEN@.

        "board_id" - The board identifier.

        "job_type" - The job type.

        "job_id" - The ID of the job that generated this buffer, or
        the job that is the recipient of this buffer.

        "data_type" - Indicates the type of data. Must be one of the
        valid @dvr_data_type_enum@ values.

        "motion_value" - Current motion detect parameter value.
        The value is between 0 and 255.

        "blind_value" - Current blind detect parameter value.
        The value is between 0 and 255.

        "night_value" - Current night detect parameter value.
        The value is between 0 and 255.

        "state_flags" - Set of one-bit state flags about the state of
        the job.

        "stream_id" - For encoder jobs, this specifies the stream ID of
        the encode stream that generated this data. For raw video frames,
        this specifies the stream ID only if the "dup_count" field is zero.
        If the dup_count field is nonzero, then this field will always be zero.
        For all other cases, this field will be set to zero.

        "dup_count" - Buffer duplication count. If this field is set to
        zero, the buffer is meant for the specified stream only. If this
        field is set to 2, the buffer needs to be duplicated for both the
        raw video streams.

        "data_size" - Size of the data buffer in bytes.

        "ts" - Low 32 bits of hardware-generated timestamp associated
        with the data.

        "ts_high" - High 32 bits of hardware-generated timestamp.
        @NOTE@: This field is valid only in firmware build versions 3.1.1
        or later.

        "mval" - Motion detect values for up to four regions of interest.
        mval[0] is the same as "motion_value".

        "bval" - Blind detect values for up to four regions of interest.
        mval[0] is the same as "blind_value".

        "nval" - Night detect values for up to four regions of interest.
        mval[0] is the same as "night_value".

        "active_width" - Active width of video frame buffer. This field is
        valid only for raw and encoded video frames.

        "padded_width" - Padded width of video frame buffer. This field is
        valid only for raw and encoded video frames.

        "active_height" - Active height of video frame buffer. This field is
        valid only for raw and encoded video frames.

        "padded_height" - Padded height of video frame buffer. This field is
        valid only for raw and encoded video frames.

        "seq_number" - Frame sequence number. This field is valid only
        for raw and encoded video frames. Each board_id/job_id/stream
        combination will have independent sequence numbering. The raw
        and encoded video streams from the same job will have independent
        sequence numbering. The sequence number starts from 1 and is reset
        when the stream is disabled.

        "frame_number" - Number of frames seen on this channel so far.
        This field is valid only for raw and encoded video frames. The
        frame number and sequence number will be identical when the
        stream is being run at full frame rate and no frames are dropped.
        The counter is reset when the stream is disabled.

        "drop_count" - Number of frames drop detected by the firmware on the
        current stream. The drop count is reset when the stream is disabled.

        "enc_audio_mode" - It is only valid when datatype field is set to
        DVR_DATA_RAW_AUDIO and the firmware is compiled such that it should
        convert audio from G711 to linear before playing it on
        audio-out port. This field indicates if the payload contains 8-bit mono
        or 16-bit stereo G711 audio data.

        The following fields are only used in receiving raw YUV frames
        from the host DVR Application:

            "yuv_format" - The current YUV format.

            "y_data_size" - The size of the Y data in bytes.

            "u_data_size" - The size of the U data in bytes.

            "v_data_size" - The size of the V data in bytes.

            "y_data_offset" -  The offset of Y data of a raw frame
            relative from the start of the header

            "u_data_offset" -  The offset of U data of a raw frame
            relative from the start of the header

            "v_data_offset" -  The offset of V data of a raw frame
            relative from the start of the header

            "layer_num" - When yuv data is sent to the board for display, this
            field indicates the layer number of this buffer used when blending.

            "alpha" - Alpha value used for blending layers.

            "width" - Width of the raw frame.  A zero value can be used if it matches
            the SMO size.

            "height" - Height of the raw frame.  A zero value can be used if it matches
            the SMO size.

            "x_coord" - X coordinate to place raw frame.

            "y_coord" - Y coordinate to place raw frame.

            "num_packets" - Number of sub packets in one dvr data packet

            "packet_size" - Array containing the packet sizes of sub packets

************************************************************************/
typedef struct dvr_data_header_struct {
    sx_uint32       signature;          // Must be set to DVR_DATA_HDR_SIG
    sx_uint16       version;            // Must be set to DVR_DATA_HDR_VER
    sx_uint16       hdr_size;           // Must be set to sizeof(dvr_data_header_t)

    // These 4 fields are backward compatible
    sx_uint8        board_id;
    dvr_job_type_e  job_type:8;
    sx_uint8        job_id;
    dvr_data_type_e data_type:8;

    // These 4 fields are backward compatible
    sx_uint8        motion_value;
    sx_uint8        blind_value;
    sx_uint8        night_value;
    sx_uint8        state_flags;

    // These 2 fields are backward compatible
    sx_uint8        stream_id;
    sx_uint8        dup_count;
    sx_uint16       reserved3;
    sx_uint32       data_size;

    // These 2 fields are backward compatible
    sx_uint32       ts;
    sx_uint32       ts_high;

    // New fields. m/b/nval[0] same as the compatible fields above
    sx_uint8        mval[4];
    sx_uint8        bval[4];
    sx_uint8        nval[4];

    // These fields are for the Y plane
    sx_uint16       active_width;
    sx_uint16       padded_width;
    sx_uint16       active_height;
    sx_uint16       padded_height;

    sx_uint32       seq_number;
    sx_uint32       frame_number;
    sx_uint32       drop_count;

    sx_uint8             reserved4[2];
    dvr_enc_audio_mode_e  enc_audio_mode:8;
    dvr_rawv_format_e    yuv_format:8;

    sx_uint32       y_data_size;
    sx_uint32       u_data_size;
    sx_uint32       v_data_size;
    sx_uint32       y_data_offset;
    sx_uint32       u_data_offset;
    sx_uint32       v_data_offset;

    // These fields are for the U and V planes. Assume U and V are always
    // equal width and height
    sx_uint16       uv_active_width;
    sx_uint16       uv_padded_width;
    sx_uint16       uv_active_height;
    sx_uint16       uv_padded_height;

    // These fields are for host overlay/video data
    sx_uint8        layer_num;
    sx_uint8        alpha;
    sx_uint8        reserved5[2];
    sx_uint16       layer_width;
    sx_uint16       layer_height;
    sx_int16        x_coord;
    sx_int16        y_coord;

    // These fields are used in SVC mode for packing packets
    sx_uint32       num_packets;
    sx_uint32       packet_size[DVR_MAX_SUB_PACKETS];

    sx_uint32       hmo_local_data;
    sx_uint8        is_test_frame;
    sx_uint8        test_pattern;
    sx_uint16       reserved1;
    sx_uint32       reserved[18];

    // For debug/internal use only.
    sx_uint32       dbg[8];
} dvr_data_header_t;
sx_static_assert(sizeof(dvr_data_header_t) == 256, dvr_data_header_t_size);

/************************************************************************
    Data header signature value. Only present in new-style header.
************************************************************************/
#define DVR_DATA_HDR_SIG    (0x00524448)

/************************************************************************
    Current data header version.
************************************************************************/
#define DVR_DATA_HDR_VER    (0x3)

/************************************************************************
    Data header size. Must always be a multiple of 16.
************************************************************************/
#define DVR_DATA_HDR_LEN    (sizeof(dvr_data_header_t))

/************************************************************************
    Motion Map data size. 9x15=135 Need to match the motion_map defined in
    isp_raw.h; added 4 more bytes for width, height and max and a blank padding byte
************************************************************************/
#define DVR_DATA_MOTION_MAP_SIZE    (135 + 4)

/************************************************************************
    VISIBLE: This structure defines the data associated with message
    @DVR_SIG_HOST@ from the board to the host.

        "sig_type" - Type of signal, e.g. @DVR_SIGNAL_SENSOR_ACTIVATED@.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "signal_data" - Information associated with the particlar signal
        type.

            For @DVR_SIGNAL_SENSOR_ACTIVATED@, it is the sensor status.

            For @DVR_SIGNAL_MOTION_DETECTED@,  it is the motion value.

            For @DVR_SIGNAL_BLIND_DETECTED@,   it is the blind value.

            For @DVR_SIGNAL_NIGHT_DETECTED@,   it is the night value.

            For @DVR_SIGNAL_VIDEO_LOST@,       there is no data.

            For @DVR_SIGNAL_VIDEO_DETECTED@,   there is no data.

            For @DVR_SIGNAL_RUNTIME_ERROR@,    it is the error code.

            For @DVR_SIGNAL_FATAL_ERROR@,      it is the error code.

            For @DVR_SIGNAL_HEARTBEAT@,        it is the board id.

            For @DVR_SIGNAL_WATCHDOG_EXPIRED@, there is no data.

            For @DVR_SIGNAL_LAST_ERROR@,       it is the error code.

            For @DVR_SIGNAL_TEMPERATURE@,      it is the temperature in deg C.

            For @DVR_SIGNAL_FRAME_TOO_LARGE@,  it is the stream ID.

            For @DVR_SIGNAL_MEMORY_USAGE@,  it is the DDR memory usage value of PE

        "extra_data" - Extra information associated with the signal, if any.

            For @DVR_SIGNAL_RUNTIME_ERROR@,    it is the extended error code.

            For @DVR_SIGNAL_FATAL_ERROR@,      it is the extended error code.

            For @DVR_SIGNAL_TEMPERATURE@,      it is the fractional part of the
            temperature in tenths of a degree C.

            For @DVR_SIGNAL_FRAME_TOO_LARGE@,  it is the frame size in bytes.

            For @DVR_SIGNAL_MEMORY_USAGE@,  it is the PE id.

    @NOTE@: For messages such as @DVR_SIGNAL_WATCHDOG_EXPIRED@, job_type
    and job_id are not meaningful, and are always zero.
************************************************************************/
typedef struct dvr_sig_host_struct {
    dvr_signal_type_e   sig_type:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            reserved1;
    sx_uint32           signal_data;
    sx_uint32           extra_data;
    sx_uint32           reserved3;
} dvr_sig_host_t;
sx_static_assert(sizeof(dvr_sig_host_t) == 16, dvr_sig_host_t_size);

/************************************************************************
    VISIBLE: Command codes for DVR RS-485 port control.

    @DVR_UART_CMD_CONFIG@ - UART configuration command.

    @DVR_UART_CMD_OUTPUT@ - UART output command.
************************************************************************/
#define DVR_UART_CMD_CONFIG    1
#define DVR_UART_CMD_OUTPUT    2

/************************************************************************
    VISIBLE: This structure is used for @DVR_GET_UART@, @DVR_SET_UART@
    and @DVR_REP_UART@ messages.

    Common Fields:

        "status" - Status of the reply.

        "cmd" - Command code, either @DVR_UART_CONFIG@ or @DVR_UART_OUTPUT@.
        Only valid for @DVR_SET_UART@ messages.

    Config Command Fields:

        "baud_rate" - The output baud rate. Must be between 50 and 115200.

        "data_bits" - Number of data bits. Valid values are 5-8.

        "stop_bits" - Number of stop bits. Valid values are 1 and 2.

        "parity_enable" - If this field is set to zero, parity is disabled.
        If set to a nonzero value, parity is enabled.

        "parity_even" - If this field is set to zero, odd parity is used.
        If set to a nonzero value, even parity is used. This field is
        ignored if parity is disabled.

    Output Command Fields:

        "count" - Number of bytes to transmit. Must be between 1 and 11.

        "data" - The data bytes to transmit. The number of valid bytes in
        the array is determined by the "count" field.

    Input Command Fields:

        "count" - For @DVR_GET_UART@, specifies the maximum number of bytes
        to read. For @DVR_REP_UART@, specifies the actual number of bytes
        read, which can be less than the number requested.

        "data" - For @DVR_REP_UART@, contains the bytes read, if any.
        The number of valid bytes is determined by the "count" field.
************************************************************************/
typedef struct dvr_uart_struct {
    dvr_status_e       status:8;
    sx_uint8           cmd;
    sx_uint16          reserved1;

    union {
        struct {
            sx_uint32    baud_rate;
            sx_uint8     data_bits;
            sx_uint8     stop_bits;
            sx_uint8     parity_enable;
            sx_uint8     parity_even;
            sx_uint32    reserved2;
        } config;
        struct {
            sx_uint8     count;
            sx_uint8     data[11];
        } output;
        struct {
            sx_uint8     count;
            sx_uint8     data[11];
        } input;
    } u1;
} dvr_uart_t;
sx_static_assert(sizeof(dvr_uart_t) == 16, dvr_uart_t_size);


/************************************************************************
    VISIBLE: Stretch DVR IOCTL codes. These codes are used by the
    @DVR_GET_IOCTL@, @DVR_SET_IOCTL@, and @DVR_REP_IOCTL@ commands.

    @DVR_IOCTL_CODE_IMG_CONTROL@ - Used to get and set video decoder image
    control parameters.

    @DVR_IOCTL_CODE_DECODER_REGS@ - Used to get and set video decoder
    registers. Provides low level access to decoder configuration.

    @DVR_IOCTL_CODE_OPEN_CHAN@ - Used to open a data channel between the
    host and the firmware for control data, e.g. region map data. Used
    only with the SET and REP commands. This command is per board.

    @DVR_IOCTL_CODE_CLOSE_CHAN@ - Used to close the data channel between
    the host and the firmware. Used only with SET and REP commands.
    This command is per board.

    @DVR_IOCTL_CODE_MOTION_FRAME_FREQ@ - Used to specify how often to send
    motion value frames. The default is zero. 1 means send for every raw
    video frame recieved, 2 means send every second frame, and so on.
    Used only with the SET and REP commands.
    This command is per board.

    @DVR_IOCTL_CODE_VPP_CONTROL@ - Controls video preprocessing actions.
    See @dvr_vpp_action_enum@ for the supported actions.

    @DVR_IOCTL_CODE_GAIN_MODE@ - TBD

    @DVR_IOCTL_CODE_TERMINATION@ - TBD

    @DVR_IOCTL_CODE_LED@ - Used to set/get the current enable status of
    different LEDs on the DVR Board.

    @DVR_IOCTL_CODE_RAWV_FORMAT@ - Used to set the format of raw video output
    from the board.

    @DVR_IOCTL_CODE_PCI_BURST_SIZE@ - Used to set the PCI DMA transfer burst
    size on the board. Tuning this parameter may be required to optimize PCI
    throughput on some systems. The default value at startup is 128 bytes for
    most Stretch boards. The allowed values are 128, 64, 32 and 16 bytes.

    @DVR_IOCTL_CODE_INTERLACED_CIF@ - Set the CIF format (as generated by
    the video decoder) to interlaced (1) or progressive (0).

    @DVR_IOCTL_CODE_UPDATE_FLASH@ - Used to download a new application ROM
    file and program it into the boot flash.

    @DVR_IOCTL_CODE_FORCE_KEY_FRAME@ - Force the encoder to produce a key
    frame immediately. This works only for the encoders that support this
    feature, on all others it is a no-op.

    @DVR_IOCTL_CODE_TEMPERATURE@ - Configure the temperature measurement
    process. Enable or disable the reporting of temperature, and set the
    measurement interval and the reporting threshold.

    @DVR_IOCTL_CODE_ENCODER_DEBUG@ - Internal use only. Used to set/get
    internal debugging flags for the video decoder.

    @DVR_IOCTL_CODE_DECODER_DEBUG@ - Internal use only. Used to set/get
    internal debugging flags for the video encoder.

    @DVR_IOCTL_CODE_KNAME_DEBUG@ - Internal use only. Used to set/get
    internal debugging flags for the video encoder.

    @DVR_IOCTL_CODE_GPIO@ - Used to read and write GPIO pins on the device.
    WARNING: Incorrect use of the GPIO pins can cause the system to hang
    or malfunction in unpredictable ways. A close study of the schematics
    for the target board is advised.

    @DVR_IOCTL_CODE_TWI@ - Used to read and write device registers on any
    device residing on the TWI bus. The firmware will check that the device
    exists, but will not validate any parameters.
    WARNING: Incorrect programming of any TWI device can cause the system
    to hang or malfunction in unpredictable ways. Please study the manual
    for the device being controlled, and make sure that any changes made
    do not conflict with the firmware's use of the device.

    @DVR_IOCTL_CODE_IMG_CROP@ - Used to specify the crop specification for
    a custom output picture size. The output resolution must be set to
    @DVR_VIDEO_RES_CUSTOM_XXX@ for this to have effect.

    @DVR_IOCTL_CODE_IMG_SCALE@ - Used to specify the scaling specification
    for a custom output picture size. The output resolution must be set to
    @DVR_VIDEO_RES_CUSTOM_XXX@ for this to have effect.

    @DVR_IOCTL_CODE_CRYPTO_EEPROM@ - Used to provide host access to an
    on-board Cryptographic EEPROM device.

    @DVR_IOCTL_CODE_IR_CUT_FILTER@ - Used to enable or disable the
    Infrared (IR) cut-off filter on a sensor of an IP-Camera. This command
    is not supported by DVR firmware. Additionally, it will only take
    effect on IP-Cameras that have hardware support for it.

    @DVR_IOCTL_CODE_SMO_DISABLE_MODE@ - Set the SMO mode when disabled.
    The mode can be (a) SMO output disabled (b) SMO output colorbar.
    This command will take effect only on boards that support it.

    @DVR_IOCTL_CODE_RAW_CMD@ - Used to get/set arbitrary commands assocated
    with a given sub-system within the DVR firmware application. The
    contents of the data is only known to sub-system and the Host DVR
    Application. The SDK sends DVR_SET_IOCTL with opcode DVR_IOCTL_CODE_RAW_CMD
    to the firmware to inform  that it is about to send a raw command data.
    The actuall message data is sent transfered the same way as OSD text.

    NOTE: This command only is sent using @DVR_SET_IOCTL@, and @DVR_REP_IOCTL@
    there is not going to be DVR_GET_IOCTL as the get command is part of the
    buffer data which is interpreted by the sub-system.

    @DVR_IOCTL_CODE_ENC_MODE@ - Used to set the global encoder mode
    applied to all the h.264 encoders on the current board.

    @DVR_IOCTL_CODE_SNAPSHOT@ - Used to take a snapshot (single-frame
    MJPEG encode) of an active stream.

    @DVR_IOCTL_CODE_FW_DEBUG@ - Used to enable additional debug logging
    from the firmware.  If the firmware encounters a fatal error assertion,
    extended information will be sent back to the host.  Enabling this
    feature requires 1 SCT buffer to be reserved.

    @DVR_IOCTL_CODE_LOAD_BALANCE@ - Used to redistribute firmware loading
    among various components.

    @DVR_IOCTL_CODE_FRAME_SKIP@ - Used to control the number of input
    frames skipped per sample size.  This operates at the dataport level,
    so it affects both encoding and HMO/SMO display.

    @DVR_IOCTL_CODE_MEMORY_USAGE@ - Configure the PE wise memory usage
    measurement process. Enable or disable the reporting of memory
    usage, and set the measurement interval.

    @DVR_IOCTL_CODE_ENC_PARAMS@ - Used to send extra encoder parameters
    that can not be sent using DVR_SET_ENCODE message.

    @DVR_IOCTL_CODE_ENC_ALARM_PARAMS@ - Used to send extra encoder parameters
    that can not be sent using DVR_SET_ALARM_ENCODE message.

    @DVR_IOCTL_CODE_EVENT_REPORTING@ - Used to specify how often an
    event should be reported to the host. The current behavior is for the
    event to be sent only once. Currenlty, we only support:

    @DVR_IOCTL_CODE_MACADDR@ - Used to set or get the MAC address kept in
    the on-board EEPROM.

    @DVR_IOCTL_CODE_TEST_FRAMES@ - Used to enable/disable generating test
    frames from the firmware.
*************************************************************************/
#define DVR_IOCTL_CODE_IMG_CONTROL          1
#define DVR_IOCTL_CODE_DECODER_REGS         2
#define DVR_IOCTL_CODE_OPEN_CHAN            3
#define DVR_IOCTL_CODE_CLOSE_CHAN           4
#define DVR_IOCTL_CODE_MOTION_FRAME_FREQ    5
#define DVR_IOCTL_CODE_VPP_CONTROL          7
#define DVR_IOCTL_CODE_GAIN_MODE            9
#define DVR_IOCTL_CODE_TERMINATION          10
#define DVR_IOCTL_CODE_LED                  11
#define DVR_IOCTL_CODE_RAWV_FORMAT          12
#define DVR_IOCTL_CODE_PCI_BURST_SIZE       13
#define DVR_IOCTL_CODE_INTERLACED_CIF       14
#define DVR_IOCTL_CODE_UPDATE_FLASH         15
#define DVR_IOCTL_CODE_FORCE_KEY_FRAME      16
#define DVR_IOCTL_CODE_TEMPERATURE          17
#define DVR_IOCTL_CODE_DECODER_DEBUG        18
#define DVR_IOCTL_CODE_ENCODER_DEBUG        19
#define DVR_IOCTL_CODE_KNAME_DEBUG          20
#define DVR_IOCTL_CODE_GPIO                 21
#define DVR_IOCTL_CODE_TWI                  22
#define DVR_IOCTL_CODE_IMG_CROP             23
#define DVR_IOCTL_CODE_IMG_SCALE            24
#define DVR_IOCTL_CODE_CRYPTO_EEPROM        25
#define DVR_IOCTL_CODE_IR_CUT_FILTER        26
#define DVR_IOCTL_CODE_SMO_DISABLE_MODE     27
#define DVR_IOCTL_CODE_RAW_CMD              28
#define DVR_IOCTL_CODE_ENC_MODE             29
#define DVR_IOCTL_CODE_SNAPSHOT             30
#define DVR_IOCTL_CODE_FW_DEBUG             31
#define DVR_IOCTL_CODE_LOAD_BALANCE         32
#define DVR_IOCTL_CODE_FRAME_SKIP           33
#define DVR_IOCTL_CODE_MEMORY_USAGE         34
#define DVR_IOCTL_CODE_ENC_PARAMS           35
#define DVR_IOCTL_CODE_ENC_ALARM_PARAMS     36
#define DVR_IOCTL_CODE_EVENT_REPORTING      37
#define DVR_IOCTL_CODE_MACADDR              38
#define DVR_IOCTL_CODE_TEST_FRAMES          39

/************************************************************************
    VISIBLE: The encoder modes supported by DVR_IOCTL_CODE_ENC_MODE for
    use with the @DVR_SET_IOCTL@ command. These flags
    indicate the global H.264 encoder behavior accross all channels.

    @DVR_ENC_MODE_DEFAULT@    - The default H.264 encoder mode.
    Adaptive encoding.

    @DVR_ENC_MODE_FRAME_RATE@ - Instruct the H.264 encoder to
    favore keeping up with the frame rate over the video quality
************************************************************************/
#define DVR_ENC_MODE_DEFAULT            0x0
#define DVR_ENC_MODE_FRAME_RATE         0x1

/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_IOCTL@ command. These flags
    indicate which fields of the data structure are valid.

    @DVR_ICFLAG_HUE@ - Use the hue parameter.

    @DVR_ICFLAG_SAT@ - Use the saturation parameter.

    @DVR_ICFLAG_BRT@ - Use the brightness parameter.

    @DVR_ICFLAG_CONT@ - Use the contrast parameter.

    @DVR_ICFLAG_SHARP@ - Use the sharpness parameter.
************************************************************************/
#define DVR_ICFLAG_HUE                  0x1
#define DVR_ICFLAG_SAT                  0x2
#define DVR_ICFLAG_BRT                  0x4
#define DVR_ICFLAG_CONT                 0x8
#define DVR_ICFLAG_SHARP                0x10

/****************************************************************************
  VISIBLE: Enumerated type describing various camera input impedance
  termination used with @DVR_IOCTL_CODE_CAMERA_SETTINGS@ for
  termination field.

    @DVR_TERM_75OHM@ - 75 ohm impedance termination.

    @DVR_TERM_HIGH_IMPEDANCE@ - High impedance termination.

***************************************************************************/
enum dvr_term_enum {
    DVR_TERM_75OHM,
    DVR_TERM_HIGH_IMPEDANCE
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_term_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_term_enum   dvr_term_e;
#else
typedef unsigned char dvr_term_e;
#endif

/****************************************************************************
  VISIBLE: Enumerated type describing various LED types.

    @DVR_LED_TYPE_RECORD@ - A group of LEDs indicating the current state of
    recording.

    @DVR_LED_TYPE_ALARM@ - A group of LEDs indicating the current state
    of the alarms.

***************************************************************************/

enum dvr_led_type_enum {
    DVR_LED_TYPE_RECORD,
    DVR_LED_TYPE_ALARM
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_led_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_led_type_enum   dvr_led_type_e;
#else
typedef unsigned char dvr_led_type_e;
#endif


/************************************************************************
    VISIBLE: Image source selections for the @DVR_IOCTL_CODE_IMG_CROP@
    IOCTL command.

    DVR_CROP_SRC_1X - Select the 1x input picture as source.

    DVR_CROP_SRC_2X - Select the 2x input picture as source.

    DVR_CROP_SRC_4X - Select the 4x input picture as source.
************************************************************************/
enum dvr_crop_src_enum {
    DVR_CROP_SRC_1X,
    DVR_CROP_SRC_2X,
    DVR_CROP_SRC_4X,
};

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with
        enum type fields within a structure is not supported. See
        @dvr_crop_src_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_crop_src_enum   dvr_crop_src_e;
#else
typedef unsigned char dvr_crop_src_e;
#endif

/****************************************************************************
    VISIBLE: Enumerated type describing various cryptographic EEPROM commands.

    @DVR_CRYPTO_EEPROM_CMD_READ@ - Read from the specified EEPROM zone (128B).
    If access control has been set to require a password, either the read or
    the write password for this zone must be correctly verified before reading.
    Access should not span multiple zones.  Only valid with DVR_GET_IOCTL.
    Required parameters are "zone", "zone_addr", "nbytes", and "data."
    Up to 8 bytes of EEPROM data may be read per command.

    @DVR_CRYPTO_EEPROM_CMD_WRITE@ - Write to the specified EEPROM zone (128B).
    If access control has been set to require a password, the write password
    for this zone must be correctly verified before reading.
    Access should not span multiple zones.  Only valid with DVR_SET_IOCTL.
    Required parameters are "zone", "zone_addr", "nbytes", and "data."
    Up to 8 bytes of EEPROM data may be written per command.

    @DVR_CRYPTO_EEPROM_CMD_ACCESS_MODE@ - Get or set the access mode
    constraints (no password, write password, or read+write passwords required)
    for the specified EEPROM zone.  "Get access mode" issued with DVR_GET_IOCTL
    and "Set access mode" issued with DVR_SET_IOCTL.  NOTE: before access
    control can be changed, either the secure code (config zone) password or
    the target zone's write password must be correctly verified.
    Required parameters are "zone" and "access_type".

    @DVR_CRYPTO_EEPROM_CMD_SET_PASS@ - Change the specified write or read
    password for the specified zone.  Write passwords allow both read and write
    access.  NOTE: before passwords can be changed, either the secure code
    (config zone) password or the target zone's write password must be
    correctly verified.  Only valid with DVR_SET_IOCTL.
    Required parameters are "zone", "pass_type", and 3 bytes of "data".  The
    MSB of the password resides in data[0].

    @DVR_CRYPTO_EEPROM_CMD_VERIFY_PASS@ - Attempt to verify the provided
    password (read or write) for the specified zone.  Write passwords allow both
    read and write access.  Four incorrect attempts are allowed before the PAC
    feature locks out the specified password, after which point the access
    mode must be reset.  NOTE: no status of password verification is returned;
    it is up to the application to determine whether subsequent data is
    correct or not.  Only valid with DVR_SET_IOCTL.
    Required parameters are "zone", "pass_type", and 3 bytes of "data".  The
    MSB of the password resides in data[0].

    @DVR_CRYPTO_EEPROM_CMD_BLOW_FUSE@ - Blow the specified fuse to make the
    associated EEPROM configuration data read-only.  There are 3 fuses which
    must be blown in-order using separate requests.  After the last fuse is
    blown, access control cannot be changed, and a zone's password can only
    be changed after verifying that zone's write password.  @Blowing fuses
    is NOT REVERSIBLE!@  Only valid with DVR_SET_IOCTL.
    Required parameter is "fuse".
***************************************************************************/
enum dvr_crypto_eeprom_cmd_enum {
    DVR_CRYPTO_EEPROM_CMD_READ,
    DVR_CRYPTO_EEPROM_CMD_WRITE,
    DVR_CRYPTO_EEPROM_CMD_ACCESS_MODE,
    DVR_CRYPTO_EEPROM_CMD_SET_PASS,
    DVR_CRYPTO_EEPROM_CMD_VERIFY_PASS,
    DVR_CRYPTO_EEPROM_CMD_BLOW_FUSE
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_led_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_crypto_eeprom_cmd_enum dvr_crypto_eeprom_cmd_e;
#else
typedef unsigned char dvr_crypto_eeprom_cmd_e;
#endif

/************************************************************************
    VISIBLE: Zone parameter values for use with the
    @DVR_IOCTL_CODE_CRYPTO_EEPROM@ subcommand of @DVR_*_IOCTL@.
    These defines indicate which zone of the device to access.

    @DVR_CRYPTO_EEPROM_ZONE_2@ - User-accessible zone #2.

    @DVR_CRYPTO_EEPROM_ZONE_3@ - User-accessible zone #3.

    @DVR_CRYPTO_EEPROM_ZONE_CFG@ - Configuration zone for modifying
    internal crypto EEPROM device state.
************************************************************************/
#define DVR_CRYPTO_EEPROM_ZONE_2        2
#define DVR_CRYPTO_EEPROM_ZONE_3        3
#define DVR_CRYPTO_EEPROM_ZONE_CFG      7

/************************************************************************
    VISIBLE: Access type parameter values for use with the
    @DVR_IOCTL_CODE_CRYPTO_EEPROM@ subcommand of @DVR_*_IOCTL@.
    These defines indicate possible access types for user-accessible zones.

    @DVR_CRYPTO_EEPROM_NO_PASS@ - No password required for read nor write.

    @DVR_CRYPTO_EEPROM_W_PASS@ - Password required for write; read free.

    @DVR_CRYPTO_EEPROM_RW_PASS@ - Password required for read or write
    (write password grants read access).
************************************************************************/
#define DVR_CRYPTO_EEPROM_NO_PASS       1
#define DVR_CRYPTO_EEPROM_W_PASS        2
#define DVR_CRYPTO_EEPROM_RW_PASS       3

/************************************************************************
    VISIBLE: Password type parameter values for use with the
    @DVR_IOCTL_CODE_CRYPTO_EEPROM@ subcommand of @DVR_*_IOCTL@.
    These defines indicate possible access types of passwords for
    user-accessible zones.

    @DVR_CRYPTO_EEPROM_PASS_TYPE_READ@ - Read password (no write access).

    @DVR_CRYPTO_EEPROM_PASS_TYPE_WRITE@ - Write password (+ read access).
************************************************************************/
#define DVR_CRYPTO_EEPROM_PASS_TYPE_READ    1
#define DVR_CRYPTO_EEPROM_PASS_TYPE_WRITE   2

/************************************************************************
    VISIBLE: Fuse ID parameter values for use with the
    @DVR_IOCTL_CODE_CRYPTO_EEPROM@ subcommand of @DVR_*_IOCTL@.
    These fuses must be blown in sequence: FAB first, then CMA, then PER.

    @DVR_CRYPTO_EEPROM_FUSE_FAB@ - Answer-to-reset and Fab code fuse.

    @DVR_CRYPTO_EEPROM_FUSE_CMA@ - Card manufacturer code fuse.

    @DVR_CRYPTO_EEPROM_FUSE_PER@ - Remainder of configuration space fuse.
************************************************************************/
#define DVR_CRYPTO_EEPROM_FUSE_FAB      2
#define DVR_CRYPTO_EEPROM_FUSE_CMA      3
#define DVR_CRYPTO_EEPROM_FUSE_PER      4

/************************************************************************
    VISIBLE: SMO disable mode values for use with the IOCTL command
    @DVR_IOCTL_CODE_SMO_DISABLE_MODE@. Note that this command is supported
    only on boards with appropriate features.

    @DVR_SMO_DISABLE_MODE_BLANK@ - SMO output is blank when disabled.

    @DVR_SMO_DISABLE_MODE_CB@ - SMO output displays a colorbar when disabled.
************************************************************************/
#define DVR_SMO_DISABLE_MODE_BLANK      0
#define DVR_SMO_DISABLE_MODE_CB         1

/****************************************************************************
    VISIBLE: Enumerated type describing various load balancing commands.

    NOTE: DEPRECATED.  DO NOT USE.

    @DVR_LOAD_BALANCE_DEC_SMO_RESIZE@ - Optionally specify whether a decoder
    job resizes its SMO image on the decoding PE or the SMO PE.
    Required parameters: [target] resize location of SMO or decoder, and
    [port] SMO port ID (1-based).
***************************************************************************/
enum dvr_load_balance_enum {
    DVR_LOAD_BALANCE_DEC_SMO_RESIZE
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_load_balance_enum@ for the description of this type.

    NOTE: DEPRECATED.  DO NOT USE.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_load_balance_enum dvr_load_balance_e;
#else
typedef unsigned char dvr_load_balance_e;
#endif


/************************************************************************
    VISIBLE: Various sub-system within the DVR firmware the can sent
    and receive raw un-formated commands.

    @DVR_SUB_SYSTEM_IPP@ - Raw commands bound for IPP.

    @DVR_SUB_SYSTEM_VPP@ - Raw commands bound for VPP.

    @DVR_SUB_SYSTEM_FIRMWARE@ - Raw commands bound for formware.

    @DVR_SUB_SYSTEM_SMO@ - Raw commands bound for SMO.

    @DVR_SUB_SYSTEM_H264_ENC@ - Raw commands bound for h264 encoder.

    @DVR_SUB_SYSTEM_H264_SVC_ENC@ - Raw commands bound for h264 SVC encoder.

    @DVR_SUB_SYSTEM_MJPEG_ENC@ - Raw commands bound for MJPEG encoder.

    @DVR_SUB_SYSTEM_MPEG4_ENC@ - Raw commands bound for MPEG4 encoder.

    @DVR_SUB_SYSTEM_H264_DEC@ - Raw commands bound for h264 decoder.

    @DVR_SUB_SYSTEM_MJPEG_DEC@ - Raw commands bound for MJPEG decoder.

    @DVR_SUB_SYSTEM_MPEG4_DEC@ - Raw commands bound for MPEG4 decoder.

    Note: this definition must be kept in sync with a similar definition in the SDK.
************************************************************************/
typedef enum {
    DVR_SUB_SYSTEM_IPP = 1,
    DVR_SUB_SYSTEM_VPP,
    DVR_SUB_SYSTEM_FIRMWARE,
    DVR_SUB_SYSTEM_SMO,
    DVR_SUB_SYSTEM_H264_ENC,
    DVR_SUB_SYSTEM_H264_SVC_ENC,
    DVR_SUB_SYSTEM_MJPEG_ENC,
    DVR_SUB_SYSTEM_MPEG4_ENC,
    DVR_SUB_SYSTEM_H264_DEC,
    DVR_SUB_SYSTEM_MJPEG_DEC,
    DVR_SUB_SYSTEM_MPEG4_DEC
} dvr_sub_system_e;


/************************************************************************
    VISIBLE: Tri-state data type.

    @DVR_TRISTATE_Z@ - Undefined/high-impedance.

    @DVR_TRISTATE_ON@ - On/high.

    @DVR_TRISTATE_OFF@ - Off/low.
************************************************************************/
enum dvr_tristate_enum {
    DVR_TRISTATE_Z = 0,
    DVR_TRISTATE_ON = 1,
    DVR_TRISTATE_OFF = 2
};

/***********************************************************************
    This is a work around for MSC compiler. The ":n" with
    enum type fields within a structure is not supported. See
    @dvr_tristate_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_tristate_enum dvr_tristate_e;
#else
typedef unsigned char dvr_tristate_e;
#endif

/************************************************************************
    VISIBLE: This structure is used by the @DVR_GET_IOCTL@, @DVR_SET_IOCTL@,
    and @DVR_REP_IOCTL@ messages.

    Common Fields:

        "status" - Status of the reply.

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@, except for per-board
        ioctl commands. For these, the field is ignored. If "ioctl_code" is
        set to DVR_IOCTL_CODE_LOAD_BALANCE, this field must be
        @DVR_JOB_CAMERA_DECODE@.

        "job_id" - A unique job ID. Ignored for per-board ioctl commands.

        "ioctl_code" - The IOCTL command code.

    Image Control Fields (used with @DVR_IOCTL_CODE_IMG_CONTROL@):

        "flags" - Bit field specifying which of the following fields is
        valid.

        "hue" - Value to set for hue control (for set), current value of
        hue control (for get).

        "sat" - Value to set for saturation control (for set), current
        value of saturation control (for get).

        "brt" - Value to set for brightness control (for set), current
        value of brightness control (for get).

        "cont" - Value to set for contrast control (for set), current
        value of contrast control (for get).

        "sharp" - Value to set for sharpness control (for set), current
        value of sharpness control (for get).

    Register R/W Fields (used with @DVR_IOCTL_CODE_DECODER_REGS@):

        "device_id" - Device ID of the decoder to access.

        "reg_num" - Register number to read or write.

        "val" - Value to write, or value read back.

    Channel control fields (used with @DVR_IOCTL_CODE_OPEN_CHAN@ etc.):
    This port ID is per board not per channel.

        "port" - The SCT port number to use for the channel.

    Motion control fields (used with @DVR_IOCTL_CODE_MOTION_FRAME_FREQ@):

        "frequency" - How often to send the motion value frames.
        0 means never, 1 means every frame, and so on. Note that
        even if this is set to zero, a motion value frame will be
        sent every time the motion alarm threshold is exceeded.

    Gain mode field (used with @DVR_IOCTL_CODE_GAIN_MODE@):

        "value" - Gain setting value.
        This value will be passed through as-is to the hardware.

    Video preprocessing control field (used with @DVR_IOCTL_CODE_VPP_CONTROL@):

        "actions" - Bitfield of VPP actions to control preprocessing
        behavior. Set the corresponding action flags to enable the
        actions, clear the flags to disable them.

        "luma_strength" - The value of luma strength. Its valid range
        is between 0 and 5. The default value is 2.

        "chroma_strength" - The value of chroma strength. Its valid range
        is between 0 and 5. The default value is 0.

    Termination value field (used with @DVR_IOCTL_CODE_TERMINATION@):

        "value" - Enumerated constant defining the termination value.

    LED enable status field (used with @DVR_IOCTL_CODE_LED@):

        "type" - The type of LED to set its enable status.

        "number" - A zero based LED number within the LED type group.

        "enable" - Set to true to turn the LED on, false to turn it off.

    Raw video format field (used with @DVR_IOCTL_CODE_RAWV_FORMAT@):

        "format" - The raw video output format. Must be one of the
        @dvr_rawv_format_e@ values. The default format at startup is
        @DVR_RAWV_FORMAT_YUV_4_2_0@.

    PCI DMA burst size field (used with @DVR_IOCTL_CODE_PCI_BURST_SIZE@):

        "size" - Burst size in bytes. Allowed values are 128, 64, 32 and 16.
        The burst size must be set before any channels starts any kind of
        streaming.

    Flash update fields (used with @DVR_IOCTL_CODE_UPDATE_FLASH@):

        "file_size" - Number of bytes that the host is going to download
        for storage into flash.

        "check_only" - If set, the firmware will only check the parameters
        and the existence of a flash device.

    Force key frame fields (used with @DVR_IOCTL_CODE_FORCE_KEY_FRAME@):

        "stream_id" - Specifies which stream the command applies to.

    Temperature fields (used with @DVR_IOCTL_CODE_TEMPERATURE@):

        "enable" - Zero to disable reporting, nonzero to enable.

        "interval" - Measurement interval in seconds, valid range 1 - 240.
        Default interval is 15 seconds.

        "threshold" - Reporting threshold in deg C. Valid range is 0 - 100.
        Default threshold is 85 deg C.

        "m_time" - Time temperature was last measured. Valid only in response
        to a @DVR_GET_IOCTL@ command.

        "degrees" - Temperature detected, with any fractional portion removed.
        Valid only in response to a @DVR_GET_IOCTL@ command.

        "tenths" - Fractional portion of temperature detected, in tenths of a
        degree.  Valid only in response to a @DVR_GET_IOCTL@ command.

    GPIO fields (used with @DVR_IOCTL_CODE_GPIO@):

        "pe_id" - The ID of the PE on which the operation is to be executed.

        "gpio_num" - GPIO pin number.

        "val" - For writes, the value to set the pin to - 0 or 1.
        For reads, the current value of the pin, 0 or 1.

        "dir" - For writes, the direction to set the pin to, IN (0) or OUT (1).
        For reads, the direction the pin is set to.

    @NOTE@: Not all products support GPIO control. In particular, S6 products
    do not support this feature.

    TWI fields (used with @DVR_IOCTL_CODE_TWI@):

        "pe_id" - The ID of the PE on which the operation is to be executed.

        "ra_16bit" - If non-zero, use 16-bit register address. If zero, use 8-bit
        register address.

        "twi_addr" - TWI bus address of the target device.

        "reg_addr" - Address of register to be read or written.

        "reg_data" - For writes, the value to be written. For reads, the value
        read from the device.

    @NOTE@: Not all products support TWI access. In particular, S6 products do
    not support this feature.

    Crop specification fields (used with @DVR_IOCTL_CODE_IMG_CROP@):

        "stream_id" - The ID of the encoded stream being changed.

        "src" - The source picture to use. Must be one of the valid @dvr_crop_src_e@
        values.

        "cropped_w" - Cropped width. Must be a multiple of 16. If this is set
        to zero, cropping will be skipped. Minimum value is 64, maximum value
        is source picture width.

        "cropped_h" - Cropped height. Must be an even number. If this is set
        to zero, cropping will be skipped. Minimum value is 64, maximum value
        is source picture height.

        "crop_x_offset" - X offset of the top left hand corner of the crop
        region within the source picture. Must be an even number, and must
        be less than the source picture width.

        "crop_y_offset" - Y offset of the top left hand corner of the crop
        region within the source picture. Must be even number, and must be
        less than the source picture height.

    Scaling specification fields (used with @DVR_IOCTL_CODE_IMG_SCALE@):

        "stream_id" - The ID of the encoded stream being changed.

        "src" - The source picture to use. Must be one of the valid @dvr_crop_src_e@
        values. This must match the "src" specified in the crop settings, if the
        crop settings are valid.

        "scaled_w" - Scaled picture width. Must be a multiple of 16. If this is
        zero then scaling is skipped. Minimum value is 64, maximum value is
        source picture width.

        "scaled_h" - Scaled picture height. Must be an even number. If this is
        zero then scaling is skipped. Minimum value is 64, maximum value is
        source picture height.

    Cryptographic EEPROM fields (used with @DVR_IOCTL_CODE_CRYPTO_EEPROM@):

        "cmd" - EEPROM command to issue.  Some commands are only supported
        through DVR_IOCTL_GET; others only through DVR_IOCTL_SET.  See
        @dvr_crypto_eeprom_cmd_e@ for more details.

        "zone" - EEPROM zone for specified command.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "access_type" - EEPROM access mode type.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "fuse_id" - EEPROM fuse enumeration.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "pass_type" - EEPROM password type.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "zone_addr" - EEPROM address within zone.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "nbytes" - Number of valid data bytes.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

        "data" - Up to 8 bytes of data.  Not all commands require
        this parameter.  See @dvr_crypto_eeprom_cmd_e@ for more details.

    IR Cut Filter fields (used with @DVR_IOCTL_CODE_IR_CUT_FILTER@):

        "enable" - Zero to disable IR cut filter, nonzero to enable.

    SMO disable mode fields (used with @DVR_IOCTL_CODE_SMO_DISABLE_MODE@):

        "mode" - Specify the mode when disabled. Must be one of the
        supported DVR_SMO_DISABLE_MODE_* values.

    Raw Command fields as result of DVR_SET_IOCTL or DVR_GET_IOCTL
    with union send (used with @DVR_IOCTL_CODE_RAW_CMD):

        "sub_system" - The sub- system to re-direct this raw command.

        "pe_num" - Ignore if job_id is not 0xFF. If it is 0xFF,
        the command is sent to the sub-system module in all
        the PEs. Otherwise, send to the specified PE.

        "big_endean" - Set to 1 if the host application is in big_endean system.
        otherwise, it is set to zero.

        "cmd_len" - The size of the command buffer that will be sent

    Raw Command fields as result of DVR_REP_IOCTL with union get
    (used with @DVR_IOCTL_CODE_RAW_CMD):

        "response" -- 12 bytes of data received return from the sub-system
        to be sent to the DVR Host Application.

    Encoder mode fields (used with @DVR_IOCTL_CODE_ENC_MODE@):

        "mode" -- The current behavior of h.264 video encoder to apply
        to all the channels within the board. Set to DVR_ENC_MODE_DEFAULT for default
        behavior. This field can be ORed of all the supported modes.

        @NOTE@: job_type and job_id fields are ignored

    Snapshot configuration fields (used with @DVR_IOCTL_CODE_SNAPSHOT@):

        "res" -- The size of the snapshot frame. Currently, the only supported size
        are: DVR_VIDEO_RES_FULL, DVR_VIDEO_RES_CIF, and DVR_VIDEO_RES_QCIF.

    Load balancing configuration fields (used with @DVR_IOCTL_CODE_LOAD_BALANCE@):
    "job_type" must be set to @DVR_JOB_CAMERA_DECODE@.

    NOTE: DEPRECATED.  DO NOT USE.

        "component" -- Type of load balancing action to be taken.
        See "dvr_load_balance_e" for more details.

        "u2" -- Union of component-specific parameters.  See "dvr_load_balance_e"
        for more details for each component's parameters.
        If the system supports more than one SMO, you can
        send @DVR_IOCTL_CODE_LOAD_BALANCE@ with different "smo_port_id" or
        "resize_at_source" criteria.

             "resize_at_source" -- Set to (0) if the resizing of decoder
             output should be done at the PE where given "smo_port_id" is located.
             Set to non-zero if the output of the decoder
             should be resized on the same PE as where the decoder job is located.

             "smo_port_id" - SMO port ID (1-based) where to resize the
             decoder's output. This field is ignored if "resize_at_source"
             is set to (1).

    Frame skip configuration fields (used with @DVR_IOCTL_CODE_FRAME_SKIP@):

        "count" -- The number of frames to skip per sample.

        "sample_size" - The size of the sample for skipping.

    Memory usage fields (used with @DVR_IOCTL_CODE_MEMORY_USAGE@):

        "enable" - Zero to disable reporting, nonzero to enable.

        "interval" - Measurement interval in seconds, valid range 1 - 240.
        Default interval is 30 seconds.

        "pe_id" - PE id on which memory usage measurement is enabled or disabled.

        "m_time" - Time memory usage was last measured. Valid only in response
        to a @DVR_GET_IOCTL@ command.

        "threshold" - Reporting threshold in Mega Bytes. Valid range is 0 - 500.
        Default threshold is 120MB.

    Encoder parameters (used with @DVR_IOCTL_CODE_ENC_PARAMS@):

        "stream_id" - The ID of the encoded stream being changed.

        "h264" -- Use this union to set any h.264 encoder paramters that can
        not be set in dvr_encode_info_t

            "i_percent" - If none-zero, each I-Frame will be iTh percent of
            the max bit rate.

    Encoder alarms parameters (used with @DVR_IOCTL_CODE_ENC_ALARM_PARAMS@):

        "stream_id" - The ID of the encoded stream being changed.

        "h264" -- Use this union to set any h.264 encoder paramters that can
        not be set in dvr_encode_info_t

            "i_percent" - If none-zero, each I-Frame will be iTh percent of
            the max bit rate.

    Signal event reporting (used with @DVR_IOCTL_CODE_EVENT_REPORTING@):

        "event_id" -- The signal event ID. The supported signals are:

            DVR_SIGNAL_PCI_CONGESTION

        "interval" - Reporting interval in seconds, valid range 0 - 240.
        A value of zero means reset the last reporting of this event and
        send the signal only once again as long as the event still exit.
        Default interval is 0 seconds.

    Get/set MAC address (used with @DVR_IOCTL_CODE_MACADDR@):

        "macaddr" -- The MAC address returned by or provided to firmware.

    Internal use only:

    decoder_debug used with @DVR_IOCTL_CODE_DECODER_DEBUG@.

    encoder_debug used with @DVR_IOCTL_CODE_ENCODER_DEBUG@.

    kname_debug used iwth @DVR_IOCTL_CODE_KNAME_DEBUG@

    All reserved fields must be set to zero.

    @NOTE@: Default values for image control parameters are dependent
    on the type of hardware used. To obtain the defaults for a specific
    board, use the @DVR_GET_IOCTL@ command to read the parameters before
    changing any of them.
************************************************************************/
typedef struct dvr_ioctl_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            ioctl_code;

    union {
        struct {
            sx_uint16   flags;
            sx_uint16   reserved2;
            sx_uint8    hue;
            sx_uint8    sat;
            sx_uint8    brt;
            sx_uint8    cont;
            sx_uint8    sharp;
            sx_uint8    reserved3;
            sx_uint16   reserved4;
        } img_ctrl;
        struct {
            sx_uint8    device_id;
            sx_uint8    reg_num;
            sx_uint16   val;
            sx_uint32   reserved2;
            sx_uint32   reserved3;
        } reg;
        struct {
            sx_uint16   port;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } chan;
        struct {
            sx_uint8    frequency;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } motion;
        struct {
            sx_uint8    value;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } gain_mode;
        struct {
            dvr_term_e  value:8;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } termination;
        struct {
            dvr_led_type_e  type:8;
            sx_uint8        number;
            sx_uint8        enable;
            sx_uint8        reserved1;
            sx_uint32       reserved2;
            sx_uint32       reserved3;
        } led;
        struct {
            sx_uint32       actions;
            sx_uint8        md_threshold;
            sx_uint8        luma_strength;
            sx_uint8        chroma_strength;
            sx_uint8        reserved2;
            sx_uint32       reserved3;
        } vpp_ctrl;
        struct {
            dvr_rawv_format_e    format:8;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } rawv_format;
        struct {
            sx_uint32            size;
            sx_uint32            reserved1;
            sx_uint32            reserved2;
        } pci_bs;
        struct {
            sx_uint8             interlaced_cif;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } cif_mode;
        struct {
            sx_uint32            file_size;
            sx_uint8             check_only;
            sx_uint8             reserved0;
            sx_uint16            reserved1;
            sx_uint32            reserved2;
        } flash;
        struct {
            sx_uint8             stream_id;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } key;
        struct {
            sx_uint8             enable;
            sx_uint8             interval;
            sx_uint8             threshold;
            sx_uint8             reserved1;
            sx_int32             m_time;
            sx_uint8             degrees;
            sx_uint8             tenths;
            sx_uint8             reserved2[2];
        } temp;
        struct {
            sx_uint8             pe_id;
            sx_uint8             gpio_num;
            sx_uint8             val;
            sx_uint8             dir;
            sx_uint32            reserved1;
            sx_uint32            reserved2;
        } gpio_data;
        struct {
            sx_uint8             pe_id;
            sx_uint8             ra_16bit;
            sx_uint16            twi_addr;
            sx_uint16            reg_addr;
            sx_uint16            reg_data;
            sx_uint32            server_task_id;
        } twi_data;
        struct {
            sx_uint8             stream_id;
            dvr_crop_src_e       src:8;
            sx_uint8             reserved1[2];
            sx_uint16            cropped_w;
            sx_uint16            cropped_h;
            sx_uint16            crop_x_offset;
            sx_uint16            crop_y_offset;
        } crop_data;
        struct {
            sx_uint8             stream_id;
            dvr_crop_src_e       src:8;
            sx_uint8             reserved1[2];
            sx_uint16            scaled_w;
            sx_uint16            scaled_h;
            sx_uint32            reserved2;
        } scale_data;
        struct {
            dvr_crypto_eeprom_cmd_e cmd:8;
            sx_uint8                zone;
            union {
                sx_uint8            access_type;
                sx_uint8            fuse_id;
                sx_uint8            pass_type;
                sx_uint8            zone_addr;
            } u2;
            sx_uint8                nbytes;
            sx_uint8                data[8];
        } crypto_eeprom;

        struct {
            sx_uint8             stream_id;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            code_version;
            sx_uint32            reserved3;
        } encoder_debug;

        struct {
            sx_uint8             stream_id;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            code_version;
            sx_uint32            reserved3;
        } decoder_debug;

        struct {
            sx_uint8             stream_id;
            sx_uint8             index;
            char                 name[10];
        } kname_debug;

        struct {
            sx_uint8             enable;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } ir_filter;

        struct {
            sx_uint8             mode;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } smo;

        union {
            struct {
                sx_uint8         sub_system;
                sx_uint8         pe_num;
                sx_uint8         big_endian;

                sx_uint8         reserved1;
                sx_uint32        cmd_len;

                sx_uint32        reserved4;
            } send;
            struct {
                //#define SDVR_MAX_RAW_CMD_RESPONSE_SIZE 8
                sx_uint8         response[8]; //for message or short response
                sx_uint32        response_len; //for data or long response
            } get;
        } raw_cmd;

        struct {
            sx_uint8             mode;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } encoder;

        struct {
            dvr_video_res_e      res:8;
            sx_uint8             reserved0;
            sx_uint16            width;
            sx_uint16            height;
            sx_uint16            reserved1;
            sx_uint32            reserved2;
        } snapshot;

        struct {
            dvr_load_balance_e   component:8;
            sx_uint8             reserved0[3];
            union {
                struct {
                    sx_uint8     resize_at_source;
                    sx_uint8     smo_port_id;
                    sx_uint8     fw_checkonly;  //DO NOT USE
                    sx_uint8     reserved1;
                    sx_uint32    reserved2;
                } params ;
            } u2;
        } load_balance;

        struct {
            sx_uint8             count;
            sx_uint8             sample_size;
            sx_uint16            reserved1;
            sx_uint32            reserved2;
            sx_uint32            reserved3;
        } frame_skip;

        struct {
            sx_uint8             enable;
            sx_uint8             reserved0;
            sx_uint16            reserved1;
            sx_uint32            port;
            sx_uint32            max_size;
        } fw_debug;

        struct{
            sx_uint8             enable;
            sx_uint8             interval;
            sx_uint8             pe_id;
            sx_int32             m_time;
            sx_uint16            threshold;
            sx_uint16            reserved1;
        } mem_use;
        struct {
            sx_uint8         stream_id;
              sx_uint8         reserverd[3];
              union {
                struct {
                    sx_uint8         i_percent;
                    sx_uint8         reserverd[3];
                    sx_uint32        reserverd3;
                } h264;
                struct {
                    sx_uint8         i_percent;
                    sx_uint8         gop_style;
                    sx_uint8         reserverd[2];
                    sx_uint32        reserverd3;
                } h264_svc;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                } jpeg;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                 } mpeg4;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                } mpeg2;
            } u1;
        } enc_params;
        struct {
            sx_uint8         stream_id;
              sx_uint8         reserverd[3];
              union {
                struct {
                    sx_uint8         i_percent;
                    sx_uint8         reserverd[3];
                    sx_uint32        reserverd3;
                } h264;
                struct {
                    sx_uint8         i_percent;
                    sx_uint8         gop_style;
                    sx_uint8         reserverd[2];
                    sx_uint32        reserverd3;
                } h264_svc;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                } jpeg;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                 } mpeg4;
                struct {
                    sx_uint32        reserverd1;
                    sx_uint32        reserverd2;
                } mpeg2;
            } u1;
        } enc_alarm_params;
        struct{
            sx_uint8             event_id;
            sx_uint8             interval;
            sx_uint8             reserverd0;
            sx_int32             reserved1;
            sx_uint32            reserved2;
        } event_reporting;

        struct {
            sx_uint8             macaddr[6];
            sx_uint8             reserved1[2];
            dvr_tristate_e       protect:8;
            sx_uint8             reserved2[3];
        } macaddr;

        struct {
            sx_uint8             enable;
        } test_frames;
    } u1;
} dvr_ioctl_t;
sx_static_assert(sizeof(dvr_ioctl_t) == 16, dvr_ioctl_t_size);

/************************************************************************
    Command codes for IPP control.

    @DVR_CMD_GAMMA_VALUE@ - Set/get IPP gamma value (used to generate
    the gamma table).

    @DVR_CMD_IMG_CONTROL@ - Used to get and set video decoder image
    control parameters.

    @DVR_CMD_EXPOSURE_CONTROL@ - Used to get and set isp exposure configuration.

    @DVR_CMD_SHUTTER_CONTROL@ - Used to get and set isp shutter configuration.

    @DVR_CMD_IRIS_CONTROL@ - Used to get and set isp iris configuration.

    @DVR_CMD_GAIN_CONTROL@ - Used to get and set isp gain configuration.

    @DVR_CMD_IRCUT_CONTROL@ - Used to get and set isp ircut filter configuration.

    @DVR_CMD_FOCUS_CONTROL@ - Used to get and set isp focus configuration.

    @DVR_CMD_WDR_CONTROL@ - Used to get and set isp wide dynamic range configuration.

    @DVR_CMD_WB_CONTROL@ - Used to get and set isp white balance configuration.

************************************************************************/
typedef enum {
    DVR_CMD_GAMMA_VALUE,
    DVR_CMD_IMG_CONTROL,
    DVR_CMD_EXPOSURE_CONTROL,
    DVR_CMD_SHUTTER_CONTROL,
    DVR_CMD_IRIS_CONTROL,
    DVR_CMD_GAIN_CONTROL,
    DVR_CMD_IRCUT_CONTROL,
    DVR_CMD_FOCUS_CONTROL,
    DVR_CMD_WDR_CONTROL,
    DVR_CMD_WB_CONTROL,
    DVR_CMD_MOTION_IOCTL,
    DVR_CMD_FLIP_CONTROL
} dvr_cmd_e;

/************************************************************************
    VISIBLE: This structure is used by the @DVR_GET_IPP@, @DVR_SET_IPP@,
    and @DVR_REP_IPP@ messages.

    Common Fields:

        "status" - Status of the reply.

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@, except for per-board
        ioctl commands. For these, the field is ignored.

        "job_id" - A unique job ID. Ignored for per-board ioctl commands.

        "opcode" - The command code.

************************************************************************/
typedef struct dvr_ipp_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            opcode;

    union {
        struct {
            sx_uint16       gamma_value; /* Ranges from 0.1 - 10.0, *1000 */
            sx_uint16       reserved1;
            sx_uint32       reserved2;
            sx_uint32       reserved3;
        } gv;
        struct {
            sx_uint8    enable;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } motion_ctrl;
        struct {
            sx_uint16   flags;
            sx_uint16   reserved2;
            sx_uint8    hue;
            sx_uint8    sat;
            sx_uint8    brt;
            sx_uint8    cont;
            sx_uint8    sharp;
            sx_uint8    flip_mode;
            sx_uint16   reserved4;
        } img_ctrl;
        struct {
            sx_uint8    exposure_mode;
            sx_uint8    setpoint;
            sx_uint16   tl_x;
            sx_uint16   tl_y;
            sx_uint16   br_x;
            sx_uint16   br_y;
            sx_uint16   reserved3;
        } exposure_ctrl;
        struct {
            sx_uint32    shutter_target;
            sx_uint32    shutter_auto_min;
            sx_uint32    shutter_auto_max;
        } shutter_ctrl;
        struct {
            sx_uint16    iris_target;
            sx_uint16    iris_auto_min;
            sx_uint16    iris_auto_max;
            sx_uint16    iris_mode;
            sx_uint32   reserved4;
        } iris_ctrl;
        struct {
            sx_uint32    gain_target;
            sx_uint32    gain_auto_min;
            sx_uint32    gain_auto_max;
        } gain_ctrl;
        struct {
            sx_uint8    ircut_mode;
            sx_uint8    reserved;
            sx_uint8    reserved1;
            sx_uint8    reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ircut_ctrl;
        struct {
            sx_uint16   focus_mode;
            sx_uint16   focus_target;
            sx_uint16   tl_x;
            sx_uint16   tl_y;
            sx_uint16   br_x;
            sx_uint16   br_y;
        } focus_ctrl;
        struct {
            sx_uint8    enable;
            sx_uint8    level;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } wdr_ctrl;
        struct {
            sx_uint8    wb_mode;
            sx_uint8    light_source;
            sx_uint8    level;
            sx_uint8    reserved1;
            sx_uint32    R2G_gain;
            sx_uint32    B2G_gain;
        } wb_ctrl;
        struct {
            sx_uint8    flip_mode;
            sx_uint8    reserved;
            sx_uint8    reserved1;
            sx_uint8    reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } flip_ctrl;

    } u1;
} dvr_ipp_t;
sx_static_assert(sizeof(dvr_ipp_t) == 16, dvr_ipp_t_size);


/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_AV_OUTPUT@ command. These flags
    indicate which fields of the data structure are valid.

    @DVR_AVOUT_OPCODE_VSTART@ - Prepare SCT channel to start receiving
    YUV video to be displayed on the given SMO port number. After this
    message, DVR firmware accepts video frames from the DVR Host Application.

    @DVR_AVOUT_OPCODE_VSTOP@ - The SMO SCT receive channel will be closed
    and no more SMO video frames will be accepted from the DVR Host
    Applicaiton. After receiving of this command, the video frames on
    the SMO display will be cleared and the display background is
    reset to black color. Hence, there is no need to clear each
    layer individually before sending this message.

    @DVR_AVOUT_OPCODE_VCLEAR_LAYER@ - All video data in the specified overlay
    layer for the requested SMO port number will be removed from the SMO display.
    Note: The SMO SCT receive channel is still active after this command and more
    video frames can still be sent to the SMO port.

    @DVR_AVOUT_OPCODE_VFREEZE@ - Freeze the video on the SMO.

    @DVR_AVOUT_OPCODE_VUNFREEZE@ - Unfreeze the video on the SMO.

    @DVR_AVOUT_OPCODE_ASTART@ - Prepare SCT channel to start receiving
    raw audio frames to be played on the given audio out port number. After this
    message, DVR firmware accepts audio frames from the DVR Host Application.

    @DVR_AVOUT_OPCODE_ASTOP@ - The audio out SCT receive channel will be closed
    and no more audio frames will be accepted from the DVR Host
    Applicaiton.

    @DVR_AVOUT_OPCODE_OVERLAY_CREATE@ - Prepare and create a memory location
    to hold video frames of given size. The firmware will send an overlay
    identifier as result of this message to the Host DVR. The Host DVR
    will use this identifier to refer to this overlay as the overlay is
    being shown/hidden, deleted, moved, or video frames are sent to it.

    @DVR_AVOUT_OPCODE_OVERLAY_DELETE@ - The DVR host sends this opcode
    with the overlay ID that was obtained from DVR_AVOUT_OPCODE_OVERLAY_CREATE
    to delete the overlay.

    @DVR_AVOUT_OPCODE_OVERLAY_SHOW@ - The DVR host sends this opcode
    with the overlay ID that was obtained from DVR_AVOUT_OPCODE_OVERLAY_CREATE
    to show/hide the overlay. The firmware does not send a response

    @DVR_AVOUT_OPCODE_OVERLAY_SETPOS@ - The DVR host sends this opcode
    with the overlay ID that was obtained from DVR_AVOUT_OPCODE_OVERLAY_CREATE
    to set the new position of the overlay within the SMO Display. The
    firmware does not send a response.

    @DVR_AVOUT_OPCODE_OVERLAY_GETPOS@ - The DVR host sends this opcode
    with the overlay ID that was obtained from DVR_AVOUT_OPCODE_OVERLAY_CREATE
    to get the current position of the overlay within the SMO Display.

    @DVR_AVOUT_OPCODE_SET_BG_COLOR@ - The DVR host sends this opcode
    to set the background color of the SMO Display. If there is no
    background layer, this is used to paint the display. If there is
    a background layer, it is initialized to this color, and the
    overlay_clear opcode will clear it to this color.

    @DVR_AVOUT_OPCODE_OVERLAY_CONVERT@ - Run one pass of RGB-to-YUV
    conversion on the video buffer for the specified overlay layer.  The
    layer must have already been created as an RGB-formatted layer.

************************************************************************/
#define DVR_AVOUT_OPCODE_VSTART             0x1
#define DVR_AVOUT_OPCODE_VSTOP              0x2
#define DVR_AVOUT_OPCODE_VCLEAR_LAYER       0x3
#define DVR_AVOUT_OPCODE_VFREEZE            0x4
#define DVR_AVOUT_OPCODE_VUNFREEZE          0x5
#define DVR_AVOUT_OPCODE_ASTART             0x6
#define DVR_AVOUT_OPCODE_ASTOP              0x7
#define DVR_AVOUT_OPCODE_OVERLAY_CREATE     0x8
#define DVR_AVOUT_OPCODE_OVERLAY_DELETE     0x9
#define DVR_AVOUT_OPCODE_OVERLAY_SHOW       0xA
#define DVR_AVOUT_OPCODE_OVERLAY_SETPOS     0xB
#define DVR_AVOUT_OPCODE_OVERLAY_GETPOS     0xC
#define DVR_AVOUT_OPCODE_SET_BG_COLOR       0xD
#define DVR_AVOUT_OPCODE_OVERLAY_CONVERT    0xE




/************************************************************************
    VISIBLE: This structure is used by the @DVR_SET_AV_OUTPUT@
    and @DVR_REP_AV_OUTPUT@ messages.

    Common Fields:

        "status" - Status of the reply.

        "opcode" - The AVOUT command code.

        "port_num" - The SMO/audio out port number, starting from 1.

    Fields (used with @DVR_AVOUT_OPCODE_VSTART@) - struct vstart:

        "sct_port_id" - The SCT port ID that should be used by the firmware
        to receive YUV frames to be displayed on the given SMO.
        This field is assumed to be set by the
        host only when the opcode is @DVR_AVOUT_OPCODE_VSTART@.

        "video_format" - The raw video format that is going to be
        received from the host DVR Application.
        see @dvr_rawv_formats_e@.

        "max_overlay_layers" - The maximum number of overlay layers
        supported over this VOUT channel.  Limited by firmware. Not
        used. Kept for backward compatiblity.

    Fields (used with @DVR_AVOUT_OPCODE_VCLEAR_LAYER@) - struct vclear_layer:

        "layer_num" - The video layer number for which to clear the last
        video frame displayed on this SMO port. This is the layer ID
        which was obtained by DVR_AVOUT_OPCODE_OVERLAY_CREATE.

    Fields (used with @DVR_AVOUT_OPCODE_ASTART@) - struct astart:

        "sct_port_id" - The SCT port ID that should be used by the firmware
        to receive audio frames to be played on the given audio out port.
        This field is assumed to be set by the
        host only when the opcode is @DVR_AVOUT_OPCODE_ASTART@.

    Fields (used with @DVR_AVOUT_OPCODE_OVERLAY_CREATE@) - struct vcreate_layer:

        "layer_num" - This layer number/ID is returned to the DVR Host
        to identify the overlay for future actions.

        "layer_order" - The layer order compare to other overlays where
        zero is the lowest layer.

        "width" - The width of the overlay.

        "height" - The height of the overlay.

        "clone_layer" - Whether or not to clone an existing layer.

        "rgb_yuv_format" - Video format for created overlay.  Must be either
        DVR_RAWV_FORMAT_YUV_4_2_0, DVR_RAWV_FORMAT_RGB_655, or
        DVR_RAWV_FORMAT_RGB_565.  This can be different from the vstart video
        format (as long as it is smaller in size) and will provide direct-mapped
        memory into the appropriate location (pre- or post-RGB->YUV conversion).
        NOTE: only 8 bits of space are available for this field vs. the standard
        16-bit video format field.

        "direct_access" - Pointer to layer buffer memory (on board).  For
        YUV-formatted overlays, this corresponds to the alpha-blend source
        buffer; for RGB-formatted overlays, this corresponds to an intermediate
        buffer that will be converted to YUV by issuing a
        DVR_AVOUT_OPCODE_OVERLAY_ORGB2YUV opcode.

    Fields (used with @DVR_AVOUT_OPCODE_OVERLAY_DELETE@) vdeleted_layer:

        "layer_num" - The overlay number for which to be deleted.
        After this call the last video frame associate to this overlay
        will be cleared from the SMO port display. All subsequent
        actions with this overlay number will be rejected.

    Fields (used with @DVR_AVOUT_OPCODE_OVERLAY_SHOW@) vshow_layer:

        "layer_num" - The overlay number to be displayed or hiddend.

        "show" - 0 means to hide the overly. Otherwise, displayed on
        the given SMO display at the currently set poistion of overly.

    Fields (used with @DVR_AVOUT_OPCODE_OVERLAY_SETPOS@) vshow_layer:

        "layer_num" - The overlay number to be moved.

        "top_left_x" - The new top left x coordinate of the overlay.

        "top_left_y" - The new top left y coordinate of the overlay.

        The coordinate is relative to top left corner of the screen
        where 0,0 is the origin. Any negative value places the overlay
        outside of the SMO display which results the frame be crop.

    Fields (used with @DVR_AVOUT_OPCODE_SET_BG_COLOR@) vbg_smo_color:

        "y_bg_color" - The y component of the background color.
        "u_bg_color" - The u component of the background color.
        "v_bg_color" - The v component of the background.

    Fields (used with @DVR_AVOUT_OPCODE_OVERLAY_ORGB2YUV@) vconv_rgb:

        "layer_num" - The overlay number to be converted.

************************************************************************/
typedef struct dvr_avout_struct {
    dvr_status_e        status:8;
    sx_uint8            opcode;
    sx_uint8            port_num;
    sx_uint8            reserved;

    union {
        struct {
            sx_uint32   sct_port_id;
            sx_uint16   video_format;
            sx_uint16   reserved1;
            sx_uint8    max_overlay_layers;
            sx_uint8    reserved2[3];
        } vstart;
        struct {
            sx_uint8    layer_num;
            sx_uint8    reserved1[11];
        } vclear_layer;
        struct {
            sx_uint32   sct_port_id;
            sx_uint32   reserved1[2];
        } astart;
        struct {
            sx_uint8    layer_num;
            sx_uint8    layer_order;
            sx_uint16   width;
            sx_uint16   height;
            sx_uint8    clone_layer;
            sx_uint8    rgb_yuv_format;
            sx_uint32   direct_access;
        } vcreate_layer;
        struct {
            sx_uint8    layer_num;
            sx_uint8    reserved1[11];
        } vdelete_layer;
        struct {
            sx_uint8    layer_num;
            sx_uint8    alpha;
            sx_uint8    reserved1[10];
        } vshow_layer;
        struct {
            sx_uint8    layer_num;
            sx_uint8    reserved1;
            sx_int16    top_left_x;
            sx_int16    top_left_y;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
        } vsetpos_layer;
        struct {
            sx_uint8    y_bg_color;
            sx_uint8    u_bg_color;
            sx_uint8    v_bg_color;
            sx_uint8    reserved1[9];
        } vbg_smo_color;
        struct {
            sx_uint8    layer_num;
            sx_uint8    reserved1[11];
        } vconv_rgb;
    } u1;

} dvr_avout_t;
sx_static_assert(sizeof(dvr_avout_t) == 16, dvr_avout_t_size);

/****************************************************************************
    VISIBLE: The following enum describes various supported SMO capabilities.
    Each feature is supported by the SMO port if the corresponding bit is set.

    DVR_SMO_CAP_OUTPUT - Supports outputing of DVR Host Application
    generated raw video.

    DVR_SMO_CAP_OSD - Supports OSD text that is displayed only on this SMO port.

    DVR_SMO_CAP_ALPHA_BLENDING - Supports alpha blending of OSD.

    DVR_SMO_CAP_TILING - Supports tiling of video, streaming from different
    Cameras or host output on this SMO display.

    DVR_SMO_CAP_ANALOG - Supports analog video output. Typically this directly
    routes one video input to the output, so features such as tiling and OSD
    will not be available.

    DVR_SMO_CAP_ANALOG_CASCADE - Supports cascading the analog SMO outputs
    across multiple boards in a single system. The boards will have to be
    physically wired together for this to work. Only one board's SMO output
    must be enabled at any time. It is the host application's responsibility
    to ensure this.

    DVR_SMO_CAP_AUDIO_OUT - Supports one or more channels of audio output.
    You must check the audio_out_ports field of dvr_smo_attrib_t to get the
    number of audio outputs associated with this SMO port.

    DVR_SMO_CAP_EMO - Supports encoded video output of the monitor rather
    than raw video output to a display device. The encoded video is sent
    to the host.  Legacy (S6) EMOs only.

    DVR_SMO_CAP_EMO_MIRROR - Supports encoded video output mirroring of the
    monitor in addition to the display device.  The encoded video is sent
    to the host.  Not supported on legacy (S6) EMOs.
****************************************************************************/
enum dvr_smo_capabilities_e {
    DVR_SMO_CAP_OUTPUT = 1,
    DVR_SMO_CAP_OSD = 2,
    DVR_SMO_CAP_ALPHA_BLENDING = 4,
    DVR_SMO_CAP_TILING = 8,
    DVR_SMO_CAP_ANALOG = 16,
    DVR_SMO_CAP_ANALOG_CASCADE = 32,
    DVR_SMO_CAP_AUDIO_OUT = 64,
    DVR_SMO_CAP_EMO = 128,
    DVR_SMO_CAP_EMO_MIRROR = 256,
};

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SMO_ATTRIB@ and @DVR_REP_SMO_ATTRIB@.

        "status" - Status of the reply.

        "smo_port_num" - The SMO port number (starting from 1) for which
        to get capablities.

        "video_formats" - A bit map to indicate supported raw video formats.
        see @dvr_rawv_formats_e@.

        "width" - The width of the SMO display.

        "height" - The number of lines of the SMO display

        "cap_flags" - A bit map specifing all the capabilities of the
        requested SMO port. See @dvr_smo_capabilities_e@.

        "audio_out_ports" - Number of audio-out ports supported by this
        SMO port. NOTE: The audio-out port number starts from 1 starting
        from the first SMO port number with audio-out capability. It
        then gets increased sequentially by one for the number of
        audio-out ports supported by any SMO with audio-out capability.
        If the DVR board supports more audio-out ports than is supported
        by the total number of SMO ports, they will be audio-out ports
        that are not associated to any SMO and as such cannot have A/V
        sync support.

        "smo_output_format" - SMO output video standard.  Returns the
        DVR_VSTD_* video standard if set for this SMO port, or the
        default SMO video standard if not.  Only one bit may be set.

        "smo_output_formats_supported" - SMO output video standards
        supported by this SMO port.  This is a bitmask of all DVR_VSTD_*
        types supported by this SMO port.
************************************************************************/
typedef struct dvr_smo_attrib_struct {
    dvr_status_e        status:8;
    sx_uint8            smo_port_num;
    sx_uint16           video_formats;

    sx_uint16           width;
    sx_uint16           height;

    sx_uint16           cap_flags;
    sx_uint8            audio_out_ports;
    sx_uint8            reserved1;
    sx_uint16           smo_output_format;
    sx_uint16           smo_output_formats_supported;
} dvr_smo_attrib_t;
sx_static_assert(sizeof(dvr_smo_attrib_t) == 16, dvr_smo_attrib_t_size);


/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_VA@, @DVR_SET_VA@, and @DVR_REP_VA@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "opcode" - command code specifying the action to be taken. This
        is a private field to be understood by the VA module and the host
        application. The firmware does not use this field.

        "payload" - The payload consists of data exchanged between the
        VA module and the host-side entity that controls the analytics.
        The firmware has no knowledge of the contents.

        @NOTE@: Since "payload" is defined as a byte array, not byte
        swapping if performed by the SDK. If 16- or 32-bit values are
        to be exchanged, then the application is responsible for doing
        endian conversion.
************************************************************************/
typedef struct dvr_va_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            opcode;
    sx_uint8            payload[12];
} dvr_va_t;
sx_static_assert(sizeof(dvr_va_t) == 16, dvr_va_t_size);


/************************************************************************
    VISIBLE: This structure is associated with the message
    @DVR_SET_DISCONNECT@.  There is no response sent for this message.
    After receiving this message, the board will not respond to any
    future messages.
************************************************************************/
typedef struct dvr_disconnect_struct {
    sx_uint32           reserved[4];
} dvr_disconnect_t;
sx_static_assert(sizeof(dvr_disconnect_t) == 16, dvr_disconnect_t_size);

#endif /* STRETCH_DVR_COMMON_H */


