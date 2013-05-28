#pragma once
#ifndef _CGI_GATEWAY_H
#define _CGI_GATEWAY_H
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
#include <fcgi/fcgiapp.h>

namespace CGI {

//! Handles are interactions with fcgi library
class Gateway {
public:
    enum Type {TEXT, JPEG, HTML, RAW, LOGFILE, STATUS_FILE}; //!< Type of the output
    enum Method { UNKNOWN_METHOD, GET, POST };  //!< HTML method used (CGI server doesn't use PUT
    Gateway();                                  //!< Allocates _buffer
    bool        accept();                       //!< Wrapper around FCGX_Accept
    const char* command() const;                //!< Current command
    const char* args() const;                   //!< Arguments to the current command
    //! Send out a reply of a given type
    void        reply(Type type, const char* buffer, int size, const char* reply_filename);
    //! Save data from input to a filename
    void        save_post_file(const char* dirname, const char* filename);
    //! Save a multipart file, while removing boundary and headers (supports only one part)
    void        save_multipart_file(const char* dirname, const char* filename = 0);
    //! Save a post file or a multipart file
    void        save_file(bool multipart, const char* dirname, const char* filename = 0) {
        if (multipart) save_multipart_file(dirname, filename);
                 else  save_post_file(dirname, filename);
    }
    //! Get path of last saved file
    const char* filepath()  const { return _file_path; }
    //! Get the size of the last saved file
    int         file_size() const { return _file_size; }
    //! This doesn't seem to work well with fcgi
    void        flush();
    //! The current method
    Method      method();
    //! return a parameter with the given name
    const char* get_param(const char* param_name) { return FCGX_GetParam(param_name, _envp); }
    //! Translate type enum to a string
    static const char*  content_type(Type type);
    //! This is normally never called, as CGI server doesn't return
    ~Gateway();
private:
    FCGX_Stream*        _in;
    FCGX_Stream*        _out; 
    FCGX_Stream*        _err;
    FCGX_ParamArray     _envp;

    // All variables below are only used by save_multipart_file()
    static const int    _buffer_size = 64 * 1024;
    char*               _buffer;
    int                 _buffer_count;
    char*               _line;        // current line
    char*               _next_line;   // next line
    int                 _file_size;   // size of  the last file

    char                _boundary[80];      // boundary is actually 72 chars per standard
    char                _file_path[128];    // output file path

    void get_filepath(const char* prefix);
    void get_boundary();
    void readline();
    void send_file(const char* filename);
};

}
#endif
