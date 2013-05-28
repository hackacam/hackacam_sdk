#include <cassert>
#include <string>
#include <sstream>
#include <iostream>
#include <sbl_thread.h>
using namespace SBL;
using namespace std;

Mutex mutex;


struct Thread1 : public Thread {
    Thread1(ostream& str, int usec = 0) : _str(str), _usec(usec) {}
    void start_thread() {
        _str << "Starting main thread" << endl;
        mutex.lock();
        _str << "Main thread locked mutex, waiting" << endl;
        bool status = mutex.wait(_usec);
        assert((status && _usec == 0) || (!status && _usec != 0));
        _str << "Main thread wait done" << endl;
        mutex.unlock();
        _str << "Main thread exiting" << endl;
    }
    ostream& _str;
    int      _usec;
};

struct Thread2 : public Thread {
    Thread2(ostream& str) : _str(str) {}
    void start_thread() {
        usleep(10000);
        _str << "starting signaling thread" << endl;
        mutex.lock();
        _str << "signaling thread got mutex" << endl;
        mutex.signal();
        _str << "signal done" << endl;
        mutex.unlock();
        _str << "signaling thread done" << endl;
    }
    ostream& _str;
};

const string golden(
"Starting main thread\n"
"Main thread locked mutex, waiting\n"
"starting signaling thread\n"
"signaling thread got mutex\n"
"signal done\n"
"signaling thread done\n"
"Main thread wait done\n"
"Main thread exiting\n"
);

int main(int argc, char* argv[]) {
    stringstream str;
    Thread1 thread1(str);
    Thread2 thread2(str);
    thread1.create_thread();
    thread2.create_thread();
    thread1.join_thread();
    thread2.join_thread();
    string actual(str.str());
    assert(actual == golden);

    Thread1 thread3(str, 1000);
    Thread2 thread4(str);
    thread3.create_thread();
    thread4.create_thread();
    thread3.join_thread();
    thread4.join_thread();

    cout << argv[0] << " passed." << endl;
    return 0;
}
