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
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "sbl_logger.h"
#include "sbl_thread.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace SBL {

namespace Log {

    static Mutex _mutex;
    unsigned int verbosity              = 1;
    static int   _logfile               = STDOUT_FILENO;
    static char  _file_name [FILENAME_MAX];
    static char  _saved_name[FILENAME_MAX];
    static unsigned int _current_size   = 0;
    static unsigned int _max_size       = 0;       
    static const char* message[] = {"MESSAGE", 
                                    "ERROR  ", 
                                    "WARNING", 
                                    "INFO   "};

    int print(const char* function, const char* file, int line, unsigned int level, const char* fmt, ...) {
        if (_max_size && _current_size >= _max_size) {
            lseek(_logfile, 0, SEEK_SET);
            _current_size = 0;
        }
        const int buffer_size = 2000;
        char buffer[buffer_size];
        struct timespec time;
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        time.tv_sec = mts.tv_sec;
        time.tv_nsec = mts.tv_nsec;
#else
        clock_gettime(CLOCK_MONOTONIC, &time);
#endif
        int count = snprintf(buffer, buffer_size, "%s [%02ld:%02ld.%06ld] [%s(), %s:%d]: ", 
                                    message[level & 3], (time.tv_sec / 60) % 60, time.tv_sec % 60, time.tv_nsec / 1000, function, file, line);
        va_list args;
        va_start(args, fmt);
        count += vsnprintf(buffer + count, buffer_size - count, fmt, args);
        va_end(args);
        if (count < buffer_size)
            buffer[count++] = '\n';
        _current_size += count;
        _mutex.lock();
        int status = write(_logfile, buffer, count) < 0 || count >= buffer_size;
        _mutex.unlock();
        return status;
    }

    int open_logfile(const char* pathname, bool append, int max_size) {
        int flags = O_CREAT | O_WRONLY | (append ? 0 : O_TRUNC);
        _logfile  = open(pathname, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (_logfile < 0)
            return -1;
        strncpy(_file_name, pathname, FILENAME_MAX);
        _max_size = max_size;
        _current_size = 0;
        if (append) {
            struct stat buffer;
            fstat(_logfile, &buffer);
            _current_size = buffer.st_size;
            lseek(_logfile, _current_size, SEEK_SET);
        }
        return 0;
    }

    const char* save_logfile(unsigned int version) {
        if (_logfile < 0 || _logfile == STDOUT_FILENO)
            return NULL;
        snprintf(_saved_name, FILENAME_MAX, "%s.%d", _file_name, version);
        _mutex.lock();
        int status = close(_logfile);
        if (status == 0)
            status |= rename(_file_name, _saved_name);
        if (status == 0)
            status |= open_logfile(_file_name, false, _max_size);
        _mutex.unlock();
        return status ? NULL : _saved_name;
    }
    
    void set_verbosity(unsigned int level) {
        verbosity = level;
    }

    int close_logfile() {
        _max_size = 0;
        return close(_logfile);
    }
}

}
