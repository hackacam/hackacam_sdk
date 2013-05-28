#pragma once
#ifndef RTSP_SDK_H
#define RTSP_SDK_H

#include <rtsp/rtsp.h>
#include <stdint.h>

extern void sdk_setup(char* romfile_path, char* streams, int gop_size = 30, int bitrate = 8000,
                      int quality = 50);

extern void rtsp_send_frame(unsigned int chan_num, unsigned int stream_id, uint8_t* frame,
                            int size, unsigned int timestamp);
#endif
