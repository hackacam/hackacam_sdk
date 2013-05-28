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
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/route.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "sbl_exception.h"
#include "sbl_net.h"

namespace SBL {

const int Net::IF_NAME_BUFF_SIZE = IFNAMSIZ;

void Net::mac_address(char* mac_address, const char* ifname) {
    struct ifreq ifr;
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    bool err = fd < 0;
    ifr.ifr_addr.sa_family = AF_INET;
    ::strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (!err) {
        err = (::ioctl(fd, SIOCGIFHWADDR, &ifr) < 0);
        ::close(fd);
    }
    SBL_PERROR(err);
    SBL_ASSERT(snprintf(mac_address, MAC_ADDR_BUFF_SIZE,
          "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
         (unsigned char)ifr.ifr_hwaddr.sa_data[0],
         (unsigned char)ifr.ifr_hwaddr.sa_data[1],
         (unsigned char)ifr.ifr_hwaddr.sa_data[2],
         (unsigned char)ifr.ifr_hwaddr.sa_data[3],
         (unsigned char)ifr.ifr_hwaddr.sa_data[4],
         (unsigned char)ifr.ifr_hwaddr.sa_data[5]) == MAC_ADDR_BUFF_SIZE - 1);
}

void Net::ip_address(char* ip_address, const char* ifname, char* real_name) {
    const unsigned int MAX_IF = 16;
    struct ifreq  ifreq[MAX_IF];
    struct ifconf ifconf;
    ifconf.ifc_buf = (char*) &ifreq;
    ifconf.ifc_len = MAX_IF * sizeof(ifreq);
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    SBL_PERROR(::ioctl(fd, SIOCGIFCONF, &ifconf) < 0);
    for (unsigned int n = 0; n < ifconf.ifc_len / sizeof(struct ifreq); n++) {
        // aliases such as eth0:0 should compare to eth0
        const char* colon = strchr(ifreq[n].ifr_name, ':');
        size_t len = colon ? colon - ifreq[n].ifr_name : strlen(ifreq[n].ifr_name);
        if (strncmp(ifname, ifreq[n].ifr_name, len) == 0) {
            struct sockaddr_in* address = (struct sockaddr_in*) &ifreq[n].ifr_addr;
            ::close(fd);
            SBL_PERROR(inet_ntop(AF_INET, &address->sin_addr, ip_address, IP_ADDR_BUFF_SIZE) == 0);
            if (real_name)
                strcpy(real_name, ifreq[n].ifr_name);
            return;
        }
    }
    SBL_THROW("Unable to find interface %s", ifname);
}

void Net::set_ip_address(const char* ip_address, const char* ifname) {
    int sockfd = -1;
    struct ifreq ifr;
    struct sockaddr_in sin;

    SBL_PERROR((sockfd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0);

    // get interface name
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    try {
        // Read interface flags
        SBL_PERROR(::ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0);

        /*
        * Expected in <net/if.h> according to
        * "UNIX Network Programming".
        */
        #ifdef ifr_flags
        # define IRFFLAGS       ifr_flags
        #else   /* Present on kFreeBSD */
        # define IRFFLAGS       ifr_flagshigh
        #endif


        // If interface is down, bring it up
        if (ifr.IRFFLAGS | ~(IFF_UP)) {
            // Device is currently down..setting up...
            ifr.IRFFLAGS |= IFF_UP;
            SBL_PERROR(::ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0);
        }

        sin.sin_family = AF_INET;

        // Convert IP from numbers and dots to binary notation
        inet_aton(ip_address, &sin.sin_addr);
        memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

        // Set interface address
        SBL_PERROR(::ioctl(sockfd, SIOCSIFADDR, &ifr) < 0);

        ::close(sockfd);
        #undef IRFFLAGS

    }  catch (SBL::Exception& ex) {
        if (sockfd != -1)
            ::close(sockfd);
        SBL_PERROR(true);
    }

 }

void  Net::subnet(char *subnet_address, const char* ifname) {
    int sock_fd = -1;
    struct ifreq ifr;

    sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    try {
        // Want an IPv4 netmask
        ifr.ifr_addr.sa_family = AF_INET;

        // Want netmask attached to given the interface name
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

        // SIOCGIFNETMASK gets the interface subnet mask.
        // On success return ifr.ifr_addr contains the subnet mask address
        SBL_PERROR(::ioctl(sock_fd, SIOCGIFNETMASK, &ifr) < 0);
        ::close(sock_fd);
        sock_fd = -1;

        struct sockaddr_in* address = (struct sockaddr_in*) &ifr.ifr_addr;
        SBL_PERROR(inet_ntop(AF_INET, &address->sin_addr, subnet_address, IP_ADDR_BUFF_SIZE) == 0);

    }  catch (SBL::Exception& ex) {
        if (sock_fd != -1)
            ::close(sock_fd);
        throw;
    }
}

void  Net::set_subnet(const char *subnet_address, const char* ifname) {
    struct ifreq ifr;
    struct sockaddr_in sin;

    int sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    SBL_PERROR(sock_fd < 0);
    // Request to set the subnet mask
    // SIOCSIFNETMASK sets the interface netmask specified in the ifr.ifr_addr field.
    sin.sin_family = AF_INET;

    // Convert IP from numbers and dots to binary notation
    inet_aton(subnet_address, &sin.sin_addr);
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

    // Want an IPv4 netmask
    ifr.ifr_addr.sa_family = AF_INET;

    // Want netmask attached to given the interface name
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    int status = ::ioctl(sock_fd, SIOCSIFNETMASK, &ifr);
    ::close(sock_fd);

    SBL_PERROR(status < 0);
}

void Net::set_gateway_address(const char *default_gateway) {
   // create the socket
    int sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    SBL_PERROR(sock_fd < 0);

    struct rtentry route;
    memset(&route, 0, sizeof(route));

    struct sockaddr_in ip_gateway ;// Gateway IP address
    memset(&ip_gateway, 0, sizeof(ip_gateway));
    ip_gateway.sin_family = AF_INET;
    ip_gateway.sin_addr.s_addr = inet_addr(default_gateway);
    ip_gateway.sin_port = 0;

/* This is workaround code because of strict aliasing rules
   see original code below
*/
    struct sockaddr_in empty;
    memset(&empty, 0, sizeof empty);
    empty.sin_family = AF_INET;
    memcpy(&route.rt_dst, &empty, sizeof empty);
    memcpy(&route.rt_genmask, &empty, sizeof empty);
/* 
    This original code doesn't compile with -O3 because of strict-aliasing rules
    (( struct sockaddr_in*)&route.rt_dst)->sin_family = AF_INET;
    (( struct sockaddr_in*)&route.rt_dst)->sin_addr.s_addr = 0;
    (( struct sockaddr_in*)&route.rt_dst)->sin_port = 0;

    (( struct sockaddr_in*)&route.rt_genmask)->sin_family = AF_INET;
    (( struct sockaddr_in*)&route.rt_genmask)->sin_addr.s_addr = 0;
    (( struct sockaddr_in*)&route.rt_genmask)->sin_port = 0;
*/
    memcpy((void *) &route.rt_gateway, &ip_gateway, sizeof(ip_gateway));
    route.rt_flags = RTF_UP | RTF_GATEWAY;

    int status = ::ioctl(sock_fd, SIOCADDRT, &route);
    ::close(sock_fd);
    SBL_PERROR(status < 0);
}

// This function is needed for Net::gateway(), it received kernel messages with route info
static int read_nl_socket(int sock, char* buffer, unsigned int seq_num, unsigned int pid, int buff_size) {
    struct nlmsghdr* nl_hdr;
    int msg_len  = 0;

    do {
        /* Receive response from the kernel */
        int read_len = recv(sock, buffer, buff_size - msg_len, 0);
        SBL_PERROR(read_len < 0);

        nl_hdr = (struct nlmsghdr *) buffer;

        /* Check if the header is valid */
        SBL_PERROR((NLMSG_OK(nl_hdr, (unsigned int) read_len) == 0) || (nl_hdr->nlmsg_type == NLMSG_ERROR));
        /* Check if the its the last message */
        if (nl_hdr->nlmsg_type == NLMSG_DONE) {
            break;
        } else {
            /* Else move the pointer to buffer appropriately */
            buffer  += read_len;
            msg_len += read_len;
        }

        /* Check if its a multi part message */
        if ((nl_hdr->nlmsg_flags & NLM_F_MULTI) == 0) {
           /* return if its not */
            break;
        }
    } while ((nl_hdr->nlmsg_seq != seq_num) || (nl_hdr->nlmsg_pid != pid));
    return msg_len;
}

struct RouteInfo {
    struct in_addr dst_addr;
    struct in_addr src_addr;
    struct in_addr gateway;
    char if_name[IF_NAMESIZE];
};

/* For parsing the route info returned */
static void parse_route(nlmsghdr* nl_hdr, RouteInfo* info, char* gateway) {
    struct rtmsg* rt_msg = (struct rtmsg *) NLMSG_DATA(nl_hdr);
    /* If the route is not for AF_INET or does not belong to main routing table
        then return. */
    if ((rt_msg->rtm_family != AF_INET) || (rt_msg->rtm_table != RT_TABLE_MAIN))
        return;

    /* get the rtattr field */
    struct rtattr* rt_attr = (struct rtattr *) RTM_RTA(rt_msg);
    int rt_len = RTM_PAYLOAD(nl_hdr);
    for (; RTA_OK(rt_attr, rt_len); rt_attr = RTA_NEXT(rt_attr, rt_len)) {
        switch (rt_attr->rta_type) {
        case RTA_OIF:
            if_indextoname(*(int *) RTA_DATA(rt_attr), info->if_name);
            break;
        case RTA_GATEWAY:
            info->gateway.s_addr= *(u_int *) RTA_DATA(rt_attr);
            break;
        case RTA_PREFSRC:
            info->src_addr.s_addr= *(u_int *) RTA_DATA(rt_attr);
            break;
        case RTA_DST:
            info->dst_addr.s_addr= *(u_int *) RTA_DATA(rt_attr);
            break;
        }
    }
    if (info->dst_addr.s_addr == 0)
        strcpy(gateway, (char *) inet_ntoa(info->gateway));
}


void Net::gateway_address(char* gateway, const char* /* if_name */) {
    /* Create socket */
    int sock = ::socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    try {
        SBL_PERROR(sock < 0);

        char buffer[8192];
        memset(buffer, 0, sizeof buffer);

    /* point the header and the msg structure pointers into the buffer */
        struct nlmsghdr* nl_msg = (struct nlmsghdr *) buffer;

    /* Fill in the nlmsg header*/
        nl_msg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));  // Length of message.
        nl_msg->nlmsg_type = RTM_GETROUTE;   // Get the routes from kernel routing table .

        nl_msg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;    // The message is a request for dump.
        nl_msg->nlmsg_seq = 0;    // Sequence of the message packet.
        nl_msg->nlmsg_pid = getpid();    // PID of process sending the request.

        /* Send the request */
        SBL_PERROR(::send(sock, nl_msg, nl_msg->nlmsg_len, 0) < 0);
        /* Read the response */
        int len = read_nl_socket(sock, buffer, 1, getpid(), sizeof buffer);
        /* Parse and print the response */
        struct RouteInfo info;
        for (; NLMSG_OK(nl_msg, (unsigned int) len); nl_msg = NLMSG_NEXT(nl_msg, len)) {
            memset(&info, 0, sizeof(struct RouteInfo));
            parse_route(nl_msg, &info, gateway);
        }
        ::close(sock);
    } catch (SBL::Exception& ex) {
        ::close(sock);
        throw;
    }
}

bool Net::verify_network_address(const char* ip_address, const char* subnet, const char* gateway) {
    struct in_addr ip, subn, gtw;
    if (!inet_aton(ip_address, &ip)
     || !inet_aton(subnet, &subn)
     || !inet_aton(gateway, &gtw))
        return false;
    return (ip.s_addr & subn.s_addr) == (gtw.s_addr & subn.s_addr);
}


}
