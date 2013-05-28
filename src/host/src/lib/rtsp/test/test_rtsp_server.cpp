#include <iostream>
#include <cstring>
#include "server.h"

using namespace std;

Server::MsgType Server::receive_msg() {
    if (!receive()) 
        return MSG_RESET;
    if (_rx_buffer[0] == '$')
        return receive_rtcp();
    return receive_rtsp();
}

Server::MsgType Server::receive_rtcp() {
    while (_rx_bytes < 4) {
        if (!receive())
            return MSG_RESET;
    }
//  RTSP_ASSERT(_rx_buffer[1] == 1, BAD_REQUEST);
    _msg_size = (((_rx_buffer[2] & 0x00ff) << 8) | (_rx_buffer[3] & 0x00ff)) + 4;
    while (_rx_bytes < _msg_size) {
        if (!receive())
            return MSG_RESET;
    }
    return MSG_RTCP;
}

Server::MsgType Server::receive_rtsp() {
    const char eom[] = "\r\n\r\n";
    const int sizeof_eom = sizeof(eom) - 1;
    int ptr = 0;
    do {
        for (; ptr <= _rx_bytes - sizeof_eom; ptr++)
            if (!memcmp(_rx_buffer + ptr, eom, sizeof_eom)) {
                _msg_size = ptr + sizeof_eom;
                return MSG_RTSP;
            }
        if (!receive())
            return MSG_RESET;
    } while (1);
}

int Server::receive() {
    int size = _socket.recv(_rx_buffer + _rx_bytes, BUFFER_SIZE  - _rx_bytes);
    _rx_bytes += size; 
    return size;
}

void Server::start_thread() {
    MsgType msg_type;
    do {
        msg_type = receive_msg();
        if (_msg_size == 0)
            break;
        if (msg_type == MSG_RTSP) {
            cout << "RTSP " << _msg_size << " " << _rx_bytes << "\n";
            reply_rtsp();
        } else if (msg_type == MSG_RTCP) {
            cout << "RTCP " << _msg_size << " " << _rx_bytes << "\n";
            reply_rtcp();
        }
        _rx_bytes -= _msg_size;
        if (_rx_bytes)
            memmove(_rx_buffer, _rx_buffer + _msg_size, _rx_bytes);
    } while (msg_type != MSG_RESET);
}


Socket::Socket() : _size(0), _ptr(0), _msg_count(0), _msg_index(0) {
    for (int n = 0; n < 100; n++) {
        if (rand() & 1)
            create_rtcp();
        else
            create_rtsp();
    }
    cout << "Generated " << _msg_size.size() << " messages\n";
}


void Socket::create_rtcp() {
    int n = random_size();
    if (_size + n + 4 >= BUFFER_SIZE)
        return;
    int msg_start = _size;
    _buffer[_size++] = '$';
    _buffer[_size++] = 1;
    _buffer[_size++] = n >> 8;
    _buffer[_size++] = n;
    for (int i = 0; i < n; i++)
        _buffer[_size++] = rand();
    cout << "RTCP " << _msg_size.size() << " " << n + 4 << "\n";
    _msg_size.push_back(n + 4);
    _msg_start.push_back(msg_start);
}

void Socket::create_rtsp() {
    int n = random_size();
    if (_size + n + 4 >= BUFFER_SIZE)
        return;
    int msg_start = _size;
    for (int i = 0; i < n; i++)
        _buffer[_size++] = 'A' + (i % 26);
    strcpy(_buffer + _size, "\r\n\r\n");
    _size += 4;
    cout << "RTSP " << _msg_size.size() << " " << n << "\n";
    _msg_size.push_back(n + 4);
    _msg_start.push_back(msg_start);
}

int Socket::random_size() {
    do {
        int n = rand() % 200;
        if (n > 6)
            return n;
    } while (1);
}

int Socket::recv(char* buffer, int buffer_size) {
    if (_msg_count < 10) {
        cout << "Msg " << _msg_count << " size " << _msg_size[_msg_count] <<  "\n";
        if (_ptr + _msg_size[_msg_count] > _size)
            return 0;
        memcpy(buffer, _buffer + _ptr, _msg_size[_msg_count]);
        _ptr += _msg_size[_msg_count];
        return _msg_size[_msg_count++];
    }
    int n = random_size();
    if (_ptr + n > _size)
        return 0;
    memcpy(buffer, _buffer + _ptr, n);
    _ptr += n;
    return n;
}


void Server::reply_rtsp() {
    reply("RTSP");
}

void Server::reply_rtcp() {
    reply("RTCP");
}

void Server::reply(const char* what) {
    int size = _socket.msg_size();
    const char* content = _socket.msg_content();
    if (size != _msg_size) {
        cout << what << " size error: msg " << _socket.msg_index()
             << " is " << size << " should be " << _msg_size << "\n";
        exit(0);
    }
    if (memcmp(_rx_buffer, content, _msg_size)) {
        cout << what << " content error for message " << _socket.msg_index() << "\n";
        exit(0);
    }
    _socket.next_msg();
}


int main() {
    Socket socket;
    Server server(socket);
    server.start_thread();
    cout << "\nAll tests passed\n";
    return 0;
}
