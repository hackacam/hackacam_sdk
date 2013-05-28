#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstring>
#include <sys/select.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_exception.h>
#include <rtsp/rtsp.h>
#include "a2a_communications.h"

extern void a2a_set_stream_desc(int chan_num,int enc_stream_id,RTSP::EncoderType encoder_type);

namespace RTSP {

void a2a_thunk_msg_recv_callback(void * context, int nPeerIndex,int nMsgClass, void * msgBuf)
    {
    reinterpret_cast<A2aCommunications*>(context)->a2a_msg_recv_callback(nPeerIndex,nMsgClass,msgBuf);
    }
void a2a_thunk_buf_recv_callback(void *context,a2a_channel_t channel, void *buffer, int size)
    {
    reinterpret_cast<A2aCommunications*>(context)->a2a_buf_recv_callback(channel,buffer,size);
    }
void a2a_thunk_file_buf_recv_callback(void *context,a2a_channel_t channel, void *buffer, int size)
    {
    reinterpret_cast<A2aCommunications*>(context)->a2a_file_buf_recv_callback(channel,buffer,size);
    }

static void fatal_err(int err , const char *msg)
{
    if (err != 0) {
        printf("*** FATAL ERROR: %s (program halted): err = %d\n", msg, err);
        exit(err);
    }
}

A2aCommunications* A2aCommunications::create() {
    A2aCommunications* a2a_comm = new A2aCommunications();
    return a2a_comm;
}

void A2aCommunications::a2a_buf_recv_callback(a2a_channel_t channel, void *buffer, int size)
{
    sdvr_av_buffer_t* av_buffer = (sdvr_av_buffer_t*)buffer;
    sx_uint8          chan_num  = av_buffer->channel_id;
    sdvr_frame_type_e frameType = av_buffer->frame_type;
    sx_uint32         stream_id = av_buffer->stream_id;
    
    RTSP::EncoderType encoder_type = RTSP::UNKNOWN_ENCODER;
    switch (frameType)
    {
        case SDVR_FRAME_AUDIO_ENCODED:
        case SDVR_FRAME_RAW_AUDIO:
        case SDVR_FRAME_RAW_VIDEO:
            break;
        case SDVR_FRAME_H264_SVC_SEI:
        case SDVR_FRAME_H264_SVC_PREFIX:
        case SDVR_FRAME_H264_SVC_SUBSET_SPS:
        case SDVR_FRAME_H264_SVC_SLICE_SCALABLE:
        case SDVR_FRAME_H264_IDR:
        case SDVR_FRAME_H264_I:
        case SDVR_FRAME_H264_P:
        case SDVR_FRAME_H264_B:
        case SDVR_FRAME_H264_SPS:
        case SDVR_FRAME_H264_PPS:
            if (encoder_type == RTSP::UNKNOWN_ENCODER)
                encoder_type = RTSP::H264;
        case SDVR_FRAME_MPEG4_I:
        case SDVR_FRAME_MPEG4_P:
        case SDVR_FRAME_MPEG4_VOL:
        case SDVR_FRAME_MPEG2_I:
        case SDVR_FRAME_MPEG2_P:
        case SDVR_FRAME_JPEG:
            if (encoder_type == RTSP::UNKNOWN_ENCODER)
                encoder_type = RTSP::MJPEG;
                {                
                //sx_uint8  chan_num   = sdvr_get_chan_num(handle);
                sx_uint8* frame      = av_buffer->payload;
                sx_uint32 frame_size = av_buffer->payload_size;

                sx_uint64  timestamp64;
                sdvr_get_buffer_timestamp(av_buffer, &timestamp64);

                /* convert from hardware 100 KHz clock to 90 KHz timestamp clock */
                //sx_uint32 timestamp = timestamp64 * 9 / 10;
                sx_uint32 timestamp = timestamp64;

                sx_uint32 seq_number, frame_number, drop_count;
                sdvr_av_buf_sequence(av_buffer, &seq_number, &frame_number, &drop_count);
                a2a_set_stream_desc(chan_num,stream_id,encoder_type);
                rtsp_send_frame(chan_num,stream_id, frame, frame_size, timestamp, encoder_type);
                }

            break;
    }
    a2a_buffer_free(channel, buffer);
}

void A2aCommunications::handleCreateMsg(void * pMsgBuf)
{
    arm_msg_hdr_t*  pMsgHdr = (arm_msg_hdr_t *)pMsgBuf;
    a2a_errcode_t   a2aErr;
    switch (pMsgHdr->opcode)
    {
        case OPCODE_DEVICE:
            switch (pMsgHdr->device_type)
            {
                case DEVICE_TYPE_ARM:
                    {
                        msg_connect_t *pMsgDeviceInfo = (msg_connect_t*) pMsgBuf;
                        //printf("Received Create:OPCODE_DEVICE:DEVICE_TYPE_ARM\n");
                        a2aErr = a2a_channel_accept(_a2a_peerHandle, pMsgDeviceInfo->a2a_chan_num, a2a_thunk_buf_recv_callback,(void*)this,&_a2a_chanHandle);
                        fatal_err(a2aErr, "a2a_channel_accept() failed");
                        a2aErr = a2a_message_send(_a2a_peerHandle, VRM_RESPONSE_MSG_CLASS,pMsgHdr, A2A_MAX_MESSAGE_LEN);
                        fatal_err(a2aErr, "a2a_message_send(VRM_RESPONSE_MSG_CLASS) failed");
                        break;
                    }
                case DEVICE_TYPE_CHAN_ENCODER:
                    {
                        //msg_device_info_t *pMsgDeviceInfo = (msg_device_info_t*) pMsgBuf;
                        //printf("Received Create:OPCODE_DEVICE:DEVICE_TYPE_CHAN_ENCODER\n");
                        a2aErr = a2a_message_send(_a2a_peerHandle, VRM_RESPONSE_MSG_CLASS,pMsgHdr, A2A_MAX_MESSAGE_LEN);
                        fatal_err(a2aErr, "a2a_message_send(VRM_RESPONSE_MSG_CLASS) failed");
                    }
                    break;
            }
            break;
        case OPCODE_FILE_STREAMS:
            handleCreateA2AFileStreams((msg_connect_t*)pMsgBuf,_a2a_peerHandle);
            break;
        default:
            break;
    }
}

//************************************************************

void A2aCommunications::handleCreateA2AFileStreams(msg_connect_t *pMsgConnect,a2a_peer_t a2a_peerHandle)
    {
    uint32_t i;
    arm_msg_hdr_t*  pMsgHdr = (arm_msg_hdr_t *)pMsgConnect;
    a2a_errcode_t   a2aErr;
    //pthread_t file_play_thread;
    
    _num_a2a_file_streams = pMsgConnect->maxCameraCount;
    if(MAX_STREAMING_FILE_COUNT < _num_a2a_file_streams)
        {
        printf("handleCreateA2AFileStreams() Too many streams = %d !!!\n",_num_a2a_file_streams);
        _num_a2a_file_streams = 0;
        return;
        }
    for(i = 0;i < _num_a2a_file_streams;i++)
        {
        _a2a_file_sources[i] = A2aFileSource::create(a2a_peerHandle,i, RTSP::Source::create_streamer(1434));
        _a2a_file_sources[i]->play();
        }
    create_thread(); // start stats
    a2aErr = a2a_channel_accept(a2a_peerHandle, pMsgConnect->a2a_chan_num, a2a_thunk_file_buf_recv_callback,(void*)this,&_a2a_file_chan_handle);
    if(A2A_NO_ERROR != a2aErr)printf("a2a_channel_accept() = %d !!!\n",a2aErr);
    a2aErr = a2a_message_send(a2a_peerHandle, VRM_RESPONSE_MSG_CLASS, pMsgHdr, A2A_MAX_MESSAGE_LEN);
    if(A2A_NO_ERROR != a2aErr)printf("a2a_message_send(VRM_RESPONSE_MSG_CLASS) = %d !!!\n",a2aErr);
    }

//************************************************************

void A2aCommunications::a2a_file_buf_recv_callback(a2a_channel_t channel, void *buffer, int size)
    {
    file_buf_header_t *phdr;
    phdr = (file_buf_header_t*)buffer;
    
    if(A2A_MAGIC_NUM != phdr->magic)
        {
        a2a_buffer_free(channel,buffer);
        printf("a2a_file_buf_recv_callback(): Bad magic = 0x%08X !!!\n",phdr->magic);
        }
    if((size-sizeof(file_buf_header_t)) != (S7_SEG_LEN_MASK & (phdr->segmentLen)))
        {
        a2a_buffer_free(channel,buffer);
        printf("a2a_file_buf_recv_callback(): Buffer size inconsistency size = %d but segmentLen = %d !!!\n",size,phdr->segmentLen);
        }
    if(phdr->fileIndex >= _num_a2a_file_streams)
        {
        a2a_buffer_free(channel,buffer);
        printf("a2a_file_buf_recv_callback(): Invalid fileIndex = %d !!!\n",phdr->fileIndex);
        }
    _a2a_file_sources[phdr->fileIndex]->push_buffer(channel,(uint8_t*)buffer);
    }

//************************************************************

void A2aCommunications::handleGetMsg(void * pMsgBuf)
{

}
void A2aCommunications::handleDeleteMsg(void * pMsgBuf)
{

}
void A2aCommunications::handleUpdateMsg(void * pMsgBuf)
{
        arm_msg_hdr_t *pMsgHdr = (arm_msg_hdr_t *)pMsgBuf;
        a2a_errcode_t   a2aErr;

        switch (pMsgHdr->opcode)
        {
            case OPCODE_STREAM_CODEC:
                switch (pMsgHdr->device_type)
                {
                    case DEVICE_TYPE_CHAN_ENCODER:
                        {
                            a2aErr = a2a_message_send(_a2a_peerHandle, VRM_RESPONSE_MSG_CLASS, pMsgHdr, A2A_MAX_MESSAGE_LEN);
                        }
                        break;
                }
                break;
        }
}

void A2aCommunications::handleResponseMsg(void * pDataBuf)
{
}

void A2aCommunications::handleResponseGetMsg(void * pDataBuf)
{
}

void A2aCommunications::a2a_msg_recv_callback(int nPeerIndex, int nMsgClass, void * msgBuf)
{
    if ( nPeerIndex == 0)
    {
        switch(nMsgClass)
        {
        case VRM_MSG_CLASS_NONE:
            break;            // No message is arrived.
        case VRM_CREATE_MSG_CLASS:
            handleCreateMsg( msgBuf);
            break;
        case VRM_GET_MSG_CLASS:
            handleGetMsg(msgBuf);
            break;
        case VRM_DELETE_MSG_CLASS:
            handleDeleteMsg(msgBuf);
            break;
        case VRM_UPDATE_MSG_CLASS:
            handleUpdateMsg(msgBuf);
            break;
        case VRM_RESPONSE_MSG_CLASS:
            handleResponseMsg(msgBuf);
            break;
        case VRM_RESPONSE_GET_MSG_CLASS:
            handleResponseGetMsg(msgBuf);
            break;
        }
    }
}

void A2aCommunications::start_thread()
    {
	uint32_t i;
    struct timespec time;
    float now_secs,delta_secs;
	sx_uint32 channel_data_total_now[MAX_STREAMING_CHAN_COUNT] = {0};
	static float last_report_secs = 0.0;
    static sx_uint32 channel_data_total_prev[MAX_STREAMING_CHAN_COUNT] = {0};
    
    if(0 == STAT_REPORT_SECS)return;    // no stats
    
    while(1)
        {
        while(1)
            {
            clock_gettime(CLOCK_MONOTONIC,&time);
            now_secs=(float)(time.tv_sec) + ((float)(time.tv_nsec)/1000000000.0);
            if(0.0 == last_report_secs)
		        {
                for(i = 0;i < _num_a2a_file_streams;i++)
                    {
                    if(NULL == _a2a_file_sources[i])break;
                    channel_data_total_prev[i]=_a2a_file_sources[i]->get_total_bytes_received();
                    }
		        last_report_secs=now_secs;
		        break;
		        }
	        delta_secs=now_secs-last_report_secs;
	        //if(delta_secs < STAT_REPORT_SECS)break;
        	
            for(i = 0;i < _num_a2a_file_streams;i++)
                {
                if(NULL == _a2a_file_sources[i])break;
                channel_data_total_now[i]=_a2a_file_sources[i]->get_total_bytes_received();
                }
	        printf("=========================================\n");
	        for(i=0;i < _num_a2a_file_streams;i++)
		        {
		        printf("channel %2d: %8.5f Mbits\n",i,8.0*(channel_data_total_now[i]-channel_data_total_prev[i])/(1000000.0*delta_secs));
		        }
	        memcpy(channel_data_total_prev,channel_data_total_now,sizeof(channel_data_total_now));
	        last_report_secs=now_secs;
	        break;
	        }
	    sleep(STAT_REPORT_SECS);
	    }
    }

A2aCommunications::A2aCommunications(void)
{
    a2a_errcode_t a2aErr;
    
    // Initialize the A2A library
    a2a_set_test_mode(1);
    a2a_set_timeout(0);
    a2aErr = a2a_open_peer(0,VIDEO_BUF_SIZE,MAX_A2A_BUF_COUNT,MAX_A2A_BUF_COUNT, &_a2a_peerHandle);
    fatal_err(a2aErr, "a2a_open_peer(0) failed");
    a2aErr = a2a_register_message_callback(_a2a_peerHandle,A2A_MSG_CLASS_ANY, a2a_thunk_msg_recv_callback,(void*)this);
    fatal_err(a2aErr, "a2a_register_message_callback(0) failed");
}

A2aCommunications::~A2aCommunications(void)
{
    a2a_close_peer(_a2a_peerHandle);
}

};    // RTSP


