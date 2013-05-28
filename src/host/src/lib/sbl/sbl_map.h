#pragma once
#ifndef _SBL_MAP_H
#define _SBL_MAP_H
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
#include <cstring>
#include <map>

/// @file sbl_map.h

/// Stretch Base Library
namespace SBL {

/// Little helper class to initialize maps
/** Usage example: @verbatim
map<const char*, int, StrCompare> my_map = CreateMap<const char*, int, StrCompare>
    ("Zero",       0)
    ("One",        1)
    ("Two",        2)
    ("Three",      3)
; 
@endverbatim */
template <typename T, typename U, typename C = std::less<T> >
class CreateMap {
private:
    std::map<T, U, C> _map;
public:
    CreateMap(const T& key, const U& val) {
        _map[key] = val;
    }
    CreateMap<T, U, C>& operator()(const T& key, const U& val) {
        _map[key] = val;
        return *this;
    }
    operator std::map<T, U, C>() {
        return _map;
    }
};

/// Functor providing comparison operator for const char*
/** This class is necessary any time map with K=const char* is used */
struct StrCompare : public std::binary_function<const char*, const char*, bool> {
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

}

#endif
