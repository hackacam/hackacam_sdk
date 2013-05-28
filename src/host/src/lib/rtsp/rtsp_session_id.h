#pragma once
#ifndef _RTSP_SESSION_ID
#define _RTSP_SESSION_ID
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
#include <cstdlib>
#include <cstdio>
#include <ostream>
#include <iomanip>

namespace RTSP {

//! Manage session_id, which is a unique identifier for a given stream.
/* When a client wants to receive a stream, it sends Setup request and the server
   replies with a session_id. Standard says that:
   Session identifiers are opaque strings of arbitrary length. Linear
   white space must be URL-escaped. A session identifier MUST be chosen
   randomly and MUST be at least eight octets long to make guessing it
   more difficult.

     session-id   =   1*( ALPHA | DIGIT | safe )
*/
class SessionID {
public:
    //! Read a session ID from a string in client request
    explicit SessionID(const char* str);
        
    //! Explicit generation of a new session_id;
    static SessionID generate() {
        SessionID id(std::rand());
        return id;
    }
    //! Required so that a SessionID may a key in std::map
    bool operator<(const SessionID& id) const { return _session_id < id._session_id; }
    //! Compare two SessionID
    bool operator==(const SessionID& id) const { return _session_id == id._session_id; }
    //! write SessionID in the server reply
    friend std::ostream& operator<<(std::ostream& str, const SessionID& session_id);
private:
    SessionID(int id) : _session_id(id) {}
    int _session_id;
};

}

#endif
