#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>
#include <sys/stat.h>
#include <sbl_logger.h>
#include <sbl_test.h>

using namespace std;

/* There are 4 tests:
   1. verify that append = tfalse works
   2. verify that append = true works
   3. limit size with append = false
   4. limit size with append = true
*/

// Default logfile, no append no size limits
void generate1(const char* logfile) {
    SBL::Log::open_logfile(logfile);
    SBL::Log::set_verbosity(3);
    SBL_ERROR("Error 1");
    SBL_WARN("Warning 1");
    SBL_INFO("Info 1");
    SBL::Log::close_logfile();
}
// verify that a line starts with tag and has msg
void test_line(ifstream& ifs, const string& tag, const string& msg) {
    string line;
    getline(ifs, line);
    SBL_TEST_EQ(line.substr(0, tag.size()), tag);
    SBL_TEST_NE(line.find(msg), string::npos);
}

void verify1(const char* logfile) {
    ifstream ifs(logfile);
    test_line(ifs, "ERROR", "Error 1");
    test_line(ifs, "WARNING", "Warning 1");
    test_line(ifs, "INFO", "Info 1");
    ifs.close();
}

// append to previous logfile
void generate2(const char* logfile) {
    SBL::Log::open_logfile(logfile, true);
    SBL::Log::set_verbosity(3);
    SBL_ERROR("Error 2");
    SBL_WARN("Warning 2");
    SBL_INFO("Info 2");
    SBL::Log::close_logfile();
}

void verify2(const char* logfile) {
    ifstream ifs(logfile);
    test_line(ifs, "ERROR", "Error 1");
    test_line(ifs, "WARNING", "Warning 1");
    test_line(ifs, "INFO", "Info 1");
    test_line(ifs, "ERROR", "Error 2");
    test_line(ifs, "WARNING", "Warning 2");
    test_line(ifs, "INFO", "Info 2");
    ifs.close();
}

// specify maximum size, with append = false
void generate3(const char* logfile) {
    SBL::Log::open_logfile(logfile, false, 200);
    SBL::Log::set_verbosity(3);
    for (int n = 0; n < 10; n++) {
        SBL_ERROR("Error %d", n);
    }
    SBL::Log::close_logfile();
}

void verify3(const char* logfile) {
    ifstream ifs(logfile);
    test_line(ifs, "ERROR", "Error 9");
    test_line(ifs, "ERROR", "Error 7");
    test_line(ifs, "ERROR", "Error 8");
    SBL::Log::close_logfile();
};

// specify maximum size, with append = true
void generate4(const char* logfile) {
    SBL::Log::open_logfile(logfile, false, 200);
    SBL::Log::set_verbosity(3);
    SBL_ERROR("Error 11");
    SBL_ERROR("Error 12");
    SBL::Log::close_logfile();
    SBL::Log::open_logfile(logfile, true, 200);
    SBL::Log::set_verbosity(3);
    SBL_ERROR("Error 13");
    SBL_ERROR("Error 14");
    SBL::Log::close_logfile();
}
void verify4(const char* logfile) {
    ifstream ifs(logfile);
    test_line(ifs, "ERROR", "Error 14");
    test_line(ifs, "ERROR", "Error 12");
    test_line(ifs, "ERROR", "Error 13");
    SBL::Log::close_logfile();
}

int main(int argc, char* argv[]) {
    string logfile = string(argv[0]) + string("-test.txt");
    const int size = 300;
    generate1(logfile.c_str());
    verify1(logfile.c_str());
    generate2(logfile.c_str());
    verify2(logfile.c_str());
    generate3(logfile.c_str());
    verify3(logfile.c_str());
    generate4(logfile.c_str());
    verify4(logfile.c_str());
    cout << argv[0] << " passed." << endl;
    return 0;
}

