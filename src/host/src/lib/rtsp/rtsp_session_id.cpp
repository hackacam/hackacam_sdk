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
#include "rtsp_session_id.h"

namespace RTSP {

std::ostream& operator<<(std::ostream& str, const SessionID& id) {
    std::ios_base::fmtflags flags = str.flags();
    str << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << id._session_id;
    str.flags(flags);
    return str;
}

SessionID::SessionID(const char* str) {
    _session_id = 0;
    int n = 0;
    while (char c = str[n++]) {
        int d = c >= '0' && c <= '9' ? c - '0' :
                c >= 'a' && c <= 'f' ? c - 'a' + 10 :
                c >= 'A' && c <= 'F' ? c - 'A' + 10 :
                -1;
        if (d < 0)
            break;
        _session_id = (_session_id << 4) | d;
    }
}

}
