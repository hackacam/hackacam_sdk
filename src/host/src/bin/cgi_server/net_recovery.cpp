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
#include <cctype>
#include <algorithm>
#include <sstream>
#include "net_recovery.h"
#include "gateway.h"
#include "cgi_server.h"

using namespace std;

namespace CGI {
  
NetRecovery::NetRecovery(Server* server, const string& param): 
    _server(server), _bcast_address("255.255.255.255"), _udp_port(8000) 
{// param has the form 'magic_bytes bcast_address port'
    stringstream str(param);
    str >> _secret >> _bcast_address >> _udp_port;
}

string NetRecovery::tolower(const string& str) {
    string result(str);
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool NetRecovery::is_enabled() {
    return _secret.length() > 0;
}

void NetRecovery::set_mac_address(const string& mac_address) {
    _mac_address = tolower(mac_address);
}

void NetRecovery::start_thread() {
    if (!is_enabled())
        return;
    try {
        Socket rx_socket(Socket::UDP);
        Socket tx_socket(Socket::UDP);
        rx_socket.bind(_udp_port);
        tx_socket.set_option(Socket::BROADCAST, 1);
        SBL_INFO("waiting for broadcast message on port %d, magic bytes are '%s', replying to %s:%d", 
                _udp_port, _secret.c_str(), _bcast_address.c_str(), _udp_port + 1);
        do {    
            const int BUFFER_SIZE = 1024;
            char buffer[BUFFER_SIZE];
            char src_address[SBL::Socket::IP_ADDR_BUFF_SIZE];
            int rx_size = rx_socket.recv(buffer, BUFFER_SIZE - 1, src_address);
            buffer[rx_size] = '\0';
            SBL_INFO("received broadcast message:\n%s", buffer);
            string command = parse_command(buffer, rx_size);
            if (command.length() > 0) {
                string parameters = Server::split_line(command);
                SBL_INFO("got command \"%s\" from %s with parameter \"%s\"", command.c_str(), src_address, parameters.c_str());
                _server->lock();
                _server->process(command.c_str(), parameters.c_str(), Gateway::GET, true);
                const char* content = _server->reply();
                unsigned int content_size = _server->content_size();
                int size = _secret.length() + 2 + content_size;
                if (size < BUFFER_SIZE) {
                    memcpy(buffer, _secret.c_str(), _secret.length());
                    memcpy(buffer + _secret.length(), "\r\n", 2);
                    memcpy(buffer + _secret.length() + 2, content, content_size);
                    tx_socket.send(buffer, size, _bcast_address.c_str(), _udp_port + 1);            
                } else
                    SBL_ERROR("Buffer overflow");

                if (_server->content_type() == Gateway::JPEG || _server->content_type() == Gateway::RAW)
                    _server->release_buffer();
                _server->unlock();
            }
        } while (true);
    } catch (SBL::Exception& ex) {
        SBL_ERROR("Net recovery exiting, caught error: %s", ex.what());
    }
}

// get next line from input buffer, lines terminate with \r\n
string NetRecovery::get_line(int& start, char* arg, int arg_size) {
    for (int i = start; i < arg_size - 1; i++){
        if (arg[i] == '\r' && arg[i + 1] == '\n'){
            arg[i] = 0;
            string segment = &arg[start];
            start = i + 2;
            return segment;
        }
    }
    start = 0;
    return "";
}

// return the command embedded in buffer, or "" if not for us
string NetRecovery::parse_command(char* arg, int arg_size) {
    int start = 0;
    string secret = get_line(start, arg, arg_size);
    if (secret != _secret)
        return "";

    string mac_address = tolower(get_line(start, arg, arg_size));
    string command     = get_line(start, arg, arg_size);
    if (mac_address == _mac_address) {
        return command;
    } else if (mac_address.length() == 0 && command.length() == 0) {
        return "device_info?action=get";
    }
    return "";
}

}
