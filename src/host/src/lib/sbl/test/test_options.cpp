#include <string>
#include <cstring>
#include <sstream>
#include <iostream>

#include <sbl_options.h>
#include <sbl_test.h>

using namespace std;
using namespace SBL;


struct Options : public OptionSet {
    Option<String>   string_param;
    Option<int>      int_param;
    Option<bool>     bool_param;
    Option<double>   double_param;
    Option<String>   similar_param;
    Options(int argc, const char* argv[]) : OptionSet("fancy options"),
            string_param   (this, "string",    "sparam",  "string param"),
            int_param      (this, "int",       5,         "int param"),
            bool_param     (this, "bool",      false,     "bool param"),
            double_param   (this, "double",    7.3,       "double param"),
            similar_param  (this, "str2",      "sim",     "similar param")
            { parse(argc, const_cast<char**>(argv)); 
            }
    
};


const char* golden1 = 
"string=str\n"
"int=10\n"
"bool=0\n"
"double=1.5\n"
"str2=str2\n"
"fancy options\n"
"Usage: test [-option(s)]\n"
"    -string <string>                     string param (sparam)\n"
"    -int <int>                           int param (5)\n"
"    -bool <bool>                         bool param (0)\n"
"    -double <double>                     double param (7.3)\n"
"    -str2 <string>                       similar param (sim)\n"
"    -help                                print this message\n"
;

void compare(const char* golden, stringstream& str) {
    int n = str.tellp();
    char buffer[n + 1];
    str.read(buffer, n);
    buffer[n] = '\0';
    SBL_TEST_EQ_STR(buffer, golden);
}

void args1() {
    const char* argv[] = { "test", "-stri", "str", "--i", "10", "-d", "1.5", "-str2", "str2", "-help"};
    Options options(sizeof(argv) / sizeof(char*), argv);
    stringstream str;
    str << options;
    options.help(str);
    compare(golden1, str);
};

int main(int argc, char* argv[]) {
    Exception::enable_log(false);
    args1();
    cout << argv[0] << " passed.\n";
    return 0;
}
