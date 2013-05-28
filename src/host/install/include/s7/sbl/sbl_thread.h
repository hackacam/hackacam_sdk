#pragma once
#ifndef _SBL_THREAD_H
#define _SBL_THREAD_H
/****************************************************************************\
*  Copyright C 2012 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/
#include <pthread.h>
#include "sbl_logger.h"
#include "sbl_exception.h"

/// @cond
#undef _SBL_MSG_
#ifdef SBL_MSG_THREAD
#define _SBL_MSG_(...) SBL_MSG(SBL_MSG_THREAD, ##__VA_ARGS__)
#else
#define _SBL_MSG_(...)
#endif
/// @endcond

//! @file sbl_thread.h

/// Stretch Base Library
namespace SBL {

/// Abstract class, which encapulates thread library. 
/** To use this class, one needs to derive from Thread and implement start_thread() method.
    Example @verbatim
    struct Server: public Thread {
        Server(int x, const char* c);
        void start_thread();
    };
    // and then in the application code:
    Server server(5, "hello");  // to pass any parameters to the thread
    server.create_thread();    // to start the thread
    @endverbatim
    @b Important: To start a new thread, call create_thread(). If you call start_thread(), the code
    will work, but in the existing thread. This may be convienient for testing.

    You need define SBL_MSG_THREAD (and set Log::verbosity correctly) to get messages from this subsystem.
*/
class Thread {
public:
    /// Allows to create a detached thread
    enum Flags {Default    /*!< Default flags for the thread */ = 0,
                Detached   /*!< Thread will be detached */      = 1
                };
    /// Call this function from derived class to start the thread
    /// @param flags        currently, the only supported flag is Detached to create a detached thread
    /// @param stack_size   the thread stack size, in bytes
    void create_thread(Flags flags = Default, unsigned int stack_size = 0) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        if (stack_size) 
            SBL_ASSERT(pthread_attr_setstacksize(&attr, stack_size) == 0);
        if (flags & Detached)
            SBL_ASSERT(pthread_attr_setdetachstate(&attr, Detached) == 0);
        SBL_ASSERT(pthread_create(&_thread, &attr, thread_entry, this) == 0);
        _SBL_MSG_("Created thread %p", this);
    }
    /// For threads that are not detached, wait until thread finishes
    void join_thread() {
        SBL_ASSERT(pthread_join(_thread, NULL) == 0);
        _SBL_MSG_("Joined thread %p", this);
    }
    /// Detach a thread
    void detach_thread() {
        SBL_ASSERT(pthread_detach(_thread) == 0);
        _SBL_MSG_("Detached thread %p", this);
    }
    /// Kill the thread, thread must be not detached.
    void kill_thread() {
        SBL_ASSERT(pthread_cancel(_thread) == 0);
        void* retval;
        SBL_ASSERT(pthread_join(_thread, &retval) == 0);
        SBL_ASSERT(retval == PTHREAD_CANCELED);
        _SBL_MSG_("Killed thread %p", this);
    }
    /// Virtual destructor required for classes with virtual functions
    virtual ~Thread() {}

protected:
    /// declare this function as public in derived class. This is thread start.
    virtual void start_thread() = 0;

private:
    pthread_t _thread;

    static void* thread_entry(void* state) {
        reinterpret_cast<Thread*>(state)->start_thread();
        return 0;
    }
};

/// Encapsulates mutexes and condition variables
/** Condition variables are used when thread A has to wait until thread B completed some tasks. 
    The sequence of actions is:
        -# ThreadA locks a mutex
        -# ThreadA waits on that mutex (which under the hood unlocks it)
        -# ThreadB locks the mutex.
        -# ThreadB performs required work
        -# ThreadB signals on the mutex (this will wake thread A waiting on it)
        -# ThreadB unlocks the mutex.
        -# ThreadA unlocks the mutex

    Code exampe: @verbatim
    Mutex mutex;
    struct WaitingThread : public Thread {
        void start_thread() {
            mutex.lock();    // acquire lock
            // ... request some action from SignalingThread .. 
            mutex.wait();   // wait until action is completed
            // Note that mutex.lock() releases lock while waiting and
            // requires when it signalled and before WaitingThread resumes
            // ... complete processing .....
            mutex.unlock(); // release the lock for next time
        }
    };
    struct SignalingThread : public Thread {
        void start_thread() {
            mutex.lock();   // acquire lock, will wait until WaitingThread call mutex.wait()
            // ... perform required action
            mutex.signal(); // wake up the WaitingThread
            // ... clean up as required
            mutex.unlock();  
        }
    };
    int main() {
        WaitingThread   wt;
        SignalingThread st;
        wt.create_thread()
        st.create_thread();
        wt.join();      
        st.join()
        return 0;
    } @endverbatim
    @note For this example to work, the WaitingThread must acquire the lock before SignalingThread
    tries to acquire his. 
*/
class Mutex {
public:
    /// Create a new mutex
    Mutex() : _mutex((pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER), 
              _cond ((pthread_cond_t)  PTHREAD_COND_INITIALIZER),
              _wait(false) 
              {}
    /// Lock the mutex. Will block if mutex is already locked.
    void lock()    { 
        _SBL_MSG_("Locking mutex %p", this);
        SBL_ASSERT(pthread_mutex_lock(&_mutex) == 0); 
    }
    /// Unlock the mutex
    void unlock()  { 
        _SBL_MSG_("Unlocking mutex %p", this);
        SBL_ASSERT(pthread_mutex_unlock(&_mutex) == 0); 
    }
    /// If mutex is unlocked, lock it and return true, otherwise return false.
    /// Unlike lock(), trylock() doesn't block
    bool trylock() { 
        _SBL_MSG_("Trying mutex %p", this);
        int ret = pthread_mutex_trylock(&_mutex);
        SBL_ASSERT(ret == 0 || ret == EBUSY);
        return ret == 0;
    }
    /// Wait (block) on the condition variable, i.e until somebody calls  
    /** This unlocks the mutex while the caller waits and relocks when the caller resumes
        If time is specified (microsecond or second), it is the relative timeout.
        Function returns true if exit is normal (i.e. somebody signalled) and false
        if exit is due to timeout
    */
    bool wait(int microsec = 0, int sec = 0) {
        _SBL_MSG_("Waiting on mutex %p", this);
        _wait = true;
        if (microsec || sec) {
            struct timespec ts;
            SBL_ASSERT(clock_gettime(CLOCK_REALTIME, &ts) == 0);
            ts.tv_nsec += microsec * 1000;
            const int one_billion = 1000 * 1000 * 1000;
            if (ts.tv_nsec >= one_billion) {
                ts.tv_nsec -= one_billion;
                ts.tv_sec++;
            }
            ts.tv_sec += sec;
            /* This loop is necessary, because the standard says that "Spurious wakeups 
               from the pthread_cond_timedwait() or pthread_cond_wait() functions may occur." */
            do {
                int status = pthread_cond_timedwait(&_cond, &_mutex, &ts);
                if (status == ETIMEDOUT) {
                    _SBL_MSG_("Returning due to timeout from mutex %p", this);
                    return false;
                }
                SBL_ASSERT(status == 0);
            } while (_wait);
            return true;
        }
        do { 
            SBL_ASSERT(pthread_cond_wait(&_cond, &_mutex) == 0);
        } while (_wait);
        return true;
    }
    /// Signal the condition variable
    /** Wake anybody waiting on the mutex. It must be called after mutex is locked and
        the caller must unlock mutex after signaling.
    */
    void signal() {
        _SBL_MSG_("Signaling from mutex %p", this);
        _wait = false;
        SBL_ASSERT(pthread_cond_signal(&_cond) == 0);
    }
private:
    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
    volatile bool   _wait;
};

}
#undef _SBL_MSG_
#endif
