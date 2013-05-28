/*****************************************************************************
*  Copyright C 2008 Stretch, Inc. All rights reserved. Stretch products are  *
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
*****************************************************************************/

#ifndef __VENC264_AUTHENTICATE_H__
#define __VENC264_AUTHENTICATE_H__
/*****************************************************************************
    Function:	Authentication

	This file declares a number of functions used to authenticate an h.264 
	bitstream.  The scheme relies on packets appeneded to slice NALs that contain
	sideband information and hash-based message authentication codes (HMACs) based
	on the SHA-1 algorithm.
	
	The scheme detects:
	 * Altered frames
	 * Added frames
	 * Deleted frames
	 * Sequences that are themselves valid but have been spliced together
	 
	There are multiple versions of authentication.  VENC264_AUTHENTICATE_CURRENT_VERSION
	is the current one and is recommended.
******************************************************************************/	
    

//#include <sx-types.h>
#ifndef __H_SXTYPES
#define __H_SXTYPES


/****************************************************************************
    8-bit unsigned integer.
****************************************************************************/
typedef unsigned char  sx_uint8;

/****************************************************************************
    16-bit unsigned integer.
****************************************************************************/
typedef unsigned short sx_uint16;

/****************************************************************************
    32-bit unsigned integer.
****************************************************************************/
typedef unsigned int   sx_uint32;

/****************************************************************************
    64-bit unsigned integer.
****************************************************************************/
typedef unsigned long long sx_uint64;

/****************************************************************************
    Boolean value.
****************************************************************************/
typedef unsigned int   sx_bool;

/****************************************************************************
    true is a non-zero value
****************************************************************************/

#ifndef true
#define true 1
#endif

/****************************************************************************
    false is zero value
****************************************************************************/

#ifndef false
#define false 0
#endif

/****************************************************************************
    8-bit signed integer.
****************************************************************************/
typedef signed char               sx_int8;     // 8  bit signed integer

/****************************************************************************
    16-bit signed integer.
****************************************************************************/
typedef  short              sx_int16;    // 16 bit signed integer

/****************************************************************************
    32-bit signed integer.
****************************************************************************/
typedef  int                sx_int32;    // 32 bit signed integer

/****************************************************************************
    64-bit signed integer.
****************************************************************************/
typedef  long long          sx_int64;    // 64 bit signed integer 
#endif


//  These structures hold authentication information for an h.264 frame.  It may be written
//  into the bitstream as unregistered user data, to be used later by the verification program.
//  A union of all the versions below can be used to hold any supported verification packet.

//  Version information:
//    authenticationVersion = 1:  Original version, 32-byte packet
//    authenticationVersion = 2:  64-byte packet that includes serial number

#define VENC264_AUTHENTICATE_CURRENT_VERSION 2
#define VENC264_AUTHENTICATE_MAX_KEY_LENGTH 64
#define VENC264_AUTHENTICATE_INSIDE_KEY_BASE 0x36
#define VENC264_AUTHENTICATE_OUTSIDE_KEY_BASE 0x5c
#define VENC264_AUTHENTICATE_SERIAL_NUM_LENGTH 20
#define VENC264_AUTHENTICATE_SIGNATURE_LENGTH 20
typedef struct
{
	 unsigned char authenticationVersion;   //  Version of authentication method (<= VENC264_AUTHENTICATE_CURRENT_VERSION)
	 sx_uint16 channelID;					//  DVR channel ID
	 unsigned char reserved1[1];            //  Align to 4 byte boundary
	 sx_uint64 timestamp;					//  Timestamp of frame
	 sx_uint32 frameNumber;					//  Number of frame in sequence
	 unsigned char signature[VENC264_AUTHENTICATE_SIGNATURE_LENGTH];			//  HMAC signature (filled in by authenticate function)
} venc264_authenticate_v1_t;

typedef struct
{
  unsigned char authenticationVersion; //  = 2
  //  SVC layer information for frame: Temporal | spatial << 4  
  unsigned char layer_info;
  sx_uint16 channelID;                 //  DVR channel ID
  //  32 bits of signature of last frame in this temporal layer
  //  SVC and SVC
  sx_uint32 last_signature;       
  //  32 bits of last base-layer signature (SVC non-base layer only)
  sx_uint32 last_base_signature;
  //  32 bits of last signature 
  //  of the temporal layer below this one
  sx_uint32 last_lower_layer_signature;
  sx_uint64 timestamp;                 //  Timestamp of frame
  //  Serial number of board
  unsigned char serialNumber[VENC264_AUTHENTICATE_SERIAL_NUM_LENGTH];
  unsigned char signature[VENC264_AUTHENTICATE_SIGNATURE_LENGTH];         //  HMAC signature
} venc264_authenticate_v2_t;

typedef union
{
	venc264_authenticate_v1_t v1Struct;
	venc264_authenticate_v2_t v2Struct;
} venc264_authenticate_t;

//  Limits to numbers of SVC layers.  Used to size arrays.  These values correspond to the SVC specification
#define MAX_TEMPORAL_LAYERS 8
#define MAX_SPATIAL_LAYERS 8

//  Control structure for authentication.  The contents of this structure will change from time to time.  
//  Please access it through the helper functions below.
//  Note that changing parameters of this structure during authentication will cause validation failures.
typedef struct
{
	unsigned char authenticationVersion; 
	sx_uint16 channelID;                 
	unsigned char serialNumber[VENC264_AUTHENTICATE_SERIAL_NUM_LENGTH];
	sx_uint32 lastSignatures[MAX_SPATIAL_LAYERS][MAX_TEMPORAL_LAYERS];
	sx_uint32 frameNumber;				
	unsigned char temporal_id_from_prefix;  
} venc264_authentication_handle_t;
#ifdef __cplusplus
extern "C" {
#endif
//  Allocate/free authentication handle
extern venc264_authentication_handle_t *venc264_authenticate_handle_alloc();
extern void venc264_authenticate_handle_free(venc264_authentication_handle_t *handle);
//  Set the channel ID associated with a handle
extern sx_int32 venc264_authentication_set_channel_id(venc264_authentication_handle_t *handle,
													  sx_uint16 channelID);
//  Set the serial number associated with a handle
extern sx_int32 venc264_authentication_set_serial_number(venc264_authentication_handle_t *handle,
														 unsigned char *serialNumber);
//  Set the authentication version to be used with this handle
extern sx_int32 venc264_authentication_set_version(venc264_authentication_handle_t *handle,
												   unsigned char authenticationVersion);

/*-----------------------------------------------------------------------------
  Compute an HMAC signature for a set of data.  Used to authenticate a series
  of buffers, which may be any length.
  
  void venc264_authenticate_compute_signature(
		unsigned char **bufs,            //  Array of buffers to authenticate
		sx_uint32 *sizes,                //  Sizes of the buffers.  A 0-length buffer terminates the list
		unsigned char *pKey,             //  The authentication key
		sx_uint32 keyLength,             //  The key length.  The key will be truncated to VENC264_AUTHENTICATE_MAX_KEY_LENGTH bytes
		unsigned char *signature);       //  Used to hold the signature, which will be VENC264_AUTHENTICATE_SIGNATURE_LENGTH bytes

-----------------------------------------------------------------------------*/
extern void venc264_authenticate_compute_signature(unsigned char **bufs, sx_uint32 *sizes, unsigned char *pKey, 
												   sx_uint32 keyLength, unsigned char *signature);

/*-----------------------------------------------------------------------------
  Get some basic information about a NAL that is useful in several places 
  in the authentication and verification process
  
  int venc264_authenticate_get_nal_info(
	unsigned char *buf,                     //  A buffer that contains a NAL, beginning with a start code
	sx_uint32 isize,                        //  The size of the buffer
	unsigned char *nalType,                 //  Pointer to return the nal type
	unsigned char *spatial_layer,           //  Pointer to return the spatial layer (always 0 for nal_types other than 14 and 20)
	unsigned char *temporal_layer,          //  Pointer to return the temporal layer (always 0 for nal_types other than 14 and 20)
	sx_uint32 *index_of_first_data_byte);   //  Pointer to return the index of the first byte after the NAL header
  
  Returns 0 when no error, a negative value on error.
-----------------------------------------------------------------------------*/
extern int venc264_authenticate_get_nal_info(unsigned char *buf, sx_uint32 isize, unsigned char *nalType, 
											 unsigned char *spatial_layer, unsigned char *temporal_layer,
											 sx_uint32 *index_of_first_data_byte);

/*-----------------------------------------------------------------------------
  Authenticate a slice by adding an SEI of type unregistered_user_data to the end of it.

  If there is no room in the buffer to hold an authentication packet, none is created.
  
  sx_uint32 venc264_authenticate(
	venc264_authentication_handle_t *handle,  //  Handle for this channel
	unsigned char *buf,		//  Buffer containing this frame
	sx_uint32 isize,		//  Size of input buffer
	sx_uint32 maxoutsize,	//  Maximum size of buffer after adding authentication packet
	sx_uint64 timestamp,	//  Timestamp of frame
	unsigned char *pKey,	//  Authentication key
	sx_uint32 keyLength);	//  Length of key (key is truncated at VENC264_AUTHENTICATE_MAX_KEY_LENGTH bytes)

  Returns the new buffer size (isize + length of authentication packet)
-----------------------------------------------------------------------------*/
extern sx_uint32 venc264_authenticate(venc264_authentication_handle_t *handle,  //  Handle for this channel
									  unsigned char *buf,						//  Buffer containing this frame
									  sx_uint32 isize,							//  Size of input buffer
									  sx_uint32 maxoutsize,						//  Maximum size of buffer after adding authentication packet
									  sx_uint64 timestamp,						//  Timestamp of frame
									  unsigned char *pKey,						//  Authentication key
									  sx_uint32 keyLength);						//  Length of key




#ifdef __cplusplus
}
#endif

#endif



