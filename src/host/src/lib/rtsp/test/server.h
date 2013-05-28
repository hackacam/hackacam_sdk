#include <vector>

using namespace std;

class Socket {
public:
    Socket();
    int recv(char* buffer, int buffer_size);
    int msg_size() const { return _msg_size.at(_msg_index); }
    int msg_index() const { return _msg_index; }
    const char* msg_content() const { return _buffer + _msg_start.at(_msg_index); }
    void next_msg() { _msg_index++; }
private:
    void create_rtcp();
    void create_rtsp();
    int  random_size();

    enum {BUFFER_SIZE = 10 * 1024};
    char _buffer[BUFFER_SIZE];
    int  _size;
    int  _ptr;
    int  _msg_count;
    int  _msg_index;
    vector<int> _msg_size;
    vector<int> _msg_start;
};

class Server {
public:
    Server(Socket& socket) : _rx_bytes(0), _msg_size(0), _socket(socket) {}
    void start_thread();

    void reply(const char*);
private:
    enum    MsgType { MSG_RESET, MSG_RTSP, MSG_RTCP};
    void    reply_rtsp();
    void    reply_rtcp();
    MsgType receive_msg();
    MsgType receive_rtsp();
    MsgType receive_rtcp();
    int     receive();

    enum    {BUFFER_SIZE = 1024};
    char    _rx_buffer[BUFFER_SIZE];
    int     _rx_bytes;
    int     _msg_size;
    Socket& _socket;
};

