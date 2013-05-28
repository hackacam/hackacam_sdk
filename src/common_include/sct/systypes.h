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


#ifndef _SYSTYPES_H_
#define _SYSTYPES_H_


#include "sx-queue-api.h"
#include "sx-queue.h"
#include "sct_msg.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_PCI_BAR						6
#define PCI_BAR_REG_MASK_IO				0x1
#define PCI_BAR_REG_MASK_MEM			0xFFFFFFF0
#define PCI_BAR_REG_MASK_TYPE			0x6

#define RESPONSE_OK 0

// Event indexes. These should match the ones defined in the driver!

#define DESTROY_EVENT                   0
#define ERROR_EVENT                     1
#define UPDATE_BUF_SEND_EVENT           2 //For send channel
#define BUF_FREE_EVENT                  2 //For receive channel 


typedef struct _sct_dll_msg
{
    INT32       classId;
    t_sct_msg   msg;
} sct_dll_msg;


typedef struct _DnDev_bar_type 
{
     _LARGE_INTEGER    log_addr;     
     unsigned long     size;	
} DnDev_bar_type;


// PCI config space info for each device
typedef struct _DnDev_Data
{
	unsigned short    vid; // 16-bit
	unsigned short    did;     
	DnDev_bar_type    bar[MAX_PCI_BAR];	
} dndev_t;


typedef struct _WINOFFLD_IOCTL
{
	unsigned long type;
	unsigned long bar;
	unsigned long offset;
	unsigned long data;
}WINOFFLD_IOCTL;


// info for each device
typedef struct QUEUE {
	INT32 status; // To hold the status of the queue.
	INT32 allocatedMessages; // to hold number of allocated messages.
} QUEUE;

// Management Queue structure
typedef struct MGMT_QUEUE {
	INT32 status; // to hold status of managment queues
} MGMT_QUEUE;


#define WINOFFLD_MGMT_QUEUE_ACTIVE              0
#define WINOFFLD_MGMT_QUEUE_NOT_ACTIVE         -1


#ifdef __cplusplus
}
#endif

#endif // _SYSTYPES_H_
