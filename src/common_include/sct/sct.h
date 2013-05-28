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

/**********************************************************************
    PACKAGE: Stretch Communication Toolkit (SCT)

    DESCRIPTION:

    SECTION: Introduction

    The Stretch Communication Toolkit (SCT) for Windows and Linux supports the
    exchange of bulk data and control information between host 
    applications and custom software running on Stretch processors 
    present on PCI adapter cards installed in the system. The block
    transfer mechanism uses the concept of "channels" while control
    information exchange is supported via "messages".


    SECTION: Endpoints
    
    The API is similar for both the host applications as well as
    the applications running on the Stretch processors, but is not
    completely symmetric. A host application may be communicating 
    with one or more boards in the system. Hence the host-side
    API uses a "board handle" to identify the board to interact with.
    Applications running on the Stretch processor can only interact
    with a single host-side application, and therefore there is no
    provision to specify a peer entity in the board-side API.
    
    A single host application may control one or more PCI boards
    in the system. A single board, however, cannot be shared among
    multiple host applications.


    SECTION: Channels

    As a simple abstraction, TCP/IP has a model of "connect" (initiate
    a connection from the current host to a specified host and port) and
    "accept" (accept a connection to the current host from a specified host
    and port).  For "accept" the host is typically a "wild-card" meaning
    allow a connection from any host on a given port.  Once a connection is
    established, it is bi-directional; both sides may read and write.  Also,
    if many hosts (or the same host, multiple times) connect to the same
    host and port, then all of the connections are queued (up to a specified
    "backlog", typically hundreds).

    The SCT model is different.  We have connect and accept, but a connection
    is uni-directional from the connect host to the accept host.  The connect
    host is allowed to "send" ("write") and the accept host is allowed to
    "recv" ("read").  At most one connection exists between the connect host
    and the accept host for a given port number; that is, connections are
    not queued as they are for TCP/IP.  Only the connect host may terminate
    the connection.  Finally, there are a limited number of connections per
    board. The current limit is 8. The API manages all of the buffering 
    avoiding the typical data copy of more abstract APIs.  This also allows
    for transparent double- or triple-buffering which is typically used to
    overlap processing and communication.

    Writing (sending) side APIs:
    {
        err = sct_channel_connect(board, port, nbuf, max_size, &channel)
        p   = sct_buffer_alloc(channel)
        sct_buffer_send(channel, p, size)
        sct_channel_close(channel)
    }
    Note that sct_buffer_send() is really "send and free".

    Reading (receiving) side API's:
    {
        channel = sct_channel_accept(board, port)
        p = sct_buffer_recv(channel, &size)
        channel = sct_buffer_poll(channels, n)
        sct_buffer_free(channel)
    }
    Note that sct_buffer_recv() is really "alloc and recv".

    sct_buffer_alloc() blocks if no buffers are available on the sending
    side.  sct_buffer_send() blocks if no buffers are available on the
    receiving side (or if no DMA resources are available).

    sct_buffer_recv() blocks until data is ready from the given channel.
    sct_buffer_poll() polls the channels in an array and returns the first
    channel which has data available.  sct_buffer_poll() never blocks,
    instead returning an indication that no channel has data available.


    SECTION: Channel Example

    This shows a simple example of an application that opens a read
    channel and a write channel to a board, and reads and writes data
    with a block size of 64K bytes. Two buffers are used to overlap
    buffer processing with buffer transfer.
    {
    static void
    func1( sct_board_t * board )
    {
        sct_channel_t * chan_send, * chan_recv;
        char * pin, * pout;
        int i, size, err;

        // open read channel from board using port 20
        chan_recv = sct_channel_accept( board, 20 );

        // open write channel to board using port 21, 2 buffers, 64KB
        err = sct_channel_connect( board, 21, 2, 65536, &chan_send );

        while (1) {
            // allocate and receive buffer from board
            pin = sct_buffer_recv( chan_recv, &size );
            if (pin == 0) {
                break;
            }

            // allocate output buffer to write
            pout = sct_buffer_alloc( chan_send );

            // process
            for( i = 0; i < size; i++ ) {
                pout[i] = pin[i] ^ 0x55;
            }

            // free receive buffer (pin) 
            sct_buffer_free( chan_recv );

            // send buffer (pout) to board and free
            sct_buffer_send( board, chan_send, size );
        }
    }
    }


    SECTION: Messages

    Messages are single point-to-point packets from the application to the
    board and vice versa.  Broadcast or multicast messages are not supported.
    Messages are intended to be the "control" channel for application-board
    communication (in contrast to channels which are the "data" path). They
    are expected to be sent and received relatively infrequently and to be
    used for small blocks of data. A typical example might be passing a
    command from the application to the board to set the board into a new
    mode.

    A message is a 16-byte block of data.  There is no support for longer
    messages or variable length messages.  It is assumed that the sender
    and receiver agree on the data format of the message.

    The message is delivered reliably and in-order.  However, note that if
    the receiver is not actively reading and responding to received messages,
    that the network can "back-up" with user messages.

    Each message has a specified class, which is an integer between 0 and 31.
    The purpose of the MessageClass is to allow the receiver of a message to
    infer type information on the message based on the MessageClass or to
    organize the messages by MessageClass.  SCT places no semantics on the
    "MessageClass"; it is entirely up to the user's application.

    A message is sent using sct_message_send(), which specifies the
    destination, the MessageClass, a buffer containing the message, and the
    size of the buffer.

    A message is received by the application using sct_message_recv().
    sct_message_recv() has blocking semantics.  There is also a nonblocking
    version, sct_message_recv_poll().  sct_message_recv() allows filtering
    of the messages based on the "src" or "MessageClass" fields.


    SECTION: Message Example

    This might be a code fragment in the host application:
    {
    // receive message from "board1", (any MessageClass)
    sct_message_recv( board1, -1, &buffer, NULL, &MessageClass );
    
    switch(MessageClass) {
    case 0:
        pmessage0 = (void *) buffer;
        .
        .
        .
        break;

    case 1:
        pmessage1 = (void *) buffer;
        .
        .
        .
        break;

    default:
        break;
    }
    }

*******************************************************************************/


#ifndef __SCT_H__
#define __SCT_H__



#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
                                   DEFINES
*******************************************************************************/


/*******************************************************************************
    The maximum number of boards in a system.
*******************************************************************************/
#define SCT_MAX_BOARDS                8

/*******************************************************************************
    The maximum port number.
*******************************************************************************/
#define SCT_MAX_PORT_NUMBER           65535

/*******************************************************************************
    The maximum number of SCT channels.
*******************************************************************************/
#define SCT_MAX_CHANNEL               128

/*******************************************************************************
    Message class IDs.
*******************************************************************************/

/*******************************************************************************
    The number of message classes.
*******************************************************************************/
#define SCT_MSG_CLASS_COUNT           128
/*******************************************************************************
    The minimum value for message class.
*******************************************************************************/
#define SCT_MSG_CLASS_MIN             0
/*******************************************************************************
    The maximum value for message class.
*******************************************************************************/
#define SCT_MSG_CLASS_MAX             (SCT_MSG_CLASS_COUNT - 1)

/*******************************************************************************
    Message class wildcard.
*******************************************************************************/
#define SCT_MSG_CLASS_ANY             ((sx_uint8)-1)

/*******************************************************************************
    The maximum message size.
*******************************************************************************/
#define SCT_MAX_MESSAGE_LEN           16

/*******************************************************************************
    Port number wildcard.
*******************************************************************************/
#define SCT_PORTNUM_ANY               ((sx_uint32)-1)


/*******************************************************************************
    VISIBLE: Error Codes.
    NOTE: The device driver error range is 1000 - 2000. This is so that we
    can distinguish between Firmware, Device Driver, and DVR SDK error
    messages.
*******************************************************************************/
typedef enum _SCT_ERROR_CODE
{
    SCT_NO_ERROR = 0,
    SCT_ERR_INVALID_PARAMETER = 1000,
    SCT_ERR_DEVICE_IN_USE,
    SCT_ERR_DEVICE_OPEN,
    SCT_ERR_DEVICE_CLOSE,
    SCT_ERR_DEVICE_RESET,
    SCT_ERR_IPC_INIT,
    SCT_ERR_NO_CHANNELS,
    SCT_ERR_CHANNEL_IN_USE,
    SCT_ERR_CHANNEL_CREATE,
    SCT_ERR_CHANNEL_CONNECT,
    SCT_ERR_CHANNEL_CLOSE,
    SCT_ERR_CHANNEL_NOT_ACTIVE,
    SCT_ERR_CHANNEL_DEAD,
    SCT_ERR_NO_RECV_BUFFERS,
    SCT_ERR_NO_SEND_BUFFERS,
    SCT_ERR_MSG_SEND,
    SCT_ERR_MSG_RECV,
    SCT_ERR_BOARD_BOOT_FAIL
} SCT_ERROR_CODE;


/*******************************************************************************
                                    TYPES
*******************************************************************************/

/* --- Data types --- */

#if (defined __STRETCH__) && (!defined __STRETCH_NATIVE__)
  /* All types defined in sx-types.h */
#else
#ifndef __H_SXTYPES
#define __H_SXTYPES
  typedef unsigned char             sx_uint8;    /* 8  bit unsigned integer */
  typedef unsigned short            sx_uint16;   /* 16 bit unsigned integer */
  typedef unsigned int              sx_uint32;   /* 32 bit unsigned integer */
  typedef unsigned long             sx_uint64;   /* 64 bit unsigned integer */
  typedef signed char               sx_int8;     /* 8  bit signed integer   */
  typedef signed short              sx_int16;    /* 16 bit signed integer   */
  typedef signed int                sx_int32;    /* 32 bit signed integer   */
  typedef signed long               sx_int64;    /* 64 bit signed integer   */

  /* Boolean type */
  typedef int                       sx_bool;
#endif
#endif


/*******************************************************************************
    A board handle.
*******************************************************************************/
#if (defined IPC_S55DB || IPC_JAMAICA)
typedef int        sct_board_t;
#else
typedef void *     sct_board_t;
#endif

/*******************************************************************************
    A channel handle.
*******************************************************************************/
typedef void *     sct_channel_t;

/*******************************************************************************
    Callback function for channels.
*******************************************************************************/
typedef void (sct_callback_func)( void * context, void * buffer, int size);

/*******************************************************************************
    Callback function for messages.
*******************************************************************************/
typedef void (sct_message_callback_func)( void * context, sx_int32  msg_class, void * buffer);


/*******************************************************************************
    GROUP: Internal
*******************************************************************************/

/*******************************************************************************
                            FUNCTION PROTOTYPES
*******************************************************************************/


/*******************************************************************************
    GROUP: API query functions.
*******************************************************************************/

/*******************************************************************************
    Returns the version number of the SCT API. The version is 4 digit number
    every number is 8 bits. Start from the highest bit: build.minus.minor.major

    Parameters:

        None.

    Returns:

        The version number.
*******************************************************************************/
extern sx_uint32
sct_get_version(void);

/*******************************************************************************
    GROUP: Board query functions.
*******************************************************************************/

extern const char * sct_get_board_name( sx_uint32 board_index );

extern sx_uint32 sct_open_board( int board_index,
                                 sct_board_t *   phandle );

extern sx_uint32 sct_close_board( sct_board_t board );


extern sx_uint32 sct_reset_board( sct_board_t board );

extern sx_uint32 sct_board_sync( sct_board_t board );

/*******************************************************************************
    Length of the serial number string.
*******************************************************************************/
#define STRETCH_BOARD_SERIAL_LENGTH      16

/*******************************************************************************
    Board information data structure.
*******************************************************************************/
typedef struct sct_board_detail_t_tag {
    sx_uint32   board_number;
    sx_uint32   board_slot_number;
    sx_uint16   vendor_id;
    sx_uint16   device_id;
    sx_uint8    revision;
    sx_uint16   subsystem_vendor;
    sx_uint16   subsystem_id;
    sx_int8     serial_string[STRETCH_BOARD_SERIAL_LENGTH+1];  /*null terminated serial number string */
	sx_uint8	driver_version[4];
    sx_uint32   buffer_size;
    sx_uint32   buffer_count;
    sx_uint32   max_recv_buf_per_chan;
} sct_board_detail_t;

extern sx_uint32 sct_get_board_detail(sct_board_t board, sct_board_detail_t * pbd);

/*******************************************************************************
    Data structure defining the different parts of boot-loader and BSP versions.
*******************************************************************************/
typedef struct _ml_versions {
    union {
        sx_uint32 version_data;

        struct _version_fields {
            sx_uint8 ml_rev_major;
            sx_uint8 ml_rev_minor;
            sx_uint8 bsp_rev_major;
            sx_uint8 bsp_rev_minor;
        } version_fields;
    } u;
} ml_versions;
/*for the old board sct_get_bootloader_version will return u.version_fields.ml_rev_major =1 and everything else is 0
 u.version_data = 1*/
extern ml_versions sct_get_bootloader_version( sct_board_t board );


/*******************************************************************************
    GROUP: API access functions.
*******************************************************************************/

extern sx_uint32 sct_init(void);
extern sx_uint32 sct_clean(void);

extern sx_uint32 sct_channel_connect( sct_board_t        board,
                                      sx_int32           port,
                                      sx_uint32          nbuf,
                                      sx_uint32          size,
                                      sct_channel_t *    chan_return );

#if (defined IPC_S55DB || IPC_JAMAICA)
extern sct_channel_t sct_channel_accept( sct_board_t     board,
                                         sx_int32        port);
#else
extern sct_channel_t sct_channel_accept( sct_board_t     board,
                                         sx_int32        port ,
                                         sx_int32        nbuf);
#endif

extern sx_uint32 sct_channel_close( sct_channel_t channel );

extern void * sct_buffer_alloc( sct_channel_t channel );
extern void * sct_buffer_alloc_wait( sct_channel_t channel , int timeout /*in milli seconds*/);

extern sx_uint32 sct_buffer_send( sct_channel_t    channel,
                                  void *           buffer,
                                  sx_int32         size );

extern void * sct_buffer_recv( sct_channel_t    channel,
                               sx_int32 *       psize );

extern sct_channel_t sct_buffer_poll( sct_channel_t *    channels,
                                      sx_int32           n );

extern sx_bool sct_register_callback( sct_channel_t       channel,
                                      sct_callback_func * func,
                                         void *              context );

#if (defined IPC_S55DB || IPC_JAMAICA)
extern void sct_buffer_free( sct_channel_t channel);
#else
extern void sct_buffer_free( sct_channel_t channel , void * p);
#endif

extern sx_uint32 sct_message_send( sct_board_t     board,
                                   sx_int32        msg_class,
                                   void *          buffer,
                                   sx_int32        size );

extern sx_uint32 sct_message_recv( sct_board_t     board,
                                   sx_int32        msg_class,
                                   void *          buffer,
                                   sct_board_t *   psrc_board,
                                   sx_int32 *      pmsg_class );

extern sx_bool sct_message_recv_poll( sct_board_t    board,
                                      sx_int32       msg_class,
                                      void *         buffer,
                                      sct_board_t *  psrc_board,
                                      sx_int32 *     pmsg_class );

extern sx_bool sct_register_message_callback( sct_board_t    board,
                                              sx_int32       msg_class,
                                              sct_message_callback_func* func,
                                              void *         context );
extern void sct_set_timeout(sct_board_t    board, sx_int32  timeout );

/****************************************************************************
    Max number of PEs on a single board.
****************************************************************************/
#define SCT_MAX_PE 32

/****************************************************************************
    Data structure for returning diagnostics results from board.
****************************************************************************/
typedef struct sct_diag_info_tag
{
    int         pe_num;
    sx_uint32   err_nums[SCT_MAX_PE];
}sct_diag_info_t;

/*****************************************************************************
Use this function to load firmware code to the board.

    @Parameters@:

        "board" - The board handle
        "path"  - The file path to the firmware code. user can only upload the .rom file

        "perrno" - Pointer to the err codes. perror needs to be a pointer to a 16 byte buffer.
        The error code returned is a int[4] arrary, the array contains error code for 4 PEs.
        When perrno equals to NULL, sct_load_firmware will not check for return code of 
        firmware loading, when perrno is not NULL,  sct_load_firmware will wait until the
        firmware loading is complete and return the error code. The perrno is usually used
        when loading the diag firmware.

    @Returns@:

        true when load firmware success.
        false when load firmware failed.
*******************************************************************************/
extern sx_bool sct_load_firmware( sct_board_t  board, char * path, sct_diag_info_t* pdiag_info);

/*****************************************************************************
Use this function to query the max receive buffer a channel can have

    @Parameters@:
        "board" - The board handle

    @Returns@:

        The max number of receive buffers a channel can have. When 
        sct_channel_accept(sct_board_t board, sx_int32 port, sx_int32 nbuf)
        is be called, nbuf needs to be smaller than this value. If nbuf is
        larger than sct_max_recv_buf_per_chan(), nbuf will be override with
        sct_max_recv_buf_per_chan() inside the sct_channel_accept function.
*******************************************************************************/
extern int sct_max_recv_buf_per_chan(sct_board_t  board);


/*******************************************************************************
    GROUP: Internal
*******************************************************************************/

/*******************************************************************************
   PCIe Target Read/write
   For interal hardware test only
*******************************************************************************/
extern int WriteMem(sct_board_t bd, int bar, int addr, void * buf, int size);
extern int ReadMem (sct_board_t bd, int bar, int addr, void * buf, int size);
#ifdef PPC
extern void * sct_malloc(int bdnum, sx_uint32 size);
extern void sct_free(int bdnum, void * p);
extern void sct_host_dma(int bdnum, void * src, sx_uint32 size, sx_uint32 dst);
#endif
#if defined(ARM) || defined(__ARM__)
extern sx_bool sct_memmap(int bdnum, sx_uint32 offset, sx_uint32 size, void **outp);
extern int sct_dma_memcpy(void * dst,void* src, sx_uint32 size);
extern void * sct_dma_malloc( sx_uint32 * psize);

#endif
#ifdef __cplusplus
}
#endif

#endif /* __SCT_H__ */

