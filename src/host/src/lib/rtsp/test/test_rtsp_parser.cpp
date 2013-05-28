#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include "st_exception.h"
#include "rtsp_parser.h"

using namespace std;

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

void compare(const RTSP::Parser& udt, const RTSP::Parser& ref) {
    const char* empty = "----";
    _assert_(udt.data.method == ref.data.method);
    _assert_(udt.data.cseq   == ref.data.cseq);
    _assert_(!strcmp(udt.data.session_id  ? udt.data.session_id  : empty , ref.data.session_id ));
    _assert_(!strcmp(udt.data.url         ? udt.data.url         : empty , ref.data.url        ));
    _assert_(!strcmp(udt.data.stream_name ? udt.data.stream_name : empty , ref.data.stream_name));
    _assert_(!strcmp(udt.data.accept      ? udt.data.accept      : empty , ref.data.accept));
    _assert_(udt.data.client_port0   == ref.data.client_port0);
    _assert_(udt.data.client_port1   == ref.data.client_port1);
    _assert_(udt.data.transport      == ref.data.transport   );
    _assert_(udt.state()             == ref.state()  );
}

int main(int argc, char* argv[]) {
    bool silent = true;
    if (argc > 1 && !strcmp(argv[1], "-p"))
        silent = false;
    char    buffer[1000];
    string  session;
    vector<Message> messages;
    RTSP::Parser parser;
    RTSP::Parser ref;
    read_messages(messages, "parser-in.txt");
    ifstream golden("parser-out.txt");
    for (unsigned int n = 0; n < messages.size(); n++) {
        strncpy(buffer, messages[n].request.c_str(), sizeof buffer);
        try {
            parser.parse(buffer, strlen(buffer));
            if (!silent)
                cout << parser;
            golden >> ref;
            compare(parser, ref);
        } catch (RTSP::Errcode errcode) {
            const char* desc = parser.errcode_desc(errcode);
            cerr << "Caught error " << errcode << ": " << (desc ? desc : "Unknown") << endl;
            cerr << "Request" << endl << "-------" << endl << messages[n].request << endl << endl;
            cerr << "Reply"   << endl << "-------" << endl << messages[n].reply   << endl << endl;
            return 1;
        }
    }
    cout << "Compared " << messages.size() << " messages, no errors found" << endl;
}
