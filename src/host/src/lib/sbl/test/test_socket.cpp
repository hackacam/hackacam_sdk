#include <iostream>
#include <sstream>
#include <pthread.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <sbl_logger.h>
#include <sbl_socket.h>
#include <sbl_exception.h>
#include <sbl_test.h>

using namespace SBL;
using namespace std;

const char* loopback_addr = "127.0.0.1";
const int   loopback_port = 61234;

const char* socket_name = "/tmp/sbl_socket_test";
bool local = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* tcp_client(void*) {
    char send_buff[100];
    char recv_buff[100];
    Socket sock(Socket::TCP, local);
    int id = rand();
    if (local) {
        sock.connect(socket_name);
    } else {
        sock.connect(loopback_addr, loopback_port);
        char local_addr[Socket::IP_ADDR_BUFF_SIZE];
        int local_port = sock.local_address(local_addr);
        SBL_TEST_EQ_STR(local_addr, loopback_addr);
        char remote_addr[Socket::IP_ADDR_BUFF_SIZE];
        int remote_port = sock.remote_address(remote_addr);
        SBL_INFO("Client %x connected to %s:%d talking to %s:%d", id, local_addr, local_port, remote_addr, remote_port);
        SBL_TEST_EQ(remote_port, loopback_port);
        SBL_TEST_EQ_STR(remote_addr, loopback_addr);
    }
    for (int n = 0; n < 5; ++n) {
        int sent = snprintf(send_buff, sizeof send_buff, "%x %d", id, n) + 1;
        SBL_INFO("Client %x, sending %s", id, send_buff);
        sock.send(send_buff, sent);
        int received = sock.recv(recv_buff, sizeof recv_buff);
        SBL_TEST_EQ(received, sent);
        SBL_TEST_EQ_STR(send_buff, recv_buff);
    }
    sock.close();
    SBL_INFO("Client %x terminating\n", id);
    return NULL;
}

struct Context {
    Socket  socket;
    Context(Socket sock) : socket(sock) {}
};

void* server_echo(void* context) {
    char buffer[1000];
    Context* ctx = (Context*) context;
    Socket client = ctx->socket;
    if (!local) {
        char ip_addr[Socket::IP_ADDR_BUFF_SIZE];
        client.remote_address(ip_addr);
        SBL_TEST_EQ_STR(ip_addr, loopback_addr);
        int port = client.local_address(ip_addr);
        SBL_TEST_EQ_STR(ip_addr, loopback_addr);
        SBL_TEST_EQ(port, loopback_port);
    }
    do {
        if (client.recv(buffer, sizeof buffer ) == 0) {
            return NULL;
        }
        client.send(buffer);
    } while (1);
    return NULL;
}

void* tcp_server(void*) { 
    char buffer[1000];
    Socket server(Socket::TCP, local);
    if (local) {
        server.bind(socket_name);
        server.listen();
    } else {
        server.bind(loopback_port);
        server.listen();
        char ip_addr[Socket::IP_ADDR_BUFF_SIZE];
        int port = server.local_address(ip_addr);
        SBL_INFO("Server listening on %s:%d", ip_addr, port);
    }
    pthread_mutex_unlock(&mutex);
    while (1) {
        Socket client = server.accept();
        pthread_t* client_thread = new pthread_t;
        Context* context = new Context(client);
        pthread_create(client_thread, NULL, server_echo, context);
    }
    return NULL;
}

void* sender(void*) {
    Socket sender(Socket::UDP, local);
    if (local)
        sender.connect(socket_name);
    else
        sender.connect(loopback_addr, loopback_port);
    const int size = 100;
    int buffer[size];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < size; j++) {
            buffer[j] = i * size + j;
        }
        sender.send(buffer, sizeof buffer);
    }
    sender.close();
}

void* sender_to(void*) {
    Socket sender(Socket::UDP, local);
    const int size = 100;
    int buffer[size];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < size; j++) {
            buffer[j] = i * size + j;
        }
        if (local) 
            sender.send(buffer, sizeof buffer, socket_name);
        else
            sender.send(buffer, sizeof buffer, loopback_addr, loopback_port);
    }
    sender.close();
}

void* receiver(void*) {
    Socket receiver(Socket::UDP, local);
    if (local)
        receiver.bind(socket_name);
    else
        receiver.bind(loopback_port);
    const int size = 100;
    int buffer[size];
    pthread_mutex_unlock(&mutex);
    for (int i = 0; i < 10; i++) {
        int n = receiver.recv(buffer, sizeof buffer);
        SBL_TEST_EQ(n, sizeof buffer);
        for (int j = 0; j < size; j++) {
            SBL_TEST_EQ(buffer[j], i * size + j);
        }
    }
    receiver.close();
}

void* receiver_from(void*) {
    Socket receiver(Socket::UDP, local);
    if (local)
        receiver.bind(socket_name);
    else
        receiver.bind(loopback_port);
    const int size = 100;
    int buffer[size];
    pthread_mutex_unlock(&mutex);
    for (int i = 0; i < 10; i++) {
        int port = 0;
        int n;
        if (local) {
            n = receiver.recv(buffer, sizeof buffer);
        } else {
            char ip_addr[Socket::IP_ADDR_BUFF_SIZE];
            n = receiver.recv(buffer, sizeof buffer, ip_addr, &port);
            SBL_TEST_EQ_STR(ip_addr, loopback_addr);
            SBL_TEST_NE(port, 0);
        }
        SBL_TEST_EQ(n, sizeof buffer);
        for (int j = 0; j < size; j++) {
            SBL_TEST_EQ(buffer[j], i * size + j);
        }
    }
    receiver.close();
}

int try_port() {
    int port = rand();
    return port | 1024;
}

void test_bind_any_port() {
    Socket socket(Socket::TCP);
    socket.bind(try_port);
}


void test() {
    const int THREAD_COUNT = 5;
    pthread_t server;
    pthread_t client[THREAD_COUNT];
    pthread_mutex_lock(&mutex);
    pthread_create(&server, NULL, tcp_server, NULL);
    pthread_mutex_lock(&mutex); // will wait here until server is ready
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&client[i], NULL, tcp_client, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(client[i], NULL);
    }
    pthread_mutex_lock(&mutex);
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_mutex_lock(&mutex); // will wait here until server is ready
    pthread_mutex_unlock(&mutex);
    pthread_t sender_thread;
    pthread_create(&sender_thread, NULL, sender, NULL);
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);

    pthread_mutex_lock(&mutex);
    pthread_create(&receiver_thread, NULL, receiver_from, NULL);
    pthread_mutex_lock(&mutex); // will wait here until server is ready
    pthread_mutex_unlock(&mutex);
    pthread_create(&sender_thread, NULL, sender_to, NULL);
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);

    test_bind_any_port();
}

int main(int argc, char* argv[]) {
    Exception::enable_backtrace(true);
    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        Log::set_verbosity(4);
    test();
    local = true;
    test();
    unlink(socket_name);
    cout << argv[0] << " passed.\n";
    return 0;
}
