#pragma once
#ifndef _CGI_ROI_H
#define _CGI_ROI_H
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

#include <string>
#include <vector>
#include <sbl/sbl_param_set.h>

namespace CGI {

class Rect {
public:
    Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
         _x(x), _y(y), _w(w), _h(h) {}

    bool intersects(const Rect& rect) const;

    unsigned int x() const { return _x; }
    unsigned int y() const { return _y; }
    unsigned int w() const { return _w; }
    unsigned int h() const { return _h; }

    unsigned int x0() const { return _x; }
    unsigned int y0() const { return _y; }
    unsigned int x1() const { return _x + _w; }
    unsigned int y1() const { return _y + _h; }
private:
    unsigned int _x;
    unsigned int _y;
    unsigned int _w;
    unsigned int _h;
};

class Roi : public SBL::ParamSet {
public:
    enum {COUNT = 5, ROUND = 32, WIDTH = 1920, HEIGHT = 1080};
    SBL::Param<int>          id;
    SBL::Param<std::string>  rectangles;

    Roi() : ParamSet("roi"),
            id(this, "id", 0, SBL::VerifyRange(0, COUNT - 1)),
            rectangles(this, "rectangles", "")
            {}
    void set(const char* value);
    void add(const char* value);
    void del(const char* value);
    void clear();
    const Rect& operator[](int index) const { return _rects[index]; }
    size_t size() const { return _rects.size(); }
private:

    static void read_points(const char* value, std::vector<int>& points);
    void read_rects(const char* value);
    void write_rects();

    std::vector<Rect>   _rects;
};

}

#endif
