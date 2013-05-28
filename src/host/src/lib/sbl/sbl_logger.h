#pragma once
#ifndef _SBL_LOGGER_H
#define _SBL_LOGGER_H
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

/// Stretch Base Library
namespace SBL {

    /// Message logging facility
    namespace Log {

/// @file sbl_logger.h 
/** @name

#  SBL Log
General purpose logging facility.
Following macros available for use in the application:

    Macro name                  Verbosity   Description
    SBL_MSG(mask, fmt, ...)     4+          print verbose messages
    SBL_INFO(fmt, ...)          3           print information messages
    SBL_WARN(fmt, ...)          2           print warning messages
    SBL_ERROR(fmt, ...)         1           print error messages

Verbosity is a runtime variable that determines what is printed. Verbosity levels 4 or higher
represent a bit mask, where each bit is typically assigned to a subsystem. To print messages
from that subsystem, set the verbosity bit for it. If the bit is not set, message is not printed.
It is possible to disable printing of #SBL_MSG messages at compile time by setting #SBL_LOG_VERBOSE to 0. 
In that case, SBL_MSGs will not be printed indepedently of verbosity and the corresponding checks will be
compiled out. However, runtime checks incur a cost of only one condition check, so it
is rarely necessary to compile that out.

Verbosity levels 1, 2, and 3 are special and represent respectively errors, warnings and infos.
These levels cannot be compiled out. Setting verbosity to

    0   disables all messages
    1   enables errors
    2   enables errors and warnings
    3   enables errors, warnings and informations

By default, verbosity is set to 1 and #SBL_LOG_VERBOSE is 1.
Output is by default sent to stdout, but open_logfile() may be used to redirect it to a file. 

This logging is thread-safe.
*/

#ifndef SBL_LOG_VERBOSE
/// If set to 0, #SBL_MSG are compiled out
#define SBL_LOG_VERBOSE 1
#endif

/// Print out an error message
#define SBL_ERROR(fmt, ...)  SBL_LOG_PRINT(1, fmt, ##__VA_ARGS__)
/// Print out a warning message
#define SBL_WARN(fmt, ...)   SBL_LOG_PRINT(2, fmt, ##__VA_ARGS__)
/// Print out an information message
#define SBL_INFO(fmt, ...)   SBL_LOG_PRINT(3, fmt, ##__VA_ARGS__)

/// @cond For internal use only
#define SBL_LOG_PRINT(level, fmt, ...) \
    do { \
        if (SBL::Log::verbosity >= (level)) \
            SBL::Log::print(__FUNCTION__, __FILE__, __LINE__, level, fmt, ##__VA_ARGS__); \
    } while (0)
/// @endcond

#if SBL_LOG_VERBOSE

/// @brief Print a message if the current verbosity matches message mask.
#define SBL_MSG(mask, fmt, ...) \
    do { \
        if (SBL::Log::verbosity & (mask)) \
            SBL::Log::print(__FUNCTION__, __FILE__, __LINE__, 0, fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define SBL_MSG(...)
#endif


        /// @cond current verbosity level
        extern unsigned int verbosity;
        /// For internal use only
        extern int print(const char* function, const char* file, int line, unsigned int level, const char* fmt, ...);
        /// @endcond

        /// @brief Set a verbosity level.
        extern void set_verbosity(unsigned int level);
        /// Print all messages to a file (by default it is stdout)
        /// @param  pathname    file name
        /// @param  append      append to that file (by default it overwrites it)
        /// @param  max_size    maximum size of the logfile
        /** If max_size is specified, then logfile write pointer is reset to 0 when the
            file size reaches max_size, and the next message will overwrite the first one.
            This effectively implements a sort of circular buffer for messages */
        extern int  open_logfile(const char* pathname, bool append = false, int max_size = 0);
        /// Close old logfile, rename it and open a new logfile with the same name
        /// @param suffix   appends suffix to the logfile name
        /// @return name of the saved logfile
        extern const char* save_logfile(unsigned int suffix);
        /// close the logfile
        extern int  close_logfile();
    }
}

#endif
