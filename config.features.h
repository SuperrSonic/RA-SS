#ifndef __RARCH_FEATURES_H
#define __RARCH_FEATURES_H

#include <stddef.h>
#include <boolean.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SDL
static const bool _sdl_supp = true;
#else
static const bool _sdl_supp = false;
#endif

#ifdef HAVE_SDL2
static const bool _sdl2_supp = true;
#else
static const bool _sdl2_supp = false;
#endif

#ifdef HAVE_THREADS
static const bool _thread_supp = true;
#else
static const bool _thread_supp = false;
#endif

#ifdef HAVE_OPENGL
static const bool _opengl_supp = true;
#else
static const bool _opengl_supp = false;
#endif

#ifdef HAVE_KMS
static const bool _kms_supp = true;
#else
static const bool _kms_supp = false;
#endif

#ifdef HAVE_UDEV
static const bool _udev_supp = true;
#else
static const bool _udev_supp = false;
#endif

#ifdef HAVE_VG
static const bool _vg_supp = true;
#else
static const bool _vg_supp = false;
#endif

#ifdef HAVE_EGL
static const bool _egl_supp = true;
#else
static const bool _egl_supp = false;
#endif

#ifdef HAVE_X11
static const bool _x11_supp = true;
#else
static const bool _x11_supp = false;
#endif

#ifdef HAVE_WAYLAND
static const bool _wayland_supp = true;
#else
static const bool _wayland_supp = false;
#endif

#ifdef HAVE_XVIDEO
static const bool _xvideo_supp = true;
#else
static const bool _xvideo_supp = false;
#endif

#ifdef HAVE_ALSA
static const bool _alsa_supp = true;
#else
static const bool _alsa_supp = false;
#endif

#if defined(HAVE_OSS) || defined(HAVE_OSS_BSD)
static const bool _oss_supp = true;
#else
static const bool _oss_supp = false;
#endif

#ifdef HAVE_AL
static const bool _al_supp = true;
#else
static const bool _al_supp = false;
#endif

#ifdef HAVE_RSOUND
static const bool _rsound_supp = true;
#else
static const bool _rsound_supp = false;
#endif

#ifdef HAVE_ROAR
static const bool _roar_supp = true;
#else
static const bool _roar_supp = false;
#endif

#ifdef HAVE_JACK
static const bool _jack_supp = true;
#else
static const bool _jack_supp = false;
#endif

#ifdef HAVE_PULSE
static const bool _pulse_supp = true;
#else
static const bool _pulse_supp = false;
#endif

#ifdef HAVE_DSOUND
static const bool _dsound_supp = true;
#else
static const bool _dsound_supp = false;
#endif

#ifdef HAVE_XAUDIO
static const bool _xaudio_supp = true;
#else
static const bool _xaudio_supp = false;
#endif

#ifdef HAVE_ZLIB
static const bool _zlib_supp = true;
#else
static const bool _zlib_supp = false;
#endif

#ifdef HAVE_DYLIB
static const bool _dylib_supp = true;
#else
static const bool _dylib_supp = false;
#endif

#ifdef HAVE_CG
static const bool _cg_supp = true;
#else
static const bool _cg_supp = false;
#endif

#ifdef HAVE_LIBXML2
static const bool _libxml2_supp = true;
#else
static const bool _libxml2_supp = false;
#endif

#ifdef HAVE_SDL_IMAGE
static const bool _sdl_image_supp = true;
#else
static const bool _sdl_image_supp = false;
#endif

#ifdef HAVE_FBO
static const bool _fbo_supp = true;
#else
static const bool _fbo_supp = false;
#endif

#ifdef HAVE_DYNAMIC
static const bool _dynamic_supp = true;
#else
static const bool _dynamic_supp = false;
#endif

#ifdef HAVE_FFMPEG
static const bool _ffmpeg_supp = true;
#else
static const bool _ffmpeg_supp = false;
#endif

#ifdef HAVE_FREETYPE
static const bool _freetype_supp = true;
#else
static const bool _freetype_supp = false;
#endif

#ifdef HAVE_NETPLAY
static const bool _netplay_supp = true;
#else
static const bool _netplay_supp = false;
#endif

#ifdef HAVE_PYTHON
static const bool _python_supp = true;
#else
static const bool _python_supp = false;
#endif

#if !defined(_WIN32) && !defined(GLOBAL_CONFIG_DIR)
#if defined(__HAIKU__)
#define GLOBAL_CONFIG_DIR "/system/settings"
#else
#define GLOBAL_CONFIG_DIR "/etc"
#endif
#endif

#ifdef _WIN32
#define RARCH_DEFAULT_CONF_PATH_STR "\n\t\tDefaults to retroarch.cfg in same directory as retroarch.exe.\n\t\tIf a default config is not found, " RETRO_FRONTEND " will attempt to create one."
#else
#define RARCH_DEFAULT_CONF_PATH_STR "\n\t\tBy default looks for config in $XDG_CONFIG_HOME/retroarch/retroarch.cfg,\n\t\t$HOME/.config/retroarch/retroarch.cfg,\n\t\tand $HOME/.retroarch.cfg.\n\t\tIf a default config is not found," RETRO_FRONTEND " will attempt to create one based on the skeleton config (" GLOBAL_CONFIG_DIR "/retroarch.cfg)."
#endif

#endif
