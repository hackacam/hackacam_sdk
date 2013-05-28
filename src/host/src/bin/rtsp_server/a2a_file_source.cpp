#include <cstring>
#include <sys/select.h>
#include <sbl/sbl_logger.h>
#include <sbl/sbl_exception.h>
#include "a2a_file_source.h"
#include <rtsp/rtsp.h>

extern pthread_mutex_t a2a_stats_mutex;
extern sx_uint32 channel_data_total[MAX_STREAMING_CHAN_COUNT];

namespace RTSP {

//************************************************************
// IndexFifo code
// these return >=0 for success or -1 for failure
IndexFifo::IndexFifo()
    {
    _read = 0;
    _write = 0;
    _mask = A2A_FILE_BUFFER_FIFO_SIZE-1;    // A2A_FILE_BUFFER_FIFO_SIZE must be a power of 2
    }
int IndexFifo::get_read()
    {
    if(_read == _write)return(-1);    // fifo empty
    return(_read);
    }
int IndexFifo::advance_read()
    {
    if(_read == _write)return(-1);    // failure - fifo empty
    _read = (_mask & (_read + 1));
    return(0);    // success
    }
int IndexFifo::get_write()
    {
    int next_write;
    next_write = (_mask & (_write + 1));
    if(_read == next_write)return(-1);    // fifo full
    return(_write);
    }
int IndexFifo::advance_write()
    {
    int next_write;
    next_write = (_mask & (_write + 1));
    if(_read == next_write)return(-1);    // failure - fifo full
    _write = next_write;
    return(0);    // success
    }
int IndexFifo::num_in()
    {
    return(_mask & (_write-_read));
    }
//************************************************************
// BinarySemaphore code
BinarySemaphore::BinarySemaphore(void)
    {
    pthread_mutex_init(&_mutex,NULL);
    pthread_cond_init(&_cond,NULL);
    _waiting = 0;
    _signaled = 0;
    }
BinarySemaphore::~BinarySemaphore(void)
    {
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
    }
void BinarySemaphore::wait(void)
    {
    pthread_mutex_lock(&_mutex);
    while(0 == _signaled)
        {
        _waiting = 1;
        pthread_cond_wait(&_cond,&_mutex);
        _waiting = 0;
        }
    _signaled = 0;
    pthread_mutex_unlock(&_mutex);
    }
void BinarySemaphore::post(void)
    {
    unsigned int was_waiting;
    pthread_mutex_lock(&_mutex);
    _signaled = 1;
    was_waiting = _waiting;
    pthread_mutex_unlock(&_mutex);
    if(was_waiting)pthread_cond_signal(&_cond);
    }
//************************************************************

A2aFileSource* A2aFileSource::create(a2a_peer_t a2a_peerHandle,const int id, Streamer* streamer, int fps, int ts_clock) {
    SBL_ASSERT(streamer);
    A2aFileSource* fs = new A2aFileSource(a2a_peerHandle, id, streamer, fps, ts_clock);
    return fs;
}

A2aFileSource::A2aFileSource(a2a_peer_t a2a_peerHandle,const int id, Streamer* streamer, int fps, int ts_clock) :
    Source((10 * id) + 1, streamer), _ts_delta(0), _tick(0), _period(ONE_SECOND / fps), 
    _buffer(NULL), _frame(0), _frame_size(0), _have_bytes(0), _buffer_top(NULL), _total_bytes_received(0) {
    
    _a2a_peerHandle = a2a_peerHandle;
    _file_index = id;
    _ts_delta = ts_clock / fps;
}

int A2aFileSource::frame_size(uint8_t* frame, int size) {
    return size;
}

void A2aFileSource::start_thread() {
    // _timestamp and _tick initialization. This cannot be done in constructor, 
    // because thread may be started way after constructor was called.
    _playing = true;
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    _tick = time.tv_nsec;
    play_file();
    //_file.close();
}

void A2aFileSource::play_file() {
    msg_file_req_info_t req;
    
    // place an order for the first file segment
    _next_segment_file_offset = 0;
    req.fileIndex  = _file_index;
    req.fileOffset = _next_segment_file_offset;
    req.segmentLen = REQUEST_BUF_SIZE;
    a2a_message_send(_a2a_peerHandle,VRM_FILE_REQ_MSG_CLASS,&req,sizeof(msg_file_req_info_t));
    next_frame();
        
    do {
       bool sps_pps = save_if_sps_pps(_frame, _frame_size);
        //streamer()->send_frame(_frame, _frame_size, _timestamp); // <- why does this not work?
        rtsp_send_frame(_file_index,1,_frame, _frame_size, _timestamp, RTSP::H264);
        next_frame();
        if (!sps_pps)
            wait();
    } while (_playing);
}

//************************************************************

static uint32_t get_next_fileOffset(uint8_t *buffer)
    {
    int chan_num;
    file_buf_header_t *phdr;
    sdvr_av_buffer_t *av_buffer;
    uint32_t fileOffset,bufferOffset,segmentLen,uAVBufSize;
    
    phdr = (file_buf_header_t*)buffer;
    if(A2A_MAGIC_NUM != phdr->magic)
        {
        printf("get_next_fileOffset(): Bad magic = 0x%08X !!!\n",phdr->magic);
        return(0);
        }
    chan_num = phdr->fileIndex;
    if(S7_FILE_END_FLAG & (phdr->segmentLen))
        {
        printf("get_next_fileOffset(): Looping channel %d...\n",chan_num);
        return(0);
        }
    fileOffset = phdr->fileOffset;
    segmentLen = S7_SEG_LEN_MASK & (phdr->segmentLen);
    bufferOffset = sizeof(file_buf_header_t);
    while(1)
        {
        if(segmentLen < AV_BUFFER_HEADER_SIZE)return(fileOffset);
        av_buffer = (sdvr_av_buffer_t*)(buffer+bufferOffset);
        if(SDVR_DATA_HDR_SIG != av_buffer->signature)
            {
            printf("get_next_fileOffset(): Bad signature = 0x%08X at offset %d for channel %d !!!\n",av_buffer->signature,bufferOffset,chan_num);
            return(fileOffset);
            }
        uAVBufSize = av_buffer->hdr_size + av_buffer->payload_size;
        uAVBufSize = ((~3L) & (uAVBufSize+3));    // frames always start on 4-byte boundary
        if(segmentLen < uAVBufSize)return(fileOffset);
        bufferOffset += uAVBufSize;
        fileOffset += uAVBufSize;
        segmentLen -= uAVBufSize;
        }
    }

//************************************************************

void A2aFileSource::next_frame() 
    {
    int i;
    file_buf_header_t *phdr;
    msg_file_req_info_t req;
    sdvr_av_buffer_t *av_buffer;
    uint32_t tot_frame_size;
    
    while(1)
        {
        while(1)
            {
            if(NULL == _buffer)break;
            if(_have_bytes < AV_BUFFER_HEADER_SIZE)break;
            av_buffer = (sdvr_av_buffer_t*)_buffer;
            if(SDVR_DATA_HDR_SIG != av_buffer->signature)
                {
                printf("next_frame(): Bad signature = 0x%08X for channel %d !!!\n",av_buffer->signature,_file_index);
                break;
                }
            tot_frame_size = av_buffer->hdr_size + av_buffer->payload_size;
            tot_frame_size = ((~3L) & (tot_frame_size+3));    // frames always start on 4-byte boundary
            if(_have_bytes < tot_frame_size)break;
            av_buffer->stream_id = 1;
            
            _buffer += tot_frame_size;
            _have_bytes -= tot_frame_size;
            
            _frame = av_buffer->payload;
            _frame_size = av_buffer->payload_size;
            return;
            }
        if(NULL != _buffer_top)
            {
            a2a_buffer_free(_channel,_buffer_top);
            _buffer_top = NULL;
            _buffer = NULL;
            }
        i = _index_fifo.get_read();
        while(-1 == i)
            {
            printf("starving channel %d...\n",_file_index);
            _data_ready_sem.wait();
            i = _index_fifo.get_read();
            }
        _buffer_top = _a2a_buffer_data[i].buffer;
        _channel = _a2a_buffer_data[i].channel;
        _index_fifo.advance_read();
        phdr = (file_buf_header_t*)_buffer_top;
        if(_next_segment_file_offset != (phdr->fileOffset))
            {
            printf("fileOffset mismatch %d != %d for channel %d...\n",phdr->fileOffset,_next_segment_file_offset,_file_index);
            }
        _have_bytes = S7_SEG_LEN_MASK & (phdr->segmentLen);
        _next_segment_file_offset = get_next_fileOffset(_buffer_top);
        if(0 == _index_fifo.num_in())
            {    // order the next segment
            req.fileIndex  = _file_index;
            req.fileOffset = _next_segment_file_offset;
            req.segmentLen = REQUEST_BUF_SIZE;
            a2a_message_send(_a2a_peerHandle,VRM_FILE_REQ_MSG_CLASS,&req,sizeof(msg_file_req_info_t));
            }
        _buffer = _buffer_top+sizeof(file_buf_header_t);
        _have_bytes -= sizeof(file_buf_header_t);
        _total_bytes_received += _have_bytes;
        }
    }

// Wait _period before sending next frame
void A2aFileSource::wait() {
    _tick += _period;
    if (_tick >= ONE_SECOND)
        _tick -= ONE_SECOND;
    _timestamp += _ts_delta;
    struct timespec time;
    SBL_PERROR(::clock_gettime(CLOCK_REALTIME, &time) < 0);
    time.tv_sec  = 0;
    time.tv_nsec = _tick - time.tv_nsec;
    if (time.tv_nsec < 0)
        time.tv_nsec += ONE_SECOND;
    // account for processing time, so that frames
    // are sent precisily _period away, without cumulative error
    SBL_PERROR(::pselect(0, NULL, NULL, NULL, &time, NULL) < 0);
}

void A2aFileSource::teardown() {
    _playing = false;
    join_thread();
}

void A2aFileSource::push_buffer(a2a_channel_t channel,uint8_t *buffer)
    {
    int i;
    i = _index_fifo.get_write();
    if(-1 == i)
        {
        a2a_buffer_free(channel, buffer);
        printf("push_buffer overflow !!!\n");
        return;
        }
    _a2a_buffer_data[i].channel = channel;
    _a2a_buffer_data[i].buffer = buffer;
    _index_fifo.advance_write();
    _data_ready_sem.post();
    }

};    // RTSP
