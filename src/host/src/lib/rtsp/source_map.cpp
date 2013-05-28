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
#include "rtsp_impl.h"
#include "source_map.h"

namespace RTSP {

Source* SourceMap::find(const char* name) {
    Map::iterator it = _map.find(name);
    Source* source = it == _map.end() ? NULL : it->second;
    SBL_MSG(MSG::SOURCE_MAP, "Source map %p, found source %p for name '%s'", this, source, name);
    return source;
}

void SourceMap::save(const char* name, Source* source) {
    _map[strdup(name)] = source;
    SBL_MSG(MSG::SOURCE_MAP, "Source map %p, saving source %p with name '%s'", this, source, name);
}

void SourceMap::erase(const char* name) {
    Map::iterator it = _map.find(name);
    if (it == _map.end()) 
        SBL_WARN("Attempting to erase non-existing source %s", name);
    else {
        _map.erase(name);
        delete[] it->first;
        SBL_MSG(MSG::SOURCE_MAP, "Source map %p, erasing name '%s'", this, name);
    }
}

Source* SourceMap::find(const unsigned int id) {
    Source* source = id >= _vector.size() ? NULL : _vector[id];
    SBL_MSG(MSG::SOURCE_MAP, "Source map %p, found source %p for id %d", this, source, id);
    return source;
}

void SourceMap::save(const unsigned int id, Source* source, const char* stream_name) {
    if (id >= _vector.size())
        _vector.resize(id + 1, NULL);
    _vector[id] = source;
    char source_number[4];
    if (stream_name == NULL) {
        snprintf(source_number, sizeof source_number, "%d", id);
        stream_name = source_number;
    }
    save(stream_name, source);
    SBL_MSG(MSG::SOURCE_MAP, "Source map %p, saving source %p with id %d", this, source, id);
}
}
