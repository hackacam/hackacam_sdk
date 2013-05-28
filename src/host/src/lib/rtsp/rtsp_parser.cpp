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
#include <sbl/sbl_exception.h>
#include "rtsp_parser.h"

namespace RTSP {

const Parser::Fields Parser::_fields = SBL::CreateMap<const char*, Field, SBL::StrCompare>
    ("CSeq:",          CSeq)
    ("Accept:",        Accept)
    ("Transport:",     _Transport)
    ("Session:",       Session)
;
const Parser::Methods Parser::_methods = SBL::CreateMap<const char*, Method, SBL::StrCompare>
    ("OPTIONS",       OPTIONS)
    ("DESCRIBE",      DESCRIBE)
    ("SETUP",         SETUP)
    ("PLAY",          PLAY)
    ("GET_PARAMETER", GET_PARAMETER)
    ("TEARDOWN",      TEARDOWN)
    ("PAUSE",         TEARDOWN)  // ffmpeg rtsp client need the pause command. wired to teardown for now
;
const Parser::TranspArgs Parser::_transps = SBL::CreateMap<const char*, TranspArg, SBL::StrCompare>
    ("RTP/AVP",       _UDP)
    ("RTP/AVP/TCP",   _TCP)
    ("client_port",   Client_port)
    ("interleaved",   Interleaved)
    ("unicast",       Unicast)
;

#define def_errcode(x) ( x, #x )
const Parser::ErrorCodes Parser::_error_codes = SBL::CreateMap<Errcode, const char*>
       def_errcode( OK )
       def_errcode( BAD_REQUEST )
       def_errcode( NOT_FOUND )
       def_errcode( METHOD_NOT_ALLOWED )
       def_errcode( REQUEST_URI_TOO_LARGE )
       def_errcode( SESSION_NOT_FOUND )
       def_errcode( METHOD_NOT_VALID_IN_THIS_STATE )
       def_errcode( UNSUPPORTED_TRANSPORT )
       def_errcode( INTERNAL_SERVER_ERROR )
       def_errcode( RTSP_VERSION_NOT_SUPPORTED )
       def_errcode( ERROR_MISSING_FIELD_ARG )
       def_errcode( ERROR_FIELD_TOO_LONG )
       def_errcode( ERROR_BAD_PORT_SPEC )
       def_errcode( ERROR_BAD_INTERLEAVED_SPEC )
       def_errcode( ERROR_TCP_WITH_PORTS )
       def_errcode( ERROR_UDP_NO_PORTS )
       def_errcode( ERROR_SUPPORT_UNICAST_ONLY )
       def_errcode( ERROR_SESSION_ID_TOO_LONG )
       def_errcode( SERVER_IN_ERROR_STATE )
       def_errcode( ERROR_SUPPORT_ONLY_SDP )
       def_errcode( ERROR_MISSING_SPS )
       def_errcode( SERVER_BUFFER_OVERFLOW )
       def_errcode( SERVER_DATE_ERROR )
;
#undef def_errorcode

void Parser::get_method_names(std::vector<const char*>& names) {
    for (Methods::const_iterator n = _methods.begin(); n != _methods.end(); ++n) {
        names.push_back(n->first);
    }
}

const char* Parser::method_name(Method method) {
    for (Methods::const_iterator n = _methods.begin(); n != _methods.end(); ++n)
        if (method == n->second)
            return n->first;    
    return NULL;
}

const char* Parser::_none = "";

// find and return error code description, or NULL if can't find
const char* Parser::errcode_desc(Errcode errcode) {
    ErrorCodes::const_iterator i = _error_codes.find(errcode);
    return i == _error_codes.end() ? _none : i->second;
}


Errcode Parser::parse_field(const Line& line) {
    Fields::const_iterator field = _fields.find(_words[line.word]);
    // we ignore fields we don't understand
    if (field == _fields.end())
        return OK;
    // verify that Field does have argument
    if (line.count == 0) return ERROR_MISSING_FIELD_ARG;
    char* arg = _words[line.word + 1];
    switch (field->second) {
    case CSeq:          data.cseq = strtol(arg, 0, 0); 
                        break; 
    case Accept:        data.accept = arg;
                        break;
    case _Transport:    parse_transport(line); 
                        break;
    case Session:       data.session_id = arg;
                        break;
    }
    return OK;
}

// Transport: RTP/AVP;unicast;client_port=1422-1423
// Transport: RTP/AVP/TCP;unicast;interleaved=0-1
Errcode Parser::parse_transport(const Line& line) {
    bool unicast = false;
    for (int n = 1; n < line.count; n++) {
        char* word = _words[line.word + n];
        char* arg = strchr(word, '=');
        if (arg)
            *arg++ = 0;
        TranspArgs::const_iterator transp = _transps.find(word);
        if (transp == _transps.end())
            continue;
        switch (transp->second) {
        case _UDP: data.transport = UDP; break;
        case _TCP: data.transport = TCP; break;
        case Client_port: if (!arg)                     return ERROR_BAD_PORT_SPEC;
                          data.client_port0 = strtol(arg, &arg, 10);
                          if(!(arg && *arg == '-'))     return ERROR_BAD_PORT_SPEC;
                          arg++;
                          data.client_port1 = strtol(arg, &arg, 10);
                          if (!(arg && *arg == 0))      return ERROR_BAD_PORT_SPEC;
                          break;
        case Interleaved: if (strcmp(arg, "0-1"))       return ERROR_BAD_INTERLEAVED_SPEC;
                          if (data.transport != TCP)    return ERROR_BAD_INTERLEAVED_SPEC;
                          break;
        case Unicast:     unicast = true;
                          break;
        }
    }
    if (data.transport == UNKNOWN)                      return UNSUPPORTED_TRANSPORT;
    if (!unicast)                                       return ERROR_SUPPORT_UNICAST_ONLY;
    if (data.transport == TCP && (data.client_port0 || data.client_port1))  return ERROR_TCP_WITH_PORTS;
    if (data.transport == UDP && !(data.client_port0 && data.client_port1)) return ERROR_UDP_NO_PORTS;
    return OK;
}

// Parse message populating fields and return Method
Method Parser::parse(char* buffer, int buffer_size) {
    data.clear();
    tokenize(buffer, buffer_size);
    Errcode errcode = parse_method(_lines[0]);
    for (unsigned int line = 1; line < _lines.size(); ++line) {
        Errcode ec = parse_field(_lines[line]);
        // We always throw the first error we find
        if (errcode == OK)
            errcode = ec;
    }
    switch (_state) {
        case INIT:      if (data.method == SETUP)    _state = READY;     
                   else if (data.method == PLAY)     throw METHOD_NOT_VALID_IN_THIS_STATE;
                        break;
        case READY:     if (data.method == PLAY)     _state = PLAYING;
                   else if (data.method == TEARDOWN) _state = INIT;      break;
        case PLAYING:   if (data.method == TEARDOWN) _state = INIT;      break;
    }
    if (errcode != OK)
        throw errcode;
    return data.method;
}


// parse first line of message
Errcode Parser::parse_method(const Line& line) {
    Methods::const_iterator method = _methods.find(_words[line.word]);
    if (method == _methods.end())                   return METHOD_NOT_ALLOWED;
    data.method = method->second;
    if (line.count != 3)                            return BAD_REQUEST;
    if (strcmp(_words[line.word + 2], "RTSP/1.0"))  return RTSP_VERSION_NOT_SUPPORTED;
    data.url = _words[line.word + 1];
    int n = strlen(data.url);
    if (n && data.url[n - 1] == '/')
        data.url[n - 1] = '\0';
    // find stream name, right after top part of url, may be empty!
    char* stream_name =  strchr(data.url, '/');
    if (!stream_name++)                        return OK;
    if (*stream_name != '/')                   return OK;
    stream_name = strchr(++stream_name, '/');
    if (!stream_name++)                        return OK;
    data.stream_name = stream_name;
    return OK;
}

void Parser::tokenize(char* buffer, int buffer_size) {
    bool in_word = false;
    bool line_start = true;
    _words.clear();
    _lines.clear();
    for (int n = 0; n < buffer_size; ++n) {
        char c = buffer[n];
        if (c == ' ' || c == ';' || c == '\r' || c == '\n') {
            buffer[n] = 0;
            in_word = false;
            line_start = (c == '\n');
        } else if (!in_word) {
            if (line_start)
                _lines.push_back(Line(_words.size(), 0));
            line_start = false;
            in_word = true;
            _words.push_back(&buffer[n]);
            _lines[_lines.size() - 1].count++;
        }
    }
}

/*
    Header Field Definitions
    -------------------------
g   general request headers to be found in both requests and responses
R   request headers
r   response headers
e   entity header fields

 Header               type   support   methods
   Accept               R      opt.      entity
   Accept-Encoding      R      opt.      entity
   Accept-Language      R      opt.      all
   Allow                r      opt.      all
   Authorization        R      opt.      all
   Bandwidth            R      opt.      all
   Blocksize            R      opt.      all but OPTIONS, TEARDOWN
   Cache-Control        g      opt.      SETUP
   Conference           R      opt.      SETUP
   Connection           g      req.      all
   Content-Base         e      opt.      entity
   Content-Encoding     e      req.      SET_PARAMETER
   Content-Encoding     e      req.      DESCRIBE, ANNOUNCE
   Content-Language     e      req.      DESCRIBE, ANNOUNCE
   Content-Length       e      req.      SET_PARAMETER, ANNOUNCE
   Content-Length       e      req.      entity
   Content-Location     e      opt.      entity
   Content-Type         e      req.      SET_PARAMETER, ANNOUNCE
   Content-Type         r      req.      entity
   CSeq                 g      req.      all
   Date                 g      opt.      all
   Expires              e      opt.      DESCRIBE, ANNOUNCE
   From                 R      opt.      all
   If-Modified-Since    R      opt.      DESCRIBE, SETUP
   Last-Modified        e      opt.      entity
   Proxy-Authenticate
   Proxy-Require        R      req.      all
   Public               r      opt.      all
   Range                R      opt.      PLAY, PAUSE, RECORD
   Range                r      opt.      PLAY, PAUSE, RECORD
   Referer              R      opt.      all
   Require              R      req.      all
   Retry-After          r      opt.      all
   RTP-Info             r      req.      PLAY
   Scale                Rr     opt.      PLAY, RECORD
   Session              Rr     req.      all but SETUP, OPTIONS
   Server               r      opt.      all
   Speed                Rr     opt.      PLAY
   Transport            Rr     req.      SETUP
   Unsupported          r      req.      all
   User-Agent           R      opt.      all
   Via                  g      opt.      all
   WWW-Authenticate     r      opt.      all

*/

std::ostream& operator<<(std::ostream& s, const Parser& p) {
    const char eol = '\n';
    const char* none = "----";
    s << "method:       "  << Parser::method_name(p.data.method) << eol
      << "cseq:         "  << p.data.cseq << eol
      << "session_id:   "  << (p.data.session_id ? p.data.session_id : none ) << eol
      << "url:          "  << (p.data.url ? p.data.url : none) << eol
      << "stream_name:  "  << (p.data.stream_name ? p.data.stream_name : none) << eol
      << "accept:       "  << (p.data.accept ? p.data.accept : none) << eol
      << "client_port0: "  << p.data.client_port0 << eol
      << "client_port1: "  << p.data.client_port1 << eol
      << "transport:    "  << p.data.transport << eol
      << "state:        "  << p._state  << eol
                           << "####" << std::endl;
    return s;
}

static char* new_string(std::istream& s) {
    std::string value;
    s >> value;
    char* data = new char[value.size() + 1];
    strcpy(data, value.c_str());
    return data;
}
template<typename T>
static T new_t(std::istream& s) {
    int v;
    s >> v;
    return static_cast<T>(v);
}

std::istream& operator>>(std::istream& s, Parser& p) {
    p.data.clear();
    while (!s.eof()) {
        std::string key;
        s >> key;
        if (key.size() == 0 || key[0] == '#') 
            return getline(s, key);        // to read until end of line
        if (key == "method:") { 
            std::string value; 
            s >> value; 
            Parser::Methods::const_iterator it = Parser::_methods.find(value.c_str());
            if (it == Parser::_methods.end())
                SBL_THROW("Unrecognized method %s", value.c_str());
            p.data.method = it->second; 
        }
        else if (key == "cseq:")            s >> p.data.cseq;
        else if (key == "session_id:")      p.data.session_id = new_string(s);
        else if (key == "url:")             p.data.url = new_string(s);
        else if (key == "stream_name:")     p.data.stream_name = new_string(s);
        else if (key == "accept:")          p.data.accept = new_string(s);
        else if (key == "client_port0:")    s >> p.data.client_port0;
        else if (key == "client_port1:")    s >> p.data.client_port1;
        else if (key == "transport:")       p.data.transport = new_t<Transport>(s);
        else if (key == "state:")           p._state = new_t<Parser::State>(s); 
        else SBL_THROW("Unrecognized data field %s", key.c_str());
    }
    return s;
}

}
