#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>
#include <sbl_logger.h>

using namespace std;

void generate(const char* logfile, vector<string>& msgs) {
    SBL::Log::open_logfile(logfile);
    SBL::Log::set_verbosity(0);
    SBL_ERROR("This shouldn't be logged");
    SBL_WARN("This shouldn't be logged");
    SBL_INFO("This shouldn't be logged");
    SBL::Log::set_verbosity(1);
    SBL_ERROR("Error");
    msgs.push_back("Error");
    SBL_WARN("This shouldn't be logged");
    SBL_INFO("This shouldn't be logged");
    SBL_MSG(4, "This shouldn't be logged");

    SBL::Log::set_verbosity(2);
    SBL_ERROR("Error 2");
    msgs.push_back("Error 2");
    SBL_WARN("Warning 2");
    msgs.push_back("Warning 2");
    SBL_INFO("This shouldn't be logged");
    SBL_MSG(4, "This shouldn't be logged");

    SBL::Log::set_verbosity(3);
    SBL_ERROR("Error 3");
    msgs.push_back("Error 3");
    SBL_WARN("Warning 3");
    msgs.push_back("Warning 3");
    SBL_INFO("Info 3");
    msgs.push_back("Info 3");
    SBL_MSG(4, "This shouldn't be logged");

    SBL::Log::set_verbosity(4);
    SBL_ERROR("Error 4");
    msgs.push_back("Error 4");
    SBL_WARN("Warning 4");
    msgs.push_back("Warning 4");
    SBL_INFO("Info 4");
    msgs.push_back("Info 4");
    SBL_MSG(4, "MSG4 4");
    msgs.push_back("MSG4 4");
    SBL_MSG(8, "This shouldn't be logged");

    SBL::Log::set_verbosity(12);
    SBL_ERROR("Error 12");
    msgs.push_back("Error 12");
    SBL_WARN("Warning 12");
    msgs.push_back("Warning 12");
    SBL_INFO("Info 12");
    msgs.push_back("Info 12");
    SBL_MSG(4, "MSG4 12");
    msgs.push_back("MSG4 12");
    SBL_MSG(8, "MSG8 12");
    msgs.push_back("MSG8 12");

    SBL::Log::close_logfile();
}

int verify(const char* logfile, vector<string>& msgs) {
    ifstream ifs(logfile);
    string line;
    int n = 0;
    int errors = 0;
    while (!ifs.eof()) {
        getline(ifs, line);
        if (line.length() == 0)
            continue;
        size_t pos = line.find("]: ");
        assert(!(pos == string::npos || pos + 3 > line.length()));
        assert(n < msgs.size());
        assert(line.substr(pos + 3) == msgs[n]);
        n++;
    }
    ifs.close();
    return 0;
}

int main(int argc, char* argv[]) {
    string logfile = string(argv[0]) + string("-test.txt");
    vector<string> msgs;
    generate(logfile.c_str(), msgs);
    verify(logfile.c_str(), msgs);
    cout << argv[0] << " passed." << endl;
    return 0;
}

