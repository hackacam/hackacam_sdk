#ifndef __STRETCH_VERSION_H__
#define __STRETCH_VERSION_H__

/***************************************************************************
Copyright (c) 2013 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/

/***************************************************************************
 * This is the central version configuration file.  It should be included
 * by each of the Stretch components that track version information.  It
 * specifies the major, minor, and revision versions across all of the
 * Stretch components.  Each component is free to manage its own build
 * version, but it is strongly suggested that the build version be
 * represented by the latest Perforce changelist number.
***************************************************************************/
#ifndef P4_CL
#define P4_CL 0
#endif
#define STRETCH_MAJOR_VERSION       7
#define STRETCH_MINOR_VERSION       10
#define STRETCH_REVISION_VERSION    0
#define STRETCH_BUILD_VERSION       P4_CL

#define STRINGIFY(s) _STRINGIFY(s)
#define _STRINGIFY(s) #s

#define STRETCH_VERSION_STRING STRINGIFY(STRETCH_MAJOR_VERSION)    "." \
                               STRINGIFY(STRETCH_MINOR_VERSION)    "." \
                               STRINGIFY(STRETCH_REVISION_VERSION) "." \
                               STRINGIFY(STRETCH_BUILD_VERSION)

#endif /* __VERSION_H__ */
