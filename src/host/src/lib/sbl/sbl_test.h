#pragma once
#ifndef _SBL_TEST_H
#define _SBL_TEST_H
#include <sstream>
#include <sbl_exception.h>

#define _SBL_TEST_MSG3(msg, a, b)   \
    {                               \
        std::stringstream str;      \
        const char* a_ = a; const char* b_ = b; int line = 1; int chr = 1;  \
        for (; *a_ && *b_ && *a_ == *b_; ++a_, ++b_, ++chr)        \
            if (*a_ == '\n')  {                             \
                line++; chr = 1;                            \
            }                                               \
        str << #msg " failed at line " << __LINE__ << " (diff on line " << line << ", char " << chr << "):\n    " \
            << #a << "=" << a << "\n    " << #b << "=" << b << "\n";    \
        throw SBL::Exception(SBL::Exception::TEST_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, str.str().c_str()); \
    }

#define _SBL_TEST_MSG2(msg, a, b)   \
    {                               \
        std::stringstream str;      \
        str << #msg " failed at line " << __LINE__ << ":\n    " << #a << "=" << a << "\n    " << #b << "=" << b << "\n";    \
        throw SBL::Exception(SBL::Exception::TEST_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, str.str().c_str()); \
    }

#define _SBL_TEST_MSG1(msg, a)      \
    {                               \
        std::stringstream str;      \
        str << #msg " failed at line " << __LINE__ << ":\n";   \
        throw SBL::Exception(SBL::Exception::TEST_ERROR, __PRETTY_FUNCTION__, __FILE__, __LINE__, str.str().c_str()); \
    }

#define SBL_TEST_EQ(a, b)       if (!((a) == (b)))      _SBL_TEST_MSG2(SBL_TEST_EQ, a, b)
#define SBL_TEST_NE(a, b)       if ( ((a) == (b)))      _SBL_TEST_MSG2(SBL_TEST_NE, a, b)
#define SBL_TEST_EQ_STR(a, b)   if (strcmp(a, b) != 0)  _SBL_TEST_MSG3(SBL_TEST_EQ_STR, a, b)
#define SBL_TEST_NE_STR(a, b)   if (strcmp(a, b) == 0)  _SBL_TEST_MSG3(SBL_TEST_NE_STR, a, b)
#define SBL_TEST_TRUE(a)        if (!(a))               _SBL_TEST_MSG1(SBL_MSG_TRUE, a)
#define SBL_TEST_FALSE(a)       if (a)                  _SBL_TEST_MSG1(SBL_MSG_FALSE, a)

#endif
