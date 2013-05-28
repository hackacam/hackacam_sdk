#pragma once
#ifndef _CGI_SDK_TRANSLATOR_H
#define _CGI_SDK_TRANSLATOR_H
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
#include <string>
#include <sdk/sdvr_sdk.h>

namespace CGI {

struct SDKTranslator {
    static sdvr_video_res_decimation_e decimation_id_to_sdk(const int decimation_id);
    static int decimation_font_table_id_to_sdk(const int decimation_id);
    static void                        decimation_to_width_height(sdvr_video_std_e vstd,
                                            sdvr_video_res_decimation_e decimation,
                                            int *width, int *height);
    static sdvr_video_res_decimation_e frame_width_height_2_decimation(sdvr_video_std_e video_std,
                                                                int width, int height);
    static const char                 *sdk_enc_mode_to_str_enc_profile(const sx_uint8 profile);
    static const char                 *sdk_id_to_str_encoder(const sdvr_venc_e codec);
    static sdvr_venc_e                 str_encoder_to_sdk_id(const std::string& codec) ;
    static sdvr_h264_encoder_mode_e    str_enc_profile_to_sdk_enc_mode(const std::string& profile);
    static sdvr_br_control_e           str_enc_rate_control_to_sdk_enc_bitrate_control(const std::string& rate_control);
    static sdvr_flip_mode_e            str_flip_mode_to_sdk_flip_mode(const std::string& flip);
    static const char                 *sdk_flip_mode_to_str_flip_mode(const sdvr_flip_mode_e flip);

    static sdvr_firmware_sub_system_e  str_to_subsystem(const std::string& subsystem);
    static sx_uint32                   sensor_sample_rate_to_video_standard(int sample_rate);
};

} // namespace CGI

#endif
