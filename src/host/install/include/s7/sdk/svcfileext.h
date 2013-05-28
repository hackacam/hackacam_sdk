/****************************************************************************\
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
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

#ifndef SVCFILEEXT_H
#define SVCFILEEXT_H

//#include "svcext.h"

#define NAL_PREVIEW_LEN                 8
#define MAX_NAL_ENTRIES_CACHED          100

typedef struct nal_entry_s{
    fpos_t              sc_pos;        /* The position of 00 00 00 01 */
    unsigned char       first_bytes[NAL_PREVIEW_LEN];
    struct nal_entry_s  *next;
} nal_entry_t;

typedef struct _svc_index_file_entry_t {
    sx_uint32   curNALOffset;           // The file offset to the current NAL entry within
                                        // the SVC file.
    sx_uint32   curNALSize;             // The size of the current NAL unit.
    sx_uint8    firstBytes[NAL_PREVIEW_LEN]; // The first 8 bytes of the current NAL unit. It is
                                        // needed by the SVC extractor.
    sx_uint64   curNALTimeStamp;        // The timestamp on the PCI header associated to this
                                        // NAL unit.

} svc_index_file_entry_t;

typedef struct _svc_file_info_t {
    FILE *fpIndex;                      // The index file handle that was open
                                        // to read or write.
    FILE *fpElementary;                 // The file pointer to the SVC elementary file.
    sx_bool bWriteMode;                    // The mode of index file handle.
                                        // TRUE - write mode.
                                        // FALSE - read mode.
    sx_uint32   lastNALOffset;          // On saving Nal Packets: The file offset to the previous NAL entry within
                                        // the SVC file
                                        // On reading Nal Packets: The file offset to read the
                                        // the next Nal packet.
} svc_file_info_t;

typedef struct _svc_context {
    FILE                    *fin;
    fpos_t                  file_end_pos;   // The size of input file.

	st_svcext_config_t	    cfg;
    st_svcext_t		        extractor_handle;

    nal_entry_t             *nal_dir_head;
    nal_entry_t             *nal_dir_pointer;  // last nal item processed.

	int                     end_of_data;
    int                     slice_nal_size;    // This is 3/2 of the largest frame size
    sx_int64                timestamp;         // This should be removed once we can get
                                               // time stamp from the file for svc.
//    sx_bool                 bUseIndexFile;     // Set to true to use the index file
                                               // to read the svc file entries. Otherwise,
                                               // it parses the svc elementary stream file
                                               // and constructs the Nal Pointer directory.
                                               // NOTE: The Tivo mode will not work in a 
                                               //       non-index file mode.
//    svc_file_info_t *      svcIndexFileInfo;   // Used if bUseIndexFile is set to true
} svc_fileext_context_t;

typedef struct
{
    sx_int32 vtrack;        // Video track handle within mp4 file
    sx_int32 atrack;        // Audio track handle within mp4 file
    mp4_handle FileHandle;  // The file handle to the mp4 file.
    sx_int64 dts;
} mp4PlayerFile_t;


#define NAL_TYPE(hdr)                           (hdr & 0x1f)

typedef enum {
    H264_NAL_UNKNOWN		= 0,
    H264_NAL_SLICE			= 1,
    H264_NAL_SLICE_DPA		= 2,
    H264_NAL_SLICE_DPB		= 3,
    H264_NAL_SLICE_DPC		= 4,
    H264_NAL_SLICE_IDR		= 5,    
    H264_NAL_SEI			= 6,    
    H264_NAL_SPS			= 7,
    H264_NAL_PPS			= 8,
    H264_NAL_AUD			= 9,
    H264_NAL_END_OF_SEQ     = 10,       
    H264_NAL_END_OF_STREAM  = 11,       
    H264_NAL_FILLER_DATA    = 12,       
    H264_NAL_PREFIX         = 14,       
    H264_NAL_SUBSET_SPS     = 15,
    H264_NAL_SLICE_SCALABLE = 20,
} h264_nal_type_e;

#define SVC_INDX_FILE_SIGNITURE         0x1BE31B1B
#define SVC_INDX_FILE_VERSION           1
#define SVC_MEM_SET_CHAR                0xDD

typedef struct _svc_index_file_hdr_t {
    sx_uint8    bInRec;                 // True while in the middel of recording to file
    sx_uint8    version;                // The index file version.
    sx_uint8    resrv1[2];
    sx_uint32   signiture;              // The index file identifier
    sx_uint32   resrv2[48];

} svc_index_file_hdr_t;

#if defined(__cplusplus)
   #define SVCFILEEXT_EXTERN              extern "C"
#else
   #define SVCFILEEXT_EXTERN              extern
#endif


SVCFILEEXT_EXTERN void svc_close_file_extractor(void *avmuxFileContext);
SVCFILEEXT_EXTERN sdvr_err_e svc_get_next_avc_frame(svc_fileext_context_t *p_svc_context, 
                           sx_uint8 *sct_outbuf, 
                           sx_uint32 *sct_outbuf_size);
SVCFILEEXT_EXTERN sx_uint8  svc_get_frame_type(sx_uint8 *payload);

SVCFILEEXT_EXTERN void svc_build_nal_direcotry(svc_fileext_context_t *p_svc_context);


#endif
