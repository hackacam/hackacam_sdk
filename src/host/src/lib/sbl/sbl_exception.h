#pragma once
#ifndef _SBL_EXCEPTION_H
#define _SBL_EXCEPTION_H
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
#include <exception>
#include <errno.h>

/*! @file sbl_exception.h
    Defines a generic Exception class. Usually, macros defined below should be used to throw exceptions,
    because this preserves context (file name and line) where throw happenend. If you don't care about
    context, use throw() directly and set pretty_function argument to NULL.

*/

/// static assert
#define SBL_STATIC_ASSERT(cond) typedef char static_assert_ ## __LINE__[(cond) ? 1 : -1]

///  Unconditionally throw an exception with a default exception code
#define SBL_THROW(format, ...) throw SBL::Exception(SBL::Exception::UNSPECIFIED_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, format, ##__VA_ARGS__)

/// Unconditionally throw an exception with an exception code
#define SBL_THROW_CODE(code, format, ...) throw SBL::Exception(code, __PRETTY_FUNCTION__, __FILE__, __LINE__, format, ##__VA_ARGS__)

/// Generic assertion, if \e expr is false, throws assertion condition, file name, line and function signature
#define SBL_ASSERT(expr) \
    do {             \
        if (!(expr)) \
            throw SBL::Exception(SBL::Exception::UNSPECIFIED_ERROR, __PRETTY_FUNCTION__,  __FILE__, __LINE__, "assertion '%s' failed in %s:%d", #expr, __FILE__, __LINE__);  \
    } while (0) 

/// Throw exception if condition is true
#define SBL_THROW_IF(cond, format, ...) \
    do {             \
        if (cond)    \
            throw SBL::Exception(SBL::Exception::UNSPECIFIED_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, format, ##__VA_ARGS__);   \
    } while (0)

/// If condition is true, throws an exception and includes system error (i.e. errno). Code is set to errno.
#define SBL_PERROR(expr) \
    do {             \
        if (expr)    \
            throw SBL::Exception(SBL::Exception::UNSPECIFIED_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, "system error").perror();  \
    } while (0)


/// Stretch Base Library
namespace SBL {

/// Generic exception throwing class. 
/** Exception has a \e code and a \e description, which are created in the constructor. Exception doesn't
    internally use the code, except that perror() sets it with errno. The code can be obtained by
    calling code() and the description by calling what().
    
    @attention You should use #SBL_THROW, #SBL_THROW_CODE, #SBL_ASSERT or #SBL_PERROR macros instead of creating Exceptions directly. 
*/
class Exception : public std::exception {
public:
    /// Standard error code used by exceptions
    enum ErrorCode {UNSPECIFIED_ERROR = -1,    //!< throwing code didn't specify it
                    NO_ERROR = 0,              //!< no error occured
                    USER_ERROR,                //!< incorrect user input
                    PROGRAM_ERROR,             //!< internal program error, recoverable
                    PROGRAM_FATAL,             //!< internal program error, unrecoverable
                    TEST_ERROR                 //!< unit test error
                   };
    /// Create an Exception, same semantics as printf
    /** @param  code                generic code, not used in Exceptions except for perror() function
        @param  pretty_function     you should pass __PRETTY_FUNCTION__ macro here.
        @param  filename            file from which exception is thrown
        @param  line                line where exception is thrown
        @param  format              the same meanining as for printf 
        'pretty_function' may be NULL, in which case it is not printed.
     */
    Exception(int code, const char* pretty_function, const char* filename, int line, const char* format, ...);
    /// return Exception message
    virtual const char* what() const throw();
    /// append to the current Exception message
    Exception& append(const char* format, ...);
    /// Appends errno error message to Exception buffer and sets code to errno
    Exception&  perror();
    /// Return exception code
    int code() const { return _code; }
    /// Virtual destructor
    virtual ~Exception() throw() {}
    /// Enable printing stack backtrace.
    /// If this is enable, stack backtrace is added to Exception buffer
    /// (make sure you link with -rdynamic if you want to use this feature)
    static void enable_backtrace(bool enable) {_print_backtrace = enable; }
    /// Enable catching of segmentation fault.
    /// After this function is called, a segmentation fault will be caught and
    /// a stack trace will be put in an Exception buffer. Buffer content is printed
    /// using #SBL_ERROR and then abort() is called.
    static void catch_segfault();
    /// Enable logging of exceptions
    static void enable_log(bool enable) { _log_exceptions = enable; }
private:
    enum         {BUFF_SIZE = 2048, STACKTRACE_FRAMES = 15};
    char         _buffer[BUFF_SIZE];
    int          _code;
    unsigned int signature(const char* pretty_function);
    unsigned int stack_trace();
    static  bool _print_backtrace;
    static  bool _log_exceptions;

    Exception();    // private and unimplemented
};
}
#endif
