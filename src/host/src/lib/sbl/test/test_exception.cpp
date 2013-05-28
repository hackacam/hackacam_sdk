#include <cassert>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sbl_exception.h>

using namespace std;
using namespace SBL;

int check(Exception& ex, const char* what) {
    return strncmp(ex.what(), what, strlen(what)) == 0;
}

int main(int argc, char* argv[]) {
    Exception::enable_log(false);
    try {
        SBL_THROW("throw");
        assert(0);
    } catch (Exception& ex) {
        assert(ex.code() == -1);
        assert(check(ex, "main() throw"));
        ex.append(", appending %d", 5);
        assert(check(ex, "main() throw, appending 5"));
    }
    try {
        SBL_THROW_CODE(1, "throw %d", 1);
        assert(0);
    } catch (Exception& ex) {
        assert(ex.code() == 1);
        assert(check(ex, "main() throw 1"));
    }
    try {
        SBL_THROW_IF(1, "Error %d", 5);
    } catch (Exception& ex) {
        assert(ex.code() == -1);
        assert(check(ex, "main() Error 5"));
    }
    SBL_ASSERT(1);
    try {
        SBL_ASSERT(0);
        assert(0);
    } catch (Exception& ex) {
        assert(ex.code() == -1);
        assert(check(ex, "main() assertion '0' failed"));
    }
    try {
        SBL_PERROR(socket(-1, 0, 0) != 0);
    } catch (Exception& ex) {
        assert(ex.code() == 97);
        assert(check(ex, "main() system error (errno = 97)"));
    }
    Exception::enable_backtrace(true);
    try {
        SBL_ASSERT(0);
    } catch (Exception& ex) {
        assert(ex.code() == -1);
        assert(string(ex.what()).find("Stack trace:") != string::npos);
    }
    cout << argv[0] << " passed." << endl;
    return 0;
}
