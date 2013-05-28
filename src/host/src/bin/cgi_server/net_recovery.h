#pragma once 
#ifndef _NET_RECOVERY_H
#define _NET_RECOVERY_H
/****************************************************************************\
*  Copyright C 2013 Stretch, Inc. All rights reserved. Stretch products are  *
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

#include <string>
#include <sbl/sbl_thread.h>
#include <sbl/sbl_socket.h>

namespace CGI {
class Server;

// Manage discovery and recovery via broadcast messages
class NetRecovery: public SBL::Thread {
public:
    NetRecovery(Server* server, const std::string& param);
    void set_mac_address(const std::string& mac_address);
    bool is_enabled();
private:
    Server*      _server;
    std::string  _secret;        // magic bytes to receive on the first line
    std::string  _bcast_address; // broadcast address to send the reply to
    int          _udp_port;      // port to listen on (will reply on udp_port + 1)
    std::string  _mac_address;   // our mac address

    std::string  parse_command(char* arg, int arg_size); // parse received command
    std::string  get_line(int& start, char* arg, int arg_size); // get next line
    void         start_thread();        
    static       std::string tolower(const std::string& str);
};

}

#endif
