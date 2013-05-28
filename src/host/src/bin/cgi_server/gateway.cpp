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
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <cstring>
#include <sbl/sbl_logger.h>
#include "sbl/sbl_exception.h"
#include "gateway.h"
#include "cgi_param_set.h"
#include "sdk_manager.h"

namespace CGI {

Gateway::Gateway() : _in(0), _out(0), _err(0), _envp(0), _buffer_count(0) {
    _buffer = new char[_buffer_size];
}

Gateway::~Gateway() {
    delete[] _buffer; 
}

/* Multipart file has few header lines after boundary. This function 'reads' each line.
   After the call, _line points to the begining of the line and the line is a 
   zero-terminated string. In the input buffer, lines are terminated by '\r\n', so
   the function replace '\r' by '\0';
   This function maintains two variables: 
        - _line points to the current line
        - _next_line poins to the next line
*/
void Gateway::readline() {
    _line = _next_line;
    while (_next_line < _buffer + _buffer_count - 1 && _next_line[0] != '\r' && _next_line[1] != '\n')
        _next_line++;
    SBL_THROW_IF(!(_next_line < _buffer + _buffer_count - 1), "Buffer overrun");
    _next_line[0] = '\0';
    _next_line += 2; // point to the next line
}

/* Copy boundary string to the _boundary array */
void Gateway::get_boundary() {
    const char* boundary = get_param("CONTENT_TYPE");
    SBL_THROW_IF(!boundary, "CONTENT_TYPE is not set");
    const char* multipart = "multipart/form-data;";
    // _boundary will be empty is we are missing multipart string
    if (strncmp(boundary, multipart, strlen(multipart)))
        return;
    for (boundary += strlen(multipart); *boundary == ' '; ++boundary)
        ;   // skip whitespace
    const  char* boundary_name = "boundary=";
    SBL_THROW_IF(strncmp(boundary, boundary_name, strlen(boundary_name)), "Missing boundary string");
    boundary += strlen(boundary_name);
    strcpy(_boundary, "--");        // CONTENT_TYPE does not have initial '--'
    strcat(_boundary, boundary);
}

/* get output file path, by concatenating prefix and filename= from Content-Disposition */
void Gateway::get_filepath(const char* prefix) {
    char* filename = strstr(_line, "filename=\"");
    SBL_THROW_IF(!filename, "filename missing in Content-Disposition");
    filename += strlen("filename=\"");
    char* end_filename = strchr(filename, '"');
    SBL_THROW_IF(!end_filename, "Missing end quote in filename");
    *end_filename = '\0';
    snprintf(_file_path, sizeof _file_path, "%s/%s", prefix, filename);
}

/* save a multipart file, stripping boundaries (start and end) and any headers */
void Gateway::save_multipart_file(const char* prefix, const char* filename) {
    // Reset all to virgin state
    _boundary[0] = '\0';
    _buffer_count = 0;
    _line = _next_line = _buffer;
    _file_path[0] = '\0';
    if (filename && strlen(filename) == 0)
        filename = NULL;
    if (filename)
        snprintf(_file_path, sizeof _file_path, "%s/%s", prefix, filename);

    get_boundary(); // retrieve mulipart boundary from CONTENT_TYPE
    _buffer_count = FCGX_GetStr(_buffer, _buffer_size, _in);
    readline();
    SBL_THROW_IF(strcmp(_line, _boundary), "missing boundary in the first line of multipart file");
    do {
        readline();
        const char* content_dispo = "Content-Disposition:";
        if (!filename && !strncmp(_line, content_dispo, strlen(content_dispo)))
            get_filepath(prefix);
    } while (strlen(_line));    // headers end with an empty line
    SBL_MSG(MSG::SERVER, "Saving multipart file %s", _file_path);
    int fd  = open(_file_path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    SBL_PERROR(fd < 0);
    _file_size = _buffer_count - (_next_line - _buffer);
    SBL_PERROR(write(fd, _next_line, _file_size) != _file_size);
    int n = 0;
    do {
        n = FCGX_GetStr(_buffer, _buffer_size, _in);
        _file_size += n;
        SBL_PERROR(write(fd, _buffer, n) != n);
    } while (n == _buffer_size);
    // we should check that the trailing stuff is indeed a boundary... maybe one day....
    _file_size -= strlen(_boundary) + 6;
    ftruncate(fd, _file_size);
    close(fd);
    SBL_MSG(MSG::SERVER, "saved multi-part file %s, size %d", _file_path, _file_size);
}

bool Gateway::accept() {
    bool status = FCGX_Accept(&_in, &_out, &_err, &_envp) >= 0;
    return status;
}

const char* Gateway::command() const {
    const char* command = FCGX_GetParam("PATH_INFO", _envp);
    if (command && *command == '/')
        command++;
    return command;
}

const char* Gateway::args() const {
    return FCGX_GetParam("QUERY_STRING", _envp);
}

const char* Gateway::content_type(Type type) {
    if (type == TEXT || type == LOGFILE || type == STATUS_FILE) return "text/plain";
    if (type == JPEG) return "video/jpeg";
    if (type == HTML) return "text/html";
    if (type == RAW)  return "application/octet-stream";
    SBL_ASSERT(0);
}

/* Send out a reply */
void Gateway::reply(Type type, const char* buffer, int size, const char* reply_filename) {
    int offs = 0;
    offs += snprintf(_buffer + offs, _buffer_size - offs, "Content-Type: %s\r\n", content_type(type)); 
    offs += snprintf(_buffer + offs, _buffer_size - offs, "Content-Length: %d\r\n", size);
    if (type == JPEG) {
        time_t cur_time = time(NULL);
        struct tm t;
        localtime_r(&cur_time, &t);
        offs += snprintf(_buffer + offs, _buffer_size - offs, "Content-Disposition: attachment; filename=\"snapshot-%04d-%02d-%02dT%02d-%02d-%02d.jpg\"\r\n",
                     t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                     t.tm_hour, t.tm_min, t.tm_sec);
    } else if (type == RAW || type == LOGFILE) {
        offs += snprintf(_buffer + offs, _buffer_size - offs, "Content-Disposition: attachment; filename=\"%s\"\r\n", reply_filename);
    }
    SBL_MSG(MSG::SERVER, "Gateway reply is:\n%s", _buffer);
    offs += snprintf(_buffer + offs, _buffer_size - offs, "\r\n");
    FCGX_PutStr(_buffer, offs, _out);
    if (type == LOGFILE || type == STATUS_FILE) 
        send_file(buffer);
    else     
        FCGX_PutStr(buffer, size, _out);
}

void Gateway::send_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    CGI_ERROR(fd < 0, "Unable to open file %s for sending", filename);
    ssize_t size = read(fd, _buffer, _buffer_size);
    ssize_t total = size;
    while (size > 0) {
        FCGX_PutStr(_buffer, size, _out);
        size = read(fd, _buffer, _buffer_size);
        total += size;
    }
    close(fd);
    SBL_MSG(MSG::SERVER, "Sent logfile %s, size %d", filename, total);
    remove(filename);
}

/* save a regular (not multipart) file */
void Gateway::save_post_file(const char* dirname, const char* filename) {
    snprintf(_file_path, sizeof _file_path, "%s/%s", dirname, SDKManager::base_name(filename));
    int fd  = open(_file_path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    SBL_PERROR(fd < 0);
    int n = 0;
    _file_size = 0;
    do {
        n = FCGX_GetStr(_buffer, _buffer_size, _in);
        _file_size += n;
        SBL_PERROR(write(fd, _buffer, n) != n);
    } while (n == _buffer_size);
    close(fd);
    SBL_MSG(MSG::SERVER, "saved file %s, size %d", _file_path, _file_size);
}

void Gateway::flush() {
    FCGX_FClose(_out);
}

Gateway::Method Gateway::method() {
    if (!_envp)
        return UNKNOWN_METHOD;
    const char* method = FCGX_GetParam("REQUEST_METHOD", _envp);
    if (strcmp(method, "GET") == 0)
        return GET;
    if (strcmp(method, "POST") == 0)
        return POST;
    return UNKNOWN_METHOD;
}

}
