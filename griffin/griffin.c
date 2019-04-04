/* RetroArch - A frontend for libretro.
* Copyright (C) 2010-2014 - Hans-Kristian Arntzen
* Copyright (C) 2011-2014 - Daniel De Matteis
*
* RetroArch is free software: you can redistribute it and/or modify it under the terms
* of the GNU General Public License as published by the Free Software Found-
* ation, either version 3 of the License, or (at your option) any later version.
*
* RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with RetroArch.
* If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(HAVE_CG) || defined(HAVE_HLSL) || defined(HAVE_GLSL)
#define HAVE_SHADERS
#endif

#if defined(HAVE_ZLIB) || defined(HAVE_7ZIP)
#define HAVE_COMPRESSION
#endif

#if defined(_MSC_VER)
#include <compat/posix_string.h>
#endif

/*============================================================
CONSOLE EXTENSIONS
============================================================ */
#ifdef RARCH_CONSOLE

#if defined(HAVE_LOGGER) && defined(__PSL1GHT__)
#include "../logger/netlogger/psl1ght_logger.c"
#elif defined(HAVE_LOGGER) && !defined(ANDROID)
#include "../logger/netlogger/logger.c"
#endif

#ifdef HW_DOL
#include "../ngc/ssaram.c"
#endif

#endif

#ifdef HAVE_ZLIB
#include "../file_extract.c"
#include "../decompress/zip_support.c"
#endif

/*============================================================
PERFORMANCE
============================================================ */

#ifdef ANDROID
#include "../performance/performance_android.c"
#endif

#include "../performance.c"

/*============================================================
COMPATIBILITY
============================================================ */
#include "../compat/compat.c"

/*============================================================
CONFIG FILE
============================================================ */
#if defined(_MSC_VER)
#undef __LIBRETRO_SDK_COMPAT_POSIX_STRING_H
#undef __LIBRETRO_SDK_COMPAT_MSVC_H
#undef strcasecmp
#endif

#include "../libretro-sdk/file/config_file.c"
#include "../libretro-sdk/file/config_file_userdata.c"
#include "../core_options.c"

/*============================================================
CHEATS
============================================================ */
#include "../cheats.c"
#include "../hash.c"

/*============================================================
VIDEO CONTEXT
============================================================ */

#include "../gfx/gfx_context.c"
#include "../gfx/context/gfx_null_ctx.c"

#if defined(__CELLOS_LV2__)
#include "../gfx/context/ps3_ctx.c"
#elif defined(_XBOX) || defined(HAVE_WIN32_D3D9)
#include "../gfx/context/d3d_ctx.cpp"
#elif defined(ANDROID)
#include "../gfx/context/androidegl_ctx.c"
#elif defined(__QNX__)
#include "../gfx/context/bbqnx_ctx.c"
#elif defined(EMSCRIPTEN)
#include "../gfx/context/emscriptenegl_ctx.c"
#endif


#if defined(HAVE_OPENGL)

#if defined(HAVE_KMS)
#include "../gfx/context/drm_egl_ctx.c"
#endif
#if defined(HAVE_VIDEOCORE)
#include "../gfx/context/vc_egl_ctx.c"
#endif
#if defined(HAVE_X11) && !defined(HAVE_OPENGLES)
#include "../gfx/context/glx_ctx.c"
#endif

#if defined(HAVE_EGL)
#include "../gfx/context/xegl_ctx.c"
#endif

#if defined(_WIN32) && !defined(_XBOX)
#include "../gfx/context/wgl_ctx.c"
#endif

#endif

#ifdef HAVE_X11
#include "../gfx/context/x11_common.c"
#endif


/*============================================================
VIDEO SHADERS
============================================================ */

#ifdef HAVE_SHADERS
#include "../gfx/shader/shader_context.c"
#include "../gfx/shader/shader_parse.c"
#include "../gfx/shader/shader_null.c"

#ifdef HAVE_CG
#ifdef HAVE_OPENGL
#include "../gfx/shader/shader_gl_cg.c"
#endif
#endif

#ifdef HAVE_HLSL
#include "../gfx/shader/shader_hlsl.c"
#endif

#ifdef HAVE_GLSL
#include "../gfx/shader/shader_glsl.c"
#endif

#endif

/*============================================================
VIDEO IMAGE
============================================================ */

#if defined(__CELLOS_LV2__)
#include "../gfx/image/image_ps3.c"
#elif defined(_XBOX1)
#include "../gfx/image/image_xdk1.c"
#else
#include "../gfx/image/image_rpng.c"
#endif

#include "../gfx/rpng/rpng.c"

/*============================================================
VIDEO DRIVER
============================================================ */

#if defined(HAVE_OPENGL)
#include "../libretro-sdk/gfx/math/matrix.c"
#elif defined(GEKKO)
#ifdef HW_RVL
#include "../wii/vi_encoder.c"
#include "../wii/mem2_manager.c"
#include "../wii/utils/playlog.c"
#endif
#endif

#ifdef HAVE_VG
#include "../gfx/vg.c"
#include "../libretro-sdk/gfx/math/matrix_3x3.c"
#endif

#ifdef HAVE_OMAP
#include "../gfx/omap_gfx.c"
#include "../gfx/fbdev.c"
#endif

#include "../gfx/gfx_common.c"

#ifdef _XBOX
#include "../xdk/xdk_resources.cpp"
#endif

#ifdef HAVE_OPENGL
#include "../gfx/gl.c"
#include "../gfx/gl_common.c"

#ifndef HAVE_PSGL
#include "../libretro-sdk/glsym/rglgen.c"
#ifdef HAVE_OPENGLES2
#include "../libretro-sdk/glsym/glsym_es2.c"
#else
#include "../libretro-sdk/glsym/glsym_gl.c"
#endif
#endif

#endif

#ifdef HAVE_XVIDEO
#include "../gfx/xvideo.c"
#endif

#if defined(_XBOX) || defined(HAVE_WIN32_D3D9)
#include "../gfx/d3d/d3d_wrapper.cpp"
#include "../gfx/d3d/d3d.cpp"
#ifndef _XBOX
#include "../gfx/d3d/render_chain.cpp"
#endif
#endif

#if defined(GEKKO)
#include "../gfx/gx/gx_gfx.c"
#elif defined(PSP)
#include "../gfx/psp/psp1_gfx.c"
#elif defined(XENON)
#include "../gfx/xenon360_gfx.c"
#endif

#include "../gfx/nullgfx.c"

/*============================================================
FONTS
============================================================ */

#include "../gfx/fonts/fonts.c"
#include "../gfx/fonts/bitmapfont.c"

#if defined(HAVE_FREETYPE)
#include "../gfx/fonts/freetype.c"
#endif

#if defined(__APPLE__)
#include "../gfx/fonts/coretext.c"
#endif

#ifdef HAVE_OPENGL
#include "../gfx/fonts/gl_font.c"
#endif

#if defined(_XBOX) || defined(HAVE_WIN32_D3D9)
#include "../gfx/fonts/d3d_font.c"
#endif

#if defined(_WIN32) && !defined(_XBOX)
#include "../gfx/fonts/d3d_w32_font.cpp"
#endif

#if defined(HAVE_LIBDBGFONT)
#include "../gfx/fonts/ps_libdbgfont.c"
#endif

#if defined(HAVE_OPENGL)
#include "../gfx/fonts/gl_raster_font.c"
#endif

#if defined(_XBOX1)
#include "../gfx/fonts/xdk1_xfonts.c"
#endif

#if defined(_XBOX360)
#include "../gfx/fonts/xdk360_fonts.cpp"
#endif

/*============================================================
INPUT
============================================================ */
#include "../input/input_autodetect.c"
#include "../input/input_common.c"
#include "../input/keyboard_line.c"

#ifdef HAVE_OVERLAY
#include "../input/overlay.c"
#endif

#ifdef HAVE_WIIUSE
#include "../wii/wiiuse/classic.c"
#include "../wii/wiiuse/dynamics.c"
#include "../wii/wiiuse/events.c"
#include "../wii/wiiuse/io.c"
#include "../wii/wiiuse/io_wii.c"
#include "../wii/wiiuse/ir.c"
#include "../wii/wiiuse/motion_plus.c"
#include "../wii/wiiuse/nunchuk.c"
#include "../wii/wiiuse/wiiuse.c"
#include "../wii/wiiuse/wpad.c"
#endif

#if defined(__CELLOS_LV2__)
#include "../input/ps3_input.c"
#include "../input/ps3_input_joypad.c"
#include "../input/autoconf/builtin_ps3.c"
#elif defined(SN_TARGET_PSP2) || defined(PSP)
#include "../input/psp_input.c"
#include "../input/psp_input_joypad.c"
#include "../input/autoconf/builtin_psp.c"
#elif defined(GEKKO)
#ifdef HAVE_LIBSICKSAXIS
#include "../input/gx_input_sicksaxis.c"
#endif
#include "../input/gx_input.c"
#include "../input/gx_input_joypad.c"
#include "../input/autoconf/builtin_gx.c"
#elif defined(_XBOX)
#include "../input/xdk_xinput_input.c"
#include "../input/xdk_xinput_input_joypad.c"
#include "../input/autoconf/builtin_xdk.c"
#elif defined(_WIN32)
#include "../input/autoconf/builtin_win.c"
#elif defined(XENON)
#include "../input/xenon360_input.c"
#elif defined(ANDROID)
#include "../input/android_input.c"
#include "../input/android_input_joypad.c"
#elif defined(__APPLE__)
#include "../input/apple_input.c"
#elif defined(__QNX__)
#include "../input/qnx_input.c"
#include "../input/qnx_input_joypad.c"
#elif defined(EMSCRIPTEN)
#include "../input/rwebinput_input.c"
#endif

#if defined(__APPLE__)
#include "../input/connect/joypad_connection.c"
#include "../input/connect/connect_ps3.c"
#include "../input/connect/connect_ps4.c"
#include "../input/connect/connect_wii.c"

#ifdef HAVE_HID
#include "../input/apple_joypad_hid.c"
#endif

#ifdef IOS
#include "../input/apple_joypad_ios.c"
#endif

#endif

#ifdef HAVE_DINPUT
#include "../input/dinput.c"
#endif

#ifdef HAVE_WINXINPUT
#include "../input/winxinput_joypad.c"
#endif

#if defined(_WIN32) && !defined(_XBOX)
#include "../gfx/context/win32_common.c"
#endif

#if defined(__CELLOS_LV2__)
#include "../input/osk/ps3_osk.c"
#endif

#include "../input/osk/nullosk.c"

#if defined(__linux__) && !defined(ANDROID) 
#include "../input/linuxraw_input.c"
#include "../input/linuxraw_joypad.c"
#endif

#ifdef HAVE_X11
#include "../input/x11_input.c"
#endif

#include "../input/nullinput.c"

/*============================================================
STATE TRACKER
============================================================ */
#include "../gfx/state_tracker.c"

#ifdef HAVE_PYTHON
#include "../gfx/py_state/py_state.c"
#endif

/*============================================================
FIFO BUFFER
============================================================ */
#include "../fifo_buffer.c"

/*============================================================
AUDIO RESAMPLER
============================================================ */
#include "../audio/resamplers/resampler.c"
#include "../audio/resamplers/sinc.c"
#include "../audio/resamplers/nearest.c"
#include "../audio/resamplers/cc_resampler.c"

/*============================================================
CAMERA
============================================================ */
#if defined(ANDROID)
#include "../camera/android.c"
#elif defined(EMSCRIPTEN)
#include "../camera/rwebcam.c"
#endif

#ifdef HAVE_V4L2
#include "../camera/video4linux2.c"
#endif

#include "../camera/nullcamera.c"

/*============================================================
LOCATION
============================================================ */
#if defined(ANDROID)
#include "../location/android.c"
#endif

#include "../location/nulllocation.c"

/*============================================================
RSOUND
============================================================ */
#ifdef HAVE_RSOUND
#include "../audio/librsound.c"
#include "../audio/rsound.c"
#endif

/*============================================================
AUDIO
============================================================ */
#if defined(__CELLOS_LV2__)
#include "../audio/ps3_audio.c"
#elif defined(XENON)
#include "../audio/xenon360_audio.c"
#elif defined(GEKKO)
#include "../audio/gx_audio.c"
#elif defined(EMSCRIPTEN)
#include "../audio/rwebaudio.c"
#elif defined(PSP)
#include "../audio/psp1_audio.c"
#endif

#ifdef HAVE_XAUDIO
#include "../audio/xaudio.c"
#include "../audio/xaudio-c/xaudio-c.cpp"
#endif

#ifdef HAVE_DSOUND
#include "../audio/dsound.c"
#endif

#ifdef HAVE_SL
#include "../audio/opensl.c"
#endif

#ifdef HAVE_ALSA
#ifdef __QNX__
#include "../audio/alsa_qsa.c"
#else
#include "../audio/alsa.c"
#include "../audio/alsathread.c"
#endif
#endif

#ifdef HAVE_AL
#include "../audio/openal.c"
#endif

#ifdef HAVE_COREAUDIO
#include "../audio/coreaudio.c"
#endif

#include "../audio/nullaudio.c"

/*============================================================
DRIVERS
============================================================ */
#include "../driver.c"

/*============================================================
SCALERS
============================================================ */
#include "../libretro-sdk/gfx/scaler/scaler_filter.c"
#include "../libretro-sdk/gfx/scaler/pixconv.c"
#include "../libretro-sdk/gfx/scaler/scaler.c"
#include "../libretro-sdk/gfx/scaler/scaler_int.c"

/*============================================================
FILTERS
============================================================ */

#ifdef HAVE_FILTERS_BUILTIN
#include "../gfx/filters/2xsai.c"
#include "../gfx/filters/super2xsai.c"
#include "../gfx/filters/supereagle.c"
#include "../gfx/filters/2xbr.c"
#include "../gfx/filters/darken.c"
#include "../gfx/filters/epx.c"
#include "../gfx/filters/scale2x.c"
#include "../gfx/filters/normal2x.c"
#include "../gfx/filters/blargg_ntsc_snes.c"
#include "../gfx/filters/lq2x.c"
#include "../gfx/filters/phosphor2x.c"

#include "../audio/filters/echo.c"
#include "../audio/filters/eq.c"
#include "../audio/filters/chorus.c"
#include "../audio/filters/iir.c"
#include "../audio/filters/panning.c"
#include "../audio/filters/phaser.c"
#include "../audio/filters/reverb.c"
#include "../audio/filters/wahwah.c"
#endif
/*============================================================
DYNAMIC
============================================================ */
#include "../dynamic.c"
#include "../dynamic_dummy.c"
#include "../gfx/filter.c"
#include "../audio/dsp_filter.c"


/*============================================================
FILE
============================================================ */
#include "../content.c"
#include "../libretro-sdk/file/file_path.c"
#include "../libretro-sdk/file/dir_list.c"
#include "../libretro-sdk/string/string_list.c"
#include "../file_ops.c"
#include "../rarch_compr_file_path.c"
#include "../libretro-sdk/file/file_list.c"

/*============================================================
MESSAGE
============================================================ */
#include "../message_queue.c"

/*============================================================
PATCH
============================================================ */
#include "../patch.c"

/*============================================================
SETTINGS
============================================================ */
#include "../settings.c"

/*============================================================
REWIND
============================================================ */
#include "../rewind.c"

/*============================================================
FRONTEND
============================================================ */

#include "../frontend/frontend_context.c"

#if defined(__CELLOS_LV2__)
#include "../frontend/platform/platform_ps3.c"
#elif defined(GEKKO)
#include "../frontend/platform/platform_gx.c"
#ifdef HW_RVL
#include "../frontend/platform/platform_wii.c"
#endif
#elif defined(_XBOX)
#include "../frontend/platform/platform_xdk.c"
#elif defined(PSP)
#include "../frontend/platform/platform_psp.c"
#elif defined(__QNX__)
#include "../frontend/platform/platform_qnx.c"
#elif defined(OSX) || defined(IOS)
#include "../frontend/platform/platform_apple.c"
#elif defined(ANDROID)
#include "../frontend/platform/platform_android.c"
#endif
#include "../frontend/platform/platform_null.c"

#include "../core_info.c"

/*============================================================
MAIN
============================================================ */
#if defined(XENON)
#include "../frontend/frontend_xenon.c"
#else
#include "../frontend/frontend.c"
#endif

/*============================================================
RETROARCH
============================================================ */
#include "../general.c"
#include "../libretro_version_1.c"
#include "../retroarch.c"
#include "../runloop.c"

/*============================================================
RECORDING
============================================================ */
#include "../movie.c"
#include "../record/ffemu.c"

/*============================================================
THREAD
============================================================ */
#if defined(HAVE_THREADS) && defined(XENON)
#include "../thread/xenon_sdl_threads.c"
#elif defined(HAVE_THREADS)
#include "../libretro-sdk/rthreads/rthreads.c"
#include "../gfx/video_thread_wrapper.c"
#include "../audio/audio_thread_wrapper.c"
#include "../autosave.c"
#endif


/*============================================================
NETPLAY
============================================================ */
#ifdef HAVE_NETPLAY
#include "../netplay.c"
#endif

/*============================================================
SCREENSHOTS
============================================================ */
#if defined(_XBOX1)
#include "../xdk/screenshot_xdk1.c"
#else
#include "../screenshot.c"
#endif

/*============================================================
PLAYLISTS
============================================================ */
#include "../playlist.c"

/*============================================================
MENU
============================================================ */
#ifdef HAVE_MENU
#include "../frontend/menu/menu_input_line_cb.c"
#include "../frontend/menu/menu_common.c"
#include "../frontend/menu/menu_action.c"
#include "../frontend/menu/menu_list.c"
#include "../frontend/menu/menu_entries.c"
#include "../frontend/menu/menu_entries_cbs.c"
#include "../frontend/menu/menu_shader.c"
#include "../frontend/menu/menu_navigation.c"
#include "../frontend/menu/menu_animation.c"

#include "../frontend/menu/backend/menu_common_backend.c"
#endif

#ifdef HAVE_RMENU
#include "../frontend/menu/disp/rmenu.c"
#endif

#ifdef HAVE_RGUI
#include "../frontend/menu/disp/rgui.c"
#endif

#ifdef HAVE_RMENU_XUI
#include "../frontend/menu/disp/rmenu_xui.cpp"
#endif

#ifdef HAVE_OPENGL

#ifdef HAVE_LAKKA
#include "../frontend/menu/backend/menu_lakka_backend.c"
#include "../frontend/menu/disp/lakka.c"
#endif

#ifdef HAVE_XMB
#include "../frontend/menu/disp/xmb.c"
#endif

#ifdef HAVE_GLUI
#include "../frontend/menu/disp/glui.c"
#endif

#endif

#ifdef HAVE_COMMAND
#include "../command.c"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================
DEPENDENCIES
============================================================ */
#ifdef WANT_MINIZ
#include "../deps/rzlib/adler32.c"
#include "../deps/rzlib/compress.c"
#include "../deps/rzlib/crc32.c"
#include "../deps/rzlib/deflate.c"
#include "../deps/rzlib/gzclose.c"
#include "../deps/rzlib/gzlib.c"
#include "../deps/rzlib/gzread.c"
#include "../deps/rzlib/gzwrite.c"
#include "../deps/rzlib/inffast.c"
#include "../deps/rzlib/inflate.c"
#include "../deps/rzlib/inftrees.c"
#include "../deps/rzlib/trees.c"
#include "../deps/rzlib/uncompr.c"
#include "../deps/rzlib/zutil.c"
#endif

/* Decompression support always requires the next two files */
#if defined(WANT_MINIZ) || defined(HAVE_ZLIB)
#include "../deps/rzlib/ioapi.c"
#include "../deps/rzlib/unzip.c"
#endif

#ifdef HAVE_7ZIP
#include "../deps/7zip/7zIn.c"
#include "../deps/7zip/7zAlloc.c"
#include "../deps/7zip/Bra86.c"
#include "../deps/7zip/CpuArch.c"
#include "../deps/7zip/7zFile.c"
#include "../deps/7zip/7zStream.c"
#include "../deps/7zip/7zBuf2.c"
#include "../deps/7zip/LzmaDec.c"
#include "../deps/7zip/7zCrcOpt.c"
#include "../deps/7zip/Bra.c"
#include "../deps/7zip/7zDec.c"
#include "../deps/7zip/Bcj2.c"
#include "../deps/7zip/7zCrc.c"
#include "../deps/7zip/Lzma2Dec.c"
#include "../deps/7zip/7zBuf.c"
#include "../decompress/7zip_support.c"
#endif

/*============================================================
XML
============================================================ */
#ifndef HAVE_LIBXML2
#define RXML_LIBXML2_COMPAT
#include "../compat/rxml/rxml.c"
#endif
/*============================================================
 SETTINGS
============================================================ */
#include "../settings_list.c"
#include "../settings_data.c"

/*============================================================
 AUDIO UTILS
============================================================ */
#include "../audio/utils.c"

#ifdef __cplusplus
}
#endif
