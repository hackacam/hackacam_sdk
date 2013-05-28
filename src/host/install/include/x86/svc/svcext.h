/************************************************************************
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

#ifndef __SVCEXT_H__
#define __SVCEXT_H__

/**************************************************************************
    PACKAGE: svcext -- SVC Sub-bitstream extractor and Rewriter

    DESCRIPTION:

    SECTION: Include
    {
    #include "svcext.h"
    }

    SECTION: Introduction
    
    @svcext@ library allows applications to efficiently parse SVC 
    NAL sequence and selectively process the NAL so that the output 
    bitstream is a subset of the input SVC stream. The output can 
    either be SVC/AVC stream.

    Note that this library assumes that the NALs are all preceded 
    with the startcode 00 00 00 01. For some NALs which are 
    rewritten, some extra zeroes may be inserted at the beginning of 
    the NALs. The application is responsible for taking out the 
    startcode before forwarding to other destination like RTP sink.

	SECTION: Usage model
     
    Step 1 - Initialize an svcext instance per desired output stream. Configure the
    instance with the desired spatial and temporal levels.

    Step 2 - Pass the first 8 bytes of the source SVC NAL packet in sequence order 
    to st_svcext_preview() function.

	Step 3 - Depending on the action suggested by the st_svcext_preview() function.

        if (action == SKIP) 

            Ignore that packet, goto Step 2.

        if (action == COPY) 
        
            Copy the complete NAL packet to the target stream, goto Step 2.

        if (action == PROCESS) 
        
            Use the st_svcext_process() to further process the complete NAL 

    Step 4 - Check skip flag. 
    
        If (skip == 1)
        
            Dump the NAL 
            
        If (skip == 0)
       
            Forward the modified NAL to the destination. Please check the size 
            returned which may be changed.

    Step 5 - Goto Step 2.

    Note - For memory efficiency, it is recommended to prestore the first 8 bytes
    of all NALs in a bitstream in a contiguous location.

    SUBSECTION:  Pseudo Code
    {
        #include "svcext.h"

        ...
        // This section of the code constructs the nal_directory for the SVC file 
        // The nal directory contains the position of the full nal in the SVC 
        // file as well as the first 8 bytes of the NAL packet (including 4 byte
        // start code)
        ...

        svcext_cfg.spatial_layer = 2;
        svcext_cfg.temporal_level = 2;
        svcext_cfg.avc            = 1;   // Output AVC compatible stream
        st_svcext_init(&extractor, &svcext_cfg);
        if (extractor == NULL) exit(-1);

        // nal_dir_pointer points to a link list of entries, each containing the starting
        // location and size of nals within the svc file
        nal_dir_pointer = nal_dir_head;
        while (nal_dir_pointer) {

            st_svcext_action_e action;
            err = st_svcext_preview(extractor, nal_dir_pointer->first_bytes, &action);
            if (err) {
                printf("st_svcext_preview(): error (%d) \n", err);
                goto finish;
            }

            if (action != ST_SVCEXT_NAL_SKIP) {
                int nal_size_with_sc, skip = 0, nal_size;
                char *out_data;

			    // Get the size of the nal packet 
                if (nal_dir_pointer->next != NULL) {
                    nal_size_with_sc = (int)(nal_dir_pointer->next->sc_pos.__pos - nal_dir_pointer->sc_pos.__pos);
                } else {
                    nal_size_with_sc = (int)(file_end_pos.__pos - nal_dir_pointer->sc_pos.__pos);
                }

                fsetpos(fin, &(nal_dir_pointer->sc_pos));
                fread(nal_buf, sizeof(char), nal_size_with_sc, fin);
                input_count += (nal_size_with_sc * 8);
                nal_size = nal_size_with_sc - 4;      // Subtract the 4 byte start code 
                if (action == ST_SVCEXT_NAL_PROCESS) {
                    err = st_svcext_process(extractor, nal_buf, &sc_offset, &nal_size, &skip);  
                    if (err) {
                        printf("st_svcext_process(): error (%d) \n", err);
                        goto finish; 
                    }
                }

                out_data = nal_buf + sc_offset;

                // Write out the packet 
                if (!skip) {
                    fwrite(out_data, sizeof(char), nal_size + 4, fout);
                    output_count += (nal_size + 4) * 8; 
                }
            }
            nal_dir_pointer = nal_dir_pointer->next;
        }
    }

 *****************************************************************************/

/**********************************************************************************
	extern "C" for C++ build
**********************************************************************************/
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef __cplusplus
    #define EXTERN            extern "C"
#else
    #define EXTERN            extern
#endif

/*--------------------------------- Defines ---------------------------------*/

/**********************************************************************************
	Maximum number of spatial layers
**********************************************************************************/
#define SVCEXT_MAX_SPATIAL_LAYERS                  3


/**********************************************************************************
	Maximum number of temporal levels
**********************************************************************************/
#define SVCEXT_MAX_TEMPORAL_LEVELS                 6 

/**********************************************************************************
    Error code used by the Stretch SVC Extractor. The negative codes are errors and
    the positive ones are just informing the used of specific conditions.

        @ST_SVCEX_NO_ERROR@ - Success.

        @ST_SVCEX_OUT_OF_MEMORY@ - Unable to allocate memory.

        @ST_SVCEX_INVALID_POINTER@ - Invalid pointer is passed to the API.

        @ST_SVCEXT_UNSUPPORTED_SEI@ - Only Scalable SEI messages are recognised. All other
        messages are not recognised by the extraction process.

        @ST_SVCEX_NO_STARTCODE@ - Cannot find the 4 bytes 0x00 0x00 0x00 0x01 startcode.

        @ST_SVCEX_UNSUPPORTED_NAL@ - Nal type is not supported.

        @ST_SVCEX_LAYERS_EXCEED_RANGE@ - Number of spatial layers exceed supported range.

        @ST_SVCEX_MISSING_SEI@ - No SEI founded in the stream.

        @ST_SVCEX_UNEXPECTED_PARAMSET@ - Inconsistent paramset.

        @ST_SVCEX_MISSING_PARAMSET@ - No paramset for the layer is found.

        @ST_SVCEX_LAYER_NOT_FOUND@ - A spatial layer is missing.

        @ST_SVCEX_FAIL@ - General failure

        @ST_SVCEXT_NOT_SUPPORTED@ - The SVC extraction is not supported.

**********************************************************************************/
typedef enum st_svcext_error_tag {
    ST_SVCEXT_NO_ERROR                  = 0,
    ST_SVCEXT_OUT_OF_MEMORY             = -1,
    ST_SVCEXT_INVALID_POINTER           = -2,
    ST_SVCEXT_UNSUPPORTED_SEI           = -3,
    ST_SVCEXT_NO_STARTCODE              = -4,
    ST_SVCEXT_UNSUPPORTED_NAL           = -5,
    ST_SVCEXT_LAYERS_EXCEED_RANGE       = -6,
    ST_SVCEXT_MISSING_SEI               = -7,
    ST_SVCEXT_UNEXPECTED_PARAMSET       = -8,
    ST_SVCEXT_MISSING_PARAMSET          = -9,
    ST_SVCEXT_LAYER_NOT_FOUND           = -10,
    ST_SVCEXT_FAIL                      = -11,
    ST_SVCEXT_NOT_SUPPORTED             = -12,

    ST_SVCEXT_BAD_VALUE                                        = -1000,

    ST_SVCEXT_BAD_SLICE_SCALABLE_EXTENSION                     = -1100,
    ST_SVCEXT_BAD_PARSE_PREFIX_NAL                             = -1101,
    ST_SVCEXT_BAD_CHROMA_FORMAT_IDC                            = -1102,

    ST_SVCEXT_BAD_SPS                                          = -1200,

    ST_SVCEXT_BAD_SSEI_TEMPORAL_LEVEL_NESTING_FLAG             = -1300,
    ST_SVCEXT_BAD_SSEI_PRIORITY_LAYER_INFO_PRESENT_FLAG        = -1301,
    ST_SVCEXT_BAD_SSEI_SUB_PIC_LAYER_FLAG                      = -1302,
    ST_SVCEXT_BAD_SSEI_IROI_DIVISION_INFO_PRESENT_FLAG         = -1303,
    ST_SVCEXT_BAD_SSEI_PARAMETER_SETS_INFO_PRESENT_FLAG        = -1304,
    ST_SVCEXT_BAD_SSEI_BITSTREAM_RESTRICTION_INFO_PRESENT_FLAG = -1305,
    ST_SVCEXT_BAD_SSEI_QUALITY_ID                              = -1306,

} st_svcext_error_e;

/**********************************************************************************
    VISIBLE: Configuration data for an extractor instance.

    @Fields@:

        "debug" - Enable debug info. 

        "avc" - This extractor handles AVC conversion

        "spatial_layer" - Targeted spatial layer of the operating point. For 
        SVC, that means it contains all the data for the spatial layer specified 
        and below. For AVC, that means it contains all the data for that spatial layer. 

        "temporal_level" - Targeted temporal level of the operating point. For 
        SVC, that means it contains all the data for the temporal level specified and 
        below. For AVC, that means it contains all the data for that temporal level. 

        "insert_dummy_frame" - If the output is AVC mode, enable this flag force
        the extractor to insert skip frames for frames taken out in temporal sublayer 
        streams. This allows the frame number in frames to be contiguous. This 
        is needed for some players to playback the sequence. [Note: frame number
        gap is allowed in H.264 specification but some players may not have
        handled it properly. The side effect of inserting dummy/skip frame is a little
        higher bit rate and more processing cycles for the decoder. Another
        impact is that the skip frames cannot be authenticated. The authentication 
        software should ignore those skip frames or the extractor should setup
        to disable insert_dummy_frame for authentication to work] 
**********************************************************************************/
typedef struct st_svcext_config_s {
    int avc; 
    int spatial_layer;
    int	temporal_level;
    int debug;
    int insert_dummy_frame;
    int stream_eye_work_around;
} st_svcext_config_t;


/**********************************************************************************
	Action to be taken indicated by @st_svcext_process@
**********************************************************************************/
typedef enum {
    ST_SVCEXT_NAL_SKIP,
    ST_SVCEXT_NAL_COPY,
    ST_SVCEXT_NAL_PROCESS
} st_svcext_action_e;


/**********************************************************************************
    VISIBLE: Information per svc spatial/temporal layer combination.

    @Fields@:

        "spatial_layer" - Spatial layer id of the layer.

        "temporal_level" - Temporal level id of the layer.

        "frame_width" - Frame width represented by the layer.

        "frame_height" - Frame height represented by the layer.

        "frame_rate" - Frame rate represented by the layer.

**********************************************************************************/
typedef struct {
    int    spatial_layer;
    int    temporal_level;
    int    frame_width;
    int    frame_height;
    float  frame_rate;
} st_svcext_layer_info_t;


/**********************************************************************************
    VISIBLE: Information regarding the svc stream.

    @Fields@:

        "num_layers" - Total number of spatial/temporal layer combinations.

        "num_spatial_layers" - Total number of spatial layers.

        "num_temporal_levels" - Total number of temporal levels.

        "layer_info" - An array of @st_svcext_layer_info_t@.

**********************************************************************************/
typedef struct {
    int						num_layers;    
    int						num_spatial_layers;
    int						num_temporal_levels;
    st_svcext_layer_info_t  layer_info[SVCEXT_MAX_SPATIAL_LAYERS * SVCEXT_MAX_TEMPORAL_LEVELS];
} st_svcext_svc_info_t;


/**********************************************************************************
    VISIBLE: More detailed information given by the preview function about 
    what to do with the NAL packet 

    @Fields@:

        "action" - The action to be taken for the NAL packet

        "max_changed_bytes" - Indicate the maximum number of bytes, starting from the 
        beginning of the buffer, that will be changed if the st_svcext_process() 
        function call is needed to further process/analyze the NAL packet. 
        0 - indicates that no byte will be changed (i.e. the st_svcext_process() 
        function will only determine if the NAL will be kept or not, but no
        modification will be needed).
        -1 - indicates that the full NAL need to be modified.

**********************************************************************************/
typedef struct {
    st_svcext_action_e  action;
    int                 max_changed_bytes;
} st_svcext_preview_info_t;

/**********************************************************************************
    VISIBLE: Information available for avc packets whose action is either ST_SVCEXT_NAL_COPY or
    ST_SVCEXT_NAL_PROCESS.

    @Fields@:

        "spatial_layer" - Spatial layer id to which the packet belongs in the svc stream.

        "temporal_level" - Temporal level id to which the packet belongs in the svc stream.

        "is_IDR_NAL" - 1 if packet contains an IDR nal, 0 otherwise.

        "is_SPS_NAL" - 1 if packet contains an SPS nal, 0 otherwise.

        "is_PPS_NAL" - 1 if packet contains a PPS nal, 0 otherwise.

**********************************************************************************/
typedef struct {
    int    spatial_layer;
    int    temporal_level;
    int    is_IDR_NAL;
    int    is_SPS_NAL;
    int    is_PPS_NAL;
} st_svcext_avc_packet_info_t;

/**********************************************************************************
 The extractor context is stored in @st_svcext_t@. 
 *********************************************************************************/
typedef struct    st_svcext_s            *st_svcext_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINDLL
__declspec( dllexport ) st_svcext_error_e st_svcext_init (st_svcext_t *handle, st_svcext_config_t *config);
__declspec( dllexport ) st_svcext_error_e st_svcext_delete (st_svcext_t handle);
__declspec( dllexport ) st_svcext_error_e st_svcext_preview (st_svcext_t handle, char *nal_part, st_svcext_preview_info_t *info);
__declspec( dllexport ) st_svcext_error_e st_svcext_process (st_svcext_t handle, char *nal, int *sc_offset, int *size, int *skip);
__declspec( dllexport ) st_svcext_error_e st_svcext_get_svc_info(st_svcext_t handle, st_svcext_svc_info_t *info);
__declspec( dllexport ) st_svcext_error_e st_svcext_get_sei_nal_info(char *nal, int size, st_svcext_svc_info_t *info);
__declspec( dllexport ) st_svcext_error_e st_svcext_read_avc_info (st_svcext_t extractor, st_svcext_avc_packet_info_t *info);

#else
EXTERN st_svcext_error_e st_svcext_init (st_svcext_t *handle, st_svcext_config_t *config);
EXTERN st_svcext_error_e st_svcext_delete (st_svcext_t handle);
EXTERN st_svcext_error_e st_svcext_preview (st_svcext_t handle, char *nal_part, st_svcext_preview_info_t *info);
EXTERN st_svcext_error_e st_svcext_process (st_svcext_t handle, char *nal, int *sc_offset, int *size, int *skip);
EXTERN st_svcext_error_e st_svcext_get_svc_info(st_svcext_t handle, st_svcext_svc_info_t *info);
EXTERN st_svcext_error_e st_svcext_get_sei_nal_info(char *nal, int size, st_svcext_svc_info_t *info);
EXTERN st_svcext_error_e st_svcext_read_avc_info (st_svcext_t extractor, st_svcext_avc_packet_info_t *info);
#endif

#ifdef __cplusplus
}
#endif

#endif  // #ifndef __SVCEXT_H__
