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

#ifndef __RARCH_GENERAL_H
#define __RARCH_GENERAL_H

#include <boolean.h>
#include <stdint.h>
#include <limits.h>
#include <setjmp.h>
#include "driver.h"
#include "message_queue.h"
#include "rewind.h"
#include "movie.h"
#include "autosave.h"
#include "cheats.h"
#include "audio/dsp_filter.h"
#include <compat/strl.h>
#include "core_options.h"
#include "core_info.h"
#include <retro_miscellaneous.h>
#include "gfx/filter.h"

#include "playlist.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef PACKAGE_VERSION
#ifdef __QNX__
/* FIXME - avoid too many decimal points in number error */
#define PACKAGE_VERSION "1002"
#else
#define PACKAGE_VERSION "1.0.0.2"
#endif
#endif

/* Platform-specific headers */

/* Windows */
#ifdef _WIN32
#ifdef _XBOX
#include <xtl.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <compat/posix_string.h>
#endif

/* Wii and PSL1GHT - for usleep (among others) */
#if defined(GEKKO) || defined(__PSL1GHT__) || defined(__QNX__)
#include <unistd.h>
#endif

/* PSP */
#if defined(PSP)
#include <pspthreadman.h>
#endif

#ifdef HAVE_COMMAND
#include "command.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PLAYERS 16

enum basic_event
{
   RARCH_CMD_NONE = 0,
   RARCH_CMD_RESET,
   RARCH_CMD_LOAD_CONTENT,
   RARCH_CMD_LOAD_CONTENT_PERSIST,
   RARCH_CMD_LOAD_CORE,
   RARCH_CMD_LOAD_STATE,
   RARCH_CMD_SAVE_STATE,
   RARCH_CMD_TAKE_SCREENSHOT,
   RARCH_CMD_PREPARE_DUMMY,
   RARCH_CMD_QUIT,
   RARCH_CMD_REINIT,
   RARCH_CMD_REWIND_DEINIT,
   RARCH_CMD_REWIND_INIT,
   RARCH_CMD_REWIND_TOGGLE,
   RARCH_CMD_AUTOSAVE_DEINIT,
   RARCH_CMD_AUTOSAVE_INIT,
   RARCH_CMD_AUTOSAVE_STATE,
   RARCH_CMD_AUDIO_STOP,
   RARCH_CMD_AUDIO_START,
   RARCH_CMD_AUDIO_MUTE_TOGGLE,
   RARCH_CMD_OVERLAY_INIT,
   RARCH_CMD_OVERLAY_DEINIT,
   RARCH_CMD_OVERLAY_SET_SCALE_FACTOR,
   RARCH_CMD_OVERLAY_SET_ALPHA_MOD,
   RARCH_CMD_OVERLAY_NEXT,
   RARCH_CMD_DSP_FILTER_INIT,
   RARCH_CMD_DSP_FILTER_DEINIT,
   RARCH_CMD_GPU_RECORD_DEINIT,
   RARCH_CMD_RECORD_INIT,
   RARCH_CMD_RECORD_DEINIT,
   RARCH_CMD_HISTORY_DEINIT,
   RARCH_CMD_HISTORY_INIT,
   RARCH_CMD_CORE_INFO_DEINIT,
   RARCH_CMD_CORE_INFO_INIT,
   RARCH_CMD_CORE_DEINIT,
   RARCH_CMD_CORE_INIT,
   RARCH_CMD_AUDIO_SET_BLOCKING_STATE,
   RARCH_CMD_AUDIO_SET_NONBLOCKING_STATE,
   RARCH_CMD_VIDEO_APPLY_STATE_CHANGES,
   RARCH_CMD_VIDEO_SET_BLOCKING_STATE,
   RARCH_CMD_VIDEO_SET_NONBLOCKING_STATE,
   RARCH_CMD_VIDEO_SET_ASPECT_RATIO,
   RARCH_CMD_AUTO_LOAD_STATE,
   RARCH_CMD_VIDEO_SET_GX_RESOLUTION,
   RARCH_CMD_VIDEO_RESOLUTION,
   RARCH_CMD_RESET_CONTEXT,
   RARCH_CMD_RESTART_RETROARCH,
   RARCH_CMD_QUIT_RETROARCH,
   RARCH_CMD_RESUME,
   RARCH_CMD_PAUSE_TOGGLE,
   RARCH_CMD_MENU_SAVE_CONFIG,
   RARCH_CMD_MENU_PAUSE_LIBRETRO,
   RARCH_CMD_SHADERS_APPLY_CHANGES,
   RARCH_CMD_SHADER_DIR_INIT,
   RARCH_CMD_SHADER_DIR_DEINIT,
   RARCH_CMD_CONTROLLERS_INIT,
   RARCH_CMD_SAVEFILES,
   RARCH_CMD_SAVEFILES_INIT,
   RARCH_CMD_SAVEFILES_DEINIT,
   RARCH_CMD_MSG_QUEUE_INIT,
   RARCH_CMD_MSG_QUEUE_DEINIT,
   RARCH_CMD_CHEATS_INIT,
   RARCH_CMD_CHEATS_DEINIT,
   RARCH_CMD_NETPLAY_INIT,
   RARCH_CMD_NETPLAY_DEINIT,
   RARCH_CMD_NETPLAY_FLIP_PLAYERS,
   RARCH_CMD_BSV_MOVIE_INIT,
   RARCH_CMD_BSV_MOVIE_DEINIT,
   RARCH_CMD_COMMAND_INIT,
   RARCH_CMD_COMMAND_DEINIT,
   RARCH_CMD_DRIVERS_DEINIT,
   RARCH_CMD_DRIVERS_INIT,
   RARCH_CMD_AUDIO_REINIT,
   RARCH_CMD_TEMPORARY_CONTENT_DEINIT,
   RARCH_CMD_SUBSYSTEM_FULLPATHS_DEINIT,
   RARCH_CMD_LOG_FILE_DEINIT,
   RARCH_CMD_DISK_EJECT_TOGGLE,
   RARCH_CMD_DISK_NEXT,
   RARCH_CMD_DISK_PREV,
   RARCH_CMD_RUMBLE_STOP,
   RARCH_CMD_GRAB_MOUSE_TOGGLE,
   RARCH_CMD_FULLSCREEN_TOGGLE,
   RARCH_CMD_PERFCNT_REPORT_FRONTEND_LOG,
};

enum action_state
{
   RARCH_ACTION_STATE_NONE = 0,
   RARCH_ACTION_STATE_LOAD_CONTENT,
   RARCH_ACTION_STATE_MENU_RUNNING,
   RARCH_ACTION_STATE_MENU_RUNNING_FINISHED,
   RARCH_ACTION_STATE_QUIT,
   RARCH_ACTION_STATE_FORCE_QUIT,
};

enum sound_mode_enums
{
   SOUND_MODE_NORMAL = 0,
#ifdef HAVE_RSOUND
   SOUND_MODE_RSOUND,
#endif
#ifdef HAVE_HEADSET
   SOUND_MODE_HEADSET,
#endif
   SOUND_MODE_LAST
};

struct defaults
{
   char menu_config_dir[PATH_MAX];
   char config_path[PATH_MAX];
   char core_path[PATH_MAX];
   char autoconfig_dir[PATH_MAX];
   char audio_filter_dir[PATH_MAX];
   char assets_dir[PATH_MAX];
   char core_dir[PATH_MAX];
   char core_info_dir[PATH_MAX];
   char overlay_dir[PATH_MAX];
   char port_dir[PATH_MAX];
   char shader_dir[PATH_MAX];
   char savestate_dir[PATH_MAX];
   char resampler_dir[PATH_MAX];
   char sram_dir[PATH_MAX];
   char screenshot_dir[PATH_MAX];
   char system_dir[PATH_MAX];
   char playlist_dir[PATH_MAX];

   struct
   {
      int out_latency;
      float video_refresh_rate;
      bool video_threaded_enable;
   } settings; 

   content_playlist_t *history;
};

/* All config related settings go here. */

struct settings
{
   struct 
   {
      char driver[32];
      char context_driver[32];
      float scale;
      bool fullscreen;
      bool windowed_fullscreen;
      unsigned monitor_index;
      unsigned fullscreen_x;
      unsigned fullscreen_y;
      bool vsync;
      bool hard_sync;
      bool black_frame_insertion;
      unsigned swap_interval;
      unsigned hard_sync_frames;
      unsigned frame_delay;
#ifdef GEKKO
      unsigned viwidth;
      bool vfilter;
	  bool rgui_reset;
	  unsigned vres;
#endif
      unsigned hover_color;
	  unsigned text_color;
      bool smooth;
      bool force_aspect;
      bool crop_overscan;
      float aspect_ratio;
      bool aspect_ratio_auto;
      bool scale_integer;
      unsigned aspect_ratio_idx;
      unsigned rotation;

      char shader_path[PATH_MAX];
      bool shader_enable;

      char softfilter_plugin[PATH_MAX];
      float refresh_rate;
      bool threaded;

      char filter_dir[PATH_MAX];
      char shader_dir[PATH_MAX];

      char font_path[PATH_MAX];
      float font_size;
      bool font_enable;
      float msg_pos_x;
      float msg_pos_y;
      float msg_color_r;
      float msg_color_g;
      float msg_color_b;

      bool disable_composition;

      bool post_filter_record;
      bool gpu_record;
      bool gpu_screenshot;

      bool allow_rotate;
      bool shared_context;
      bool force_srgb_disable;
   } video;

#ifdef HAVE_MENU
   struct 
   {
      char driver[32];
      bool pause_libretro;
   } menu;
#endif

   struct
   {
      char driver[32];
      char device[PATH_MAX];
      bool allow;
      unsigned width;
      unsigned height;
   } camera;

   struct
   {
      char driver[32];
      bool allow;
      int update_interval_ms;
      int update_interval_distance;
   } location;

   struct
   {
      char driver[32];
      bool enable;
   } osk;

   struct
   {
      char driver[32];
      bool enable;
      unsigned out_rate;
      unsigned block_frames;
      char device[PATH_MAX];
      unsigned latency;
      bool sync;

      char dsp_plugin[PATH_MAX];
      char filter_dir[PATH_MAX];

      bool rate_control;
      float rate_control_delta;
      float volume; /* dB scale. */
      char resampler[32];
   } audio;

   struct
   {
      char driver[32];
      char joypad_driver[32];
      char keyboard_layout[64];
      struct retro_keybind binds[MAX_PLAYERS][RARCH_BIND_LIST_END];

      /* Set by autoconfiguration in joypad_autoconfig_dir.
       * Does not override main binds. */
      struct retro_keybind autoconf_binds[MAX_PLAYERS][RARCH_BIND_LIST_END];
      bool autoconfigured[MAX_PLAYERS];

      unsigned libretro_device[MAX_PLAYERS];
      unsigned analog_dpad_mode[MAX_PLAYERS];

      float axis_threshold;
      unsigned joypad_map[MAX_PLAYERS];
      unsigned device[MAX_PLAYERS];
      char device_names[MAX_PLAYERS][64];
      bool autodetect_enable;
      bool netplay_client_swap_input;

      unsigned turbo_period;
      unsigned turbo_duty_cycle;

      char overlay[PATH_MAX];
      float overlay_opacity;
      float overlay_scale;

      char autoconfig_dir[PATH_MAX];
   } input;

   int state_slot;

   char core_options_path[PATH_MAX];
   char content_history_path[PATH_MAX];
   unsigned content_history_size;

   char libretro[PATH_MAX];
   char libretro_directory[PATH_MAX];
   unsigned libretro_log_level;
   char libretro_info_path[PATH_MAX];
   char cheat_database[PATH_MAX];
   char cheat_settings_path[PATH_MAX];

   char resampler_directory[PATH_MAX];
   char screenshot_directory[PATH_MAX];
   char system_directory[PATH_MAX];

   char extraction_directory[PATH_MAX];
   char playlist_directory[PATH_MAX];

   bool history_list_enable;
   bool rewind_enable;
   size_t rewind_buffer_size;
   unsigned rewind_granularity;

   float slowmotion_ratio;
   float fastforward_ratio;
   bool fastforward_ratio_throttle_enable;

   bool pause_nonactive;
   unsigned autosave_interval;

   bool block_sram_overwrite;
   bool savestate_auto_index;
   bool savestate_auto_save;
   bool savestate_auto_load;

   bool network_cmd_enable;
   uint16_t network_cmd_port;
   bool stdin_cmd_enable;

   char content_directory[PATH_MAX];
   char assets_directory[PATH_MAX];
   char menu_config_directory[PATH_MAX];
#if defined(HAVE_MENU)
   char menu_content_directory[PATH_MAX];
   bool menu_show_start_screen;
#endif
   bool fps_show;
   bool load_dummy_on_core_shutdown;

   bool core_specific_config;

   char username[32];
   unsigned int user_language;

   bool config_save_on_exit;
};

typedef struct rarch_resolution
{
   unsigned idx;
   unsigned id;
} rarch_resolution_t;

typedef struct rarch_viewport
{
   int x;
   int y;
   unsigned width;
   unsigned height;
   unsigned full_width;
   unsigned full_height;
} rarch_viewport_t;

#define AUDIO_BUFFER_FREE_SAMPLES_COUNT (8 * 1024)
#define MEASURE_FRAME_TIME_SAMPLES_COUNT (2 * 1024)

/* All run-time- / command line flag-related globals go here. */

struct global
{
   bool verbosity;
   bool perfcnt_enable;
   bool force_fullscreen;
   bool core_shutdown_initiated;

   struct string_list *temporary_content;

   core_info_list_t *core_info;
   core_info_t *core_info_current;

   uint32_t content_crc;

   char gb_rom_path[PATH_MAX];
   char bsx_rom_path[PATH_MAX];
   char sufami_rom_path[2][PATH_MAX];
   bool has_set_save_path;
   bool has_set_state_path;
   bool has_set_libretro_device[MAX_PLAYERS];
   bool has_set_libretro;
   bool has_set_libretro_directory;
   bool has_set_verbosity;

   bool has_set_netplay_mode;
   bool has_set_username;
   bool has_set_netplay_ip_address;
   bool has_set_netplay_delay_frames;
   bool has_set_netplay_ip_port;

   /* Config associated with global "default" config. */
   char config_path[PATH_MAX];
   char append_config_path[PATH_MAX];
   char input_config_path[PATH_MAX];

#ifdef HAVE_FILE_LOGGER
   char default_log_file[PATH_MAX];
#endif
   
   char basename[PATH_MAX];
   char fullpath[PATH_MAX];

   /* A list of save types and associated paths for all content. */
   struct string_list *savefiles;

   /* For --subsystem content. */
   char subsystem[PATH_MAX];
   struct string_list *subsystem_fullpaths;

   char savefile_name[PATH_MAX];
   char savestate_name[PATH_MAX];

   /* Used on reentrancy to use a savestate dir. */
   char savefile_dir[PATH_MAX];
   char savestate_dir[PATH_MAX];

#ifdef HAVE_OVERLAY
   char overlay_dir[PATH_MAX];
#endif

   bool block_patch;
   bool ups_pref;
   bool bps_pref;
   bool ips_pref;
   char ups_name[PATH_MAX];
   char bps_name[PATH_MAX];
   char ips_name[PATH_MAX];


   struct
   {
      retro_time_t minimum_frame_time;
      retro_time_t last_frame_time;
   } frame_limit;

   struct
   {
      struct retro_system_info info;
      struct retro_system_av_info av_info;
      float aspect_ratio;

      unsigned rotation;
      bool shutdown;
      unsigned performance_level;
      enum retro_pixel_format pix_fmt;

      bool block_extract;
      bool force_nonblock;
      bool no_content;

      const char *input_desc_btn[MAX_PLAYERS][RARCH_FIRST_CUSTOM_BIND];
      char valid_extensions[PATH_MAX];
      
      retro_keyboard_event_t key_event;

      struct retro_audio_callback audio_callback;

      struct retro_disk_control_callback disk_control; 
      struct retro_hw_render_callback hw_render_callback;
      struct retro_camera_callback camera_callback;
      struct retro_location_callback location_callback;

      struct retro_frame_time_callback frame_time;
      retro_usec_t frame_time_last;

      core_option_manager_t *core_options;

      struct retro_subsystem_info *special;
      unsigned num_special;

      struct retro_controller_info *ports;
      unsigned num_ports;
   } system;

   struct
   {
      float *data;

      size_t data_ptr;
      size_t chunk_size;
      size_t nonblock_chunk_size;
      size_t block_chunk_size;

      double src_ratio;
      float in_rate;

      bool use_float;
      bool mute;

      float *outsamples;
      int16_t *conv_outsamples;

      int16_t *rewind_buf;
      size_t rewind_ptr;
      size_t rewind_size;

      rarch_dsp_filter_t *dsp;

      bool rate_control; 
      double orig_src_ratio;
      size_t driver_buffer_size;

      float volume_gain;
   } audio_data;

   struct
   {
      unsigned buffer_free_samples[AUDIO_BUFFER_FREE_SAMPLES_COUNT];
      uint64_t buffer_free_samples_count;

      retro_time_t frame_time_samples[MEASURE_FRAME_TIME_SAMPLES_COUNT];
      uint64_t frame_time_samples_count;
   } measure_data;

   struct
   {
      rarch_softfilter_t *filter;

      void *buffer;
      unsigned scale;
      unsigned out_bpp;
      bool out_rgb32;
   } filter;

#ifdef HAVE_MENU
   struct
   {
      struct retro_system_info info;
      bool bind_mode_keyboard;
   } menu;
#endif

   msg_queue_t *msg_queue;

   bool exec;

   /* Rewind support. */
   state_manager_t *state_manager;
   size_t state_size;
   bool frame_is_reverse;

   /* Movie playback/recording support. */
   struct
   {
      bsv_movie_t *movie;
      char movie_path[PATH_MAX];
      bool movie_playback;
      bool eof_exit;

      /* Immediate playback/recording. */
      char movie_start_path[PATH_MAX];
      bool movie_start_recording;
      bool movie_start_playback;
      bool movie_end;
   } bsv;

   struct
   {
      bool (*cb_init)(void *data);
      bool (*cb_callback)(void *data);
   } osk;

   bool sram_load_disable;
   bool sram_save_disable;
   bool use_sram;

   /* Lifecycle state checks. */
   bool is_paused;
   bool is_menu;
   bool is_slowmotion;

   /* Turbo support. */
   bool turbo_frame_enable[MAX_PLAYERS];
   uint16_t turbo_enable[MAX_PLAYERS];
   unsigned turbo_count;

   /* Autosave support. */
   autosave_t **autosave;
   unsigned num_autosave;

#ifdef HAVE_NETPLAY
   /* Netplay. */
   char netplay_server[PATH_MAX];
   bool netplay_enable;
   bool netplay_is_client;
   bool netplay_is_spectate;
   unsigned netplay_sync_frames;
   unsigned netplay_port;
#endif

   /* Recording. */
   char record_path[PATH_MAX];
   char record_config[PATH_MAX];
   bool recording_enable;
   unsigned record_width;
   unsigned record_height;

   uint8_t *record_gpu_buffer;
   size_t record_gpu_width;
   size_t record_gpu_height;

   struct
   {
      const void *data;
      unsigned width;
      unsigned height;
      size_t pitch;
   } frame_cache;

   unsigned frame_count;
   unsigned max_frames;

   char title_buf[64];

   struct
   {
      struct string_list *list;
      size_t ptr;
   } shader_dir;

   struct
   {
      struct string_list *list;
      size_t ptr;
   } filter_dir;

   char sha256[64 + 1];

   cheat_manager_t *cheat;

   bool block_config_read;

   /* Settings and/or global state that is specific to 
    * a console-style implementation. */
   struct
   {
      struct
      {
         struct
         {
            rarch_resolution_t current;
            rarch_resolution_t initial;
            uint32_t *list;
            unsigned count;
            bool check;
         } resolutions;


         struct
         {
            rarch_viewport_t custom_vp;
         } viewports;

         unsigned gamma_correction;
         unsigned char flicker_filter_index;
         unsigned char soft_filter_index;
         bool pal_enable;
         bool pal60_enable;
      } screen;

      struct
      {
         unsigned mode;
         bool system_bgm_enable;
      } sound;

      bool flickerfilter_enable;
      bool softfilter_enable;
   } console;

   uint64_t lifecycle_state;

   /* If this is non-NULL. RARCH_LOG and friends 
    * will write to this file. */
   FILE *log_file;

   bool main_is_init;
   bool error_in_init;
   char error_string[PATH_MAX];
   jmp_buf error_sjlj_context;

   bool libretro_no_content;
   bool libretro_dummy;

   /* Config file associated with per-core configs. */
   char core_specific_config_path[PATH_MAX];

   retro_keyboard_event_t frontend_key_event;
};

struct rarch_main_wrap
{
   const char *content_path;
   const char *sram_path;
   const char *state_path;
   const char *config_path;
   const char *libretro_path;
   bool verbose;
   bool no_content;

   bool touched;
};

/* Public data structures. */
extern struct settings g_settings;
extern struct global g_extern;
extern struct defaults g_defaults;

/* Public functions. */
void config_load(void);
const char *config_get_default_camera(void);
const char *config_get_default_location(void);
const char *config_get_default_osk(void);
const char *config_get_default_video(void);
const char *config_get_default_audio(void);
const char *config_get_default_audio_resampler(void);
const char *config_get_default_input(void);
const char *config_get_default_joypad(void);
#ifdef HAVE_MENU
const char *config_get_default_menu(void);
#endif

bool config_save_file(const char *path);

void rarch_main_state_new(void);
void rarch_main_state_free(void);

int rarch_main(int argc, char *argv[]);
bool rarch_replace_config(const char *path);
void rarch_main_init_wrap(const struct rarch_main_wrap *args,
      int *argc, char **argv);
int rarch_main_init(int argc, char *argv[]);
void rarch_main_set_state(unsigned action);
bool rarch_main_command(unsigned action);
int rarch_main_iterate(void);
void rarch_main_deinit(void);
void rarch_render_cached_frame(void);
void rarch_disk_control_set_eject(bool state, bool log);
void rarch_disk_control_set_index(unsigned index);
void rarch_disk_control_append_image(const char *path);

void rarch_playlist_load_content(content_playlist_t *playlist,
      unsigned index);

int rarch_defer_core(core_info_list_t *data,
      const char *dir, const char *path, char *deferred_path,
      size_t sizeof_deferred_path);

void rarch_update_system_info(struct retro_system_info *info,
      bool *load_no_content);

void rarch_recording_dump_frame(const void *data, unsigned width,
      unsigned height, size_t pitch);

#ifdef __cplusplus
}
#endif

static inline float db_to_gain(float db)
{
   return powf(10.0f, db / 20.0f);
}

static inline void rarch_fail(int error_code, const char *error)
{
   /* We cannot longjmp unless we're in rarch_main_init().
    * If not, something went very wrong, and we should 
    * just exit right away. */
   rarch_assert(g_extern.error_in_init);

   strlcpy(g_extern.error_string, error,
         sizeof(g_extern.error_string));
   longjmp(g_extern.error_sjlj_context, error_code);
}

#endif


