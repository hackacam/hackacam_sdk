#pragma once
#ifndef _RTSP_A2A_COMMUNICATIONS_H
#define _RTSP_A2A_COMMUNICATIONS_H

#include <fstream>
#include <sbl/sbl_thread.h>
#include <rtsp/rtsp.h>
#include <rtsp/rtsp_source.h>
#include "a2a_file_source.h"

/// @cond
namespace RTSP {

//************************************************************

class A2aCommunications : public SBL::Thread
    {
public:
    static A2aCommunications* create();
    A2aCommunications(void);
    ~A2aCommunications(void);
    void a2a_msg_recv_callback(int nPeerIndex,int nMsgClass, void * msgBuf);
    void a2a_buf_recv_callback(a2a_channel_t channel, void *buffer, int size);
    void a2a_file_buf_recv_callback(a2a_channel_t channel, void *buffer, int size);
    void handleCreateMsg(void * pMsgBuf);
    void handleGetMsg(void * pMsgBuf);
    void handleDeleteMsg(void * pMsgBuf);
    void handleUpdateMsg(void * pMsgBuf);
    void handleResponseMsg(void * pDataBuf);
    void handleResponseGetMsg(void * pDataBuf);
    void handleCreateA2AFileStreams(msg_connect_t *pMsgConnect,a2a_peer_t a2a_peerHandle);
    void start_thread();    // for stats
private:
    a2a_peer_t      _a2a_peerHandle;
    a2a_channel_t   _a2a_chanHandle;
    a2a_channel_t   _a2a_file_chan_handle;
    uint32_t        _num_a2a_file_streams;
    A2aFileSource*  _a2a_file_sources[MAX_STREAMING_FILE_COUNT];
    };

//************************************************************

};    // RTSP
/// @endcond

#endif
