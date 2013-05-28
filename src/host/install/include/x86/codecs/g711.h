/************************************************************************
Copyright (c) 2007 Stretch, Inc. All rights reserved.  Stretch products
are protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.

This source code and the related tools, software code and documentation,
and your use thereof, are subject to and governed by the terms and
conditions of the applicable Stretch IDE or SDK and RDK License Agreement
(either as agreed by you or found at www.stretchinc.com).  By using
these items, you indicate your acceptance of such terms and conditions
between you and Stretch, Inc.  In the event that you do not agree with
such terms and conditions, you may not use any of these items and must
immediately destroy any copies you have made.
************************************************************************/

/************************************************************************
    PACKAGE: g711 -- G711 Speech Codec

    DESCRIPTION:

    SECTION: Include
    {
    #include "g711.h"
    }

    SECTION: Introduction

    @g711@ is an implementation of the G711 speech coding standard
    optimized for the Stretch S6000 processors.  The codec support 
    both mu law and alaw encoding/decoding.

    SECTION: Usage model

    To use @g711@, a @g711@ encoder/decoder instance must be created first
    using g711_open().  After that, g711_encode() and g711_decode() can be
    called.  When finished, g711_close() must be called to free memories
    associated with the instance.  Due to the simplicify of g711 algorith,
    both encoding and decoding can be done using a single instance.

    This package supports single or multi channel interleaved inputs.
    In the multi-channel case, the API allows the encoding/decoding to
    be performed on a specific channel, without requiring the caller to
    extract the channel first.

    SECTION: Data representation

    Linear data has 16 bits.  Only the most significant 12 bits
    are used for encoding.  Compressed data has 8 bits stored in
    the least significant 8 bits of a short.
************************************************************************/
#ifndef G711_HEADER
#define G711_HEADER

/*******************************************************************************
   A necessary evil introduced for C++ compatibility.  C source files must
   not declare a function "extern"; instead, they must declare the function
   "EXTERN_G711".  For example:
   {
       EXTERN_G711 void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#if defined(__cplusplus)
   #define EXTERN_G711              extern "C"
#else
   #define EXTERN_G711              extern
#endif

/************************************************************************
    Return status for g711 routines
************************************************************************/
typedef enum g711_status_enum {
    G711_OK
} g711_status_e;

/************************************************************************
    Type of companding to use for encoding or decoding

    Use G711_ULAW for USA
    Use G711_ALAW for Asia and Europe
************************************************************************/
typedef enum g711_au_enum {
    G711_ALAW,
    G711_ULAW,
} g711_au_e;

/************************************************************************
    Structure used to configure a g711 encoder or decoder instance.
    Both encoding and decoding can use the same instance provided
    that they have the same configuration parameters.

    region (law) = ulaw, Use G711_ALAW for Asia and Europe
    num_channels - number of audio channels for 1 for mono, 2 for stereo
    channel_select - The channel used. First one is 0. Use 0 for both
    Mono and Stereo since in the case of stereo we are just duplicating
    left channel twice.

************************************************************************/
typedef struct g711_config_struct {
    g711_au_e law;
    unsigned char num_channels;
    unsigned char channel_select;
} g711_config_t;

/************************************************************************
    A g711 encoder/decoder handle
************************************************************************/
typedef struct g711_struct g711_t;

/************************************************************************
    GROUP: g711 instance management
************************************************************************/
EXTERN_G711 g711_status_e g711_open(g711_config_t *config, g711_t **g711);
EXTERN_G711 g711_status_e g711_close(g711_t *g711);

/************************************************************************
    GROUP: Encoding and decoding routines
************************************************************************/
EXTERN_G711 g711_status_e g711_encode_mono(g711_t *g711, unsigned num, short *ibuf, unsigned char *obuf);
EXTERN_G711 g711_status_e g711_encode_stereo(g711_t *g711, unsigned num, short *ibuf, unsigned char *obuf);
EXTERN_G711 g711_status_e g711_decode(g711_t *g711, unsigned num, unsigned char *ibuf, short *obuf);
#endif


