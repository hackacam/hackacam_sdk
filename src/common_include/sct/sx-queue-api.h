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

#ifndef _SX_QUEUE_API_H
#define _SX_QUEUE_API_H

#include "sx-port.h"

// Callback function
typedef void (*sx_queue_callback)(sx_uint32);

#include "sx-queue.h"

#ifdef __cplusplus
extern "C" {
#endif


//**********************************************************
// Define API types
//**********************************************************

// Module and Queue handles
typedef sx_queue_mod  *sx_queue_mod_ha;

typedef sx_queue      *sx_queue_ha;
typedef sx_queue_mgmt *sx_queue_mgmt_ha;

// Message header
typedef struct sx_queue_datahdr_struct  sx_queue_datahdr;

// Error codes 
#define SX_QUEUE_ERR_PARMS     ((SX_QUEUE_ERR_BASE) + 1)
#define SX_QUEUE_ERR_ALIGN     ((SX_QUEUE_ERR_BASE) + 2)
#define SX_QUEUE_ERR_EXISTS    ((SX_QUEUE_ERR_BASE) + 3)
#define SX_QUEUE_ERR_NOTFOUND  ((SX_QUEUE_ERR_BASE) + 4)
#define SX_QUEUE_ERR_EMPTY     ((SX_QUEUE_ERR_BASE) + 5)
#define SX_QUEUE_ERR_FULL      ((SX_QUEUE_ERR_BASE) + 6)

//**********************************************************
// Macros used for converting addresses between MIPS/PPC and S5610 space
//**********************************************************
//#define ADJ_ADDR_W(addr) SWAP_END32(ADJ_PHYS_W(CACHED_TO_PHYS(addr))) 
//#define ADJ_ADDR_R(addr) PHYS_TO_CACHED(ADJ_PHYS_R(SWAP_END32(addr)))

//**********************************************************
// Function prototypes
//**********************************************************

//**********************************************************
// Name        : sx_queue_module_init
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Initialise queue module
//**********************************************************
sx_uint32
sx_queue_module_init(
  sx_queue_mod     *queue_mod,     // Pointer to queue module structure in local memory.
  sx_queue_mod_rsc *queue_mod_sha, // Pointer to sx_queue_mod_rsc object in shared memory.
  sx_uint32        options,        // Options (see below defines)

#define SX_QUEUE_OPTION_MULTI          0x0001   // If set, this module will be used for multi-CPU
                                                // or multi-thread communication.

#define SX_QUEUE_OPTION_PRIMARY        0x0002   // In a multi-CPU/multi-thread system (see option MULTI above), 
                                                // one and only one of the CPUs/threads must set this
                                                // flag when initialising a queueing module.
                                                // If MULTI flag is not set, this flag is don't care.
                                             
#define SX_QUEUE_OPTION_SAVE_CONTEXT   0x0004   // This flag is reserved for future use and is currently
                                                // don't care.

  sx_queue_mod_ha  *mod_ha );   // Output : Module handle

//**********************************************************
// Name        : sx_queue_create
//
// Returns     : 0 if operation is ok, otherwise error code
//
//**********************************************************
sx_uint32
sx_queue_create(
  sx_queue_mod_ha  mod_ha,      // Module handle
  sx_queue         *queue,      // Pointer to queue structure (32 byte aligned)
  sx_uint32        id,          // Unique ID of queue
  sx_uint8         dir,         // Queue direction.  0=board2host, 1=host2board.
  sx_queue_ha      *queue_ha ); // Output : Queue handle

//**********************************************************
// Name        : sx_queue_lookup
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Lookup queue handle based on queue ID.
//**********************************************************
sx_uint32 
sx_queue_lookup(
  sx_queue_mod_ha  mod_ha,      // Module handle
  sx_uint32        id,          // ID of queue
  sx_queue_ha      *queue_ha ); // Output : Queue handle

//**********************************************************
// Name        : sx_queue_callback_register
//
// Returns     : 0 if operation is ok, otherwise error code
//
//**********************************************************
sx_uint32 
sx_queue_callback_register(
  sx_queue_mod_ha    mod_ha,         // Module handle
  sx_queue_ha        queue_ha,       // Queue handle
  sx_queue_callback  callback,       // Callback function
  sx_uint32          param,          // Parameter for callback
  sx_bool            save_context ); // For future use, must be set to TRUE

//**********************************************************
// Name        : sx_queue_delete
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Delete queue.
//               It is strongly recommended that the queue
//               is emptied before performing this operation.
//**********************************************************
sx_uint32 
sx_queue_delete(
  sx_queue_mod_ha  mod_ha,     // Module handle
  sx_queue_ha      queue_ha ); // Queue handle

//**********************************************************
// Name        : sx_queue_write
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Write entry to queue.
//**********************************************************
sx_uint32 
sx_queue_write(
	sx_queue_mod_ha  mod_ha,      // Module handle
    sx_queue_ha       queue_ha, // Queue handle
    sx_queue_datahdr *data_hdr, // Pointer to data header (32 byte aligned)
    sx_uint32         size );   // Size of message

//**********************************************************
// Name        : sx_queue_read
//
// Returns     : 0 if operation is ok (data available), otherwise error code
//
// Description : Read entry from queue.
//**********************************************************
sx_uint32 
sx_queue_read(
	sx_queue_mod_ha  mod_ha,      // Module handle
    sx_queue_ha       queue_ha,     // Queue handle
    sx_queue_datahdr  **data_hdr ); // Output : Data header (possibly NULL)

//**********************************************************
// Name        : sx_queue_isr
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Queue module ISR to be invoked by interrupt handler
//**********************************************************
sx_uint32
sx_queue_isr( 
  sx_queue_mod_ha  mod_ha ); // Module handle

//**********************************************************
// Name        : sx_queue_status_register
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Register STATUS word for particular queue.
//               'status' must be in shared memory.
//               Only one status word can be registered
//               for any given queue, but same status word
//               can be used for several queues as long
//               as 'bitpos' is different.
//               For each bit, a '1' signifies that queue 
//               is non empty.
//**********************************************************
sx_uint32
sx_queue_status_register(
	sx_queue_mod_ha  mod_ha,      // Module handle
	sx_queue_ha       queue_ha,     // Queue handle
    sx_uint32         *status,      // Status word
    sx_uint8          bitpos );     // Bit position in status word

//**********************************************************
// Name        : sx_queue_status_unregister
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Unregister STATUS word for particular queue.
//**********************************************************
sx_uint32
sx_queue_status_unregister(
	sx_queue_mod_ha  mod_ha,      // Module handle
    sx_queue_ha queue_ha );  // Queue handle

//**********************************************************
// Name        : sx_queue_status_read
//
// Returns     : Status word
//
// Description : Read STATUS word
//**********************************************************
sx_uint32
sx_queue_status_read(
	sx_queue_mod_ha  mod_ha,      // Module handle
    sx_uint32 *status );  // Status word to be read

//**********************************************************
// Name        : sx_queue_write
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Write entry to queue.
//**********************************************************
sx_uint32 
sx_queue_mgmt_write(
	sx_queue_mod_ha  mod_ha,    // Module handle
    sx_queue_mgmt_ha queue_ha,  // Queue handle
    sx_queue_datahdr *data_hdr, // Pointer to data header (32 byte aligned)
    sx_uint32        size );    // Size of message

//**********************************************************
// Name        : sx_queue_read
//
// Returns     : 0 if operation is ok (data available), otherwise error code
//
// Description : Read entry from queue.
//**********************************************************
sx_uint32 
sx_queue_mgmt_read(
	sx_queue_mod_ha  mod_ha,       // Module handle
    sx_queue_mgmt_ha queue_ha,     // Queue handle
    sx_queue_datahdr **data_hdr ); // Output : Data header (possibly NULL)

#ifdef __cplusplus
}
#endif

#endif // #ifdef _SX_QUEUE_API_H


