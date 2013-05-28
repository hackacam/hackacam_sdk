#pragma once
#ifndef _SBL_SOCKET_H
#define _SBL_SOCKET_H
/****************************************************************************\
*  Copyright C 2012 Stretch, Inc. All rights reserved. Stretch products are  *
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
\****************************************************************************/
#include <ostream>
#include <string>
#include <netinet/in.h>

/// @file sbl_socket.h

/// Stretch Base Library
namespace SBL {

//! Linux socket wrapper, to encapsulate all messy socket programming (both internet and unix).
/*! The objective is to hide all complex options and expose only the minimum functionality
    required for sockets.  
    The class holds only the socket itself, so it can be copied as an integer.
    All errors make the class throw an Exception (there are few calls
    that allow it with bool 'abort' argument), so it is best to
    enclose it in try/catch clauses:@verbatim
    try {
        do something with sockets
    } catch {Exception& ex) {
        std::cerr << ex.what() << "\n";
    } @endverbatim
*/
class Socket {
public:
    //! Protocol associated with the socket
    enum Proto {NONE    /*!< unused socket */,
                TCP     /*!< TCP socket    */,
                UDP     /*!< UDP socket    */,
                };
    //! buffer size for local_address() and _remote_address()
    enum {IP_ADDR_BUFF_SIZE  /*!< size of ip address array used in remote_address() and local_address() */ = INET_ADDRSTRLEN,
          MAC_ADDR_BUFF_SIZE /*!< sizeof mac address array used in get_mac_address */ = 18
          };
    //! Socket options
    enum Option {NO_DELAY       /*!< set TCP_NODELAY */,
                 CORK           /*!< set TCP_CORK */,
                 SEND_BUFF_SIZE /*!< set size of send buffer (if non-zero) */,
                 RECV_BUFF_SIZE /*!< set size of send buffer (if non-zero) */,
                 BROADCAST      /*!< set broadcast options */
                 };
    //! copy constructor
    Socket(const Socket& sock) : _sock(sock._sock) {}
    //! each Socket must be created with a protocol. 'local' is true for unix sockets
    explicit Socket(Proto proto, bool local = false) : _sock(-1) { open(proto, local); }
    //! open a socket 
    void open(Proto proto, bool local = false);
    //! do @b not close socket in the destructor; this would be a problem in threaded applications
    virtual ~Socket() {}
    //! explicit close, checks if it still open before closing
    void close();

    //! set Socket option
    Socket& set_option(Option option, int value);
    //! Binds socket to a local port. Returns self for chaining.
    Socket& bind (const short int port );   // return self
    //! Bind socket to any port.
    //! @param  try_port    pointer to function that returns a port to try.
    //! @details Tries to find any local port to bind to. It will
    //! call try_port repetitively until it finds a port to bind to.
    //! If try_port is NULL, it uses rand(). Throws if cannot bind after 100 iterations.
    Socket& bind(int (*try_port)() = 0);
    //! bind a local socket to a filename
    Socket& bind(const char* filename);
    //! Listen on a socket (must be TCP) for remote connections
    Socket& listen();
    //! Accept a connection request and return new socket to use for communication
    Socket accept();

    //! Connect to remote peer
    void connect(const char* ip_addr, const short int port );

    //! Connect to local server
    void connect(const char* filename);

    //! Data Transimission
    //! If buffer_size is 0, buffer must be zero-terminated char*
    //! If abort is false, returns false instead of throwing
    bool send(const void* buffer,  int buffer_size = -1, bool abort = true);

    //! send to an address, socket must be UDP
    //! If buffer_size is 0, buffer must be zero-terminated char*
    //! If abort is false, returns false instead of throwing
    bool send(const void* buffer, int buffer_size , const char* ip_addr, int port, bool abort = true);

    //! send to an local socket, socket must be UDP
    //! If buffer_size is 0, buffer must be zero-terminated char*
    //! If abort is false, returns false instead of throwing
    bool send(const void* buffer, int buffer_size , const char* filename, bool abort = true);

    //! returns actual number of bytes received (0 when peer disconnected)
    int  recv(void* buffer,  int buffer_size);

    //! Overloaded recv(), says from who data was received.
    //! @e from is filled with remote peer address in 192.168.1.101:2567 format
    int  recv(void* buffer,  int buffer_size, char* from_ip, int* from_port = 0);
    //! return socket protocol
    Proto proto() const;

    //! Return remote IP address (as xxx.xxx.xxx.xxx string) and port associated with this socket
    //! ip_addr buffers below @b must be at least IP_ADDR_BUFF_SIZE long
    int remote_address(char* ip_addr = NULL) const;

    //! Return local IP address (as xxx.xxx.xxx.xxx string) and port associated with this socket
    //! TCP socket must accept something, otherwise we get 0.0.0.0 (but port is fine)
    int local_address(char* ip_addr = NULL) const;

    //! test if socket is open
    bool is_valid()      const { return _sock >= 0; }
    //! sometimes, having an id (which really is just the socket number) is convienient
    unsigned int  id() const { return _sock & ~UNIX; }
    //! return true if this is a unix socket
    bool is_unix()    const { return _sock & UNIX; }
    //! needed so that we can put compare sockets
    bool operator==(const Socket& socket) const { return socket._sock == _sock; }
    //! prints out socket number
    friend std::ostream& operator<<(std::ostream&, const Socket&);
private:
    // we use the second topmost bit as marker for unix socket, so that it is still positive
    enum { MAXCONNECTIONS  = 10, UNIX = ~(~0u >> 1) >> 1 };
    int         _sock;
    Socket() {};

    void translate(Option option, int& level, int& optname);
    static const char* option_name[];
};

inline std::ostream& operator<<(std::ostream& str, const Socket& sock) {
    return str << sock.id();
}

}
#endif
