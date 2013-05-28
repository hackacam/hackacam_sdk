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
#include "watchdog.h"
#include "cgi_param_set.h"
#include "cgi_server.h"
#include "sdk_manager.h"

namespace CGI {

Watchdog::Watchdog(Server* server, int fail_limit) : 
                  _server(server), _fail_count(0), _fail_limit(fail_limit), _reboot(true) {
    memset(&_points[0], 0, sizeof _points);
}

void Watchdog::enable(bool enable) {
    if (_fail_limit)
        _server->sdk_manager()->enable_watchdog(enable && _reboot, SDK_WATCHDOG_TIMEOUT);
}

void Watchdog::start_thread() {
    if (_fail_limit == 0) {
        SBL_INFO("Watchdog disabled, watchdog thread exiting...");
        return;
    }
    if (_fail_limit < 0) {
        SBL_WARN("Watchdog will not reboot, only exit the application");
        _fail_limit = -_fail_limit;
        _reboot = false;
    }
    SBL_INFO("Starting watchdog thread");
    SDKManager* sdk = _server->sdk_manager();
    _points[0].frame_count = sdk->frame_count();
    clock_gettime(CLOCK_MONOTONIC, &_points[0].time); 
    if (_reboot) // dont take over the watchdog if _reboot is false
        _server->sdk_manager()->enable_watchdog(true, SDK_WATCHDOG_TIMEOUT);
    do {
        try {
            sleep(WATCHDOG_SLEEP);
            // we need to wait here, server may be executing flashing firmware and
            // we don't want to fire a watchdog at that time
            // Note that server calls watchdog.enable(false) before starting flashing.
            _server->lock();

            memmove(&_points[1], &_points[0], sizeof(DataPoint) * FPS_FILTER_SIZE);
            // even if we are interrupted after clock_gettime, we are counting frames after that so it is safe.
            clock_gettime(CLOCK_MONOTONIC, &_points[0].time); 
            _points[0].frame_count = sdk->frame_count();
          
            unsigned int actual = _points[0].frame_count - _points[1].frame_count;
            float expected  = expected_frames();
            if (actual < FAIL_THRESH * expected) {
                if (++_fail_count >= _fail_limit) {
                    _server->write_status_file(Server::WATCHDOG_REBOOT, "watchdog expected %f frames, got %d in %d tries, %s", 
                                            expected, actual, _fail_count, _reboot? "rebooting" : "exiting" );
                    // if _reboot is true, then reboot the camera, otherwise just exit application
                    if (_reboot && reboot(RB_AUTOBOOT) != 0)
                            SBL_ERROR("unable to reboot the system");
                    exit(Server::WATCHDOG_REBOOT);
                }
            } else 
                _fail_count = 0;
            SBL_MSG(MSG::WATCHDOG, "got %d frames, expected %.1f, %d failures, ave fps %.1f", actual, expected, _fail_count, compute_ave_fps());
            if (_reboot)    // if _reboot is false, we didn't take over watchdog, so we should not send refreshes.
                sdk->refresh_watchdog(SDK_WATCHDOG_TIMEOUT);
        } catch (Exception& ex) {
            _server->write_status_file(Server::WATCHDOG_REBOOT, "watchdog refresh failed with %s", ex.what());
            if (reboot(RB_AUTOBOOT) != 0) {
                SBL_ERROR("unable to reboot the system");
            }
        }
        _server->unlock();
    } while (true);
}

float Watchdog::compute_ave_fps() {
    float seconds = (_points[0].time.tv_sec  - _points[FPS_FILTER_SIZE].time.tv_sec) + 
                    (_points[0].time.tv_nsec - _points[FPS_FILTER_SIZE].time.tv_nsec) * 1.0e-9;
    _ave_fps = seconds > 0.0 ? (_points[0].frame_count - _points[FPS_FILTER_SIZE].frame_count) / seconds : 0.0;
    return _ave_fps;
}


float Watchdog::expected_frames() {
    float seconds = (_points[0].time.tv_sec  - _points[1].time.tv_sec) + 
                    (_points[0].time.tv_nsec - _points[1].time.tv_nsec) * 1.0e-9;
    float frames = 0;
    for (int i = 0; i < Stream::COUNT; ++i) {
        if (_server->stream(i).enable) {
            int fps = _server->stream(i).fps;
            frames += fps * seconds;
        }
    }
    return frames;
}

}
