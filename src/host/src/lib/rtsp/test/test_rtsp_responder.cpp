#include "rtsp_responder.h"
#include "rtsp_server.h"
#include "stream_source.h"
#include "rtp_streamer.h"
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

static char base64DecodeTable[256];

static void initBase64DecodeTable() 
{
    int i;
    for (i = 0; i < 256; ++i) base64DecodeTable[i] = (char)0x80;
    // default value: invalid

    for (i = 'A'; i <= 'Z'; ++i) base64DecodeTable[i] = 0 + (i - 'A');
    for (i = 'a'; i <= 'z'; ++i) base64DecodeTable[i] = 26 + (i - 'a');
    for (i = '0'; i <= '9'; ++i) base64DecodeTable[i] = 52 + (i - '0');
    base64DecodeTable[(unsigned char)'+'] = 62;
    base64DecodeTable[(unsigned char)'/'] = 63;
    base64DecodeTable[(unsigned char)'='] = 0;
}

int base64decode(const char* in, uint8_t* out) 
{
    static bool haveInitedBase64DecodeTable = false;
    if (!haveInitedBase64DecodeTable) 
    {
        initBase64DecodeTable();
        haveInitedBase64DecodeTable = true;
    }
    int k = 0;
    int const jMax = strlen((char*) in) - 3;
    // in case "in" is not a multiple of 4 bytes (although it should be)
    for (int j = 0; j < jMax; j += 4) 
    {
        char inTmp[4], outTmp[4];
        for (int i = 0; i < 4; ++i) 
        {
            inTmp[i] = in[i+j];
            outTmp[i] = base64DecodeTable[(unsigned char)inTmp[i]];
            if ((outTmp[i]&0x80) != 0) outTmp[i] = 0; // pretend the input was 'A'
        }

        out[k++] = (outTmp[0]<<2) | (outTmp[1]>>4);
        out[k++] = (outTmp[1]<<4) | (outTmp[2]>>2);
        out[k++] = (outTmp[2]<<6) | outTmp[3];
    }
    while (k > 0 && out[k-1] == 0) 
        --k;
    return k;
}

namespace RTSP {

    struct TestSource : Source {
        virtual void play() {}
        virtual int payload_type() const { return 96; }
        TestSource() : Source("", new Streamer) { streamer()->set_source(this); }
        void set(string& fmtp);
        void send_frame(uint8_t* frame, int size, uint32_t timestamp) {}
        void teardown() {}
        bool is_live() const { return false; }

        int _timestamp;
        int _seq_number;

        int timestamp() { return _timestamp; }
        int seq_number() { return _seq_number; }

    };

    void copy_integer(string& from, const char* label, int* dest) {
        size_t pos = from.find(label);
        if (pos == string::npos) {
            *dest = 0;
        } else {
            pos += strlen(label);
            size_t end = from.find_first_not_of("0123456789", pos);
            sscanf(from.substr(pos, end - pos).c_str(), "%d", dest);
        }
    }

    void TestSource::set(string& line) {
        const char param_set[] = "sprop-parameter-sets=";
        uint8_t buffer[1000];
        size_t pos;
        if ((pos = line.find(param_set)) != string::npos) {
            pos += strlen(param_set);
            size_t comma = line.find(',', pos);
            int size = base64decode(line.substr(pos, comma - pos).c_str(), buffer);
            save_sps(buffer, size);
            size_t end = line.find('\r', ++comma);
            size = base64decode(line.substr(comma, end - comma).c_str(), buffer);
            save_pps(buffer, size);
        } else {
//          _sps = NULL; _sps_size = 0;
//          _pps = NULL; _pps_size = 0;
        }
        copy_integer(line, "seq=", &_seq_number);
        copy_integer(line, "rtptime=", &_timestamp);
    }

    TestSource source;
    Streamer streamer;

    struct TestServer : public Server {
        TestServer() : Server(0) {}
        Source* get_source(const char*) { return &source; }
        SessionID setup_tcp(const char* stream_name);
        SessionID setup_udp(const char* stream_name, int client_port);
        void set(string& message);

        char session_id[20];
    };

    Client* Server::find_client(SessionID) {
        return streamer.add_client(Socket(Socket::UDP));
    }


    void Server::teardown(SessionID) {}

    void Server::start_thread() {}

    Source* Server::get_source(char const*) { return &source; }

    SessionID Server::setup_tcp(const char*) { return SessionID::generate(); }
    SessionID TestServer::setup_tcp(const char*) {  
        return SessionID(session_id); 
    }

    SessionID Server::setup_udp(const char*, int) { return SessionID::generate(); } 
    SessionID TestServer::setup_udp(const char*, int) { 
        return SessionID(session_id); 
    }

    Server::Server(short port) : _socket(Socket::UDP) {}


    void copy_ip_addr(string& from, const char* label, char* dest) {
        size_t pos = from.find(label);
        if (pos == string::npos) {
            dest[0] = '\0';
        } else {
            pos += strlen(label);
            size_t end = from.find_first_not_of("09123456789.", pos);
            string ip_addr = from.substr(pos, end - pos);
            strcpy(dest, ip_addr.c_str());
        }
    }

    void copy_session_id(string& from, char* session_id) {
        const char session[] = "Session: ";
        size_t pos = from.find(session);
        if (pos == string::npos) {
            session_id = 0;
        } else {
            pos += strlen(session);
            size_t end = from.find_first_not_of("0123456789abcdefABCDEF", pos);
            strcpy(session_id, from.substr(pos, end - pos).c_str());
        }
    }

    void TestServer::set(string& message) {
        copy_ip_addr(message, "1 IN IP4 ", _server_ip);
        copy_ip_addr(message, "destination=", _client_ip);
        copy_ip_addr(message, "source=", _server_ip);
        copy_integer(message, "client_port=", (int*) &_client_port);
        copy_integer(message, "server_port=", (int*) &_server_port);
        copy_session_id(message, session_id);
        source.set(message);
    }

}

void print(char* data) {
    stringstream buffer;
    buffer << data;
    while (!buffer.eof()) {
        string line;
        getline(buffer, line);
        int size = line.size();
        if (line[size - 1] == '\r') {
            line[size - 1] = '\\';
            line += "r\\n\n";
        }
        cout << line;
        cout.flush();
    }
}

struct Message {
    string request;
    string reply;
};

void read_messages(vector<Message>& messages, const char* filename) {
    ifstream file(filename);
    if (file.fail())
        _throw_("Cannot open file %s", filename);
    bool request = true;
    bool empty_line = true;
    char buffer[200];
    while (!file.eof()) {
        file.getline(buffer, sizeof buffer);
        if (strlen(buffer) == 0) {
            empty_line = true;
        } else {
            if (empty_line) {
                empty_line = false;
                if ((request = strncmp(buffer, "RTSP/1.0", 8)))
                    messages.push_back(Message());
            }
            int n = strlen(buffer) - 4;
            if (!strcmp(buffer + n, "\\r\\n")) {
                buffer[n] = '\r';
                buffer[n + 1] = '\n';
                buffer[n + 2] = '\0';
            }
            if (request)
                messages.back().request += buffer;
            else
                messages.back().reply += buffer;
        }
    }
}

void split(vector<string>& vec, string& str) {
    size_t pos = 0;
    while (pos < str.size()) {
        size_t end = str.find("\r\n", pos);
        vec.push_back(str.substr(pos, end - pos));
        pos = end + 2;
    }
}       

bool skip(const string& line, const char* pattern) {
    return line.substr(0, strlen(pattern)) == pattern;
}

bool compare(string& actual_str, string golden_str) {
    vector<string> actual;
    vector<string> golden;
    split(actual, actual_str);
    split(golden, golden_str);
    _assert_(actual.size() == golden.size());
    for (unsigned int n = 0; n < actual.size(); n++) {
        if (!skip(actual[n], "Date:") &&
            !skip(actual[n], "Public:") &&
            !skip(actual[n], "Content-Length:") &&
            !skip(actual[n], "o=-") &&
            !skip(actual[n], "s=H.264") &&
            !skip(actual[n], "a=tool:") &&
            !skip(actual[n], "a=x-qt-text-nam:") &&
            !skip(actual[n], "b=AS:") &&
            !skip(actual[n], "RTP-Info:") &&
            actual[n] != golden[n]) {
            cout << actual[n] << endl << golden[n] << endl;
            cout << "ERROR at line " << n << endl;
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    bool silent = true;
    if (argc > 1 && !strcmp(argv[1], "-p"))
        silent = false;
    const int buffer_size = 1000;
    char buffer[buffer_size];
    vector<Message> messages;
    RTSP::TestServer server;
    RTSP::Parser parser;
    RTSP::Responder responder(&server, buffer, buffer_size);
    RTSP::streamer.set_source(&RTSP::source);
    ifstream parser_out("parser-out.txt");
    read_messages(messages, "parser-in.txt");
    unsigned int n = 0;
    while (!parser_out.eof()) {
        parser_out >> parser;
        if (n < messages.size()) {
            server.set(messages[n].reply);
            responder.reply(parser.data);
            if (!silent) {
                print(buffer);
                cout << endl;
            }
            if (!compare(messages[n++].reply, buffer))
                return 1;
        }
    }
    cout << "Tested " << n << " messages, OK." << endl;
    return 0;
}
         
