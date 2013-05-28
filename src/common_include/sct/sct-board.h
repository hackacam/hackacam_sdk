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

    SECTION: Usage
    {
    #include "sct-board.h"
    }

    SECTION: Introduction

    The Stretch Communication Toolkit (SCT) supports the exchange
    of bulk data and control information between host applications and
    application running on Stretch boards.  The block transfer
    of data uses the concept of "channels" while control information exchange
    is supported via "messages".

    The SCT must be run in conjunction with the SCT for Windows or SCT for Linux
    module on the host machine.  SCT is implemented as a library for
    both the firmware and host, but the host side also requires a custom
    driver to complete the SCT module.
    
    Stretch boards contains contain one or more Stretch processors (or Processor Entity, PE).
    PE0 is the master PE and is the only PE with a interface connected to the host system, 
    thus all SCT channel and message data must pass through PE0.

    Only application tasks on PE0 can actually use the SCT API.  Application
    tasks on remote PEs must use the SCPA API to interface with the SCT API,
    to communicate with the host applications over the interface.  To
    reduce complexity, application tasks on PE0 should also use the SCPA API to
    interface to the SCT API, with only certain dedicated PE0 task or tasks
    using the SCT API directly.

    Multiple Stretch boards can be plugged into a host machine.  A
    single host application can control more than one board concurrently.
    However, multiple host applications cannot share the same board.  In
    other words, each board can only interact with a single host application.

    The SCT API for firmware and host support the same functions, but are
    not completely symmetric with each other.  To allow a host application
    to access multiple boards, the host-side API uses a "board handle" to
    identify a particular board and needs extra board-specific control routines.
    Other implementation requirements have also resulted in some differences
    between the APIs.


    SECTION: Assumptions

    This implementation assumes that the PCI Aperture Space will be mapped to
    a range of PCI addresses that is sufficient to cover all possible host
    buffers so that remapping will not be required.

    This implementation also assumes that host-side buffers will be
    contiguous so that only one DMA will be required per buffer.


    SECTION: Channels

    The board and host SCT applications must each call the sct_init()
    function to allocate resources and sync up with each other before the rest
    of the SCT channel and message API can be used.

    An SCT channel is used to transfer bulk data.  Each channel is
    uni-directional, allowing only one side to send.  A channel is created when
    a sender connects and the receiver accepts.  To terminate a channel
    connection, the sender closes the channel first.  The receiver can only
    close the channel after checking that the channel has been closed by the
    sender.  There are a limited number of connections per board. The current
    limit is 64.

    SCT supports management of data buffers but does not actually allocate or
    free any internally.  Data buffers on the board side are allocated by the
    board application tasks, which may be running on any PE.  Data buffers on
    the Host side are accessed by the board using pointers sent by the Host to
    the board.  The SCT handler for windows interrupts translates the pointers
    from PCI addresses into PCI Aperture Space addresses and either stores them
    in a free list or passes them to specific SCT channels, depending on whether
    the buffers are for send or receive.  For send channels, all channels pull
    from the same free list of pointers to empty Host buffers.  For receive
    channels, the buffers already contain data destined for a specific channel,
    so are passed on to the channel.

    SCT supports the protocol for data transfer with the host driver, but
    does not actually perform the data transfers between the board and host.
    The data transfers are left up to the application tasks, which will use
    the SCPA API to initiate the tranfers.  For more details on how to use the
    SCPA API, please see the SCPA API documentation.

    @NOTE@:  SCPA running on PE0 will transfer data to/from PCIe using
    dedicated PCIe DMA channels.  SPCA running on remote PEs will transfer
    data using NI DMA channels, which will go through the NB interface.  These
    transfers are no longer DMAs at the PCIe interface.  This results in a
    performance hit for buffer transfers from remote PEs.

    Writing (sending) side API's:
    {
        err = sct_chan_connect(port, buf_size, &channel);
        p   = sct_chan_tx_getbuf(channel);
        SCPA API calls transfer data;
        sct_chan_tx_send(channel, p, size);
        sct_chan_close(channel);
    }
    Note that sct_chan_tx_send() is only "send" and not "send and putbuf".  The
    buffer cannot automatically be returned to the free list.  It is up to the
    Host to give free buffers to back to the board.

    sct_chan_tx_getbuf() blocks if no host buffers are available for sending.

    Reading (receiving) side API's:
    {
        channel = sct_chan_accept(port);
        channel = sct_chan_rx_poll(channels);   // call is optional here
        p = sct_chan_rx_recv(channel, &size);
        SCPA API calls to transfer data;
        sct_chan_rx_putbuf(channel);
    }
    Note that sct_chan_rx_recv() is really "getbuf and recv".

    sct_chan_rx_recv() blocks until a host buffer for the given channel is
    available.  sct_chan_rx_poll() polls the channels in an array and returns
    the first channel which has data available.  sct_chan_rx_poll() never
    blocks, instead returning an indication that no channel has data available.


    SECTION: Channel Example
    TODO


    SECTION: Host Buffer Usage

    When the host driver starts, it allocates DMA buffers in kernel memory.
    For buffer sends from board to host, the host driver sends all the
    buffer pointers during SCT initialization, before any channels are created.
    The board firmware translates these addresses from PCI bus addresses to
    addresses in the PCI Aperture Space, and stores them in a free list.
    If SCT supports two different buffer sizes, two different free lists are
    needed.  The board firmware pulls from a free list for each
    sct_chan_tx_getbuf() call by the board-side application.  After the buffer
    is sent by the application, it calls sct_chan_tx_send() and the buffer
    pointer is returned to host.  The host driver can recycle or send new
    buffer pointers to the board at any time, and the board firmware will
    replenish its free list.

    For buffer receive, the host driver sends a pointer to the host-side
    buffer.  The board firmware translates the address to a PCI aperture
    address, then stores in in a per-channel recv list.  When the board-side
    application calls sct_chan_rx_recv() for a channel, the first buffer in the
    channel's recv list is returned.  When the application is done with the
    buffer, it calls sct_chan_rx_putbuf(), and the board returns the pointer
    to host.

    Each SCT channel keeps track of the buffer pointers it has returned to the
    board firmware when sct_chan_tx_getbuf() or sct_chan_rx_recv() is called.
    Each channel can only store SCT_MAX_APP_OWNED_BUFFERS pointers, so whenever
    the board firmware is still holding this many buffers, sct_chan_tx_getbuf()
    or sct_chan_rx_recv() will return NULL.  Calling sct_chan_tx_send() or
    sct_chan_tx_putbuf() will release tx buffers to the SCT channel and calling
    sct_chan_rx_putbuf() will release rx buffers to the SCT channel.

    Each SCT receive channel can receive and store up to SCT_MAX_RECV_BUFFERS
    from the Host, which are then passed to the board firmware when it calls
    sct_chan_rx_recv().  If the application does not call sct_chan_rx_recv()
    with enough frequency, the receive channel can back up.  At that point,
    the channel will stop receiving new buffers and will return any incoming
    buffers to the Host with an error code.

    SECTION: Messages

    Messages are single point-to-point packets from the host application to
    the board and vice versa.  Broadcast or multicast messages are not
    supported.  Messages are intended to be the "control" channel for
    host application-board application communication (in contrast to
    channels which are the "data" paths). They are expected to be sent and
    received relatively infrequently and to be used for small blocks of data.
    A typical example might be passing a command from the host application   
    to the board to set the board into a new mode.

    A message is a 16-byte block of data.  There is no support for longer
    messages or variable length messages.  It is assumed that the sender
    and receiver agree on the data format of the message.

    The message is delivered reliably and in-order.  However, note that if
    the receiver is not actively reading and responding to received messages,
    that the network can "back-up" with messages.

    Each message has a specified class, which is an integer between 0 and 127.
    The purpose of the message class is to allow the receiver of a message to
    infer type information on the message based on the message class or to
    organize the messages by message class.

    A message is sent using sct_message_send(), which specifies the
    destination, the message class, a buffer containing the message, and the
    size of the buffer.

    A message is received by the application using sct_message_recv().
    sct_message_recv() has blocking semantics.  There is also a nonblocking
    version, sct_message_recv_poll().  sct_message_recv() allows filtering
    of the messages based on the "class" field, if an application is trying
    in receive messages of a specific class.


    SECTION: Message Example

    {
        sx_uint8 msg_buffer[SCT_MAX_MESSAGE_LEN];
        sx_uint8 msg_class;

        // receive message, any class
        sct_message_recv(-1, &msg_buffer, NULL, &msg_class );
        
        // handle message based on class
        switch(msg_class) {
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

#include "sx-port.h"

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
#define SCT_MAX_CHANNEL				  128

/*******************************************************************************
    The maximum buffer size.
*******************************************************************************/
#define SCT_MAX_BUFFER_SIZE           (512 * 1024)

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
    The maximum number of outstanding buffers (returned by sct_chan_tx_getbuf() 
    or sct_chan_rx_recv()) an application can hold per channel.  An application
    must return buffers to the SCT channel before it can request again.
*******************************************************************************/
#define SCT_MAX_APP_OWNED_BUFFERS     20

/*******************************************************************************
    The maximum number of receive buffers that can be received from the Host
    by each SCT channel.  These buffers are stored internally in the SCT channel
    and are passed to an application when it calls sct_chan_rx_recv().
*******************************************************************************/
#define SCT_MAX_RECV_BUFFERS          20


/*******************************************************************************
    Error Codes.
*******************************************************************************/
typedef enum _SCT_ERROR_CODE
{
    SCT_NO_ERROR = 0,
    SCT_ERR_INVALID_PARAMETER,
    SCT_ERR_CHANNEL_IN_USE,
    SCT_ERR_CHANNEL_ALLOC,
    SCT_ERR_CHANNEL_STATE,
    SCT_ERR_CHANNEL_CONNECT,
    SCT_ERR_CHANNEL_CLOSE,
    SCT_ERR_CHANNEL_NOT_ACTIVE,
    SCT_ERR_BUFFERS_IN_USE,
    SCT_ERR_FSM_INIT,
    SCT_ERR_SEND_BUF_SIZE,
} SCT_ERROR_CODE;


/*******************************************************************************
                                    TYPES
*******************************************************************************/


/*******************************************************************************
    A board handle.
*******************************************************************************/
typedef int        sct_board_t;

/*******************************************************************************
    A channel handle.
*******************************************************************************/
typedef void *     sct_channel_t;


/*******************************************************************************
    GROUP: Internal
*******************************************************************************/

/*******************************************************************************
    API Version.
*******************************************************************************/
#define SCT_API_VERSION               1


/*******************************************************************************
                            FUNCTION PROTOTYPES
*******************************************************************************/


/*******************************************************************************
    GROUP: API version query.
*******************************************************************************/

/*******************************************************************************
    Returns the version number of the SCT API. The version is a single number.
    There are no major/minor version numbers.

    Parameters:

        None.

    Returns:

        The version number.
*******************************************************************************/
static inline sx_uint32
sct_get_version()
{
    return SCT_API_VERSION;
}


/*******************************************************************************
    Callback function signature for sct_chan_connect_with_callback().
*******************************************************************************/
typedef void (sct_connect_callback_func)();


/*******************************************************************************
    GROUP: API access functions.
*******************************************************************************/

SX_EXTERN void sct_init();

SX_EXTERN sx_uint32 sct_init_nonblock();

SX_EXTERN sx_uint32 sct_chan_connect( sx_int32          port,
                                      sx_uint32         max_size,
                                      sct_channel_t   * chan_return );

SX_EXTERN sx_uint32 sct_chan_connect_with_callback( sx_int32                    port,
                                                    sx_uint32                   max_size,
                                                    sct_channel_t             * chan_return, 
                                                    sct_connect_callback_func * cb );

SX_EXTERN sx_uint32 sct_chan_connect_with_timeout( sx_int32          port,
                                                   sx_uint32         max_size,
                                                   sct_channel_t   * chan_return, 
                                                   sx_uint32         ms_timeout );

SX_EXTERN sx_uint32 sct_chan_connect_with_callback_and_timeout(
        sx_int32                    port,
        sx_uint32                   max_size,
        sct_channel_t            *  chan_return, 
        sct_connect_callback_func*  cb,
        sx_uint32                   ms_timeout );

SX_EXTERN sct_channel_t sct_chan_accept( sx_int32       port,
                                         sx_uint32    * max_size );

SX_EXTERN sct_channel_t sct_chan_accept_with_timeout( sx_int32       port,
                                                      sx_uint32    * max_size,
                                                      sx_uint32      ms_timeout );

SX_EXTERN sx_uint32 sct_chan_close( sct_channel_t   channel );

SX_EXTERN void * sct_chan_tx_getbuf( sct_channel_t   channel );

SX_EXTERN sx_uint32 sct_chan_tx_putbuf( sct_channel_t   channel,
                                        void          * buffer );

SX_EXTERN sx_uint32 sct_chan_tx_send( sct_channel_t     channel,
                                      void            * buffer,
                                      sx_uint32         size );

SX_EXTERN void * sct_chan_rx_recv( sct_channel_t     channel,
                                   sx_int32        * psize );

SX_EXTERN sct_channel_t sct_chan_rx_poll( sct_channel_t   * channels,
                                          sx_int32          n );

SX_EXTERN sx_uint32 sct_chan_rx_putbuf( sct_channel_t   channel,
                                        void          * buffer,
                                        sx_uint32       failed );

SX_EXTERN void sct_message_send( sx_uint8      msg_class,
                                 void        * buffer,
                                 sx_int32      size );

SX_EXTERN void sct_message_recv( sx_uint8      msg_class,
                                 void        * buffer,
                                 sx_uint8    * pmsg_class );

SX_EXTERN sx_bool sct_message_recv_poll( sx_uint8     msg_class,
                                         void       * buffer,
                                         sx_uint8   * pmsg_class );
SX_EXTERN int   sct_get_hostbuf_count();

#ifdef __cplusplus
}
#endif

#endif // __SCT_H__
