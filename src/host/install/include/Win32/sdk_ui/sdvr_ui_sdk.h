/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/
#ifndef SDVR_UI_SDK_H
#define SDVR_UI_SDK_H

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif
#include "sdk/sdvr_sdk.h"

/****************************************************************************
  PACKAGE: Stretch DVR Display SDK (sdvr_ui_sdk)

  DESCRIPTION:

  SECTION: Include
  {
  #include "sdvr_ui_sdk.h"
  }

  SECTION: Introduction
  In conjunction with the Stretch DVR SDK, there is a UI SDK library that lets you
  display raw video frames on the display monitor and play raw audio frames.

  This is an optional library that can be added to your Stretch DVR Application
  development environment.  If you are developing an embedded  DVR
  Application or choose to use a different
  display/audio API, you may choose not to include this library.

  This document describes the Application Programming Interface (API)
  implemented in the UI SDK.

  The SDK provides the ability to:

    Display the specified raw video frame into one or more regions
    within a given window.

    Display a raw video frame in a rectangle within a display window.

    Clear the contents of a rectangle within a display window.

  SECTION: Linking with UI SDK API on the MS Window Platform
  This library uses DirectX functions to draw directly into the
  video buffer and play sound. As a result, ddraw.lib, dsound.lib, and dxguid.lib must be added to 
  your development environment.
  (ddraw.lib and ddraw.dll are shipped as part of the MSVC++ SDK.) Additionally, you need to
  add "<program files>/Microsoft DirectX SDK (February 2010)\Lib\x86" to your library path and
  "<program files>/Microsoft DirectX SDK (February 2010)\Include" to your include path.
  Lastly, make sure WIN32 is added to your C/C++ preprocessor definitions.

  You can link with the UI SDK API statically or dynamically.

  1) Statically Linking - You must add sdvr_sdk.lib and sdvr_ui_sdk.lib to your
  library dependecies.

  2) Dynamically Linking - To do so, you must add sdvr_ui_sdk_dll.lib
  and sdvr_sdk_dll.lib to your library dependecies. Additionally, you must
  include sdvr_ui_sdk_dll.dll and sdvr_sdk_dll.dll in the same directory as
  your DVR Application.

  SECTION: Linking with UI SDK API on the Linux Platform
  This library requires an X Server that supports Xv extensions
  to display live video. As a result, in addition to sdvr_sdk.lib and sdvr_ui_sdk.lib,
  you must include the following libraries to your  development environment:

  1) CentOS 5 - libXv-devel
  2) Ubuntau 7.10 - libxv-dev

  SECTION: Using the UI SDK API to preview raw video frames
  This section provides the steps required to display raw video frames on
  the display screen.

    You must provide the SDK the largest video frame size that would ever be
    displayed. This size is passed as a parameter to sdvr_ui_init(). This function
    must be called before any of the display functions can be called and should only
    be called once within your application.

    The UI-SDK has three different modes to display raw video frames on the host display.

      + SDVR_UI_PREVIEW_MODE_1 - This mode draws video frames directly in the 
      windows's DC.

      + SDVR_UI_PREVIEW_MODE_2 and SDVR_UI_PREVIEW_MODE_3 - These two modes are only supported in MS Windows OS.
      In these modes the video frames are displayed using overlays instead of direct draw in the 
      windows's device context (DC). This allows the host application to draw text
      or bitmaps on the window without them  being overwritten by the video frames as
      they get displayed on the same window.

    Prior using the UI-SDK, you must specify the preview mode. Calling sdvr_ui_init(),
    uses SDVR_UI_PREVIEW_MODE_1. You can call sdvr_ui_init_ex() to choose different
    modes.

    SDVR_UI_PREVIEW_MODE_1:  There are two different methods that you can choose in order to display raw
    video frames in this mode. In this mode, the color key defines one specific color that should not
    be drawn to the  render surface.  After initializing the UI SDK, you can specify the color key by calling
    sdvr_ui_set_key_color().  This is an optional step in this mode. The SDK uses white
    as the default color key.

        Method 1: This method is used if you don't need to access the YUV buffers to perform any video
        analytics, save into a file, or any other operations on raw video frames. To
        start raw video display on a specific region within a window, call
        sdvr_ui_start_video_preview(). To stop raw video display, call
        sdvr_ui_stop_video_preview(). In cases where you are interested in displaying
        the raw video frames from one channel in different regions within a preview window,
        you should call sdvr_ui_start_video_preview() multiple times for each region.
        After sdvr_ui_start_video_preview() is called for a specific
        encoder or decoder channel, the SDK starts displaying any raw video frame received
        from the DVR board into all the regions within the given window handle. You direct
        the SDK to stop displaying raw video for each region by calling sdvr_ui_stop_video_preview().
        It is also possible to get notification with every video frame by setting a
        preview callback. With the preview callback you can put additional graphics
        over the video, such as highlighting some regions, and so on. To set the preview
        callback call sdvr_ui_set_preview_callback().

        Method 2: This method is used if you need to have more control over displaying the video frames.
        To display YUV frames, get the YUV buffers by calling sdvr_get_av_buffer().
        In general, you can either poll this function or wait to be called back in your
        video frame callback. After you acquire the YUV buffer, set the
        buffer by calling sdvr_ui_set_buffer() for displaying and then release it using
        sdvr_release_av_buffer(). After a YUV buffer is
        set to be displayed, you can call sdvr_ui_draw_yuv() to draw the buffer
        into a specific region within a display window. There is no need to call
        sdvr_ui_set_buffer() multiple times to display the same
        video frame in different regions of the screen.

        Under Linux, It is also possible to get notification  with every video frame by setting a
        preview callback. With the preview callback you can put additional graphics
        over the video, such as highlighting some regions, and so on. To set the preview
        callback call sdvr_ui_set_preview_callback(). sdvr_ui_set_preview_callback() is only supported in Linux.
        In windows, user application can draw directly in the video window, the overlay video will only show up in
        the key color area.


    SDVR_UI_PREVIEW_MODE_2 - This mode in only supported in the MS Windows operating
    system. It only provides video frame displays equivalent to method 2 of mode 1.
    The color key defines one specific color that is used as key color for overlay display
    The video window needs to use key color as background color.
    You can specify the color key by calling
    sdvr_ui_set_key_color().  This key color must be exactly the same as the background
    color used for the window where the video is going to be place. No video will be displayed
    if the key color and window's background color don't match. The SDK uses white
    as the default color key.

        To start raw video display on a specific region within a window, call
        sdvr_ui_start_video_preview(). To stop raw video display, call
        sdvr_ui_stop_video_preview(). In cases where you are interested in displaying
        the raw video frames from one channel in different regions within a preview window,
        you should call sdvr_ui_start_video_preview() multiple times for each region.
        Once sdvr_ui_start_video_preview() is called for a specific
        encoder or decoder channel, the SDK starts displaying any raw video frame received
        from the DVR board into all the regions within the given window handle. You direct
        the SDK to stop displaying raw video for each region by calling sdvr_ui_stop_video_preview().
        
    SDVR_UI_PREVIEW_MODE_3 - This mode is similar to SDVR_UI_PREVIEW_MODE_2 but is recommended to be
    used in Windows 7 environment specially if Aero desktop theme is selected. See
    sdvr_ui_set_preview_callback() for an explanation of how preview callbacks are different in this
    mode verses the other modes.

  SECTION: Using the UI SDK API to play raw video frames
  This section provides the steps required to play raw audio frames on
  the Host PC.

    After calling sdvr_ui_init(), if you intend to use any of the audio preview
    APIs, you must call sdvr_ui_init_audio() to specify the sample audio rate, this
    is SDVR_AUDIO_RATE_8KHZ for most of the DVR boards, and whether the audio
    samples are mono (channel equal to one (1)) or stereo (channel equal to two (2)).
    Additionally, you need to specify the window handle to your DVR Application.

    After audio is initialized, you can set the audio volume at any time by
    calling sdvr_ui_adjust_volume().

    To start raw audio preview for a camera or decoded channel, call
    sdvr_ui_start_audio_preview(). To stop raw audio display, call
    sdvr_ui_stop_video_preview(). 
    Once sdvr_ui_start_audio_preview() is called for a specific
    encoder or decoder channel, the SDK starts playing of raw audio frames 
    as it receives them.
    
  SECTION: Important Restrictions
  Note the following restrictions when using the UI SDK. These
  restrictions, apart from those explicitly noted, are not permanent
  and will be removed in future versions of the SDK.
    

    There are only four display regions per channel
    when using sdvr_ui_start_video_preview() to display raw video frames.

    Dynamic linking of the display and board SDK libraries is only
    available in the MS Windows environment.

    In Windows, DirectDraw acceleration is required to be enabled on your video card.

    Audio preview is only support in the MS Windows environment.

    SDVR_UI_PREVIEW_MODE_2 is not supported in conjunction 
    with aero theme in Microsoft Windows 7.

*****************************************************************************/

/****************************************************************************
  VISIBLE: Typedef for the errors returned by the UI SDK.

    SDVR_ERR_UI_INIT - Error code for failure to initialize the video or
    audio preview.

    SDVR_ERR_UI_INVALID_PARAM - Error code if the any of the parameters
    passed to any of the SDVR UI functions is invalid.

    SDVR_ERR_UI_SET_BUFFER - Error code for failure to lock the draw surface.

    SDVR_ERR_UI_DRAW_YUV - Error code for failure to draw the surface.

    SDVR_ERR_UI_INVALID_YUV_BUF - Error code if the given YUV buffer is
    not valid.

    SDVR_ERR_UI_VIDEO_SIZE - Error code if the YUV buffer size is larger
    than the maximum supported video frame size.

    SDVR_ERR_UI_NO_BUFFER - Error code if no buffer was set.

    SDVR_ERR_UI_MAX_PREVIEW_REGIONS - Error code if no more preview regions
    can be added.

    SDVR_ERR_UI_DRAWINFO_FULL - Error code if the maximum number of preview
    channels is reached.

    SDVR_ERR_UI_CHANNEL_NOT_START - Error code if the preview was not started
    for the specified channel handle.

    SDVR_ERR_UI_NO_INIT - Error code if sdvr_ui_init() was not called prior
    calling a video preview API or sdvr_ui_init_audio() was not called prior to
    an audio preview API.

    SDVR_ERR_UI_MEMORY - Error code if failed to allocate enough memory for
    the internal data structures.

    SDVR_ERR_UI_AUDIO_PREVIEW - Error code on failure to playback the audio.

    SDVR_ERR_UI_UNSUPPORTED - Error code if the API is not currently supported.
****************************************************************************/
typedef enum _sdvr_err_ui_e {
    SDVR_ERR_UI_INIT = 3000,
    SDVR_ERR_UI_INVALID_PARAM,
    SDVR_ERR_UI_SET_BUFFER,
    SDVR_ERR_UI_DRAW_YUV,
    SDVR_ERR_UI_INVALID_YUV_BUF,
    SDVR_ERR_UI_VIDEO_SIZE,
    SDVR_ERR_UI_NO_BUFFER,
    SDVR_ERR_UI_MAX_PREVIEW_REGIONS,
    SDVR_ERR_UI_DRAWINFO_FULL,
    SDVR_ERR_UI_CHANNEL_NOT_START,
    SDVR_ERR_UI_NO_INIT,
    SDVR_ERR_UI_MEMORY,
    SDVR_ERR_UI_AUDIO_PREVIEW,
    SDVR_ERR_UI_UNSUPPORTED

} sdvr_err_ui_e;
/****************************************************************************
  SDVR_UI_ERR_NONE - Error coder if no error.
****************************************************************************/
#define SDVR_UI_ERR_NONE  ((sdvr_err_ui_e)SDVR_ERR_NONE)


/************************************************************************
    VISIBLE: There are two different modes from which you
    can choose to display video frame on the host monitor.

    SDVR_UI_PREVIEW_MODE_1 - This mode draws video frames directly in the 
    windows's DC.

    SDVR_UI_PREVIEW_MODE_2 - This mode is only supported in MS Windows OS.
    It displays the video using overlays instead of direct draw in the 
    windows's device context (DC). This allows the host application to draw text
    or icons on the window without being overwritten by the video frames as
    they get displayed on the same window.

    SDVR_UI_PREVIEW_MODE_3 - This mode is similar to SDVR_UI_PREVIEW_MODE_2 but 
    is recommended to be used in Windows 7 environment specially if Aero 
    desktop theme is selected.

************************************************************************/
typedef enum _sdvr_ui_video_preview_mode {
    SDVR_UI_PREVIEW_MODE_1 = 1,
    SDVR_UI_PREVIEW_MODE_2,
	SDVR_UI_PREVIEW_MODE_3,
	SDVR_UI_PREVIEW_MODE_MAX
} sdvr_ui_video_preview_mode;

/************************************************************************
    VISIBLE: This data structure is used to initialize the UI-SDK.

        @mode@ - The video display method. see sdvr_ui_video_preview_mode for detailed
        description.

        @max_width@ - The maximum width video frame that will ever be displayed.
        This field is ignore in SDVR_UI_PREVIEW_MODE_2.

        @max_lines@ - The maximum number of lines in a video frame that will ever
        be displayed.  This field is ignore in SDVR_UI_PREVIEW_MODE_2.

    Remarks:

        You must initialize this data structure to zero prior using it.
************************************************************************/
typedef struct _sdvr_ui_init_def_t {
    sdvr_ui_video_preview_mode mode;
    sx_uint32 max_width;
    sx_uint32 max_lines;
    sx_uint32 reserve[20];
} sdvr_ui_init_def_t;

/************************************************************************
    VISIBLE: It is possible to display raw video frames for each encoder
    or decoder channel in multiple regions within a given window handle.
    SDVR_MAX_DRAW_REGIONS defines the maximum number of display regions
    that can be added to the display preview regions.
************************************************************************/
#define SDVR_MAX_DRAW_REGIONS   4

/************************************************************************
    VISIBLE: This data structure is used to define a display region.
    A display region is the area within a window in which the video frame
    is displayed. The top left corner of a window is specified by (0,0).
    Each window can include many of such regions.

        @top_left_x@ - X-coordinate of the upper left corner.

        @top_left_y@ - Y-coordinate of the upper left corner.

        @width@ - The width of the display region within the given window.

        @height@ - The height of the display region  within the given window.
************************************************************************/
typedef struct _sdvr_ui_region_t {
    sx_uint32 top_left_x;
    sx_uint32 top_left_y;
    sx_uint32 width;
    sx_uint32 height;
} sdvr_ui_region_t;

/************************************************************************
    VISIBLE: The window handle for where to display the raw video frames. In
    the MS Windows environment  this is HWND.
************************************************************************/
#ifdef WIN32
typedef HWND sdvr_ui_hwnd_t;
#else
typedef Drawable sdvr_ui_hwnd_t;
#endif

/************************************************************************
    VISIBLE: The color key defines one specific color, for example white,
    which should not be drawn to the render surface. The color key usually
    is defined by the RGB macro.
************************************************************************/
#ifdef WIN32
typedef DWORD sdvr_ui_color_key_t;
#else
typedef unsigned long sdvr_ui_color_key_t;
#endif

/************************************************************************
    VISIBLE: A macro to construct an RGB color scheme to RGB 24 bit.
************************************************************************/
#define sdvr_rgb(r,g,b)    (((unsigned long)(unsigned char)(r))|(((unsigned long)(unsigned char)(g)) << 8)|(((unsigned long)(unsigned char)(b)) << 16))

/************************************************************************
    VISIBLE: Preview callback function type.
************************************************************************/
#ifdef WIN32
typedef void (*sdvr_ui_preview_cb)(HDC);
#else
typedef void (*sdvr_ui_preview_cb)(Display *dpy, Drawable d);
#endif

/* Pen Styles */
/************************************************************************
    VISIBLE: The line style to use to draw a frame around a
    region.

    @SDVR_UI_LS_SOLID@ - Use solid lines.  ____

    @SDVR_UI_LS_DASH@ - Use dashed lines.  -----

    @SDVR_UI_LS_DOT@ - Use dotted lines.   .....
************************************************************************/
#define SDVR_UI_LS_SOLID            0       /* _______  */
#define SDVR_UI_LS_DASH             1       /* -------  */
#define SDVR_UI_LS_DOT              2       /* .......  */


/************************************************************************
    The function value to be used with sdvr_ui_internal_setup()

    @SDVR_UI_DIRECTDRAW_NOLOCK@ - Internal switch to turn off directdraw
    surface lock. This can improve the display performance but it is only
    compatible with certain graphic cards (Intel integrated video cards
    are the only ones tested). It is not recommended to turn this feature
    on in normal use.
************************************************************************/
#define SDVR_UI_DIRECTDRAW_NOLOCK	1  

/****************************************************************************
  GROUP: Internal
****************************************************************************/
/*******************************************************************************
   A necessary evil introduced for C++ compatibility.  C source files must
   not declare a function "extern"; instead, they must declare the function
   "EXTERN".  For example:
   {
       EXTERN void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#if defined(__cplusplus)
   #define EXTERN              extern "C"
#else
   #define EXTERN              extern
#endif
#define sdvr_ui_set_yuv_buffer(yuv_buffer) sdvr_ui_set_buffer(yuv_buffer)

/****************************************************************************
  GROUP: The Video Preview APIs
****************************************************************************/

EXTERN sdvr_err_ui_e sdvr_ui_init(sx_uint32 max_width, sx_uint32 max_lines);
EXTERN sdvr_err_ui_e sdvr_ui_init_ex(sdvr_ui_init_def_t *ui_init_def);

EXTERN sdvr_err_ui_e sdvr_ui_init_audio( sdvr_ui_hwnd_t wnd_handle,
                                      sdvr_audio_rate_e sample_rate,
                                      sx_int32 channels);
EXTERN sdvr_err_ui_e sdvr_ui_close();
EXTERN void sdvr_ui_set_key_color( sdvr_ui_color_key_t color_key);
//+1
EXTERN sdvr_err_ui_e sdvr_ui_set_buffer(sdvr_av_buffer_t *raw_vbuffer);
EXTERN sdvr_err_ui_e sdvr_ui_draw_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region );
EXTERN sdvr_err_ui_e sdvr_ui_draw_frame(sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t region, sdvr_ui_color_key_t rgb_color, int line_style);
EXTERN sdvr_err_ui_e sdvr_ui_refresh();
//-1
EXTERN sdvr_err_ui_e sdvr_ui_clear_yuv( sdvr_ui_hwnd_t hwnd, sdvr_ui_region_t *region);
EXTERN void sdvr_ui_version(sx_uint8 *major, sx_uint8 *minor, sx_uint8 *revision, sx_uint8 *build);
EXTERN sdvr_err_ui_e sdvr_ui_start_video_preview(sdvr_chan_handle_t handle,
                                                 sdvr_ui_hwnd_t wnd_handle,
                                                 sdvr_ui_region_t *region,
                                                 int *preview_id);
EXTERN sdvr_err_ui_e sdvr_ui_start_video_preview_ex(sdvr_chan_handle_t handle,
                                         sdvr_ui_hwnd_t wnd_handle,
                                         sx_uint8 raw_stream_id,
                                         sdvr_ui_region_t *region,
                                         int *preview_id);
EXTERN sdvr_err_ui_e sdvr_ui_stop_video_preview( sdvr_chan_handle_t h_channel_handle,
                                                 int preview_id);

EXTERN sdvr_err_ui_e sdvr_ui_set_preview_callback(sdvr_chan_handle_t h_channel_handle,
                                                  int preview_id,
                                                  sdvr_ui_preview_cb cb);
EXTERN void sdvr_ui_internal_setup(int func, int enable);

/****************************************************************************
  GROUP: The Audio Preview APIs
****************************************************************************/

EXTERN sdvr_err_ui_e sdvr_ui_init_audio(sdvr_ui_hwnd_t wnd_handle,
                              sdvr_audio_rate_e sample_rate,
                              sx_int32 channels);
EXTERN sdvr_err_ui_e sdvr_ui_start_audio_preview(sdvr_chan_handle_t handle,
                                                 sx_bool wait);
EXTERN sdvr_err_ui_e sdvr_ui_stop_audio_preview();
EXTERN sdvr_err_ui_e sdvr_ui_adjust_volume(sx_uint16 volume);


#endif /* SDVR_UI_SDK_H */



