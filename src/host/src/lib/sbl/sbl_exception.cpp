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
// needed so that strerror_r returns char*
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <execinfo.h>
#include <cxxabi.h>

#include "sbl_logger.h"
#include "sbl_exception.h"

namespace SBL {

bool Exception::_print_backtrace = false;
bool Exception::_log_exceptions  = true;

Exception::Exception(int code, const char* pretty_function, const char* filename, int line, const char* format, ...) : _code(code) {
    size_t offs = signature(pretty_function);
    va_list args;
    va_start(args, format);
    vsnprintf(_buffer + offs, BUFF_SIZE - offs, format, args);
    va_end(args);
    if (_print_backtrace)
        stack_trace();
    if (_log_exceptions)
        SBL::Log::print("Exception", filename, line, 3, what());
};

Exception& Exception::append(const char* format, ...) {
    size_t offs = strlen(_buffer);
    va_list args;
    va_start(args, format);
    vsnprintf(_buffer + offs, BUFF_SIZE - offs, format, args);
    va_end(args);
    return *this;
};

const char* Exception::what() const throw() {
    return _buffer;
}

unsigned int Exception::signature(const char* pretty_function) {
    if (!pretty_function)
        return 0;
    const char trailer[] = ") ";
    // install function name (without return value, qualifiers or arguments, but with class name)
    const char* open_par = std::strchr(pretty_function, '(');
    if (open_par) {
        const char* start = open_par;
        while (start > pretty_function && *(start - 1) != ' ')
            --start;
        int size = open_par - start + 1;
        if (size > BUFF_SIZE - 1)
            size = BUFF_SIZE - 1;
        std::strncpy(_buffer, start, size);
        _buffer[size] = 0;
        size = BUFF_SIZE - std::strlen(_buffer) - std::strlen(trailer);
        if (size > 0)
            std::strncat(_buffer, trailer, size);
    } else {
        std::strncpy(_buffer, pretty_function, BUFF_SIZE);
        _buffer[BUFF_SIZE - 1] = 0;
    }
    return std::strlen(_buffer);
}

Exception& Exception::perror() {
    _code = errno;
    size_t len = strlen(_buffer);
    len += snprintf(_buffer + len, BUFF_SIZE - len, " (errno = %d)", errno);
    len += snprintf(_buffer + len, BUFF_SIZE - len, " ");
    char* error_msg = (char*) strerror_r(errno, _buffer + len, BUFF_SIZE - len);
    strncpy(_buffer + len, error_msg, BUFF_SIZE - len);
    return *this;
}

/* Print a demangled stack backtrace of the caller function to _buffer. */
unsigned int Exception::stack_trace() {
    unsigned int pos = strlen(_buffer);

    pos += snprintf(_buffer+ pos, BUFF_SIZE - pos, "\nStack trace:\n");

    // storage array for stack trace address data
    void* addr_list[STACKTRACE_FRAMES + 1];

    // retrieve current stack addresses
    int addr_len = backtrace(addr_list, sizeof(addr_list) / sizeof(void*));

    if (addr_len == 0) {
        pos += snprintf(_buffer + pos, BUFF_SIZE - pos, "  <empty, possibly corrupt>\n");
        return pos;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbol_list = backtrace_symbols(addr_list, addr_len);

    // allocate string which will be filled with the demangled function name
    size_t func_name_size = 256;
    char func_name[func_name_size];

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addr_len; i++) {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbol_list[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset
            && begin_name < begin_offset) {

            *begin_name++   = '\0';
            *begin_offset++ = '\0';
            *end_offset     = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name,
                            func_name, &func_name_size, &status);
            if (status == 0) {
                pos += snprintf(_buffer + pos, BUFF_SIZE - pos, "  %s : %s+%s\n",
                                 symbol_list[i], ret, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                pos += snprintf(_buffer + pos, BUFF_SIZE - pos, "  %s : %s()+%s\n",
                                symbol_list[i], begin_name, begin_offset);
            }
        }
        else {
            // couldn't parse the line? print the whole line.
            pos += snprintf(_buffer + pos, BUFF_SIZE - pos, "  %s\n", symbol_list[i]);
        }
    }

    free(symbol_list);
    return pos;
}

// Seg fault handler, print stack backtrace and abort
static void segfault(int) {
    signal(SIGSEGV, SIG_DFL);
    Exception ex(-1, __PRETTY_FUNCTION__, __FILE__, __LINE__, "Segmentation fault");
    SBL_ERROR(ex.what());
    abort();
}

// Install seg fault handler
void Exception::catch_segfault() {
    Exception::enable_backtrace(true);
    Exception::enable_log(true);
    signal(SIGSEGV, segfault);
}


}
