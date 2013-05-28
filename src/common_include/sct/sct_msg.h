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

#ifdef _SCT_MSG_H_
#else
#define _SCT_MSG_H_

#include "sx-queue-api.h"
#include "sct.h"    // for number of SCT message classes

#ifdef __cplusplus
extern "C" {
#endif

// ################################
// # Management message structure #
// ################################
typedef struct
{
    // Message header (Don't touch - reserved for IPC)
    sx_queue_datahdr hdr;

    // Message identifier (see below #defines)
    sx_uint32 id;

#define SCT_MGMT_MSG_INIT               1    // Sender = Board only
#define SCT_MGMT_MSG_CONNECT            2    // Sender = Board or Host
#define SCT_MGMT_MSG_DESTROY            3    // Sender = Board or Host
#define SCT_MGMT_MSG_DMA_DONE           4    // Sender = Board only
#define SCT_MGMT_MSG_HOST_RECVBUF       5    // Sender = Host only
#define SCT_MGMT_MSG_HOST_SENDBUF       6    // Sender = Host only

    // Response code (0=OK, any other value=ERROR)
    sx_uint32 rsp;

    // Message specific data (determined by 'id' field)
    union
    {
        // #### sct_init  ####
        // id = SCT_MGMT_MSG_INIT
        // Message is only sent from Board to Host since Board allocates the
        // messages.  The allocated messages (mgmt messages first, then regular
        // messages) are written by Board to MGMT queue.
        struct
        {
            sx_uint32 mgmt_msg_count; // Number of management messages sent to MGMT queue
            sx_uint32 msg_count;      // Number of regular messages sent to MGMT queue
            sx_uint32 status;        // Status word (PCI address) for Host to use for IPC queues.
            sx_uint32 pci_min;        // Lowest PCI address Host will use for buffers (low 32 bits)
            sx_uint32 pci_min_hi;     // Lowest PCI address Host will use for buffers (high 32 bits)
            sx_uint32 pci_max;        // Highest PCI address Host will use for buffers (low 32 bits)
            sx_uint32 pci_max_hi;     // Highest PCI address Host will use for buffers (high 32 bits)
        } init;

        // #### sct_channel_connect ####
        // id = SCT_MGMT_MSG_CONNECT
        // Message is sent by either Host or Board, depending on which side receives the
        // sct_channel_connect() call.
        struct
        {
            sx_uint32 port;           // port (aka channel)
            sx_uint32 max_size;       // Size of the buffers (max DMA transfer)
        } connect;

        // #### sct_channel_destroy ####
        // id = SCT_MGMT_MSG_DESTROY
        struct
        {
            sx_uint32 port;           // port (aka channel)
        } destroy;

        // id = SCT_MGMT_MSG_DMA_DONE
        // id = SCT_MGMT_MSG_HOST_RECVBUF
        // id = SCT_MGMT_MSG_HOST_SENDBUF
        // This message used by Host to send buffers to Board, and by Board
        // to send DMA done (i.e., return buffer) to Host.
        // If buffer is for DMA from Board to Host (Board send), port is set to
        // SCT_PORT_INVALID.  Board does not assign buffer to a SCT channel
        // until a send transfer is actually requested.
        // If buffer is for DMA from Host to Board (Board receive), port is used
        // to identify the SCT channel for which the buffer is intended.  Board
        // adds buffer to pending receives for that SCT channel.
        struct
        {
            sx_int32  port;         // port (aka channel), or SCT_PORT_INVALID
            sx_uint32 pci_addr;     // PCI address of buffer (low 32 bits)
            sx_uint32 pci_addr_hi;  // PCI address of buffer (high 32 bits)
            sx_uint32 size;         // Size of buffer
        } buffer;
    }
    data;
}
t_sct_mgmt_msg;

#define SCT_MGMT_MSG_SIZE  sizeof(t_sct_mgmt_msg)
#define SCT_MSG_DATA_SIZE 16

// ################################
// #  Non mgmt message structure  #
// ################################
typedef struct
{
    // Message header (Don't touch - reserved for IPC)
    sx_queue_datahdr hdr;

    sx_uint32        size;
    sx_uint8         data[SCT_MSG_DATA_SIZE];
} 
t_sct_msg;

#define SCT_MSG_SIZE  sizeof(t_sct_msg)

// Message counts
#define SCT_MGMT_MSG_COUNT            2048   // per direction
#define SCT_MSG_COUNT                 256   // per direction (shared by queues)

// ******** Main queue handle (address in PCI space) ********
// TODO:  This value will now be read from a scratch register on the S6 chip,
//        accessed at offset 0x000b0060 from PCI BAR0.  (chip addr 0x200b0060)
//#define SCT_QUEUE_MOD_PA              0x00000000 /* Offset into BAR 1 PCI space */

// ******** Number of Queue Status words available for each side ********
#define SCT_QUEUE_STATUS_CNT          8

// ******** Number of Message classes ********
// These are defined in sct.h, since they need to be exposed to the applications
// as part of the API.  However, changing the number of message classes can
// require other code changes (number of queue status registers needed, etc.).
//#define SCT_MSG_CLASS_COUNT
//#define SCT_MSG_CLASS_MIN
//#define SCT_MSG_CLASS_MAX

// ******** Max number of DMA channels ********
#define SCT_DMA_CHANNEL_COUNT         2   

// ******* SCT channel port *******
#define SCT_PORT_INVALID             (-1)

// ******* Queue IDs *******
// Every queue must have a unique Queue ID.

// Direction
#define SCT_DIR_BOARD2HOST            0  // Board to Host
#define SCT_DIR_HOST2BOARD            1  // Host to Board
// TODO:  Defines below should be obsoleted
#define SCT_DIR_S552WIN               SCT_DIR_BOARD2HOST  // S55DB to Windows
#define SCT_DIR_WIN2S55               SCT_DIR_HOST2BOARD  // Windows to S55DB

// Queues for Message classes, 2 queues for each
#define SCT_QUEUE_MSG_ID(class, dir)  (0x20000 + ((class) << 1) + (dir))   // 0x20000 .. 0x200ff

// Queues for return of messages, any class
#define SCT_QUEUE_MSG_RTN_ID(dir)     (0x21000 + (dir))                    // 0x21000 .. 0x21001

// Management queues, 1 queue in each direction
#define SCT_QUEUE_MGMT_ID(dir)        (0x30000 + (dir))                    // 0x30000 .. 0x30001

#ifdef __cplusplus
}
#endif

#endif // #ifdef _SCT_MSG_H_


