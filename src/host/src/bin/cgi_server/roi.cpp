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
#include <cmath>
#include <algorithm>
#include <sstream>
#include "cgi.h"
#include "roi.h"

namespace CGI {

bool Rect::intersects(const Rect& rect) const {
    return x0() <= rect.x1() && rect.x0() <= x1() 
        && y0() <= rect.y1() && rect.y0() <= y1();
}

void Roi::read_points(const char* value, std::vector<int>& points) {
    do {
        CGI_ERROR(!value || *value == ' ', "error reading points");
        const char* comma = strchr(value, ',');
        const char* dot   = strchr(value, '.');
        int range = (points.size() & 1) ? HEIGHT : WIDTH;
        if (dot && (dot < comma || !comma)) {
            float data = strtof(value, (char**) &comma);
            CGI_ERROR(data < 0.0 || data > 1.0, "invalid value %f for point coordinate", data);
            points.push_back(int(data * range));
        } else {
            int data = strtol(value, (char**) &comma, 0);
            CGI_ERROR(data < 0 || data > range, "invalid value %d for point coordinate", data);
            points.push_back(data);
        }
        CGI_ERROR(value == comma || (*comma && *comma != ','), "error reading points");
        value = comma + (*comma ? 1 : 0);
    } while (*value);
    CGI_ERROR(points.size() & 3, "number of values must be multiple of 4");
}

// read rectangles from a string, convert to integer and snap to grid
void Roi::read_rects(const char* value) {
    std::vector<int> points;
    read_points(value, points);
    for (unsigned int i = 0; i < points.size(); i += 4) {
        _rects.push_back(Rect(std::min(points[i],      points[i + 2])  & ~(ROUND - 1),
                              std::min(points[i + 1],  points[i + 3])  & ~(ROUND - 1),
                             (std::abs(points[i]     - points[i + 2]) + ROUND - 1) & ~(ROUND - 1),
                             (std::abs(points[i + 1] - points[i + 3]) + ROUND - 1) & ~(ROUND - 1)
                             ));
    }
}

void Roi::set(const char* value) {
    _rects.clear();
    read_rects(value);
    write_rects();
}

void Roi::add(const char* value) {
    read_rects(value);
    write_rects();
}

void Roi::del(const char* value) {
    Roi roi;
    roi.read_rects(value);
    for (unsigned int i = 0; i < roi.size(); i++) {
        for (std::vector<Rect>::iterator it = _rects.begin(); 
             it != _rects.end();
             it = it->intersects(roi[i]) ? _rects.erase(it) : it + 1)
            ;
    }
    write_rects();
}

void Roi::clear() {
    _rects.clear();
    write_rects();
}

void Roi::write_rects() {
    std::stringstream str;
    for (unsigned int i = 0; i < _rects.size(); i++) {
        if (i) 
            str << ',';
        str << _rects[i].x0() << ',' 
            << _rects[i].y0() << ',' 
            << _rects[i].x1() << ',' 
            << _rects[i].y1();
    }
    rectangles.set(str.str());
}

}
