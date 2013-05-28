#pragma once
#ifndef _RTSP_PARSER_H
#define _RTSP_PARSER_H
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
#include <vector>
#include <map>
#include <ostream>
#include <istream>
#include <sbl/sbl_map.h>
#include "rtsp_impl.h"

namespace RTSP {

//! Responsible for parsing client request and populating Parser::Data structure.
/*! All errors result in the Parser throwing Errcode. */
class Parser {
private:
    enum State     {INIT, READY, PLAYING};
    enum Field     {CSeq, Accept, _Transport, Session};
    enum TranspArg {_UDP, _TCP, Client_port, Unicast, Interleaved};
public:
    //! Parser constructor
    Parser() : _state(INIT) {}
    //! parse incoming message and populate internal data structures. @b Overwrites buffer.
    /*! Parses messages received from a socket and populates Parser::Data. The content
        of the buffer is @b overwritten. 
    */
    Method parse(char* buffer, int buffer_size);

    //! Return parser State
    State state() const { return _state; }

    //! Return description of an error code
    static const char* errcode_desc(Errcode errcode);

    //! Return Method name
    static const char* method_name(Method method);

    //! Put all method names in a vector
    //  This is required by a reply to OPTIONS Method.
    static void get_method_names(std::vector<const char*>&);

    //! Parser outputs results populating this structure
    struct Data {
        Method      method;         //!< method, this is first field in the first line
        int         cseq;           //!< RTSP sequence number
        char*       session_id;     //!< session ID found in the request
        char*       url;            //!< URL (second field of the first line)
        //! assumes this format: rtsp://server_name/stream_name\n
        //! this is a pointer into url after the third slash
        char*       stream_name;  
        char*       accept;         //!< Accept: field of DESCRIBE message
        int         client_port0;   //!< client port 0, in SETUP
        int         client_port1;   //!< client port 1, in SETUP
        Transport   transport;      //!< UDP or TCP
        //! clear the whole Data structure
        void clear() { memset(this, 0, sizeof(Data)); }
    };
    Data            data;           //!< Parser output result
private:
    typedef std::map<const char*, Method, SBL::StrCompare>      Methods;
    typedef std::map<const char*, Field, SBL::StrCompare>       Fields;
    typedef std::map<const char*, TranspArg, SBL::StrCompare>   TranspArgs;
    typedef std::map<Errcode, const char*>                 ErrorCodes;
    struct Line {
        int word;
        int count;
        Line(int w = 0, int c = 0) : word(w), count(c) {}
    };
    State               _state;
    std::vector<char*>  _words;
    std::vector<Line>   _lines;

    const static Methods     _methods;
    const static Fields      _fields;
    const static TranspArgs  _transps;
    const static ErrorCodes  _error_codes;
    const static char*       _none;

    void    tokenize(char* buffer, int buffer_size);
    Errcode parse_field(const Line&);
    Errcode parse_transport(const Line&);
    Errcode parse_method(const Line&);

    friend std::ostream& operator<<(std::ostream& s, const Parser& p);
    friend std::istream& operator>>(std::istream& s, Parser& p);
};

}
#endif
