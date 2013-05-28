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
#include "sdk_translator.h"

namespace CGI {

//////////////////////////////////////////////////////////////////////////
//               Utility  Class SDKTranslator Implementation
//               This class translates between CGI server and SDK
//////////////////////////////////////////////////////////////////////////

sdvr_video_res_decimation_e SDKTranslator::decimation_id_to_sdk(const int decimation_id)
{
    switch (decimation_id)
    {
        case 1:  return SDVR_VIDEO_RES_DECIMATION_EQUAL;
        case 4:  return SDVR_VIDEO_RES_DECIMATION_FOURTH;
        case 16: return SDVR_VIDEO_RES_DECIMATION_SIXTEENTH;
        default: return SDVR_VIDEO_RES_DECIMATION_NONE;
    }
}

int SDKTranslator::decimation_font_table_id_to_sdk(const int decimation_id)
{
    switch (decimation_id)
    {
        case 1:  return SDVR_FOSD_FONT_TABLE_LARGE;
        case 4:  return SDVR_FOSD_FONT_TABLE_MEDIUM;
        case 16: return SDVR_FOSD_FONT_TABLE_SMALL;
        default: return SDVR_FOSD_FONT_TABLE_LARGE;
    }
}

/*
 *  This method returns frame width and height of the specified resolution decimation
 *  for given video standard.
 *
 *  Parameters:
 *      vstd: The video standard to use the decimation against
 *
 *      decimation: The decimation to get its width and height.
 *
 *      *width, *height: The  frame size after we apply the
 *      decimation to the given video standard.
 *
 *  Return:
 *
 *     None.
 */
void SDKTranslator::decimation_to_width_height(sdvr_video_std_e vstd,
                                        sdvr_video_res_decimation_e decimation,
                                        int *width, int *height)
{
    *height = sutil_vres_height(vstd, decimation, true);
    *width  = sutil_vres_width(vstd, decimation);
}

/*
 *  This method returns a resolution decimation which after it is applied to
 *  the camera's video standard produces a video frame equal to the given
 *  width and height.
 *
 *  Parameters:
 *
 *      width, height: The desired video frame size after we apply the
 *      decimation to the given video standard
 *
 *  Return:
 *
 *      The video resolution decimation to produce a video frame size of
 *      Width and height. If the specified width and height does not match
 *      any standard resolution decimation and the width and height are multiple
 *      of two, then the resolution custom will be returned.
 */
sdvr_video_res_decimation_e
SDKTranslator::frame_width_height_2_decimation(sdvr_video_std_e video_std, int width, int height)
{
    int new_width, new_height;

    new_height = sutil_vres_height(video_std, SDVR_VIDEO_RES_DECIMATION_EQUAL, true);
    new_width = sutil_vres_width(video_std, SDVR_VIDEO_RES_DECIMATION_EQUAL);
    if (new_height == height && new_width == width)
        return SDVR_VIDEO_RES_DECIMATION_EQUAL;

    new_height = sutil_vres_height(video_std, SDVR_VIDEO_RES_DECIMATION_FOURTH, true);
    new_width = sutil_vres_width(video_std, SDVR_VIDEO_RES_DECIMATION_FOURTH);
    if (new_height == height && new_width == width)
        return SDVR_VIDEO_RES_DECIMATION_FOURTH;

    new_height = sutil_vres_height(video_std, SDVR_VIDEO_RES_DECIMATION_SIXTEENTH, true);
    new_width = sutil_vres_width(video_std, SDVR_VIDEO_RES_DECIMATION_SIXTEENTH);
    if (new_height == height && new_width == width)
        return SDVR_VIDEO_RES_DECIMATION_SIXTEENTH;

    // This is a custom resolution only if width and height are non-zero
    // and even number otherwise, it is a bad decimation
    if ((width != 0) && (height != 0) && ((width % 2) == 0) && ((height % 2) == 0))
        return SDVR_VIDEO_RES_CUSTOM;

    return SDVR_VIDEO_RES_DECIMATION_NONE;
}

const char *SDKTranslator::sdk_enc_mode_to_str_enc_profile(const sx_uint8 profile)
{
    switch (profile)
    {
        case SDVR_H264_ENC_PROFILE_BASE: return "base";
        case SDVR_H264_ENC_PROFILE_MAIN: return "main";
        case SDVR_H264_ENC_PROFILE_HIGH: return "high";
        default:    return "base";
    }
}

const char*SDKTranslator::sdk_id_to_str_encoder(const sdvr_venc_e codec)
{
    switch (codec)
    {
        case SDVR_VIDEO_ENC_H264: return "h264";
        case SDVR_VIDEO_ENC_JPEG: return "mjpeg";
        case SDVR_VIDEO_ENC_MPEG4: return "mpeg4";
        default:                  return "none";
    }

}                                   

sdvr_venc_e SDKTranslator::str_encoder_to_sdk_id(const std::string& codec)
{
    if (codec == "mjpeg")
        return SDVR_VIDEO_ENC_JPEG;
    if (codec == "h264")
        return SDVR_VIDEO_ENC_H264;
    if (codec == "mpeg4")
        return SDVR_VIDEO_ENC_MPEG4;
    return SDVR_VIDEO_ENC_NONE;
}

sdvr_h264_encoder_mode_e SDKTranslator::str_enc_profile_to_sdk_enc_mode(const std::string& profile)
{
    if (profile == "base")
        return SDVR_H264_ENC_PROFILE_BASE;
    if (profile == "main")
        return SDVR_H264_ENC_PROFILE_MAIN;
    if (profile == "high")
        return SDVR_H264_ENC_PROFILE_HIGH;
    return SDVR_H264_ENC_MODE_BASE_LINE;
}

sdvr_br_control_e SDKTranslator::str_enc_rate_control_to_sdk_enc_bitrate_control(const std::string& rate_control)
{
    if (rate_control == "cbr")
        return SDVR_BITRATE_CONTROL_CBR;
    if (rate_control == "vbr")
        return SDVR_BITRATE_CONTROL_VBR;
    if (rate_control == "cq")
        return SDVR_BITRATE_CONTROL_CONSTANT_QUALITY;
    return SDVR_BITRATE_CONTROL_CBR;
}

sdvr_flip_mode_e SDKTranslator::str_flip_mode_to_sdk_flip_mode(const std::string& flip)
{
    if (flip == "both")
        return SDVR_FLIP_MODE_BOTH;
    if (flip == "horizontal")
        return SDVR_FLIP_MODE_HORIZONTAL;
    if (flip == "vertical")
        return SDVR_FLIP_MODE_VERTICAL;
    return SDVR_FLIP_MODE_NORMAL;
}

const char * SDKTranslator::sdk_flip_mode_to_str_flip_mode(const sdvr_flip_mode_e flip)
{
    if (flip == SDVR_FLIP_MODE_BOTH)
        return "both";
    if (flip == SDVR_FLIP_MODE_HORIZONTAL)
        return "horizontal";
    if (flip == SDVR_FLIP_MODE_VERTICAL)
        return "vertical";
    return "none";
}

sdvr_firmware_sub_system_e SDKTranslator::str_to_subsystem(const std::string& subsystem) {
    if (subsystem == "ipp")      return SDVR_FIRMWARE_SUB_SYSTEM_IPP;
    if (subsystem == "firmware") return SDVR_FIRMWARE_SUB_SYSTEM_FIRMWARE;
    return SDVR_FIRMWARE_SUB_SYSTEM_NONE;
}

sx_uint32  SDKTranslator::sensor_sample_rate_to_video_standard(int sample_rate) {
    switch (sample_rate) {
        case 25: return SDVR_VIDEO_STD_PAL_MASK;      
        case 30: return SDVR_VIDEO_STD_NTSC_MASK;
        default: return 0;
    }
}

}
