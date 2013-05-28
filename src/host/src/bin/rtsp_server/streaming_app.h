#pragma once
#ifndef _STREAMING_APP_H
#define _STREAMING_APP_H
#include <cstdio>
#include "a2a_communications.h"


//! A dummy application, which is used in standalone rtsp_server and rtp_streamer
/*! When linked with PSIA server (or any other application using server), PSIA
    server must derive from StreamingApp and provide these service */
class Application : public RTSP::Application {
public:
    //! Constructor checks if running on vrm or ipcam
    Application() : _pe_id (0) {
        FILE *fp;
        fp = fopen("/proc/stretch_pe_id", "rb");
        if (NULL != fp) {
            fscanf(fp, "%d", &_pe_id);
            fclose(fp);
        }
    }
    //! Return PE number
    int pe_id() const { return _pe_id; }

    //! Return stream_id from channel number and stream number
    //! @param  channel_num     corresponds to a distinct video source
    //! @param  stream_num      denotes primary or secondary stream from the same source
    //! @return unique stream_id
    virtual int get_stream_id(unsigned int channel_num, unsigned int stream_num);

    //! Return stream_id from stream name. It must be a number.
    virtual int get_stream_id(const char* name);

    //! Empty play method
    virtual void play(int) {}
    //! Empty teardown method
    virtual void teardown(int) {}

    virtual int describe(int stream_id, RTSP::Application::StreamDesc& stream_desc);

private:
    int    _pe_id;
};

extern Application application;
#endif
