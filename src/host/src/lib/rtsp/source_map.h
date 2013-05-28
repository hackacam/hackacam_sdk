#pragma once
#ifndef _SOURCE_MAP_H
#define _SOURCE_MAP_H
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

#include <map>
#include <vector>
#include "rtsp_impl.h"

namespace RTSP {
class Source;

/// Manages sources for a server.
/** This class maps source names and ids to sources. It is instantiated only in master server
*/
class SourceMap {
private:
    typedef std::map<const char*, Source*, SBL::StrCompare>  Map;
    typedef std::vector<Source*> Vector;
public:
    /// SourceMap iterator
    typedef Map::iterator Iterator;
    /// Find a source given its name, return NULL if not found
    Source* find(const char* name);
    /// Find a source given its name, return NULL if not found
    Source* find(const unsigned int id);
    /// Save a name for the source
    void save(const char* name, Source* source);
    /// Save an id for the source (and optionally name)
    void save(const unsigned int id, Source* source, const char* name = NULL);
    /// Delete a source
    void erase(const char* name);
    /// STL iterator to source map begin
    Iterator begin() { return _map.begin(); }
    /// STL iterator to source map end
    Iterator end()   { return _map.end();   }
private:
    Map     _map;
    Vector  _vector;
};

}
#endif
