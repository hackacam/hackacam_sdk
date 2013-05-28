#pragma once
#ifndef _CGI_H
#define _CGI_H
/****************************************************************************\
*  Copyright C 2013 Stretch, Inc. All rights reserved. Stretch products are  *
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

#include <sbl/sbl_exception.h>

#define PARAM(name, ...) name(this, #name, ##__VA_ARGS__)

#define CGI_ERROR(cond, format, ...) \
    do {    \
        if (cond)       \
            throw SBL::Exception(USER_ERROR, NULL, __FILE__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

/// CGI Server
namespace CGI {

/// Message codes for logger
struct MSG  {
    static int SERVER;
    static int SDK;
    static int WATCHDOG;
    static int TEMP;
    static int MV;
    static int FRAME;
};

/// error codes
enum ErrorCode {NO_ERROR                = SBL::Exception::NO_ERROR,
                USER_ERROR              = SBL::Exception::USER_ERROR,
                PROGRAM_ERROR           = SBL::Exception::PROGRAM_ERROR,
                FATAL_ERROR             = SBL::Exception::PROGRAM_FATAL,
                NOT_FOUND               = 404,
                REQUEST_URI_TOO_LONG    = 414,
                INTERNAL_SERVER_ERROR   = 500
                };
}
#endif
