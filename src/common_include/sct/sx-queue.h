/**************************************************************************
Copyright (c) 2007 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.  

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/

#ifndef _SX_QUEUE_H
#define _SX_QUEUE_H

#include "sx-port.h"
#if (defined __STRETCH__)
#define INLINE inline
#else /* WINDOWS */
#define INLINE __inline
#ifdef STRETCH_WINDOWS_DRIVER // For Windows driver only
#include "stddcls.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif


//**********************************************************
// Define types
//**********************************************************

// Message header
struct sx_queue_datahdr_struct
{
#ifdef _WIN64
	sx_uint32	queue_data_nxt;
#else
  struct sx_queue_datahdr_struct    *queue_data_nxt;  
#endif
};

// Software semaphore structure
typedef struct sx_queue_sem_t
{
	sx_uint32	  flag[2];
	sx_uint32	  turn;
} sx_queue_sem;

// Queue structure (must be 32 byte aligned)
typedef struct SX_QUEUE_ALIGN(32) sx_queue_struct
{
#ifdef _WIN64
  sx_uint32						 data_last;   // 4 byte
  sx_uint32						 data_first;  // 4 byte
  sx_uint32						 callback;     // 4 byte
  sx_uint32                      param;        // 4 byte
  sx_uint32						 queue_nxt;   // 4 byte
  sx_uint32                      id;           // 4 byte
  sx_uint32                      status;      // 4 byte
#else
  struct sx_queue_datahdr_struct *data_last;   // 4 byte
  struct sx_queue_datahdr_struct *data_first;  // 4 byte
  sx_queue_callback              callback;     // 4 byte
  sx_uint32                      param;        // 4 byte
  struct sx_queue_struct         *queue_nxt;   // 4 byte
  sx_uint32                      id;           // 4 byte
  sx_uint32                      *status;      // 4 byte
#endif
  sx_uint8                       bitpos;       // 1 byte
  sx_uint8                       options;      // 1 byte
  sx_uint8                       dir;          // 1 byte
}
sx_queue;
#define SX_QUEUE_DESCR_SIZE   (sizeof(sx_queue))

// Size of mgmt_queue[] must be large enough for all mgmt msgs to be sent
// to host during sct_init + 1 used to denote "full queue".
// init msg             1
// host mgmt messages   SCT_MGMT_MSG_COUNT (see sct_msg.h)
// host messages        SCT_MSG_COUNT (see sct_msg.h)
// "full queue" marker  1
// Cannot include sct_msg.h, because sct_msg.h includes this file.
// If this size also happens to be large enough that the mgmt_queue[] can never
// be full, define SX_QUEUE_MGMT_NEVER_FULL to optimize code.
// Maximum number of messages that can be in mgmt_queue[] after init is
// SCT_MGMT_MSG_COUNT * 2.  Since this is less than SX_QUEUE_MGMT_QUEUE_SIZE,
// it should be okay to define SX_QUEUE_MGMT_NEVER_FULL below.
// if SCT_MGMT_MSG_COUNT get changed, SX_QUEUE_MGMT_QUEUE_SIZE needs to be updated
#define SX_QUEUE_MGMT_NEVER_FULL
#define SX_QUEUE_MGMT_QUEUE_SIZE    (1 + 2048*2 + 1)
typedef struct SX_QUEUE_ALIGN(32) sx_queue_mgmt_struct
{
    sx_queue                        base_queue;
#ifdef _WIN64
	sx_uint32						mgmt_queue[SX_QUEUE_MGMT_QUEUE_SIZE];
#else
    struct sx_queue_datahdr_struct  *mgmt_queue[SX_QUEUE_MGMT_QUEUE_SIZE];
#endif
}
sx_queue_mgmt;

#if (defined __STRETCH__)
typedef sx_uint32 * KSPIN_LOCK;
#endif

typedef struct sx_queue_mod_rsc_t
{
#ifdef _WIN64
	sx_uint32		queue;
#else
	sx_queue      * queue;
#endif
	sx_uint32	  init_done;
    sx_queue_sem  sem[2];   // sem[0] = board2host, sem[1]=host2board
} sx_queue_mod_rsc;



// Module structure
typedef struct
{	
#ifdef _WIN64
  sx_uint32		*	pp_queue;
#else
  sx_queue			** pp_queue;
#endif
  sx_queue_mod_rsc  *smem;	//shared memory pointer
  sx_uint8  options;
  KSPIN_LOCK spinlock; // Windows spin lock  
}
sx_queue_mod;

#define SWAP32(n) \
  ( \
    (((sx_uint32)(n) & 0x000000ff) << 24) | \
    (((sx_uint32)(n) & 0x0000ff00) <<  8) | \
    (((sx_uint32)(n) & 0x00ff0000) >>  8) | \
    (((sx_uint32)(n) & 0xff000000) >> 24) \
  )

// Adjustment of physical addressess due to endianness
#ifdef EB
#define SWAP_END32(n)    SWAP32((sx_uint32)(n))
#else
#ifdef EL
#define SWAP_END32(n)    ((sx_uint32)(n))
#else
#define SWAP_END32(n)    ((sx_uint32)(n))
//#error "No Endianness specified"
#endif
#endif


INLINE void sx_queue_crit_section_enter(sx_queue_sem *sem, sx_uint32 id)
{
    volatile sx_uint32 *turn = &sem->turn;
    volatile sx_uint32 *flag = &sem->flag[!id];

    sem->flag[id] = 1;
    *turn = id;

    while ( *flag && (*turn == id))
        ;
}

INLINE void sx_queue_crit_section_exit(sx_queue_sem *sem, sx_uint32 id)
{
    sem->flag[id] = 0;
}

#ifdef __cplusplus
}
#endif

#endif // _SX_QUEUE_H


