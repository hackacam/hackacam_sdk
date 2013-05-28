/*******************************************************************************
   Copyright 2005 Stretch, Inc. All rights reserved.
   "THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
   STRETCH, INC.  USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT THE
   PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC."
*******************************************************************************/


#ifndef __H_SXTYPES
#define __H_SXTYPES


/*******************************************************************************
   C++ related macros. Use these in all SBIOS code.
*******************************************************************************/

#ifdef __cplusplus

#define SX_EXTERN            extern "C"
#define SX_EXTERN_BEGIN      extern "C" {
#define SX_EXTERN_END        }

#else

#define SX_EXTERN            extern
#define SX_EXTERN_BEGIN
#define SX_EXTERN_END

#endif


SX_EXTERN_BEGIN

/*******************************************************************************
   8-bit types
*******************************************************************************/
typedef signed char     sx_int8;
typedef unsigned char   sx_uint8;

/*******************************************************************************
   16-bit types
*******************************************************************************/
typedef short           sx_int16;
typedef unsigned short  sx_uint16;

/*******************************************************************************
   32-bit types
*******************************************************************************/
typedef int             sx_int32;
typedef unsigned int    sx_uint32;

/*******************************************************************************
   64-bit types
*******************************************************************************/
typedef long long          sx_int64;
typedef unsigned long long sx_uint64;


/*******************************************************************************
   Boolean type
*******************************************************************************/
#ifdef __STRETCH__

#include <stdbool.h>

typedef bool  sx_bool;

#else

typedef unsigned int   sx_bool;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#endif // __STRETCH__


/*******************************************************************************
   Other miscellaneous defines
*******************************************************************************/

#ifndef NULL
#define NULL           ((void *)0x0)
#endif

#ifndef SUCCESS
#define SUCCESS        0
#endif


/*******************************************************************************
   Static assert for compile-time constant checks
*******************************************************************************/
#define sx_static_assert(e, x) \
    typedef char assertion_failed_for_ ## x [(e) ? 1 : -1]


SX_EXTERN_END

#endif


