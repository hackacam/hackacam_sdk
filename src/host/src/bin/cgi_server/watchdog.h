#pragma once
#ifndef _CGI_WATCHDOG_H
#define _CGI_WATCHDOG_H
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
#include <queue>
#include <time.h>
#include <sbl/sbl_thread.h>

namespace CGI {

class Server;

class Watchdog : public SBL::Thread {
public:
	Watchdog(Server* server, int fail_limit);
    float ave_fps() const { return _ave_fps; }
    void  enable(bool enable);
private:
    Server* _server;
    int     _fail_count;
    float   _ave_fps;
    int     _fail_limit;
    bool    _reboot;
    static const float FAIL_THRESH = 0.2;

    enum {WATCHDOG_SLEEP = 1, SDK_WATCHDOG_TIMEOUT = 6000, FPS_FILTER_SIZE = 10}; 
    void start_thread();
    float expected_frames();
    float compute_ave_fps();

    struct DataPoint {
        unsigned int      frame_count;
        struct timespec   time;
        DataPoint() : frame_count(0) {
            time.tv_sec = 0;
            time.tv_nsec = 0;
        }
    };
    DataPoint           _points[FPS_FILTER_SIZE + 1];
};


}

#endif
