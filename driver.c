/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2014 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#include "driver.h"
#include "general.h"
#include "libretro.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "compat/posix_string.h"
#include "audio/utils.h"
#include "gfx/video_thread_wrapper.h"
#include "audio/audio_thread_wrapper.h"
#include "gfx/gfx_common.h"

#ifdef HAVE_X11
#include "gfx/context/x11_common.h"
#endif

#ifdef HAVE_MENU
#include "frontend/menu/menu_common.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

driver_t driver;

static const audio_driver_t *audio_drivers[] = {
#ifdef HAVE_ALSA
   &audio_alsa,
#ifndef __QNX__
   &audio_alsathread,
#endif
#endif
#if defined(HAVE_OSS) || defined(HAVE_OSS_BSD)
   &audio_oss,
#endif
#ifdef HAVE_RSOUND
   &audio_rsound,
#endif
#ifdef HAVE_COREAUDIO
   &audio_coreaudio,
#endif
#ifdef HAVE_AL
   &audio_openal,
#endif
#ifdef HAVE_SL
   &audio_opensl,
#endif
#ifdef HAVE_ROAR
   &audio_roar,
#endif
#ifdef HAVE_JACK
   &audio_jack,
#endif
#if defined(HAVE_SDL) || defined(HAVE_SDL2)
   &audio_sdl,
#endif
#ifdef HAVE_XAUDIO
   &audio_xa,
#endif
#ifdef HAVE_DSOUND
   &audio_dsound,
#endif
#ifdef HAVE_PULSE
   &audio_pulse,
#endif
#ifdef __CELLOS_LV2__
   &audio_ps3,
#endif
#ifdef XENON
   &audio_xenon360,
#endif
#ifdef GEKKO
   &audio_gx,
#endif
#ifdef EMSCRIPTEN
   &audio_rwebaudio,
#endif
#ifdef PSP
   &audio_psp1,
#endif   
   &audio_null,
   NULL,
};

static const video_driver_t *video_drivers[] = {
#ifdef GEKKO
   &video_gx,
#endif
/*
   &video_null, */
   NULL,
};

static const input_driver_t *input_drivers[] = {
#ifdef GEKKO
   &input_gx,
#endif
   &input_null,
   NULL,
};

static const input_osk_driver_t *osk_drivers[] = {
   &input_null_osk,
   NULL,
};

static const camera_driver_t *camera_drivers[] = {
#ifdef HAVE_V4L2
   &camera_v4l2,
#endif
#ifdef EMSCRIPTEN
   &camera_rwebcam,
#endif
#ifdef ANDROID
   &camera_android,
#endif
#ifdef IOS
   &camera_ios,
#endif
   &camera_null,
   NULL,
};

static const location_driver_t *location_drivers[] = {
#ifdef ANDROID
   &location_android,
#endif
#if defined(IOS) || defined(OSX)
   &location_apple,
#endif
   &location_null,
   NULL,
};

#ifdef HAVE_MENU
static const menu_ctx_driver_t *menu_ctx_drivers[] = {
#if defined(HAVE_RMENU)
   &menu_ctx_rmenu,
#endif
#if defined(HAVE_RMENU_XUI)
   &menu_ctx_rmenu_xui,
#endif
#if defined(HAVE_LAKKA)
   &menu_ctx_lakka,
#endif
#if defined(HAVE_GLUI)
   &menu_ctx_glui,
#endif
#if defined(HAVE_XMB)
   &menu_ctx_xmb,
#endif
#if defined(HAVE_RGUI)
   &menu_ctx_rgui,
#endif
   NULL
};
#endif

static const void *find_driver_nonempty(const char *label, int i,
      char *str, size_t sizeof_str)
{
   const void *drv = NULL;

   if (!strcmp(label, "camera_driver"))
   {
      drv = camera_drivers[i];
      if (drv)
         strlcpy(str, camera_drivers[i]->ident, sizeof_str);
   }
   else if (!strcmp(label, "location_driver"))
   {
      drv = location_drivers[i];
      if (drv)
         strlcpy(str, location_drivers[i]->ident, sizeof_str);
   }
   else if (!strcmp(label, "osk_driver"))
   {
      drv = osk_drivers[i];
      if (drv)
         strlcpy(str, osk_drivers[i]->ident, sizeof_str);
   }
#ifdef HAVE_MENU
   else if (!strcmp(label, "menu_driver"))
   {
      drv = menu_ctx_drivers[i];
      if (drv)
         strlcpy(str, menu_ctx_drivers[i]->ident, sizeof_str);
   }
#endif
   else if (!strcmp(label, "input_driver"))
   {
      drv = input_drivers[i];
      if (drv)
         strlcpy(str, input_drivers[i]->ident, sizeof_str);
   }
   else if (!strcmp(label, "input_joypad_driver"))
   {
      drv = joypad_drivers[i];
      if (drv)
         strlcpy(str, joypad_drivers[i]->ident, sizeof_str);
   }
   else if (!strcmp(label, "video_driver"))
   {
      drv = video_drivers[i];
      if (drv)
         strlcpy(str, video_drivers[i]->ident, sizeof_str);
   }
   else if (!strcmp(label, "audio_driver"))
   {
      drv = audio_drivers[i];
      if (drv)
         strlcpy(str, audio_drivers[i]->ident, sizeof_str);
   }

   return drv;
}

static int find_driver_index(const char * label, const char *drv)
{
   unsigned i;
   char str[PATH_MAX];
   const void *obj = NULL;

   for (i = 0; (obj = (const void*)
            find_driver_nonempty(label, i, str, sizeof(str))) != NULL; i++)
   {
      if (!obj)
         return -1;
      if (str[0] == '\0')
         break;
      if (!strcasecmp(drv, str))
         return i;
   }

   return -1;
}

void find_prev_driver(const char *label, char *str, size_t sizeof_str)
{
   int i = find_driver_index(label, str);
   if (i > 0)
      find_driver_nonempty(label, i - 1, str, sizeof_str);
   else
      RARCH_WARN(
            "Couldn't find any previous driver (current one: \"%s\").\n", str);
}

void find_next_driver(const char *label, char *str, size_t sizeof_str)
{
   int i = find_driver_index(label, str);
   if (i >= 0)
      find_driver_nonempty(label, i + 1, str, sizeof_str);
   else
      RARCH_WARN("Couldn't find any next driver (current one: \"%s\").\n", str);
}

static void find_osk_driver(void)
{
   int i = find_driver_index("osk_driver", g_settings.osk.driver);
   if (i >= 0)
      driver.osk = osk_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any OSK driver named \"%s\"\n",
            g_settings.osk.driver);
      RARCH_LOG_OUTPUT("Available OSK drivers are:\n");
      for (d = 0; osk_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", osk_drivers[d]->ident);

      RARCH_WARN("Going to default to first OSK driver...\n");
       
      driver.osk = osk_drivers[0];
       
      if (!driver.osk)
         rarch_fail(1, "find_osk_driver()");
   }
}

static void init_osk(void)
{
   /* Resource leaks will follow if osk is initialized twice. */
   if (driver.osk_data)
      return;

   find_osk_driver();

   /* FIXME - refactor params later based on semantics  */
   driver.osk_data = driver.osk->init(0);

   if (!driver.osk_data)
   {
      RARCH_ERR("Failed to initialize OSK driver. Will continue without OSK.\n");
      driver.osk_active = false;
   }
}

static void uninit_osk(void)
{
   if (driver.osk_data && driver.osk && driver.osk->free)
      driver.osk->free(driver.osk_data);
   driver.osk_data = NULL;
}



static void find_camera_driver(void)
{
   int i = find_driver_index("camera_driver", g_settings.camera.driver);
   if (i >= 0)
      driver.camera = camera_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any camera driver named \"%s\"\n",
            g_settings.camera.driver);
      RARCH_LOG_OUTPUT("Available camera drivers are:\n");
      for (d = 0; camera_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", camera_drivers[d]->ident);
       
      RARCH_WARN("Going to default to first camera driver...\n");
       
      driver.camera = camera_drivers[0];
       
      if (!driver.camera)
         rarch_fail(1, "find_camera_driver()");
   }
}


bool driver_camera_start(void)
{
   if (driver.camera && driver.camera_data && driver.camera->start)
   {
      if (g_settings.camera.allow)
         return driver.camera->start(driver.camera_data);

      msg_queue_push(g_extern.msg_queue,
            "Camera is explicitly disabled.\n", 1, 180);
   }
   return false;
}

void driver_camera_stop(void)
{
   if (driver.camera && driver.camera->stop && driver.camera_data)
      driver.camera->stop(driver.camera_data);
}

void driver_camera_poll(void)
{
   if (driver.camera && driver.camera->poll && driver.camera_data)
      driver.camera->poll(driver.camera_data,
            g_extern.system.camera_callback.frame_raw_framebuffer,
            g_extern.system.camera_callback.frame_opengl_texture);
}

static void init_camera(void)
{
   /* Resource leaks will follow if camera is initialized twice. */
   if (driver.camera_data)
      return;

   find_camera_driver();

   driver.camera_data = driver.camera->init(
         *g_settings.camera.device ? g_settings.camera.device : NULL,
         g_extern.system.camera_callback.caps,
         g_settings.camera.width ?
         g_settings.camera.width : g_extern.system.camera_callback.width,
         g_settings.camera.height ?
         g_settings.camera.height : g_extern.system.camera_callback.height);

   if (!driver.camera_data)
   {
      RARCH_ERR("Failed to initialize camera driver. Will continue without camera.\n");
      driver.camera_active = false;
   }

   if (g_extern.system.camera_callback.initialized)
      g_extern.system.camera_callback.initialized();
}

static void uninit_camera(void)
{
   if (driver.camera_data && driver.camera)
   {
      if (g_extern.system.camera_callback.deinitialized)
         g_extern.system.camera_callback.deinitialized();

      if (driver.camera->free)
         driver.camera->free(driver.camera_data);
   }
   driver.camera_data = NULL;
}

static void find_location_driver(void)
{
   int i = find_driver_index("location_driver", g_settings.location.driver);
   if (i >= 0)
      driver.location = location_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any location driver named \"%s\"\n",
            g_settings.location.driver);
      RARCH_LOG_OUTPUT("Available location drivers are:\n");
      for (d = 0; location_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", location_drivers[d]->ident);
       
      RARCH_WARN("Going to default to first location driver...\n");
       
      driver.location = location_drivers[0];

      if (!driver.location)
         rarch_fail(1, "find_location_driver()");
   }
}


bool driver_location_start(void)
{
   if (driver.location && driver.location_data && driver.location->start)
   {
      if (g_settings.location.allow)
         return driver.location->start(driver.location_data);

      msg_queue_push(g_extern.msg_queue, "Location is explicitly disabled.\n", 1, 180);
   }
   return false;
}

void driver_location_stop(void)
{
   if (driver.location && driver.location->stop && driver.location_data)
      driver.location->stop(driver.location_data);
}

void driver_location_set_interval(unsigned interval_msecs,
      unsigned interval_distance)
{
   if (driver.location && driver.location->set_interval
         && driver.location_data)
      driver.location->set_interval(driver.location_data,
            interval_msecs, interval_distance);
}

bool driver_location_get_position(double *lat, double *lon,
      double *horiz_accuracy, double *vert_accuracy)
{
   if (driver.location && driver.location->get_position
         && driver.location_data)
      return driver.location->get_position(driver.location_data,
            lat, lon, horiz_accuracy, vert_accuracy);

   *lat = 0.0;
   *lon = 0.0;
   *horiz_accuracy = 0.0;
   *vert_accuracy = 0.0;
   return false;
}

static void init_location(void)
{
   /* Resource leaks will follow if location interface is initialized twice. */
   if (driver.location_data)
      return;

   find_location_driver();

   driver.location_data = driver.location->init();

   if (!driver.location_data)
   {
      RARCH_ERR("Failed to initialize location driver. Will continue without location.\n");
      driver.location_active = false;
   }

   if (g_extern.system.location_callback.initialized)
      g_extern.system.location_callback.initialized();
}

static void uninit_location(void)
{
   if (driver.location_data && driver.location)
   {
      if (g_extern.system.location_callback.deinitialized)
         g_extern.system.location_callback.deinitialized();

      if (driver.location->free)
         driver.location->free(driver.location_data);
   }
   driver.location_data = NULL;
}

#ifdef HAVE_MENU
static void find_menu_driver(void)
{
   int i = find_driver_index("menu_driver", g_settings.menu.driver);
   if (i >= 0)
      driver.menu_ctx = menu_ctx_drivers[i];
   else
   {
      unsigned d;
      RARCH_WARN("Couldn't find any menu driver named \"%s\"\n",
            g_settings.menu.driver);
      RARCH_LOG_OUTPUT("Available menu drivers are:\n");
      for (d = 0; menu_ctx_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", menu_ctx_drivers[d]->ident);
      RARCH_WARN("Going to default to first menu driver...\n");

      driver.menu_ctx = menu_ctx_drivers[0];

      if (!driver.menu_ctx)
         rarch_fail(1, "find_menu_driver()");
   }
}
#endif


static void find_audio_driver(void)
{
   int i = find_driver_index("audio_driver", g_settings.audio.driver);
   if (i >= 0)
      driver.audio = audio_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any audio driver named \"%s\"\n",
            g_settings.audio.driver);
      RARCH_LOG_OUTPUT("Available audio drivers are:\n");
      for (d = 0; audio_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", audio_drivers[d]->ident);
      RARCH_WARN("Going to default to first audio driver...\n");

      driver.audio = audio_drivers[0];

      if (!driver.audio)
         rarch_fail(1, "find_audio_driver()");
   }
}


static void find_video_driver(void)
{
   //int i;

 /*  if (driver.frontend_ctx &&
       driver.frontend_ctx->get_video_driver)
   {
      driver.video = driver.frontend_ctx->get_video_driver();

      if (driver.video)
         return;
    //  RARCH_WARN("Frontend supports get_video_driver() but did not specify one.\n");
   } */

   driver.video = &video_gx;
  /* i = find_driver_index("video_driver", g_settings.video.driver);
   if (i >= 0)
      driver.video = video_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any video driver named \"%s\"\n",
            g_settings.video.driver);
      RARCH_LOG_OUTPUT("Available video drivers are:\n");
      for (d = 0; video_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", video_drivers[d]->ident);
      RARCH_WARN("Going to default to first video driver...\n");

      driver.video = video_drivers[0];

      if (!driver.video)
         rarch_fail(1, "find_video_driver()");
   }*/
}


static void find_input_driver(void)
{
   int i = find_driver_index("input_driver", g_settings.input.driver);
   if (i >= 0)
      driver.input = input_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any input driver named \"%s\"\n",
            g_settings.input.driver);
      RARCH_LOG_OUTPUT("Available input drivers are:\n");
      for (d = 0; input_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", input_drivers[d]->ident);
      RARCH_WARN("Going to default to first input driver...\n");

      driver.input = input_drivers[0];

      if (!driver.input)
         rarch_fail(1, "find_input_driver()");
   }
}

void init_drivers_pre(void)
{
   find_audio_driver();
   find_video_driver();
   find_input_driver();
   find_camera_driver();
   find_location_driver();
   find_osk_driver();
#ifdef HAVE_MENU
   find_menu_driver();
#endif
}

static void adjust_system_rates(void)
{
   float timing_skew;
   const struct retro_system_timing *info = 
      (const struct retro_system_timing*)&g_extern.system.av_info.timing;

   g_extern.system.force_nonblock = false;

   if (info->fps <= 0.0 || info->sample_rate <= 0.0)
      return;

   timing_skew = fabs(1.0f - info->fps / g_settings.video.refresh_rate);

   if (timing_skew > 0.05f)
   {
      /* We don't want to adjust pitch too much. If we have extreme cases,
       * just don't readjust at all. */
      RARCH_LOG("Timings deviate too much. Will not adjust. (Display = %.2f Hz, Game = %.2f Hz)\n",
            g_settings.video.refresh_rate,
            (float)info->fps);

      /* We won't be able to do VSync reliably as game FPS > monitor FPS. */
    /*  if (info->fps > g_settings.video.refresh_rate)
      {
         g_extern.system.force_nonblock = true;
         RARCH_LOG("Game FPS > Monitor FPS. Cannot rely on VSync.\n");
      } */
	  
	  // Esto no parece ser correcto,
	  // si el emulador usa 60 fps pero el Wii esta en 60/1.001 entonces no hay vsync!

      g_extern.audio_data.in_rate = info->sample_rate;
   }
   else
      g_extern.audio_data.in_rate = info->sample_rate *
         (g_settings.video.refresh_rate / info->fps);

   RARCH_LOG("Set audio input rate to: %.2f Hz.\n",
         g_extern.audio_data.in_rate);

   if (driver.video_data)
   {
      if (g_extern.system.force_nonblock)
         rarch_main_command(RARCH_CMD_VIDEO_SET_NONBLOCKING_STATE);
      else
         driver_set_nonblock_state(driver.nonblock_state);
   }
}

void driver_set_monitor_refresh_rate(float hz)
{
   //char msg[PATH_MAX];
   //snprintf(msg, sizeof(msg), "Setting refresh rate to: %.3f Hz.", hz);
   //msg_queue_push(g_extern.msg_queue, msg, 1, 180);
   //RARCH_LOG("%s\n", msg);

   g_settings.video.refresh_rate = hz;
   adjust_system_rates();

   g_extern.audio_data.orig_src_ratio =
      g_extern.audio_data.src_ratio =
      (double)g_settings.audio.out_rate / g_extern.audio_data.in_rate;
}

void driver_set_nonblock_state(bool nonblock)
{
   /* Only apply non-block-state for video if we're using vsync. */
   if (driver.video_active && driver.video_data)
   {
      bool video_nonblock = nonblock;
      if (!g_settings.video.vsync || g_extern.system.force_nonblock)
         video_nonblock = true;
      driver.video->set_nonblock_state(driver.video_data, video_nonblock);
   }

   if (driver.audio_active && driver.audio_data)
      driver.audio->set_nonblock_state(driver.audio_data,
            g_settings.audio.sync ? nonblock : true);

   g_extern.audio_data.chunk_size = nonblock ?
      g_extern.audio_data.nonblock_chunk_size : 
      g_extern.audio_data.block_chunk_size;
}

bool driver_set_rumble_state(unsigned port,
      enum retro_rumble_effect effect, uint16_t strength)
{
   if (driver.input && driver.input_data && driver.input->set_rumble)
      return driver.input->set_rumble(driver.input_data,
            port, effect, strength);
   return false;
}

bool driver_set_sensor_state(unsigned port,
      enum retro_sensor_action action, unsigned rate)
{
   if (driver.input && driver.input_data &&
         driver.input->set_sensor_state)
      return driver.input->set_sensor_state(driver.input_data,
            port, action, rate);
   return false;
}

float driver_sensor_get_input(unsigned port, unsigned id)
{
   if (driver.input && driver.input_data &&
         driver.input->get_sensor_input)
      return driver.input->get_sensor_input(driver.input_data,
            port, id);
   return 0.0f;
}

uintptr_t driver_get_current_framebuffer(void)
{
#ifdef HAVE_FBO
   if (driver.video_poke && driver.video_poke->get_current_framebuffer)
      return driver.video_poke->get_current_framebuffer(driver.video_data);
#endif
   return 0;
}

retro_proc_address_t driver_get_proc_address(const char *sym)
{
#ifdef HAVE_FBO
   if (driver.video_poke && driver.video_poke->get_proc_address)
      return driver.video_poke->get_proc_address(driver.video_data, sym);
#endif
   return NULL;
}

bool driver_update_system_av_info(const struct retro_system_av_info *info)
{
   g_extern.system.av_info = *info;
   //rarch_main_command(RARCH_CMD_REINIT);

   /* Cannot continue recording with different parameters.
    * Take the easiest route out and just restart the recording. */
  /* if (driver.recording_data)
   {
      static const char *msg = "Restarting recording due to driver reinit.";
      msg_queue_push(g_extern.msg_queue, msg, 2, 180);
      RARCH_WARN("%s\n", msg);
      rarch_main_command(RARCH_CMD_RECORD_DEINIT);
      rarch_main_command(RARCH_CMD_RECORD_INIT);
   } */

   return true;
}

#ifdef HAVE_MENU
static void init_menu(void)
{
   if (driver.menu)
      return;

   find_menu_driver();
   if (!(driver.menu = (menu_handle_t*)menu_init(driver.menu_ctx)))
   {
      RARCH_ERR("Cannot initialize menu.\n");
      rarch_fail(1, "init_menu()");
   }

   if (!(menu_init_list(driver.menu)))
   {
      RARCH_ERR("Cannot initialize menu lists.\n");
      rarch_fail(1, "init_menu()");
   }
}
#endif

static void init_audio(void)
{
   size_t max_bufsamples = AUDIO_CHUNK_SIZE_NONBLOCKING * 2;
   size_t outsamples_max;

   audio_convert_init_simd();

   /* Resource leaks will follow if audio is initialized twice. */
   if (driver.audio_data)
      return;

   /* Accomodate rewind since at some point we might have two full buffers. */
   outsamples_max = max_bufsamples * AUDIO_MAX_RATIO * 
      g_settings.slowmotion_ratio;

   /* Used for recording even if audio isn't enabled. */
   rarch_assert(g_extern.audio_data.conv_outsamples =
         (int16_t*)malloc(outsamples_max * sizeof(int16_t)));

   g_extern.audio_data.block_chunk_size    = AUDIO_CHUNK_SIZE_BLOCKING;
   g_extern.audio_data.nonblock_chunk_size = AUDIO_CHUNK_SIZE_NONBLOCKING;
   g_extern.audio_data.chunk_size          = 
      g_extern.audio_data.block_chunk_size;

   /* Needs to be able to hold full content of a full max_bufsamples
    * in addition to its own. */
   rarch_assert(g_extern.audio_data.rewind_buf = (int16_t*)
         malloc(max_bufsamples * sizeof(int16_t)));
   g_extern.audio_data.rewind_size             = max_bufsamples;

   if (!g_settings.audio.enable)
   {
      driver.audio_active = false;
      return;
   }

   find_audio_driver();
#ifdef HAVE_THREADS
   if (g_extern.system.audio_callback.callback)
   {
      RARCH_LOG("Starting threaded audio driver ...\n");
      if (!rarch_threaded_audio_init(&driver.audio, &driver.audio_data,
               *g_settings.audio.device ? g_settings.audio.device : NULL,
               g_settings.audio.out_rate, g_settings.audio.latency,
               driver.audio))
      {
         RARCH_ERR("Cannot open threaded audio driver ... Exiting ...\n");
         rarch_fail(1, "init_audio()");
      }
   }
   else
#endif
   {
      driver.audio_data = driver.audio->init(*g_settings.audio.device ?
            g_settings.audio.device : NULL,
            g_settings.audio.out_rate, g_settings.audio.latency);
   }

   if (!driver.audio_data)
   {
      RARCH_ERR("Failed to initialize audio driver. Will continue without audio.\n");
      driver.audio_active = false;
   }

   g_extern.audio_data.use_float = false;
   if (driver.audio_active && driver.audio->use_float(driver.audio_data))
      g_extern.audio_data.use_float = true;

   if (!g_settings.audio.sync && driver.audio_active)
   {
      rarch_main_command(RARCH_CMD_AUDIO_SET_NONBLOCKING_STATE);
      g_extern.audio_data.chunk_size = 
         g_extern.audio_data.nonblock_chunk_size;
   }

   /* Should never happen. */
   if (g_extern.audio_data.in_rate <= 0.0f)
   {
      RARCH_WARN("Input rate is invalid (%.3f Hz). Using output rate (%u Hz).\n",
            g_extern.audio_data.in_rate, g_settings.audio.out_rate);
      g_extern.audio_data.in_rate = g_settings.audio.out_rate;
   }

   g_extern.audio_data.orig_src_ratio =
      g_extern.audio_data.src_ratio =
      (double)g_settings.audio.out_rate / g_extern.audio_data.in_rate;

   if (!rarch_resampler_realloc(&driver.resampler_data,
            &driver.resampler,
         g_settings.audio.resampler, g_extern.audio_data.orig_src_ratio))
   {
      RARCH_ERR("Failed to initialize resampler \"%s\".\n",
            g_settings.audio.resampler);
      driver.audio_active = false;
   }

   rarch_assert(g_extern.audio_data.data = (float*)
         malloc(max_bufsamples * sizeof(float)));

   g_extern.audio_data.data_ptr = 0;

   rarch_assert(g_settings.audio.out_rate <
         g_extern.audio_data.in_rate * AUDIO_MAX_RATIO);
   rarch_assert(g_extern.audio_data.outsamples = (float*)
         malloc(outsamples_max * sizeof(float)));

   g_extern.audio_data.rate_control = false;
   if (!g_extern.system.audio_callback.callback && driver.audio_active &&
         g_settings.audio.rate_control)
   {
      if (driver.audio->buffer_size && driver.audio->write_avail)
      {
         g_extern.audio_data.driver_buffer_size = 
            driver.audio->buffer_size(driver.audio_data);
         g_extern.audio_data.rate_control = true;
      }
      else
         RARCH_WARN("Audio rate control was desired, but driver does not support needed features.\n");
   }

   rarch_main_command(RARCH_CMD_DSP_FILTER_DEINIT);

   g_extern.measure_data.buffer_free_samples_count = 0;

   if (driver.audio_active && !g_extern.audio_data.mute &&
         g_extern.system.audio_callback.callback)
   {
      /* Threaded driver is initially stopped. */
      driver.audio->start(driver.audio_data);
   }
}

static void deinit_pixel_converter(void)
{
   scaler_ctx_gen_reset(&driver.scaler);
   memset(&driver.scaler, 0, sizeof(driver.scaler));
   free(driver.scaler_out);
   driver.scaler_out = NULL;
}

static bool init_video_pixel_converter(unsigned size)
{
   /* This function can be called multiple times
    * without deiniting first on consoles. */
   deinit_pixel_converter();

   if (g_extern.system.pix_fmt == RETRO_PIXEL_FORMAT_0RGB1555)
   {
      RARCH_WARN("0RGB1555 pixel format is deprecated, and will be slower. For 15/16-bit, RGB565 format is preferred.\n");

      driver.scaler.scaler_type = SCALER_TYPE_POINT;
      driver.scaler.in_fmt      = SCALER_FMT_0RGB1555;

      /* TODO: Pick either ARGB8888 or RGB565 depending on driver. */
      driver.scaler.out_fmt     = SCALER_FMT_RGB565;

      if (!scaler_ctx_gen_filter(&driver.scaler))
         return false;

      driver.scaler_out = calloc(sizeof(uint16_t), size * size);
   }

   return true;
}

static void deinit_video_filter(void)
{
   rarch_softfilter_free(g_extern.filter.filter);
   free(g_extern.filter.buffer);
   memset(&g_extern.filter, 0, sizeof(g_extern.filter));
}

static void init_video_filter(enum retro_pixel_format colfmt)
{
   unsigned width, height, pow2_x, pow2_y, maxsize;
   struct retro_game_geometry *geom = NULL;

   deinit_video_filter();
   if (!*g_settings.video.softfilter_plugin) {
       g_settings.video.use_filter = false;
      return;
   }
   g_settings.video.use_filter = true;

   /* Deprecated format. Gets pre-converted. */
   if (colfmt == RETRO_PIXEL_FORMAT_0RGB1555)
      colfmt = RETRO_PIXEL_FORMAT_RGB565;

  /* if (g_extern.system.hw_render_callback.context_type)
   {
      RARCH_WARN("Cannot use CPU filters when hardware rendering is used.\n");
      return;
   }*/

   geom    = (struct retro_game_geometry*)&g_extern.system.av_info.geometry;
   width   = geom->max_width;
   height  = geom->max_height;

   g_extern.filter.filter = rarch_softfilter_new(
         g_settings.video.softfilter_plugin, colfmt, width, height);

   if (!g_extern.filter.filter)
   {
      RARCH_ERR("Failed to load filter.\n");
      return;
   }

   rarch_softfilter_get_max_output_size(g_extern.filter.filter,
         &width, &height);

   pow2_x                = next_pow2(width);
   pow2_y                = next_pow2(height);
   maxsize               = max(pow2_x, pow2_y); 
   g_extern.filter.scale = maxsize / RARCH_SCALE_BASE;

   g_extern.filter.out_rgb32 = rarch_softfilter_get_output_format(
         g_extern.filter.filter) == RETRO_PIXEL_FORMAT_XRGB8888;

   g_extern.filter.out_bpp = g_extern.filter.out_rgb32 ?
      sizeof(uint32_t) : sizeof(uint16_t);

   /* TODO: Aligned output. */
   g_extern.filter.buffer = malloc(width * height * g_extern.filter.out_bpp);
   if (!g_extern.filter.buffer)
      goto error;

   return;

error:
   RARCH_ERR("Softfilter initialization failed.\n");
   deinit_video_filter();
}

static void init_video_input(void)
{
   unsigned max_dim, scale, width, height;
   rarch_viewport_t *custom_vp;
   const input_driver_t *tmp = NULL;
   const struct retro_game_geometry *geom = NULL;
   video_info_t video = {0};
   static uint16_t dummy_pixels[32] = {0};

   init_video_filter(g_extern.system.pix_fmt);
   //rarch_main_command(RARCH_CMD_SHADER_DIR_INIT);

   geom = (const struct retro_game_geometry*)&g_extern.system.av_info.geometry;
   max_dim = max(geom->max_width, geom->max_height);
   scale = next_pow2(max_dim) / RARCH_SCALE_BASE;
   scale = max(scale, 1);

   if (g_extern.filter.filter)
      scale = g_extern.filter.scale;

   /* Update core-dependent aspect ratio values. */
   gfx_set_square_pixel_viewport(geom->base_width, geom->base_height);
   gfx_set_core_viewport();
   gfx_set_config_viewport();

   /* Update CUSTOM viewport. */
   custom_vp = (rarch_viewport_t*)&g_extern.console.screen.viewports.custom_vp;
   if (g_settings.video.aspect_ratio_idx == ASPECT_RATIO_CUSTOM)
   {
      float default_aspect = aspectratio_lut[ASPECT_RATIO_CORE].value;
      aspectratio_lut[ASPECT_RATIO_CUSTOM].value = 
         (custom_vp->width && custom_vp->height) ?
         (float)custom_vp->width / custom_vp->height : default_aspect;
   }

   g_extern.system.aspect_ratio = 
      aspectratio_lut[g_settings.video.aspect_ratio_idx].value;

   if (g_settings.video.fullscreen)
   {
      width = g_settings.video.fullscreen_x;
      height = g_settings.video.fullscreen_y;
   }
   else
   {
      if (g_settings.video.force_aspect)
      {
         /* Do rounding here to simplify integer scale correctness. */
         unsigned base_width = roundf(geom->base_height *
               g_extern.system.aspect_ratio);
         width = roundf(base_width * g_settings.video.scale);
         height = roundf(geom->base_height * g_settings.video.scale);
      }
      else
      {
         width = roundf(geom->base_width * g_settings.video.scale);
         height = roundf(geom->base_height * g_settings.video.scale);
      }
   }

   if (width && height)
      RARCH_LOG("Video @ %ux%u\n", width, height);
   else
      RARCH_LOG("Video @ fullscreen\n");

   driver.display_type  = RARCH_DISPLAY_NONE;
   driver.video_display = 0;
   driver.video_window  = 0;

   if (!init_video_pixel_converter(RARCH_SCALE_BASE * scale))
   {
      RARCH_ERR("Failed to initialize pixel converter.\n");
      rarch_fail(1, "init_video_input()");
   }

   video.width = width;
   video.height = height;
   video.fullscreen = g_settings.video.fullscreen;
   video.vsync = g_settings.video.vsync; // && !g_extern.system.force_nonblock;
   video.force_aspect = g_settings.video.force_aspect;
#ifdef GEKKO
   video.drawdone = g_settings.video.drawdone;
   video.viwidth = g_settings.video.viwidth;
   video.vfilter = g_settings.video.vfilter;
   video.dither = g_settings.video.dither;
   video.vbright = g_settings.video.vbright;
   video.vres = g_settings.video.vres;
   video.blendframe = g_settings.video.blendframe;
   video.blend_smooth = g_settings.video.blend_smooth;
#ifdef HAVE_RENDERSCALE
   video.renderscale = g_settings.video.renderscale;
   video.top = g_settings.video.top;
   video.bottom = g_settings.video.bottom;
   video.left = g_settings.video.left;
   video.right = g_settings.video.right;
#endif
#endif
   video.smooth = g_settings.video.smooth;
   video.menu_smooth = g_settings.video.menu_smooth;
   video.input_scale = scale;
   video.rgb32 = g_extern.filter.filter ? 
      g_extern.filter.out_rgb32 : 
      (g_extern.system.pix_fmt == RETRO_PIXEL_FORMAT_XRGB8888);

   tmp = (const input_driver_t*)driver.input;
   /* Need to grab the "real" video driver interface on a reinit. */
   find_video_driver();

#ifdef HAVE_THREADS
   if (g_settings.video.threaded && !g_extern.system.hw_render_callback.context_type)
   {
      /* Can't do hardware rendering with threaded driver currently. */
      RARCH_LOG("Starting threaded video driver ...\n");

      if (!rarch_threaded_video_init(&driver.video, &driver.video_data,
               &driver.input, &driver.input_data,
               driver.video, &video))
      {
         RARCH_ERR("Cannot open threaded video driver ... Exiting ...\n");
         rarch_fail(1, "init_video_input()");
      }
   }
   else
#endif
      driver.video_data = driver.video->init(&video, &driver.input,
            &driver.input_data);

   if (!driver.video_data)
   {
      RARCH_ERR("Cannot open video driver ... Exiting ...\n");
      rarch_fail(1, "init_video_input()");
   }

   driver.video_poke = NULL;
   if (driver.video->poke_interface)
      driver.video->poke_interface(driver.video_data, &driver.video_poke);

   if (driver.video->viewport_info && (!custom_vp->width ||
            !custom_vp->height))
   {
      /* Force custom viewport to have sane parameters. */
      custom_vp->width = width;
      custom_vp->height = height;
      driver.video->viewport_info(driver.video_data, custom_vp);
   }

   if (driver.video->set_rotation)
      driver.video->set_rotation(driver.video_data,
            (g_settings.video.rotation + g_extern.system.rotation) % 4);

   if (!driver.input)
   {
      /* Video driver didn't provide an input driver,
       * so we use configured one. */
      RARCH_LOG("Graphics driver did not initialize an input driver. Attempting to pick a suitable driver.\n");

      if (tmp)
         driver.input = tmp;
      else
         find_input_driver();

      if (!driver.input)
      {
         /* This should never really happen as tmp (driver.input) is always 
          * found before this in find_driver_input(), or we have aborted 
          * in a similar fashion anyways. */
         rarch_fail(1, "init_video_input()");
      }

      driver.input_data = driver.input->init();

      if (!driver.input_data)
      {
         RARCH_ERR("Cannot initialize input driver. Exiting ...\n");
         rarch_fail(1, "init_video_input()");
      }
   }

   rarch_main_command(RARCH_CMD_OVERLAY_DEINIT);
   rarch_main_command(RARCH_CMD_OVERLAY_INIT);

   g_extern.measure_data.frame_time_samples_count = 0;

   // Only do this once, for dummy to not show glowy garbage
   if (g_extern.libretro_dummy) {
   g_extern.frame_cache.width = 4;
   g_extern.frame_cache.height = 4;
   g_extern.frame_cache.pitch = 8;
   g_extern.frame_cache.data = &dummy_pixels;

   if (driver.video_poke && driver.video_poke->set_texture_frame)
      driver.video_poke->set_texture_frame(driver.video_data,
               &dummy_pixels, false, 1, 1, 1.0f);
  }

}

void init_drivers(int flags)
{
   if (flags & DRIVER_VIDEO)
      driver.video_data_own = false;
   if (flags & DRIVER_AUDIO)
      driver.audio_data_own = false;
   if (flags & DRIVER_INPUT)
      driver.input_data_own = false;
   if (flags & DRIVER_CAMERA)
      driver.camera_data_own = false;
   if (flags & DRIVER_LOCATION)
      driver.location_data_own = false;
   if (flags & DRIVER_OSK)
      driver.osk_data_own = false;

#ifdef HAVE_MENU
   /* By default, we want the menu to persist through driver reinits. */
   driver.menu_data_own = true;
#endif

   if (flags & (DRIVER_VIDEO | DRIVER_AUDIO))
      adjust_system_rates();

   if (flags & DRIVER_VIDEO)
   {
      g_extern.frame_count = 0;

      init_video_input();

      if (!driver.video_cache_context_ack
            && g_extern.system.hw_render_callback.context_reset)
         g_extern.system.hw_render_callback.context_reset();
      driver.video_cache_context_ack = false;

      g_extern.system.frame_time_last = 0;
   }

   if (flags & DRIVER_AUDIO)
      init_audio();

   /* Only initialize camera driver if we're ever going to use it. */
   if ((flags & DRIVER_CAMERA) && driver.camera_active)
      init_camera();

   /* Only initialize location driver if we're ever going to use it. */
   if ((flags & DRIVER_LOCATION) && driver.location_active)
      init_location();

   if (flags & DRIVER_OSK)
      init_osk();

#ifdef HAVE_MENU
   if (flags & DRIVER_MENU)
   {
      init_menu();

      if (driver.menu && driver.menu_ctx && driver.menu_ctx->context_reset)
         driver.menu_ctx->context_reset(driver.menu);
   }
#endif

   if (flags & (DRIVER_VIDEO | DRIVER_AUDIO))
   {
      /* Keep non-throttled state as good as possible. */
      if (driver.nonblock_state)
         driver_set_nonblock_state(driver.nonblock_state);
   }
}



static void compute_monitor_fps_statistics(void)
{
   double avg_fps = 0.0, stddev = 0.0;
   unsigned samples = 0;

   if (g_settings.video.threaded)
   {
      RARCH_LOG("Monitor FPS estimation is disabled for threaded video.\n");
      return;
   }

   if (g_extern.measure_data.frame_time_samples_count <
         2 * MEASURE_FRAME_TIME_SAMPLES_COUNT)
   {
      RARCH_LOG(
            "Does not have enough samples for monitor refresh rate estimation. Requires to run for at least %u frames.\n",
            2 * MEASURE_FRAME_TIME_SAMPLES_COUNT);
      return;
   }

   if (driver_monitor_fps_statistics(&avg_fps, &stddev, &samples))
   {
      RARCH_LOG("Average monitor Hz: %.6f Hz. (%.3f %% frame time deviation, based on %u last samples).\n",
            avg_fps, 100.0 * stddev, samples);
   }
}

static void compute_audio_buffer_statistics(void)
{
   unsigned i, low_water_size, high_water_size, avg, stddev;
   float avg_filled, deviation;
   uint64_t accum = 0, accum_var = 0;
   unsigned low_water_count = 0, high_water_count = 0;
   unsigned samples = min(g_extern.measure_data.buffer_free_samples_count,
         AUDIO_BUFFER_FREE_SAMPLES_COUNT);

   if (samples < 3)
      return;

   for (i = 1; i < samples; i++)
      accum += g_extern.measure_data.buffer_free_samples[i];

   avg = accum / (samples - 1);

   for (i = 1; i < samples; i++)
   {
      int diff = avg - g_extern.measure_data.buffer_free_samples[i];
      accum_var += diff * diff;
   }

   stddev = (unsigned)sqrt((double)accum_var / (samples - 2));

   avg_filled = 1.0f - (float)avg / g_extern.audio_data.driver_buffer_size;
   deviation = (float)stddev / g_extern.audio_data.driver_buffer_size;

   low_water_size = g_extern.audio_data.driver_buffer_size * 3 / 4;
   high_water_size = g_extern.audio_data.driver_buffer_size / 4;

   for (i = 1; i < samples; i++)
   {
      if (g_extern.measure_data.buffer_free_samples[i] >= low_water_size)
         low_water_count++;
      else if (g_extern.measure_data.buffer_free_samples[i] <= high_water_size)
         high_water_count++;
   }

   RARCH_LOG("Average audio buffer saturation: %.2f %%, standard deviation (percentage points): %.2f %%.\n",
         avg_filled * 100.0, deviation * 100.0);
   RARCH_LOG("Amount of time spent close to underrun: %.2f %%. Close to blocking: %.2f %%.\n",
         (100.0 * low_water_count) / (samples - 1),
         (100.0 * high_water_count) / (samples - 1));
}

static void uninit_audio(void)
{
   if (driver.audio_data && driver.audio)
      driver.audio->free(driver.audio_data);

   free(g_extern.audio_data.conv_outsamples);
   g_extern.audio_data.conv_outsamples = NULL;
   g_extern.audio_data.data_ptr        = 0;

   free(g_extern.audio_data.rewind_buf);
   g_extern.audio_data.rewind_buf = NULL;

   if (!g_settings.audio.enable)
   {
      driver.audio_active = false;
      return;
   }

   rarch_resampler_freep(&driver.resampler,
         &driver.resampler_data);

   free(g_extern.audio_data.data);
   g_extern.audio_data.data = NULL;

   free(g_extern.audio_data.outsamples);
   g_extern.audio_data.outsamples = NULL;

   rarch_main_command(RARCH_CMD_DSP_FILTER_DEINIT);

   compute_audio_buffer_statistics();
}

static void uninit_video_input(void)
{
   rarch_main_command(RARCH_CMD_OVERLAY_DEINIT);

   if (
         !driver.input_data_own &&
         (driver.input_data != driver.video_data) &&
         driver.input &&
         driver.input->free)
      driver.input->free(driver.input_data);

   if (
         !driver.video_data_own &&
         driver.video_data &&
         driver.video &&
         driver.video->free)
      driver.video->free(driver.video_data);

   deinit_pixel_converter();

   deinit_video_filter();

   compute_monitor_fps_statistics();
}

void uninit_drivers(int flags)
{
   if (flags & DRIVER_AUDIO)
      uninit_audio();

   if (flags & DRIVER_VIDEO)
   {
      if (g_extern.system.hw_render_callback.context_destroy &&
               !driver.video_cache_context)
            g_extern.system.hw_render_callback.context_destroy();
   }

#ifdef HAVE_MENU
   if (flags & DRIVER_MENU)
   {
      if (driver.menu && driver.menu_ctx && driver.menu_ctx->context_destroy)
            driver.menu_ctx->context_destroy(driver.menu);

         if (!driver.menu_data_own)
         {
            menu_free_list(driver.menu);
            menu_free(driver.menu);
            driver.menu = NULL;
         }
   }
#endif

   if (flags & DRIVERS_VIDEO_INPUT)
      uninit_video_input();

   if ((flags & DRIVER_VIDEO) && !driver.video_data_own)
      driver.video_data = NULL;

   if ((flags & DRIVER_CAMERA) && !driver.camera_data_own)
   {
      uninit_camera();
      driver.camera_data = NULL;
   }

   if ((flags & DRIVER_LOCATION) && !driver.location_data_own)
   {
      uninit_location();
      driver.location_data = NULL;
   }
   
   if ((flags & DRIVER_OSK) && !driver.osk_data_own)
   {
      uninit_osk();
      driver.osk_data = NULL;
   }

   if ((flags & DRIVER_INPUT) && !driver.input_data_own)
      driver.input_data = NULL;

   if ((flags & DRIVER_AUDIO) && !driver.audio_data_own)
      driver.audio_data = NULL;
}




bool driver_monitor_fps_statistics(double *refresh_rate,
      double *deviation, unsigned *sample_points)
{
   unsigned i;
   retro_time_t accum = 0, avg, accum_var = 0;
   unsigned samples = min(MEASURE_FRAME_TIME_SAMPLES_COUNT,
         g_extern.measure_data.frame_time_samples_count);

   if (g_settings.video.threaded || (samples < 2))
      return false;

   /* Measure statistics on frame time (microsecs), *not* FPS. */
   for (i = 0; i < samples; i++)
      accum += g_extern.measure_data.frame_time_samples[i];

#if 0
   for (i = 0; i < samples; i++)
      RARCH_LOG("Interval #%u: %d usec / frame.\n",
            i, (int)g_extern.measure_data.frame_time_samples[i]);
#endif

   avg = accum / samples;

   /* Drop first measurement. It is likely to be bad. */
   for (i = 0; i < samples; i++)
   {
      retro_time_t diff = g_extern.measure_data.frame_time_samples[i] - avg;
      accum_var += diff * diff;
   }

   *deviation = sqrt((double)accum_var / (samples - 1)) / avg;
   *refresh_rate = 1000000.0 / avg;
   *sample_points = samples;

   return true;
}






void *driver_video_resolve(const video_driver_t **drv)
{
#ifdef HAVE_THREADS
   if (g_settings.video.threaded
         && !g_extern.system.hw_render_callback.context_type)
      return rarch_threaded_video_resolve(drv);
#endif
   if (drv)
      *drv = driver.video;

   return driver.video_data;
}
