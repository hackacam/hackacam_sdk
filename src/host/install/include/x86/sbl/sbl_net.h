#pragma once
#ifndef _SBL_NET_H
#define _SBL_NET_H
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

/// @file sbl_net.h

/// Stretch Base Library
namespace SBL {

/// Wraps various network utilities
class Net {
public:
    /// size of a IP address buffer
    static const int IP_ADDR_BUFF_SIZE  = 16;
    /// size of a mac address buffer
    static const int MAC_ADDR_BUFF_SIZE = 18;
    /// size of a interface name
    static const int IF_NAME_BUFF_SIZE;
    //! get default gateway address
    static void gateway_address(char* gateway, const char* ifname = "eth0");    
    //! get MAC addresses of the interface
    //! @param mac_address  output mac_address (must be MAC_ADDR_BUFF_SIZE characters or more)
    //! @param ifname   interface name ("eth0")
    static void mac_address(char* mac_address, const char* ifname = "eth0");
    //! get IP address of the local computer.
    /** @param ip_address   output ip_address (must be IP_ADDR_BUFF_SIZE characters or more)
        @param ifname       interface name. It is defaulted to "eth0"
        @param real_name    if not 0, return real interface name (ex. eth0:0)
        This is the same name as local_address, but doesn't need a connected socket */
    static void ip_address(char* ip_address, const char* ifname = "eth0", char* real_name = NULL);
    //! set IP address of the local computer.
    //! @param ip_address   the new ip-address to set for the given interface name
    //! @param ifname        interface name. It is defaulted to "eth0"
    static void set_ip_address(const char* ip_address, const char* ifname = "eth0");
    //! get the subnet of the local computer.
    //! @param subnet_address  output subnet IP address (must be IP_ADDR_BUFF_SIZE characters or more)
    //! @param ifname       interface name. It is defaulted to "eth0"
    static void subnet(char *subnet_address, const char* ifname = "eth0");
    //! set the subnet of the local computer.
    //! @param subnet_address  subnet IP address to set
    //! @param ifname       interface name. It is defaulted to "eth0"
    static void set_subnet(const char *subnet_address, const char* ifname  = "eth0");
    //! set the default gateway IP address of the local computer.
    //! @param default_gateway   the new default gateway IP address to set for the given interface name
    static void set_gateway_address(const char *default_gateway);
    //! verify network address
    //! @return true if all three addresses are valid and gateway matches ip_address
    static bool verify_network_address(const char* ip_address, const char* subnet, const char* gateway);
};

}

#endif
