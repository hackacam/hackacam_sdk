#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "st_exception.h"
#include "st_socket.h"
#include "st_thread.h"

using namespace std;

#define PRINT(fmt, ...)             \
    if (!quiet) {                   \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout);             \
    }


const char server_addr[] = "192.168.1.104";
const short int server_port = 13456;

static bool quiet   = false;

struct TCPClient : public RTSP::Thread {
    TCPClient(int repeat_count = 1, int sleep_time = 0) :
                  _repeat_count(repeat_count), _sleep_time(sleep_time) {}
    int     _repeat_count;
    int     _sleep_time;
    void start_thread() {
        const size_t buffer_size = 30;
        char recv_buffer[buffer_size];
        char send_buffer[buffer_size];
        int id = rand() % 1000;
        RTSP::Socket sock(RTSP::Socket::TCP);
        sock.connect(server_addr, server_port);
        PRINT("Client %d connected to server\n", id);
        for (int n = 0; n < _repeat_count; ++n) {
            sprintf(send_buffer, "Client %d, message %d", id, n); 
            int sent_bytes = strlen(send_buffer) + 1;
            sock.send(send_buffer, sent_bytes);
            PRINT("Client %d sent % d bytes: %s\n", id, sent_bytes, send_buffer);
            int received_bytes = sock.recv(&recv_buffer[0], buffer_size);
            if (received_bytes != sent_bytes) {
                PRINT("Error: sent %d bytes, received %d bytes", sent_bytes, received_bytes);
                exit(2);
            }
            if (recv_buffer[received_bytes - 1]) {
                PRINT("Error: received message is not zero-terminated");
                exit(2);
            }
            if (strcmp(send_buffer, recv_buffer)) {
                PRINT("Error: sent %s, received %s", send_buffer, recv_buffer);
                exit(2);
            }
            PRINT("Client %d received %d bytes: %s\n", id, received_bytes, &recv_buffer[0]);
            if (_sleep_time)
                sleep(_sleep_time);
        }
        PRINT("Client %d terminated\n", id);
        sock.close();
        delete this;
    }
};

void tcp_client(int client_count, int repeat_count, int sleep_time) {
    vector<TCPClient*> clients;
    for (int n = 0; n < client_count; n++) {
        TCPClient* client = new TCPClient(repeat_count, sleep_time);
        client->create_thread();
        clients.push_back(client);
    }
    for (unsigned int n = 0; n < clients.size(); n++) {
        clients[n]->join_thread();
        PRINT("Client %d finished\n", n);
    }
}

class TCPServer : public RTSP::Thread {
public:
    TCPServer(const short port) : _socket(RTSP::Socket::TCP) {
        _socket.bind(port).listen();
        do {
            RTSP::Socket client_socket(_socket.accept());
            Thread* client_thread = new TCPServer(client_socket);
            client_thread->create_thread(Thread::Detached);
        } while (1);
    }
    
    virtual ~TCPServer() {}

    void start_thread() {
        const size_t BUFFER_SIZE = 30;
        char buffer[BUFFER_SIZE];
        PRINT("Entering new thread on socket %d\n",  _socket.id());
        do {
            memset(&buffer[0], 'x', BUFFER_SIZE);
            int received_bytes = 0;
            try {
                received_bytes = _socket.recv(&buffer[0], BUFFER_SIZE);
            } catch (RTSP::Exception& ex) {
                printf("Exiting thread on socket %d from catch: %d, %s\n", _socket.id(), ex.get_errno(), ex.what());
                _socket.close();
                delete this;
                return;
            }
            if (received_bytes == 0) {
                PRINT("Exiting thread on socket %d, client terminated\n", _socket.id());
                _socket.close();
                delete this;
                return;
            }
            PRINT("Thread %d received %d bytes: %s\n", _socket.id(), received_bytes, &buffer[0]);
            _socket.send(buffer);
        } while (1);
    }
private:
    TCPServer(RTSP::Socket socket) : _socket(socket) {}
    RTSP::Socket  _socket;
};

void tcp_server() {
    cout << "Starting server listening to port " << server_port << endl;
    TCPServer server(server_port);
}

static const char* usage =
    "Test TCP server\n"
    "Usage: test_tcp_server [-c] [-q] [-s] [-h]\n"
    "   -c      : execute client side (by default it is server\n"
    "   -q      : very few messages\n"
    "   -s      : stress test\n"
    "   -h      : print this message\n";

int main(int argc, char* argv[]) {
    bool client = false;
    bool stress = false;
    int c;
    while ((c = getopt(argc, argv, "cqsh")) != -1) 
        switch (c) {
            case 'c':   client = true; break;
            case 'q':   quiet  = true; break;
            case 's':   stress = true; break;
            case 'h':   cout << usage; return 0;
        }

    try {
        if (client) {
            if (stress)
                tcp_client(50, 10000, 0);
            else
                tcp_client(4, 10, 1);
        } else {
            tcp_server();
        }
    } catch (RTSP::Exception& ex) {
        cerr << ex.what() << endl;
        return 1;
    }
}
