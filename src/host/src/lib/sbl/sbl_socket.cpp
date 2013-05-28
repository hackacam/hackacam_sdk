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
#include <iostream>
#include <cstring>
#include <cstdlib>

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include "sbl_socket.h"
#include "sbl_exception.h"
#include "sbl_logger.h"

#ifndef SBL_MSG_SOCKET
#undef SBL_MSG
#define SBL_MSG(...)
#endif

#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

namespace SBL {

void Socket::open(Proto proto, bool local) {
    if (proto == TCP || proto == UDP)
        SBL_PERROR( (_sock = ::socket(local ? AF_UNIX : AF_INET, proto == TCP ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0);
    if (local)
        _sock |= UNIX;
}

const char* Socket::option_name[] = {"TCP_NO_DELAY", "TCP_CORK", "SO_SNDBUF", "SO_RCVBUF", "SO_BROADCAST"};

void Socket::translate(Socket::Option option, int& level, int& optname) {
    switch (option) {
        case NO_DELAY: 
            SBL_THROW_IF(is_unix(), "unable to set option %s for local sockets", option_name[optname]);
            level = IPPROTO_TCP; optname = TCP_NODELAY; 
            break;
        case CORK:     
            SBL_THROW_IF(is_unix(), "unable to set option %s for local sockets", option_name[optname]);
#ifdef __APPLE__
            SBL_THROW("Illegal option CORK");
#else
            level = IPPROTO_TCP; optname = TCP_CORK;    
#endif
            break;
        case SEND_BUFF_SIZE: 
            level = SOL_SOCKET; optname = SO_SNDBUF; 
            break;
        case RECV_BUFF_SIZE: 
            level = SOL_SOCKET; optname = SO_RCVBUF; 
            break;
        case BROADCAST:
            level = SOL_SOCKET, optname = SO_BROADCAST;
            break;
        default: SBL_ASSERT(0);
    }
}

Socket& Socket::set_option(Option option, int value) {
    int level, optname;
    translate(option, level, optname);
    SBL_MSG(SBL_MSG_SOCKET, "Setting %s to %d", option_name[option], value);
    SBL_PERROR(::setsockopt(_sock, level, optname, &value, sizeof value) != 0);
    return *this;
}

Socket& Socket::bind(const short int port) {
    SBL_ASSERT(is_valid() && !is_unix());
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    SBL_PERROR(::bind(id(), (struct sockaddr *) &addr, sizeof(addr)) < 0);
    return *this;
}

Socket& Socket::bind(const char* filename) {
    SBL_ASSERT(is_valid() && is_unix());
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    SBL_ASSERT(strlen(filename) < sizeof(addr.sun_path));
    strcpy(addr.sun_path, filename);
    unlink(filename);
    int len = strlen(addr.sun_path) + sizeof(addr.sun_family) + 1;
    SBL_PERROR(::bind(id(), (struct sockaddr*) &addr, len) < 0);
    return *this;
}

Socket& Socket::bind(int (*try_port)()) {
    if (try_port == 0)
        try_port = &rand;
    SBL_ASSERT(is_valid());

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    int status = -1;
    for (int n = 0; n < 100 && status < 0; n++) {
        addr.sin_port = htons(try_port());
        status = ::bind(id(), (struct sockaddr *) &addr, sizeof(addr));
    }
    SBL_THROW_IF(status < 0, "unable to find a port to bind");
    return *this;
}

Socket& Socket::listen() {
    SBL_ASSERT(is_valid());
    SBL_PERROR(::listen(id(), MAXCONNECTIONS) < 0);
    return *this;
}

Socket Socket::accept() {
    SBL_ASSERT(is_valid());
    Socket socket;
    socket._sock = ::accept(id(), 0, 0) | (_sock & UNIX);
    SBL_PERROR(!socket.is_valid());
    return socket;
}

bool Socket::send(const void* buf, int len, bool abort) {
    if (!is_valid()) {
        SBL_WARN("Attempt to send thru closed socket");
        SBL_THROW_IF(!abort, "Attempt to send thru closed socket");
        return false;
    }
    SBL_MSG(SBL_MSG_SOCKET, "sending %d bytes, socket %d", len, id());
    if (len < 0)
        len = strlen((char*) buf) + 1;
    int n = 0;
    do {
        int sent_bytes = ::send(id(), buf, len, MSG_NOSIGNAL);
        if (sent_bytes < 0) {
            Exception ex(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Socket %d, send error: ", id());
            ex.perror();
            SBL_WARN(ex.what());
            if (abort)
                throw ex;
            return false;
        }
        len -= sent_bytes;
        buf = (char*) buf + sent_bytes;
        n++;
    } while (len > 0);
    SBL_MSG(SBL_MSG_SOCKET, "socket %d sent done, in %d chunks", id(), n);
    return true;
}

bool Socket::send(const void* buffer, int len, const char* ip_addr, int port, bool abort) {
    SBL_ASSERT(is_valid() && !is_unix());
    SBL_MSG(SBL_MSG_SOCKET, "sending %d bytes, socket %d to %s:%d", len, id(), ip_addr, port);
    if (len < 0)
        len = strlen((char*) buffer) + 1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    int status = ::inet_pton(AF_INET, ip_addr, &addr.sin_addr);
    if (status == 0)
        SBL_THROW("Invalid network address: %s:%d", ip_addr, port);
    SBL_PERROR(status != 1);
    int sent_bytes = ::sendto(id(), buffer, len, MSG_NOSIGNAL, (struct sockaddr*) &addr, sizeof(addr));
    if (sent_bytes != len) {
        Exception ex(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Socket %d, send error: ", id());
        ex.perror();
        if (abort)
            throw ex;
        return false;
    }
    return true;
}

bool Socket::send(const void* buffer, int len, const char* filename, bool abort) {
    SBL_ASSERT(is_valid() && is_unix());
    SBL_MSG(SBL_MSG_SOCKET, "sending %d bytes, socket %d to %s", len, id(), filename);
    if (len < 0)
        len = strlen((char*) buffer) + 1;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    SBL_ASSERT(strlen(filename) < sizeof(addr.sun_path));
    strcpy(addr.sun_path, filename);
    int addr_len = strlen(addr.sun_path) + sizeof(addr.sun_family) + 1;
    int sent_bytes = ::sendto(id(), buffer, len, MSG_NOSIGNAL, (struct sockaddr*) &addr, addr_len);
    if (sent_bytes != len) {
        Exception ex(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Socket %d, send error: ", id());
        ex.perror();
        if (abort)
            throw ex;
        return false;
    }
    return true;
}

int Socket::recv(void* buffer, int len) {
    SBL_ASSERT(is_valid());
    int recv = ::recv(id(), buffer, len, 0);
    SBL_MSG(SBL_MSG_SOCKET, "Received %d bytes, socket %d", recv, id());
    SBL_PERROR(recv < 0);
    return recv;
}

int Socket::recv(void* buffer, int len, char* from_ip, int* from_port) {
    SBL_ASSERT(is_valid() && !is_unix());
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof addr;
    int recv = ::recvfrom(id(), buffer, len, 0, (struct sockaddr*) &addr, &addr_len);
    SBL_PERROR(recv < 0);
    SBL_PERROR(::inet_ntop(AF_INET, &addr.sin_addr, from_ip, IP_ADDR_BUFF_SIZE) == NULL);
    if (from_port)
        *from_port = ntohs(addr.sin_port);
    return recv;
}

void Socket::connect(const char* ip_addr, const short int port) {
    SBL_ASSERT(is_valid() && !is_unix());

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    int status = ::inet_pton(AF_INET, ip_addr, &addr.sin_addr);
    if (status == 0)
        SBL_THROW("Invalid network address: %s:%d", ip_addr, port);
    SBL_PERROR(status != 1);
    SBL_PERROR(::connect(id(), (struct sockaddr*) &addr, sizeof(addr)) != 0);
}

void Socket::connect(const char* filename) {
    SBL_ASSERT(is_valid() && is_unix());
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    SBL_ASSERT(strlen(filename) < sizeof(addr.sun_path));
    strcpy(addr.sun_path, filename);
    int len = strlen(addr.sun_path) + sizeof(addr.sun_family);
    SBL_PERROR(::connect(id(), (struct sockaddr*) &addr, len) < 0);
}


void Socket::close() {
    if (is_valid())
        ::close(id());
    _sock = -1;
}

int Socket::remote_address(char* ip_addr) const {
    SBL_ASSERT(is_valid() && !is_unix());
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof addr;
    SBL_PERROR(::getpeername(id(), (struct sockaddr*) &addr, &addr_len) != 0);
    if (ip_addr)
        SBL_PERROR(::inet_ntop(AF_INET, &addr.sin_addr, ip_addr, IP_ADDR_BUFF_SIZE) == 0);
    return ntohs(addr.sin_port);
}

int Socket::local_address(char* ip_addr) const {
    SBL_ASSERT(is_valid() && !is_unix());
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof addr;
    SBL_PERROR(::getsockname(id(), (struct sockaddr*) &addr, &addr_len) != 0);
    if (ip_addr)
        SBL_PERROR(::inet_ntop(AF_INET, &addr.sin_addr, ip_addr, IP_ADDR_BUFF_SIZE) == 0);
    return ntohs(addr.sin_port);
}

Socket::Proto Socket::proto() const {
    SBL_ASSERT(is_valid());
    int so_type = -1;
    socklen_t optlen = sizeof so_type;
    SBL_PERROR(::getsockopt(id(), SOL_SOCKET, SO_TYPE, &so_type, &optlen) != 0);
    return so_type == SOCK_STREAM ? TCP :
           so_type == SOCK_DGRAM  ? UDP :
                                    NONE;
}


}// namespace SBL
