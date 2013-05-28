#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//#undef STRETCH_TARGET

/* use buffered io to read and write mp4 file
 * It helps file access performance for Tickets 11438 and 9555.  */
#define _AVMUX_MP4_BUFFERED_IO_ 1

/*  There are 3 options below:
 *  - STRETCH_TARGET
 *  - WIN32
 *  - LINUX
 */
#if defined STRETCH_TARGET

#include <sx-misc.h>
#include <sx-event.h>
#include <stdarg.h>
#include <ctype.h>
#include "FS.h"

#define INT64_MAX  (sx_int64)0x7fffffffffffffff
#define UINT32_MAX  0xffffffff
#define INT32_MAX   0x7fffffff

typedef sx_int64   offset_t;

#elif WIN32

#include <io.h>
#include <ctype.h>
#include <sys/stat.h>
#define ANSI

#include <stdarg.h>

#pragma warning(disable : 4996)

#define INT32_MAX   0x7fffffff
#define UINT32_MAX  0xffffffff
#define INT64_MAX  (__int64)0x7fffffffffffffff

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#define O_CREAT     _O_CREAT
#define O_RDONLY    _O_RDONLY
#define O_WRONLY    _O_WRONLY
#define O_RDWR      _O_RDWR
#define O_BINAARY   _O_BINARY

#if (_AVMUX_MP4_BUFFERED_IO_ == 1)
/*  WIN32 */
/* define mp4lib file pointer (descriptor) */
typedef FILE       *sx_file_t;              /*  file pointer */
#define sx_is_valid_file(fptr)              (fptr != NULL)

#define sx_open_wr(file_name)               fopen(file_name, "wb")
#define sx_open_rdonly(file_name)           fopen(file_name, "rb")
#define sx_open_rdwr(file_name)             fopen(file_name, "rb+")
#define sx_close(fptr)                      fclose(fptr)

#define sx_read(fptr, buf, count)           (sx_int32)fread(buf, 1, count, fptr)
#define sx_write(fptr, buf, count)          (sx_int32)fwrite(buf, 1, count, fptr)

#define sx_lseek(fptr, offset, whence)      fseek(fptr, (long)offset, whence)
#define sx_lseek64(fptr, offset, whence)    _fseeki64(fptr, offset, whence)
#define sx_ftell(fptr)                      _ftelli64(fptr)

#define sx_stat(file_name, buf)             stat(file_name, buf)
#define sx_flush(fptr)                      fflush(fptr)
typedef struct stat    fstat_t;
#else
/* define mp4lib file pointer (descriptor) */
//typedef sx_int32    sx_file_t;        /*  a descriptor */
typedef int			sx_file_t;        /*  a descriptor */
#define sx_is_valid_file(fptr)              (fptr != -1)

#define sx_open         _open
#define sx_open_wr(file_name)       sx_open(file_name, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE)
#define sx_open_rdonly(file_name)   sx_open(file_name, O_BINARY | O_RDONLY)
#define sx_open_rdwr(file_name)     sx_open(file_name, O_BINARY | O_RDWR)

#define sx_read         _read
#define sx_write        _write
#define sx_lseek        _lseek
#define sx_lseek64      _lseeki64
#define sx_close        _close
#define sx_stat         _stat
#define sx_flush        _commit
typedef struct _stat    fstat_t;
#endif


#else // Linux|UNIX

#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>

#if (_AVMUX_MP4_BUFFERED_IO_ == 1)
/*  Linux/UNIX */
/* define mp4lib file pointer (descriptor) */
typedef FILE       *sx_file_t;              /*  file pointer */
#define sx_is_valid_file(fptr)              (fptr != NULL)

#define sx_open_wr(file_name)               fopen(file_name, "wb")
#define sx_open_rdonly(file_name)           fopen(file_name, "rb")
#define sx_open_rdwr(file_name)             fopen(file_name, "rb+")
#define sx_close(fptr)                      fclose(fptr)

#define sx_read(fptr, buf, count)           fread(buf, 1, count, fptr)
#define sx_write(fptr, buf, count)          fwrite(buf, 1, count, fptr)

#define sx_lseek(fptr, offset, whence)      fseeko(fptr, offset, whence)
#define sx_lseek64(fptr, offset, whence)    fseeko(fptr, offset, whence)
#define sx_ftell(fptr)                      ftello(fptr)

#define sx_stat(file_name, buf)             stat(file_name, buf)
#define sx_flush(fptr)                      fflush(fptr)
#else
/* define mp4lib file pointer (descriptor) */
typedef sx_int32    sx_file_t;        /*  a descriptor */
#define sx_is_valid_file(fptr)              (fptr != -1)

#define sx_open         open
#define sx_open_wr(file_name)       sx_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0644);
#define sx_open_rdonly(file_name)   sx_open(file_name, O_RDONLY)
#define sx_open_rdwr(file_name)     sx_open(file_name, O_RDWR)

#define sx_read         read
#define sx_write        write
#define sx_lseek        lseek
#define sx_close        close
#define sx_lseek64      lseek
#define sx_stat         stat
#define sx_flush        fsync
#endif
typedef struct stat     fstat_t;

#endif 

#define MAX_UINT64 -1LLU
#define D64F "lld"
#define U64F "llu"
#define X64F "llx"

#define D64  "%"D64F
#define U64  "%"U64F
#define X64 "%"X64F

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

#define BE32(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define LE32(a, b, c, d) (((d) << 24) + ((c) << 16) + ((b) << 8) + (a))
#define BE2LE32(a) ((((a) & 0xff) << 24) + (((a) & 0xff00) << 8) + (((a) & 0xff0000) >> 8) + (((a) & 0xff000000) >> 24))

#ifndef __H_SXTYPES
#define __H_SXTYPES

#ifndef STRETCH_TARGET
typedef unsigned int    sx_bool;

typedef char    sx_int8;
typedef short   sx_int16;
typedef int     sx_int32;
typedef int64_t sx_int64;

typedef unsigned char   sx_uint8;
typedef unsigned short  sx_uint16;
typedef unsigned int    sx_uint32;
typedef uint64_t        sx_uint64;
#endif

typedef sx_int64   offset_t;

#endif //#ifndef __H_SXTYPES

// codec id
typedef enum {
    CODEC_ID_NONE = 0,
    // video codecs
    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO,
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_JPEG,
    CODEC_ID_MJPEG,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_WMV1,
    CODEC_ID_WMV2,
    CODEC_ID_H263P,
    CODEC_ID_H263I,
    CODEC_ID_H264,
    CODEC_ID_VC1,
    CODEC_ID_WMV3,
    CODEC_ID_JPEG2000,
    CODEC_ID_YV12_VIDEO,
    // audio codecs 
    CODEC_ID_PCM_S16LE,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,
    CODEC_ID_ADPCM_IMA_QT,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_G723,
    CODEC_ID_ADPCM_G726,
} codec_id_t;

// error code
typedef enum {
    // no error
    MP4MUX_ERR_OK = 0,              // succeed
    MP4MUX_ERR_INVALID_HANDLE,      // invalid handle
    // file operations
    MP4MUX_ERR_OPEN_FILE = 1000,    // fail to open file
    MP4MUX_ERR_FILE_EXIST,          // file exists and can not be over-written
    MP4MUX_ERR_MUX_MODE,            // unsupported mux mode
    MP4MUX_ERR_MEMORY,              // memory fail
    // track operations
    MP4MUX_ERR_TRACK_NUMBER = 2000, // track number out of range
    MP4MUX_ERR_INVALID_TRACK,       // invalid track
    MP4MUX_ERR_UNKNOWN_CODEC,       // unknown codec
    MP4MUX_ERR_NO_VIDEO_TRACK,      // no video track
    MP4MUX_ERR_ADD_TRACK_NA,        // adding more tracks is not allowed
    // parsing operations
    MP4MUX_ERR_PARSE_ERROR = 3000,  // parse file error
    MP4MUX_ERR_PARSE_NO_MDAT,       // there is no mdat in the file
    MP4MUX_ERR_PARSE_DONE,          // parsing is done
    MP4MUX_ERR_NOT_ENOUGH_DATA,     // not enough data for parsing
    // playback operations
    MP4MUX_ERR_END_OF_FILE = 4000,  // end of file for playback
    MP4MUX_ERR_NO_MORE_SAMPLES,     // no more samples for playback at this moment
    MP4MUX_ERR_SEEK_SAMPLE,         // seek sample error
    // get packet
    MP4MUX_ERR_PACKET_SIZE = 5000,  // packet size is not enough
    MP4MUX_ERR_PACKET_BUFFER,       // packet buffer invalid
    // put packet
    MP4MUX_ERR_START_CODE = 6000,   // H.264 NAL start code error
    MP4MUX_ERR_UNSUPPORTED_NAL_TYPE,     // unsupported H.264 NAL type
    MP4MUX_ERR_TOO_MANY_CONFIG,     // too many config (SPS/PPS/VOL)
    MP4MUX_ERR_MISSING_CONFIG,      // missing config (SPS/PPS/VOL) frame
                                    // for the GOP. This is mostly the first
                                    // GOP.

} error_code_t;

#endif // #ifndef _COMMON_H_

