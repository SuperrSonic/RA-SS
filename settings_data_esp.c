/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2013-2014 - Jason Fetters
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

#include "settings_data.h"
#include "dynamic.h"
#include <file/file_path.h>
#include "input/input_common.h"
#include "config.def.h"
#include "retroarch_logger.h"

#ifdef APPLE
#include "input/apple_keycode.h"
#endif

#if defined(__CELLOS_LV2__)
#include <sdk_version.h>

#if (CELL_SDK_VERSION > 0x340000)
#include <sysutil/sysutil_bgmplayback.h>
#endif

#endif

#ifdef HAVE_MENU
#include "frontend/menu/menu_list.h"
#include "frontend/menu/menu_entries.h"
#include "frontend/menu/menu_input_line_cb.h"
#include "frontend/menu/menu_shader.h"
#endif

/* Translation binaries */

// Core = Núcleo
static const char nucleo[7] = { 0x4E, 0xFA, 0x63, 0x6C, 0x65, 0x6F, 0x00 };

// Load Content = Selecciona el juego
//static const char selecciona_juego[20] = { 0x53, 0x65, 0x6C, 0x65, 0x63, 0x63, 0x69, 0x6F, 0x6E, 0x61, 0x20, 0x65, 0x6C, 0x20, 0x6A, 0x75, 0x65, 0x67, 0x6F, 0x00 };

// Core Options = Opciones del núcleo
static const char nucleo_opciones[20] = { 0x4F, 0x70, 0x63, 0x69, 0x6F, 0x6E, 0x65, 0x73, 0x20, 0x64, 0x65, 0x6C, 0x20, 0x6E, 0xFA, 0x63, 0x6C, 0x65, 0x6F, 0x00 };

// Save State = Crear punto de restauración
//static const char crear_punto[28] = { 0x43, 0x72, 0x65, 0x61, 0x72, 0x20, 0x70, 0x75, 0x6E, 0x74, 0x6F, 0x20, 0x64, 0x65, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61, 0x75, 0x72, 0x61, 0x63, 0x69, 0xF3, 0x6E, 0x00 };

// Load State
//static const char cargar_punto[29] = { 0x43, 0x61, 0x72, 0x67, 0x61, 0x72, 0x20, 0x70, 0x75, 0x6E, 0x74, 0x6F, 0x20, 0x64, 0x65, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61, 0x75, 0x72, 0x61, 0x63, 0x69, 0xF3, 0x6E, 0x00 };

// Menu = Menú
//static const char menu_simpl[5] = { 0x4D, 0x65, 0x6E, 0xFA, 0x00 };

// Title Position = Posición del título
static const char title_pos[20] = { 0x50, 0x6F, 0x73, 0x69, 0x63, 0x69, 0xF3, 0x6E, 0x20, 0x64, 0x65, 0x6C, 0x20, 0x74, 0xED, 0x74, 0x75, 0x6C, 0x6F, 0x00 };

// pos x
static const char pos_generalx[11] = { 0x50, 0x6F, 0x73, 0x69, 0x63, 0x69, 0xF3, 0x6E, 0x20, 0x58, 0x00 };

// pos y
static const char pos_generaly[11] = { 0x50, 0x6F, 0x73, 0x69, 0x63, 0x69, 0xF3, 0x6E, 0x20, 0x59, 0x00 };

// YES = Sí
static const char sip[3] = { 0x53, 0xED, 0x00 };

// Aspect Ratio Index = Índice relación de aspecto
static const char indice_aspect[27] = { 0xCD, 0x6E, 0x64, 0x69, 0x63, 0x65, 0x20, 0x72, 0x65, 0x6C, 0x61, 0x63, 0x69, 0xF3, 0x6E, 0x20, 0x64, 0x65, 0x20, 0x61, 0x73, 0x70, 0x65, 0x63, 0x74, 0x6F, 0x00 };

// Integer Scale = Por íntegro
static const char por_int[12] = { 0x50, 0x6F, 0x72, 0x20, 0xED, 0x6E, 0x74, 0x65, 0x67, 0x72, 0x6F, 0x00 };

// Rotation = Rotación
static const char rotacion[9] = { 0x52, 0x6F, 0x74, 0x61, 0x63, 0x69, 0xF3, 0x6E, 0x00 };


static void get_input_config_prefix(char *buf, size_t sizeof_buf,
      rarch_setting_t *setting)
{
   if (!buf || !setting)
      return;

   snprintf(buf, sizeof_buf, "input%cplayer%u",
         setting->index ? '_' : '\0', setting->index);
}

static void get_input_config_key(char *buf, size_t sizeof_buf,
      rarch_setting_t* setting, const char* type)
{
   char prefix[32];

   if (!buf || !setting)
      return;

   get_input_config_prefix(prefix, sizeof(prefix), setting);
   snprintf(buf, sizeof_buf, "%s_%s%c%s", prefix, setting->name,
         type ? '_' : '\0', type);
}

#ifdef APPLE
/* FIXME - make portable */

static void get_key_name(char *buf, size_t sizeof_buf,
      rarch_setting_t* setting)
{
   uint32_t hidkey, i;

   if (!buf || !setting)
      return;
   if (BINDFOR(*setting).key == RETROK_UNKNOWN)
      return;

   hidkey = input_translate_rk_to_keysym(BINDFOR(*setting).key);

   for (i = 0; apple_key_name_map[i].hid_id; i++)
   {
      if (apple_key_name_map[i].hid_id == hidkey)
      {
         strlcpy(buf, apple_key_name_map[i].keyname, sizeof_buf);
         break;
      }
   }
}
#endif

static void get_button_name(char *buf, size_t sizeof_buf,
      rarch_setting_t* setting)
{
   if (!buf || !setting)
      return;
   if (BINDFOR(*setting).joykey == NO_BTN)
      return;

   snprintf(buf, sizeof_buf, "%lld",
         (long long int)(BINDFOR(*setting).joykey));
}

static void get_axis_name(char *buf, size_t sizeof_buf,
      rarch_setting_t* setting)
{
   uint32_t joyaxis;

   if (!buf || !setting)
      return;

   joyaxis = BINDFOR(*setting).joyaxis;

   if (AXIS_NEG_GET(joyaxis) != AXIS_DIR_NONE)
      snprintf(buf, sizeof_buf, "-%u", AXIS_NEG_GET(joyaxis));
   else if (AXIS_POS_GET(joyaxis) != AXIS_DIR_NONE)
      snprintf(buf, sizeof_buf, "+%u", AXIS_POS_GET(joyaxis));
}

void setting_data_reset_setting(rarch_setting_t* setting)
{
   if (!setting)
      return;

   switch (setting->type)
   {
      case ST_BOOL:
         *setting->value.boolean          = setting->default_value.boolean;
         break;
      case ST_INT:
         *setting->value.integer          = setting->default_value.integer;
         break;
      case ST_UINT:
         *setting->value.unsigned_integer = setting->default_value.unsigned_integer;
         break;
      case ST_FLOAT:
         *setting->value.fraction         = setting->default_value.fraction;
         break;
      case ST_BIND:
         *setting->value.keybind          = *setting->default_value.keybind;
         break;
      case ST_STRING:
      case ST_PATH:
      case ST_DIR:
         if (setting->default_value.string)
         {
            if (setting->type == ST_STRING)
               strlcpy(setting->value.string, setting->default_value.string,
                     setting->size);
            else
               fill_pathname_expand_special(setting->value.string,
                     setting->default_value.string, setting->size);
         }
         break;
         /* TODO */
      case ST_ACTION:
         break;
      case ST_HEX:
         break;
      case ST_GROUP:
         break;
      case ST_SUB_GROUP:
         break;
      case ST_END_GROUP:
         break;
      case ST_END_SUB_GROUP:
         break;
      case ST_NONE:
         break;
   }

   if (setting->change_handler)
      setting->change_handler(setting);
}

void setting_data_reset(rarch_setting_t* settings)
{
   for (; settings->type != ST_NONE; settings++)
      setting_data_reset_setting(settings);
}

static bool setting_data_load_config(
      rarch_setting_t* settings, config_file_t* cfg)
{
   if (!settings || !cfg)
      return false;

   for (; settings->type != ST_NONE; settings++)
   {
      switch (settings->type)
      {
         case ST_BOOL:
            config_get_bool(cfg, settings->name,
                  settings->value.boolean);
            break;
         case ST_PATH:
         case ST_DIR:
            config_get_path(cfg, settings->name,
                  settings->value.string, settings->size);
            break;
         case ST_STRING:
            config_get_array(cfg, settings->name,
                  settings->value.string, settings->size);
            break;
         case ST_INT:
            config_get_int(cfg, settings->name,
                  settings->value.integer);

            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.integer < settings->min)
                  *settings->value.integer = settings->min;
               if (*settings->value.integer > settings->max)
                  *settings->value.integer = settings->max;
            }
            break;
         case ST_UINT:
            config_get_uint(cfg, settings->name,
                  settings->value.unsigned_integer);

            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.unsigned_integer < settings->min)
                  *settings->value.unsigned_integer = settings->min;
               if (*settings->value.unsigned_integer > settings->max)
                  *settings->value.unsigned_integer = settings->max;
            }
            break;
         case ST_FLOAT:
            config_get_float(cfg, settings->name,
                  settings->value.fraction);

            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.fraction < settings->min)
                  *settings->value.fraction = settings->min;
               if (*settings->value.fraction > settings->max)
                  *settings->value.fraction = settings->max;
            }
            break;         
         case ST_BIND:
            {
               char prefix[32];
               get_input_config_prefix(prefix, sizeof(prefix), settings);
               input_config_parse_key(cfg, prefix, settings->name,
                     settings->value.keybind);
               input_config_parse_joy_button(cfg, prefix, settings->name,
                     settings->value.keybind);
               input_config_parse_joy_axis(cfg, prefix, settings->name,
                     settings->value.keybind);
            }
            break;
         case ST_ACTION:
            break;
            /* TODO */
         case ST_HEX:
            break;
         case ST_GROUP:
            break;
         case ST_SUB_GROUP:
            break;
         case ST_END_GROUP:
            break;
         case ST_END_SUB_GROUP:
            break;
         case ST_NONE:
            break;
      }

      if (settings->change_handler)
         settings->change_handler(settings);
   }

   return true;
}

bool setting_data_load_config_path(rarch_setting_t* settings,
      const char* path)
{
   config_file_t *cfg = (config_file_t*)config_file_new(path);

   if (!cfg)
      return NULL;

   setting_data_load_config(settings, cfg);
   config_file_free(cfg);

   return cfg;
}

bool setting_data_save_config(rarch_setting_t* settings,
      config_file_t* cfg)
{
   if (!settings || !cfg)
      return false;

   for (; settings->type != ST_NONE; settings++)
   {
      switch (settings->type)
      {
         case ST_BOOL:
            config_set_bool(cfg, settings->name,
                  *settings->value.boolean);
            break;
         case ST_PATH:
         case ST_DIR:
            config_set_path(cfg, settings->name,
                  settings->value.string);
            break;
         case ST_STRING:
            config_set_string(cfg, settings->name,
                  settings->value.string);
            break;
         case ST_INT:
            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.integer < settings->min)
                  *settings->value.integer = settings->min;
               if (*settings->value.integer > settings->max)
                  *settings->value.integer = settings->max;
            }
            config_set_int(cfg, settings->name, *settings->value.integer);
            break;
         case ST_UINT:
            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.unsigned_integer < settings->min)
                  *settings->value.unsigned_integer = settings->min;
               if (*settings->value.unsigned_integer > settings->max)
                  *settings->value.unsigned_integer = settings->max;
            }
            config_set_uint64(cfg, settings->name,
                  *settings->value.unsigned_integer);
            break;
         case ST_FLOAT:
            if (settings->flags & SD_FLAG_HAS_RANGE)
            {
               if (*settings->value.fraction < settings->min)
                  *settings->value.fraction = settings->min;
               if (*settings->value.fraction > settings->max)
                  *settings->value.fraction = settings->max;
            }
            config_set_float(cfg, settings->name, *settings->value.fraction);
            break;
         case ST_BIND:
            {
               char button_name[32], axis_name[32], key_name[32],
                    input_config_key[64];
               strlcpy(button_name, "nul", sizeof(button_name));
               strlcpy(axis_name,   "nul", sizeof(axis_name));
               strlcpy(key_name,    "nul", sizeof(key_name));
               strlcpy(input_config_key, "nul", sizeof(input_config_key));

               get_button_name(button_name, sizeof(button_name), settings);
               get_axis_name(axis_name, sizeof(axis_name), settings);
#ifdef APPLE
               get_key_name(key_name, sizeof(key_name), settings);
#endif
               get_input_config_key(input_config_key,
                     sizeof(input_config_key), settings, 0);
               config_set_string(cfg, input_config_key, key_name);
               get_input_config_key(input_config_key,
                     sizeof(input_config_key), settings, "btn");
               config_set_string(cfg, input_config_key, button_name);
               get_input_config_key(input_config_key,
                     sizeof(input_config_key), settings, "axis");
               config_set_string(cfg, input_config_key, axis_name);
            }
            break;
            /* TODO */
         case ST_ACTION:
         case ST_HEX:
            break;
         case ST_GROUP:
            break;
         case ST_SUB_GROUP:
            break;
         case ST_END_GROUP:
            break;
         case ST_END_SUB_GROUP:
            break;
         case ST_NONE:
            break;
      }
   }

   return true;
}

rarch_setting_t* setting_data_find_setting(rarch_setting_t* setting,
      const char* name)
{
   bool found = false;

   if (!setting || !name)
      return NULL;

   for (; setting->type != ST_NONE; setting++)
   {
      if (setting->type <= ST_GROUP && !strcmp(setting->name, name))
      {
         found = true;
         break;
      }
   }

   if (!found)
      return NULL;

   if (setting->short_description && setting->short_description[0] == '\0')
      return NULL;

   if (setting->read_handler)
      setting->read_handler(setting);

   return setting;
}

void setting_data_set_with_string_representation(rarch_setting_t* setting,
      const char* value)
{
   if (!setting || !value)
      return;

   switch (setting->type)
   {
      case ST_INT:
         sscanf(value, "%d", setting->value.integer);
         if (setting->flags & SD_FLAG_HAS_RANGE)
         {
            if (*setting->value.integer < setting->min)
               *setting->value.integer = setting->min;
            if (*setting->value.integer > setting->max)
               *setting->value.integer = setting->max;
         }
         break;
      case ST_UINT:
         sscanf(value, "%u", setting->value.unsigned_integer);
         if (setting->flags & SD_FLAG_HAS_RANGE)
         {
            if (*setting->value.unsigned_integer < setting->min)
               *setting->value.unsigned_integer = setting->min;
            if (*setting->value.unsigned_integer > setting->max)
               *setting->value.unsigned_integer = setting->max;
         }
         break;      
      case ST_FLOAT:
         sscanf(value, "%f", setting->value.fraction);
         if (setting->flags & SD_FLAG_HAS_RANGE)
         {
            if (*setting->value.fraction < setting->min)
               *setting->value.fraction = setting->min;
            if (*setting->value.fraction > setting->max)
               *setting->value.fraction = setting->max;
         }
         break;
      case ST_PATH:
      case ST_DIR:
      case ST_STRING:
         strlcpy(setting->value.string, value, setting->size);
         break;

         /* TODO */
      case ST_ACTION:
         break;
      case ST_HEX:
         break;
      case ST_GROUP:
         break;
      case ST_SUB_GROUP:
         break;
      case ST_END_GROUP:
         break;
      case ST_END_SUB_GROUP:
         break;
      case ST_NONE:
         break;
      case ST_BOOL:
         break;
      case ST_BIND:
         break;
   }

   if (setting->change_handler)
      setting->change_handler(setting);
}

static void menu_common_setting_set_label_st_bool(rarch_setting_t *setting,
      char *type_str, size_t type_str_size)
{
   if (setting)
      strlcpy(type_str, *setting->value.boolean ? setting->boolean.on_label :
            setting->boolean.off_label, type_str_size);
}

static void menu_common_setting_set_label_st_uint(rarch_setting_t *setting,
      char *type_str, size_t type_str_size)
{
   if (!setting)
      return;

  /* if (!strcmp(setting->name, "video_monitor_index"))
   {
      if (*setting->value.unsigned_integer)
         snprintf(type_str, type_str_size, "%u",
               *setting->value.unsigned_integer);
      else
         strlcpy(type_str, "0 (Auto)", type_str_size);
   }*/
   if (!strcmp(setting->name, "video_rotation"))
   {
      strlcpy(type_str, rotation_lut[*setting->value.unsigned_integer],
            type_str_size);
   }
   else if (!strcmp(setting->name, "aspect_ratio_index"))
   {
      strlcpy(type_str,
            aspectratio_lut[*setting->value.unsigned_integer].name,
            type_str_size);
   }
   else if (!strncmp(setting->name, "input_libretro_device_p", 23))
   {
      const struct retro_controller_description *desc = NULL;
      if (setting->index_offset < g_extern.system.num_ports)
         desc = libretro_find_controller_description(
               &g_extern.system.ports[setting->index_offset],
               g_settings.input.libretro_device
               [setting->index_offset]);

      const char *name = desc ? desc->desc : NULL;
      if (!name)
      {
         /* Find generic name. */

         switch (g_settings.input.libretro_device
               [setting->index_offset])
         {
            case RETRO_DEVICE_NONE:
               name = "None";
               break;
            case RETRO_DEVICE_JOYPAD:
               name = "RetroPad";
               break;
            case RETRO_DEVICE_ANALOG:
               name = "RetroPad w/ Analog";
               break;
            default:
               name = "Unknown";
               break;
         }
      }

      strlcpy(type_str, name, type_str_size);
   }
   else if (strstr(setting->name, "analog_dpad_mode"))
   {
      static const char *modes[] = {
         "None",
         "Left Analog",
         "Right Analog",
      };

      strlcpy(type_str, modes[g_settings.input.analog_dpad_mode
            [setting->index_offset] % ANALOG_DPAD_LAST],
            type_str_size);
   }
   else if (!strcmp(setting->name, "autosave_interval"))
   {
      if (*setting->value.unsigned_integer)
         snprintf(type_str, type_str_size, "%u seconds",
               *setting->value.unsigned_integer);
      else
         strlcpy(type_str, "No", type_str_size);
   }
  /* else if (!strcmp(setting->name, "user_language"))
   {
      static const char *modes[] = {
         "English",
         "Japanese",
         "French",
         "Spanish",
         "German",
         "Italian",
         "Dutch",
         "Portuguese",
         "Russian",
         "Korean",
         "Chinese (Traditional)",
         "Chinese (Simplified)"
      };

      strlcpy(type_str, modes[g_settings.user_language], type_str_size);
   }
   else if (!strcmp(setting->name, "libretro_log_level"))
   {
      static const char *modes[] = {
         "0 (Debug)",
         "1 (Info)",
         "2 (Warning)",
         "3 (Error)"
      };

      strlcpy(type_str, modes[*setting->value.unsigned_integer],
            type_str_size);
   }*/
   else
   {
      snprintf(type_str, type_str_size, "%u",
            *setting->value.unsigned_integer);
   }
}

static void menu_common_setting_set_label_st_float(rarch_setting_t *setting,
      char *type_str, size_t type_str_size)
{
   if (!setting)
      return;

   if (!strcmp(setting->name, "video_refresh_rate_auto"))
   {
      double video_refresh_rate = 0.0;
      double deviation = 0.0;
      unsigned sample_points = 0;

      if (driver_monitor_fps_statistics(&video_refresh_rate, &deviation, &sample_points))
         snprintf(type_str, type_str_size, "%.3f Hz (%.1f%% dev, %u samples)",
               video_refresh_rate, 100.0 * deviation, sample_points);
      else
         strlcpy(type_str, "N/A", type_str_size);

      return;
   }

   snprintf(type_str, type_str_size, setting->rounding_fraction,
         *setting->value.fraction);
}

void setting_data_get_string_representation(rarch_setting_t* setting,
      char* buf, size_t sizeof_buf)
{
   if (!setting || !buf || !sizeof_buf)
      return;

   switch (setting->type)
   {
      case ST_BOOL:
         menu_common_setting_set_label_st_bool(setting, buf, sizeof_buf);
         break;
      case ST_INT:
         snprintf(buf, sizeof_buf, "%d", *setting->value.integer);
         break;
      case ST_UINT:
         menu_common_setting_set_label_st_uint(setting, buf, sizeof_buf);
         break;
      case ST_FLOAT:
         menu_common_setting_set_label_st_float(setting, buf, sizeof_buf);
         break;
      case ST_DIR:
         strlcpy(buf,
               *setting->value.string ?
               setting->value.string : setting->dir.empty_path,
               sizeof_buf);
         break;
      case ST_PATH:
         strlcpy(buf, path_basename(setting->value.string), sizeof_buf);
         break;
      case ST_STRING:
         strlcpy(buf, setting->value.string, sizeof_buf);
         break;
      case ST_BIND:
         {
            const struct retro_keybind* keybind = (const struct retro_keybind*)
               setting->value.keybind;
            const struct retro_keybind* auto_bind = 
               (const struct retro_keybind*)
               input_get_auto_bind(setting->index_offset, keybind->id);

            input_get_bind_string(buf, keybind, auto_bind, sizeof_buf);
         }
         break;
      case ST_ACTION:
         if (setting->get_string_representation)
            setting->get_string_representation(setting, buf, sizeof_buf);
         break;
         /* TODO */
      case ST_HEX:
         break;
      case ST_GROUP:
         strlcpy(buf, "...", sizeof_buf);
         break;
      case ST_SUB_GROUP:
         strlcpy(buf, "...", sizeof_buf);
         break;
      case ST_END_GROUP:
         break;
      case ST_END_SUB_GROUP:
         break;
      case ST_NONE:
         break;
   }
}
/*
static int setting_data_bool_action_start_savestates(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   g_settings.state_slot = 0;

   return 0;
} */

static int setting_data_uint_action_toggle_analog_dpad_mode(void *data, unsigned action)
{
   unsigned port = 0;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   port = setting->index_offset;

   switch (action)
   {
      case MENU_ACTION_RIGHT:
         g_settings.input.analog_dpad_mode[port] =
            (g_settings.input.analog_dpad_mode[port] + 1)
            % ANALOG_DPAD_LAST;
         break;

      case MENU_ACTION_LEFT:
         g_settings.input.analog_dpad_mode[port] =
            (g_settings.input.analog_dpad_mode
             [port] + ANALOG_DPAD_LAST - 1) % ANALOG_DPAD_LAST;
         break;
   }

   return 0;
}

static int setting_data_uint_action_toggle_libretro_device_type(void *data, unsigned action)
{
   unsigned current_device, current_idx, i, devices[128];
   const struct retro_controller_info *desc;
   unsigned types = 0, port = 0;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   port = setting->index_offset;

   devices[types++] = RETRO_DEVICE_NONE;
   devices[types++] = RETRO_DEVICE_JOYPAD;

   /* Only push RETRO_DEVICE_ANALOG as default if we use an 
    * older core which doesn't use SET_CONTROLLER_INFO. */
   if (!g_extern.system.num_ports)
      devices[types++] = RETRO_DEVICE_ANALOG;

   desc = port < g_extern.system.num_ports ?
      &g_extern.system.ports[port] : NULL;
   if (desc)
   {
      for (i = 0; i < desc->num_types; i++)
      {
         unsigned id = desc->types[i].id;
         if (types < ARRAY_SIZE(devices) &&
               id != RETRO_DEVICE_NONE &&
               id != RETRO_DEVICE_JOYPAD)
            devices[types++] = id;
      }
   }

   current_device = g_settings.input.libretro_device[port];
   current_idx = 0;
   for (i = 0; i < types; i++)
   {
      if (current_device == devices[i])
      {
         current_idx = i;
         break;
      }
   }

   switch (action)
   {
      case MENU_ACTION_LEFT:
         current_device = devices
            [(current_idx + types - 1) % types];

         g_settings.input.libretro_device[port] = current_device;
         pretro_set_controller_port_device(port, current_device);
         break;

      case MENU_ACTION_RIGHT:
         current_device = devices
            [(current_idx + 1) % types];

         g_settings.input.libretro_device[port] = current_device;
         pretro_set_controller_port_device(port, current_device);
         break;
   }

   return 0;
}
/*
static int setting_data_bool_action_toggle_savestates(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   switch (action)
   {
      case MENU_ACTION_LEFT:
         // Slot -1 is (auto) slot.
         if (g_settings.state_slot >= 0)
            g_settings.state_slot--;
         break;
      case MENU_ACTION_RIGHT:
         g_settings.state_slot++;
         break;
   }

   return 0;
} */

static int setting_data_action_start_bind_device(void *data)
{
   unsigned *p = NULL;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   p = (unsigned*)&g_settings.input.joypad_map[setting->index_offset];

   (*p) = setting->index_offset;

   return 0;
}

static int setting_data_action_toggle_bind_device(void *data, unsigned action)
{
   unsigned *p = NULL;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   p = (unsigned*)&g_settings.input.joypad_map[setting->index_offset];

   switch (action)
   {
      case MENU_ACTION_LEFT:
         if ((*p) > 0)
            (*p)--;
         break;
      case MENU_ACTION_RIGHT:
         if (*p < MAX_PLAYERS - 12)
            (*p)++;
         break;
   }

   return 0;
}

static int setting_data_action_ok_bind_all(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting || !driver.menu)
      return -1;

   driver.menu->binds.target = &g_settings.input.binds
      [setting->index_offset][0];
   driver.menu->binds.begin = MENU_SETTINGS_BIND_BEGIN;
   driver.menu->binds.last = MENU_SETTINGS_BIND_LAST;

   menu_list_push_stack(
         driver.menu->menu_list,
         "",
         "",
         g_extern.menu.bind_mode_keyboard ?
         MENU_SETTINGS_CUSTOM_BIND_KEYBOARD :
         MENU_SETTINGS_CUSTOM_BIND,
         driver.menu->selection_ptr);

   if (g_extern.menu.bind_mode_keyboard)
   {
      driver.menu->binds.timeout_end =
         rarch_get_time_usec() + 
         MENU_KEYBOARD_BIND_TIMEOUT_SECONDS * 1000000;
      input_keyboard_wait_keys(driver.menu,
            menu_custom_bind_keyboard_cb);
   }
   else
   {
      menu_poll_bind_get_rested_axes(&driver.menu->binds);
      menu_poll_bind_state(&driver.menu->binds);
   }
   return 0;
}

static int setting_data_action_ok_bind_defaults(void *data, unsigned action)
{
   unsigned i;
   struct retro_keybind *target = NULL;
   const struct retro_keybind *def_binds = NULL;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   target = (struct retro_keybind*)
      &g_settings.input.binds[setting->index_offset][0];
   def_binds =  (setting->index_offset) ? 
      retro_keybinds_rest : retro_keybinds_1;

   if (!driver.menu || !target)
      return -1;

   driver.menu->binds.begin = MENU_SETTINGS_BIND_BEGIN;
   driver.menu->binds.last  = MENU_SETTINGS_BIND_LAST;

   for (i = MENU_SETTINGS_BIND_BEGIN;
         i <= MENU_SETTINGS_BIND_LAST; i++, target++)
   {
      if (g_extern.menu.bind_mode_keyboard)
         target->key = def_binds[i - MENU_SETTINGS_BIND_BEGIN].key;
      else
      {
         target->joykey = NO_BTN;
         target->joyaxis = AXIS_NONE;
      }
   }

   return 0;
}

static int setting_data_bool_action_ok_exit(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
   {
      rarch_main_command(setting->cmd_trigger.idx);
      rarch_main_command(RARCH_CMD_RESUME);
   }

   return 0;
}

static int setting_data_bool_action_start_default(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.boolean = setting->default_value.boolean;

   return 0;
}

static int setting_data_string_dir_action_start_default(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.string = '\0';

   return 0;
}

static int setting_data_bool_action_toggle_default(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   switch (action)
   {
      case MENU_ACTION_LEFT:
      case MENU_ACTION_RIGHT:
         *setting->value.boolean = !(*setting->value.boolean);
         break;
   }

   return 0;
}

static int setting_data_bool_action_ok_default(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
      setting->cmd_trigger.triggered = true;

   return 0;
}

static int setting_data_uint_action_start_analog_dpad_mode(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.unsigned_integer = 0;

   return 0;
}

static int setting_data_uint_action_start_default(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.unsigned_integer = setting->default_value.unsigned_integer;

   return 0;
}

static int setting_data_uint_action_start_libretro_device_type(void *data)
{
   unsigned current_device, i, devices[128];
   const struct retro_controller_info *desc;
   unsigned types = 0;
   unsigned port = 0;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.unsigned_integer = setting->default_value.unsigned_integer;

   port = setting->index_offset;

   devices[types++] = RETRO_DEVICE_NONE;
   devices[types++] = RETRO_DEVICE_JOYPAD;

   /* Only push RETRO_DEVICE_ANALOG as default if we use an 
    * older core which doesn't use SET_CONTROLLER_INFO. */
   if (!g_extern.system.num_ports)
      devices[types++] = RETRO_DEVICE_ANALOG;

   desc = port < g_extern.system.num_ports ?
      &g_extern.system.ports[port] : NULL;
   if (desc)
   {
      for (i = 0; i < desc->num_types; i++)
      {
         unsigned id = desc->types[i].id;
         if (types < ARRAY_SIZE(devices) &&
               id != RETRO_DEVICE_NONE &&
               id != RETRO_DEVICE_JOYPAD)
            devices[types++] = id;
      }
   }

   current_device = g_settings.input.libretro_device[port];

   current_device = RETRO_DEVICE_JOYPAD;

   g_settings.input.libretro_device[port] = current_device;
   pretro_set_controller_port_device(port, current_device);

   return 0;
}

static int setting_data_uint_action_toggle_default(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   switch (action)
   {
      case MENU_ACTION_LEFT:
         if (*setting->value.unsigned_integer != setting->min)
            *setting->value.unsigned_integer =
               *setting->value.unsigned_integer - setting->step;

         if (setting->enforce_minrange)
         {
            if (*setting->value.unsigned_integer < setting->min)
               *setting->value.unsigned_integer = setting->min;
         }
         break;

      case MENU_ACTION_RIGHT:
         *setting->value.unsigned_integer =
            *setting->value.unsigned_integer + setting->step;

         if (setting->enforce_maxrange)
         {
            if (*setting->value.unsigned_integer > setting->max)
               *setting->value.unsigned_integer = setting->max;
         }
         break;
   }

   return 0;
}

static int setting_data_uint_action_ok_default(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
      setting->cmd_trigger.triggered = true;

   return 0;
}

/*static int setting_data_fraction_action_start_video_refresh_rate_auto(
      void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   g_extern.measure_data.frame_time_samples_count = 0;

   return 0;
}

static int setting_data_fraction_action_ok_video_refresh_rate_auto(
      void *data, unsigned action)
{
   double video_refresh_rate, deviation = 0.0;
   unsigned sample_points = 0;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (driver_monitor_fps_statistics(&video_refresh_rate,
            &deviation, &sample_points))
   {
      driver_set_monitor_refresh_rate(video_refresh_rate);
      // In case refresh rate update forced non-block video.
      rarch_main_command(RARCH_CMD_VIDEO_SET_BLOCKING_STATE);
   }

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
      setting->cmd_trigger.triggered = true;

   return 0;
}*/

static int setting_data_fraction_action_toggle_default(
      void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   switch (action)
   {
      case MENU_ACTION_LEFT:
         *setting->value.fraction =
            *setting->value.fraction - setting->step;

         if (setting->enforce_minrange)
         {
            if (*setting->value.fraction < setting->min)
               *setting->value.fraction = setting->min;
         }
         break;

      case MENU_ACTION_RIGHT:
         *setting->value.fraction = 
            *setting->value.fraction + setting->step;

         if (setting->enforce_maxrange)
         {
            if (*setting->value.fraction > setting->max)
               *setting->value.fraction = setting->max;
         }
         break;
   }

   return 0;
}

static int setting_data_fraction_action_start_default(
      void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.fraction = setting->default_value.fraction;

   return 0;
}

static int setting_data_fraction_action_ok_default(
      void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
      setting->cmd_trigger.triggered = true;

   return 0;
}

static int setting_data_uint_action_start_linefeed(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   *setting->value.unsigned_integer = setting->default_value.unsigned_integer;

   return 0;
}

static int setting_data_uint_action_ok_linefeed(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   menu_key_start_line(driver.menu, setting->short_description,
         setting->name, st_uint_callback);

   return 0;
}

static int setting_data_action_action_ok(void *data, unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
      rarch_main_command(setting->cmd_trigger.idx);

   return 0;
}

rarch_setting_t setting_data_action_setting(const char* name,
      const char* short_description,
      const char *group, const char *subgroup)
{
   rarch_setting_t result;
   result.type                  = ST_ACTION;
   result.name                  = name;
   result.short_description     = short_description;
   result.group                 = group;
   result.subgroup              = subgroup;
   result.change_handler        = NULL;
   result.deferred_handler      = NULL;
   result.read_handler          = NULL;
   
   result.get_string_representation = NULL;
   result.action_start              = NULL;
   result.action_toggle             = NULL;
   result.action_ok                 = setting_data_action_action_ok;
   return result;
}

rarch_setting_t setting_data_group_setting(enum setting_type type, const char* name)
{
   rarch_setting_t result = { type, name };

   result.short_description = name;
   return result;
}

rarch_setting_t setting_data_subgroup_setting(enum setting_type type, const char* name,
      const char *parent_name)
{
   rarch_setting_t result = { type, name };

   result.short_description = name;
   result.group = parent_name;
   return result;
}

rarch_setting_t setting_data_float_setting(const char* name,
      const char* short_description, float* target, float default_value,
      const char *rounding, const char *group, const char *subgroup, change_handler_t change_handler,
      change_handler_t read_handler)
{
   rarch_setting_t result = { ST_FLOAT, name, sizeof(float), short_description,
      group, subgroup };

   result.rounding_fraction = rounding;
   result.change_handler = change_handler;
   result.read_handler = read_handler;
   result.value.fraction = target;
   result.original_value.fraction = *target;
   result.default_value.fraction = default_value;
   
   result.action_start     = setting_data_fraction_action_start_default;
   result.action_toggle    = setting_data_fraction_action_toggle_default;
   result.action_ok        = setting_data_fraction_action_ok_default;
   return result;
}

rarch_setting_t setting_data_bool_setting(const char* name,
      const char* short_description, bool* target, bool default_value,
      const char *off, const char *on,
      const char *group, const char *subgroup, change_handler_t change_handler,
      change_handler_t read_handler)
{
   rarch_setting_t result = { ST_BOOL, name, sizeof(bool), short_description,
      group, subgroup };
   result.change_handler = change_handler;
   result.read_handler = read_handler;
   result.value.boolean = target;
   result.original_value.boolean = *target;
   result.default_value.boolean = default_value;
   result.boolean.off_label = off;
   result.boolean.on_label = on;

   result.action_start = setting_data_bool_action_start_default;
   result.action_toggle= setting_data_bool_action_toggle_default;
   result.action_ok    = setting_data_bool_action_ok_default;
   return result;
}

rarch_setting_t setting_data_int_setting(const char* name,
      const char* short_description, int* target, int default_value,
      const char *group, const char *subgroup, change_handler_t change_handler,
      change_handler_t read_handler)
{
   rarch_setting_t result = { ST_INT, name, sizeof(int), short_description,
      group, subgroup };

   result.change_handler = change_handler;
   result.read_handler = read_handler;
   result.value.integer = target;
   result.original_value.integer = *target;
   result.default_value.integer = default_value;
   return result;
}

rarch_setting_t setting_data_uint_setting(const char* name,
      const char* short_description, unsigned int* target,
      unsigned int default_value, const char *group, const char *subgroup,
      change_handler_t change_handler, change_handler_t read_handler)
{
   rarch_setting_t result = { ST_UINT, name, sizeof(unsigned int),
      short_description, group, subgroup };

   result.change_handler = change_handler;
   result.read_handler = read_handler;
   result.value.unsigned_integer = target;
   result.original_value.unsigned_integer = *target;
   result.default_value.unsigned_integer = default_value;

   result.action_start  = setting_data_uint_action_start_default;
   result.action_toggle = setting_data_uint_action_toggle_default;
   result.action_ok     = setting_data_uint_action_ok_default;

   return result;
}

rarch_setting_t setting_data_string_setting(enum setting_type type,
      const char* name, const char* short_description, char* target,
      unsigned size, const char* default_value, const char *empty,
      const char *group, const char *subgroup, change_handler_t change_handler,
      change_handler_t read_handler)
{
   rarch_setting_t result = { type, name, size, short_description, group,
      subgroup };

   result.dir.empty_path       = empty;
   result.change_handler       = change_handler;
   result.read_handler         = read_handler;
   result.value.string         = target;
   result.default_value.string = default_value;
   result.action_start         = NULL;

   switch (type)
   {
      case ST_DIR:
         result.action_start  = setting_data_string_dir_action_start_default;
         break;
      case ST_PATH:
         result.action_start  = setting_data_string_dir_action_start_default;
         break;
      default:
         break;
   }

   return result;
}

static int setting_data_bind_action_start(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting || !driver.menu)
      return -1;

   struct retro_keybind *def_binds = (struct retro_keybind *)retro_keybinds_1;
   struct retro_keybind *keybind = (struct retro_keybind*)setting->value.keybind;

   if (!keybind)
      return -1;

   if (!g_extern.menu.bind_mode_keyboard)
   {
      keybind->joykey = NO_BTN;
      keybind->joyaxis = AXIS_NONE;
      return 0;
   }

   if (setting->index_offset)
      def_binds = (struct retro_keybind*)retro_keybinds_rest;

   if (!def_binds)
      return -1;

   keybind->key = def_binds[setting->bind_type - MENU_SETTINGS_BIND_BEGIN].key;

   return 0;
}

static int setting_data_bind_action_ok(void *data, unsigned action)
{
   struct retro_keybind *keybind = NULL;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   if (!driver.menu || !driver.menu->menu_list)
      return -1;
   
   keybind = (struct retro_keybind*)setting->value.keybind;

   if (!keybind)
      return -1;

   driver.menu->binds.begin  = setting->bind_type;
   driver.menu->binds.last   = setting->bind_type;
   driver.menu->binds.target = keybind;
   driver.menu->binds.player = setting->index_offset;
   menu_list_push_stack(
         driver.menu->menu_list,
         "",
         "",
         g_extern.menu.bind_mode_keyboard ?
         MENU_SETTINGS_CUSTOM_BIND_KEYBOARD : MENU_SETTINGS_CUSTOM_BIND,
         driver.menu->selection_ptr);

   if (g_extern.menu.bind_mode_keyboard)
   {
      driver.menu->binds.timeout_end = rarch_get_time_usec() +
         MENU_KEYBOARD_BIND_TIMEOUT_SECONDS * 1000000;
      input_keyboard_wait_keys(driver.menu,
            menu_custom_bind_keyboard_cb);
   }
   else
   {
      menu_poll_bind_get_rested_axes(&driver.menu->binds);
      menu_poll_bind_state(&driver.menu->binds);
   }

   return 0;
}

rarch_setting_t setting_data_bind_setting(const char* name,
      const char* short_description, struct retro_keybind* target,
      uint32_t idx, uint32_t idx_offset,
      const struct retro_keybind* default_value,
      const char *group, const char *subgroup)
{
   rarch_setting_t result = { ST_BIND, name, 0, short_description, group,
      subgroup };

   result.value.keybind = target;
   result.default_value.keybind = default_value;
   result.index        = idx;
   result.index_offset = idx_offset;

   result.action_start = setting_data_bind_action_start;
   result.action_ok    = setting_data_bind_action_ok;

   return result;
}

int setting_data_get_description(const char *label, char *msg,
      size_t sizeof_msg)
{
   if (!strcmp(label, "input_driver"))
   {
      snprintf(msg, sizeof_msg,
               " -- Input driver.\n");
   }
  /* else if (!strcmp(label, "load_content"))
   {
      snprintf(msg, sizeof_msg,
            " -- Load Content. \n"
            "Browse for content. \n"
            " \n"
            "To load content, you need a \n"
            "libretro core to use, and a \n"
            "content file. \n"
            " \n"
            "To control where the menu starts \n"
            " to browse for content, set  \n"
            "Browser Directory. If not set,  \n"
            "it will start in root. \n"
            " \n"
            "The browser will filter out \n"
            "extensions for the last core set \n"
            "in 'Core', and use that core when \n"
            "content is loaded."
            );
   }
   else if (!strcmp(label, "core_list"))
   {
      snprintf(msg, sizeof_msg,
            " -- Core Selection. \n"
            " \n"
            "Browse for a libretro core \n"
            "implementation. Where the browser \n"
            "starts depends on your Core Directory \n"
            "path. If blank, it will start in root. \n"
            " \n"
            "If Core Directory is a directory, the menu \n"
            "will use that as top folder. If Core \n"
            "Directory is a full path, it will start \n"
            "in the folder where the file is.");
   }
   else if (!strcmp(label, "history_list"))
   {
      snprintf(msg, sizeof_msg,
            " -- Loading content from history. \n"
            " \n"
            "As content is loaded, content and libretro \n"
            "core combinations are saved to history. \n"
            " \n"
            "The history is saved to a file in the same \n"
            "directory as the config file. \n"
            "If no config file was loaded in startup, \n"
            "history will not be saved or loaded, \n"
			"and will not exist in the main menu. \n"
            );
   }*/
   else if (!strcmp(label, "audio_resampler_driver"))
   {
      if (!strcmp(g_settings.audio.resampler, "SINC"))
         snprintf(msg, sizeof_msg,
               " -- Windowed SINC");
      else if (!strcmp(g_settings.audio.resampler, "CC"))
         snprintf(msg, sizeof_msg,
               " -- Convoluted Cosine");
	  else if (!strcmp(g_settings.audio.resampler, "Nearest"))
         snprintf(msg, sizeof_msg,
               " -- Nearest");
   }
 /*  else if (!strcmp(label, "video_driver"))
   {
      snprintf(msg, sizeof_msg,
               " -- Current Video Driver.");
   }
   else if (!strcmp(label, "audio_driver"))
   {
      snprintf(msg, sizeof_msg,
               " -- Current Audio Driver.");
   }
   else if (!strcmp(label, "audio_out_rate"))
   {
      snprintf(msg, sizeof_msg,
            " -- Audio Sample Rate.\n"
            "Change from 32 kHZ to 48 kHZ \n"
			"sampling rate. Restart required."
            );
   }
   else if (!strcmp(label, "audio_dsp_plugin"))
   {
      snprintf(msg, sizeof_msg,
            " -- Audio DSP plugin.\n"
            " Processes audio before it's sent to \n"
            "the driver."
            );
   }
   else if (!strcmp(label, "libretro_dir_path"))
   {
      snprintf(msg, sizeof_msg,
            " -- Core Directory. \n"
            " \n"
            "Where to search for core \n"
            "implementations.");
   } */
  /* else if (!strcmp(label, "video_disable_composition"))
   {
      snprintf(msg, sizeof_msg,
            "-- Forcibly disable composition.\n"
            "Only valid on Windows Vista/7 for now.");
   }
   else if (!strcmp(label, "libretro_log_level"))
   {
      snprintf(msg, sizeof_msg,
            "-- Sets log level for libretro cores \n"
            "(GET_LOG_INTERFACE). \n"
            " \n"
            " If a log level issued by a libretro \n"
            " core is below libretro_log level, it \n"
            " is ignored.\n"
            " \n"
            " DEBUG logs are always ignored unless \n"
            " verbose mode is activated (--verbose).\n"
            " \n"
            " DEBUG = 0\n"
            " INFO  = 1\n"
            " WARN  = 2\n"
            " ERROR = 3"
            );
   }
   else if (!strcmp(label, "log_verbosity"))
   {
      snprintf(msg, sizeof_msg,
            "-- Enable or disable verbosity level \n"
            "of frontend.");
   }
   else if (!strcmp(label, "perfcnt_enable"))
   {
      snprintf(msg, sizeof_msg,
            "-- Enable or disable frontend \n"
            "performance counters.");
   }*/
 /*  else if (!strcmp(label, "system_directory"))
   {
      snprintf(msg, sizeof_msg,
            "-- System Directory. \n"
            " \n"
            "Sets the 'system' directory.\n"
            "Implementations can query for this\n"
            "directory to load a BIOS, \n"
            "system-specific configs, etc.");
   } */
  /* else if (!strcmp(label, "rgui_show_start_screen"))
   {
      snprintf(msg, sizeof_msg,
            " -- Show startup screen in menu.\n"
            "Is automatically set to false when seen\n"
            "for the first time.\n"
            " \n"
            "This is only updated in config if\n"
            "'Config Save On Exit' is set to true.\n");
   }*/
 /*  else if (!strcmp(label, "config_save_on_exit"))
   {
      snprintf(msg, sizeof_msg,
            " -- Flushes config to disk on exit.\n"
            "Overwrites the configurations file.\n");
   }
   else if (!strcmp(label, "core_specific_config"))
   {
      snprintf(msg, sizeof_msg,
            " -- Load up a specific config file \n"
            "based on the core being used.\n");
   } */
  /* else if (!strcmp(label, "video_scale"))
   {
      snprintf(msg, sizeof_msg,
            " -- Fullscreen resolution.\n"
            " \n"
            "Resolution of 0 uses the \n"
            "resolution of the environment.\n");
   }*/
   else if (!strcmp(label, "video_vsync"))
   {
      snprintf(msg, sizeof_msg,
            " -- Video V-Sync.\n");
   }
   else if (!strcmp(label, "audio_sync"))
   {
      snprintf(msg, sizeof_msg,
            " -- Audio Sync.\n");
   }
  /* else if (!strcmp(label, "video_hard_sync"))
   {
      snprintf(msg, sizeof_msg,
            " -- Attempts to hard-synchronize \n"
            "CPU and GPU.\n"
            " \n"
            "Can reduce latency at cost of \n"
            "performance.");
   }
   else if (!strcmp(label, "video_hard_sync_frames"))
   {
      snprintf(msg, sizeof_msg,
            " -- Sets how many frames CPU can \n"
            "run ahead of GPU when using 'GPU \n"
            "Hard Sync'.\n"
            " \n"
            "Maximum is 3.\n"
            " \n"
            " 0: Syncs to GPU immediately.\n"
            " 1: Syncs to previous frame.\n"
            " 2: Etc ...");
   }*/
   else if (!strcmp(label, "video_frame_delay"))
   {
      snprintf(msg, sizeof_msg,
            " -- Cuantos ms retrasar el juego\n"
            "despues del vsync. Esto puede"
            "\n"
            "hacer que el control sea mas activo,\n"
            "pero puede causar problemas de velocidad.\n"
            " \n"
            "El max es 15.");
   }
   else if (!strcmp(label, "audio_rate_control_delta"))
   {
      snprintf(msg, sizeof_msg,
            " -- Control del audio constante.\n"
            " \n"
            "Requiere que el video y audio\n"
			"esten sincronizados.");
   }
   else if (!strcmp(label, "video_filter"))
   {
#ifdef HAVE_FILTERS_BUILTIN
      snprintf(msg, sizeof_msg,
            " -- Filtra imagen usando la CPU.\n"
			"Requiere un reinicio.");
#else
      snprintf(msg, sizeof_msg,
            " -- CPU-based video filter.\n"
            " \n"
            "Path to a dynamic library.");
#endif
   }
   else if (!strcmp(label, "input_menu_combos"))
   {
      snprintf(msg, sizeof_msg,
            " -- Tipo de combo.\n"
            " \n"
            "Solo para GameCube Controllers. \n"
            "0: Desactivado. \n"
            "1: L, R, Z, Start. \n"
			"2: X, B, Start. ");
   }
  /* else if (!strcmp(label, "video_fullscreen"))
   {
      snprintf(msg, sizeof_msg, " -- Toggles fullscreen.");
   }
   else if (!strcmp(label, "audio_device"))
   {
      snprintf(msg, sizeof_msg,
            " -- Override the default audio device \n"
            "the audio driver uses.\n"
            "This is driver dependent. E.g.\n"
#ifdef HAVE_ALSA
            " \n"
            "ALSA wants a PCM device."
#endif
#ifdef HAVE_OSS
            " \n"
            "OSS wants a path (e.g. /dev/dsp)."
#endif
#ifdef HAVE_JACK
            " \n"
            "JACK wants portnames (e.g. system:playback1\n"
            ",system:playback_2)."
#endif
#ifdef HAVE_RSOUND
            " \n"
            "RSound wants an IP address to an RSound \n"
            "server."
#endif
            );
   }*/
  /* else if (!strcmp(label, "video_black_frame_insertion"))
   {
      snprintf(msg, sizeof_msg,
            " -- Inserts a black frame inbetween \n"
            "frames.\n"
            " \n"
            "Useful for 120 Hz monitors who want to \n"
            "play 60 Hz material with eliminated \n"
            "ghosting.\n"
            " \n"
            "Video refresh rate should still be \n"
            "configured as if it is a 60 Hz monitor \n"
            "(divide refresh rate by 2).");
   }*/
 /*  else if (!strcmp(label, "video_threaded"))
   {
      snprintf(msg, sizeof_msg,
            " -- Use threaded video driver.\n"
            " \n"
            "Using this might improve performance at \n"
            "possible cost of latency and more video \n"
            "stuttering.");
   }*/
  /* else if (!strcmp(label, "video_scale_integer"))
   {
      snprintf(msg, sizeof_msg,
            " -- Scale video in integer steps. \n"
            " \n"
            "The base size depends on system-reported \n");
   }*/
 /*  else if (!strcmp(label, "video_crop_overscan"))
   {
      snprintf(msg, sizeof_msg,
            " -- Forces cropping of overscanned \n"
            "frames.\n"
            " \n"
            "Exact behavior of this option is \n"
            "core-implementation specific.");
   }
   else if (!strcmp(label, "video_monitor_index"))
   {
      snprintf(msg, sizeof_msg,
            " -- Which monitor to prefer.\n"
            " \n"
            "0 (default) means no particular monitor \n"
            "is preferred, 1 and up (1 being first \n"
            "monitor), suggests RetroArch to use that \n"
            "particular monitor.");
   }*/
 /*  else if (!strcmp(label, "video_rotation"))
   {
      snprintf(msg, sizeof_msg,
            " -- Forces a certain rotation \n"
            "of the screen.\n"
            " \n"
            "The rotation is added to rotations which\n"
            "the libretro core sets (see Video Allow\n"
            "Rotate).");
   }*/
 /*  else if (!strcmp(label, "video_viwidth"))
   {
      snprintf(msg, sizeof_msg,
            " -- Set VI scaling width. \n"
            " \n"
            "Scale width up to 720 pixels. \n"
            "This allows setting the screen to \n"
            "match your TV's overscan. \n"
			"It can also be used to correct PAR \n"
			"without any visible quality loss.");
   }
   else if (!strcmp(label, "video_vfilter"))
   {
      snprintf(msg, sizeof_msg,
            " -- Control GX hardware filtering. \n"
            " \n"
            "Softens the image to prevent flicker in \n"
            "interlaced mode. Default: ON \n"
			" \n"
            "This has no effect in 240p mode.");
   }
   else if (!strcmp(label, "video_vbright"))
   {
      snprintf(msg, sizeof_msg,
            " -- Control GX hardware brightness. \n"
            " \n"
            "Positive values brighten the output, \n"
            "negative values dim the output. \n"
			" \n"
            "No change: 0");
   }
   else if (!strcmp(label, "video_dither"))
   {
      snprintf(msg, sizeof_msg,
            " -- Enable GX hardware dithering. \n"
            " \n"
            "Uses a simpler pixel format \n"
            "and enables a bayer 4x4 dither. \n"
			" \n"
            " Default: OFF");
   }
   else if (!strcmp(label, "soft_filter"))
   {
      snprintf(msg, sizeof_msg,
            " -- Hardware composite filtering \n"
            " \n"
            "Improve video quality if using composite out, \n"
            "this has no effect otherwise. Default: OFF");
   }
   else if (!strcmp(label, "reset_fade"))
   {
      snprintf(msg, sizeof_msg,
            " -- Fade effect while resetting.\n"
            " \n"
            " 0 = no fade\n"
            " 1 = fade-out\n"
            " 2 = fade-out and fade-in");
   }
   else if (!strcmp(label, "audio_volume"))
   {
      snprintf(msg, sizeof_msg,
            " -- Audio volume, expressed in dB.\n"
            " \n"
            " 0 dB is normal volume. No gain will be applied.\n"
            "Gain can be controlled in runtime with Input\n"
            "Volume Up / Input Volume Down.");
   }
   else if (!strcmp(label, "block_sram_overwrite"))
   {
      snprintf(msg, sizeof_msg,
            " -- Block SRAM from being overwritten \n"
            "when loading save states.\n"
            " \n"
            "Might potentially lead to buggy games.");
   }
   else if (!strcmp(label, "fastforward_ratio"))
   {
      snprintf(msg, sizeof_msg,
            " -- Fastforward ratio."
            " \n"
            "The maximum rate at which content will\n"
            "be run when using fast forward.\n"
            " \n"
            " (E.g. 5.0 for 60 fps content => 300 fps \n"
            "cap).\n"
            " \n"
            "Do not rely on this cap to be perfectly \n"
            "accurate.");
   }*/
  /* else if (!strcmp(label, "pause_nonactive"))
   {
      snprintf(msg, sizeof_msg,
            " -- Pause gameplay when window focus \n"
            "is lost.");
   }*/
   /*else if (!strcmp(label, "video_gpu_screenshot")) // Won't be using
   {
      snprintf(msg, sizeof_msg,
            " -- Screenshots output of GPU shaded \n"
            "material if available.");
   }*/
  /* else if (!strcmp(label, "autosave_interval"))
   {
      snprintf(msg, sizeof_msg,
            " -- Autosaves the non-volatile SRAM \n"
            "at a regular interval.\n"
            " \n"
            "Default is off unless set otherwise. \n"
            "The interval is measured in seconds. \n"
            " \n"
            "A value of 0 disables autosave.");
   }
   else if (!strcmp(label, "screenshot_directory"))
   {
      snprintf(msg, sizeof_msg,
            " -- Screenshot Directory. \n"
            " \n"
            "Directory to save screenshots to."
            );
   }*/
  /* else if (!strcmp(label, "video_swap_interval"))
   {
      snprintf(msg, sizeof_msg,
            " -- VSync Swap Interval.\n"
            " \n"
            "Uses a custom swap interval for VSync. Set this \n"
            "to effectively halve monitor refresh rate.");
   }
   else if (!strcmp(label, "video_refresh_rate_auto"))
   {
      snprintf(msg, sizeof_msg,
            " -- Current Refresh Rate.\n"
            " \n"
            "The accurate refresh rate of our monitor (Hz).\n"
            "This is used to calculate audio input rate with \n"
            "the formula: \n"
            " \n"
            "audio_input_rate = game input rate * display \n"
            "refresh rate / game refresh rate \n");
   }*/
  /* else if (!strcmp(label, "savefile_directory"))
   {
      snprintf(msg, sizeof_msg,
            " -- Savefile Directory. \n"
            " \n"
            "Save all save files (*.srm) to this \n"
            "directory. This includes related files like \n"
            ".bsv, .rt, .psrm, etc. \n");
   }
   else if (!strcmp(label, "savestate_directory"))
   {
      snprintf(msg, sizeof_msg,
            " -- Savestate Directory. \n"
            " \n"
            "Save all save states (*.state) to this \n"
            "directory. \n");
   }*/
  /* else if (!strcmp(label, "assets_directory"))
   {
      snprintf(msg, sizeof_msg,
            " -- Assets Directory. \n"
            " \n"
            " This location is queried by default when \n"
            "menu interfaces try to look for loadable \n"
            "assets, etc.");
   }*/
/*   else if (!strcmp(label, "slowmotion_ratio"))
   {
      snprintf(msg, sizeof_msg,
            " -- Slowmotion ratio."
            " \n"
            "When slowmotion, content will slow\n"
            "down by factor.");
   }*/
#ifdef HAVE_5PLAY
 /*  else if (!strcmp(label, "input_key_profile"))
   {
      snprintf(msg, sizeof_msg,
            " -- Change Key Layout.\n"
            " \n"
			"Change the profile for the following: \n"
			" \n"
            "Profile 0: No change. \n"
            "Profile 1: Mirror to unused keys. \n"
            "Profile 2: Alternate X/Y. \n"
			"Profile 3: Separate GC from Wii keys. "
			);
   }*/
#else
  /* else if (!strcmp(label, "input_key_profile"))
   {
      snprintf(msg, sizeof_msg,
            " -- Change Key Layout.\n"
            " \n"
			"Change the profile for the following: \n"
			" \n"
            "Profile 0: No change. \n"
            "Profile 1: Mirror to unused keys. \n"
            "Profile 2: Alternate X/Y. \n"
			);
   }*/
#endif
 /*  else if (!strcmp(label, "input_poll_rate"))
   {
      snprintf(msg, sizeof_msg,
            " -- Defines polling rate.\n"
            " \n"
			"Affects the GameCube Controller only. \n"
			" \n"
            "Default is 1, which is 1000 Hz, \n"
            "normally a value of 0 (VSync) \n"
            "would be default, but since there's \n"
			"less compatibility it has been changed. \n"
			" A restart is required.");
   }
   else if (!strcmp(label, "input_trigger_threshold"))
   {
      snprintf(msg, sizeof_msg,
            " -- Defines trigger threshold.\n"
            " \n"
            "When using a GameCube Controller \n"
            "the L and R triggers might appear \n"
			"to be too sensitive to some players.");
   }
   else if (!strcmp(label, "input_axis_threshold"))
   {
      snprintf(msg, sizeof_msg,
            " -- Defines axis threshold.\n"
            " \n"
            "How far an axis must be tilted \n"
            "to result in a button press.\n"
            " Possible values are [0.0, 1.0].");
   }
   else if (!strcmp(label, "input_turbo_period"))
   {
      snprintf(msg, sizeof_msg, 
            " -- Turbo period.\n"
            " \n"
            "How fast turbo buttons will toggle. \n"
            );
   }*/
  /* else if (!strcmp(label, "rewind_granularity"))
   {
      snprintf(msg, sizeof_msg,
            " -- Rewind granularity.\n"
            " \n"
            " When rewinding defined number of \n"
            "frames, you can rewind several frames \n"
            "at a time, increasing the rewinding \n"
            "speed.");
   }
   else if (!strcmp(label, "rewind_enable"))
   {
      snprintf(msg, sizeof_msg,
            " -- Enable rewinding.\n"
            " \n"
            "This will take a performance hit, \n"
            "so it is disabled by default.");
   }*/
 /*  else if (!strcmp(label, "input_autodetect_enable"))
   {
      snprintf(msg, sizeof_msg,
            " -- Enable input auto-detection.\n"
            " \n"
            "Will attempt to auto-configure \n"
            "controllers with basic mappings.");
   }*/
  /* else if (!strcmp(label, "camera_allow"))
   {
      snprintf(msg, sizeof_msg,
            " -- Allow or disallow camera access by \n"
            "cores.");
   }
   else if (!strcmp(label, "location_allow"))
   {
      snprintf(msg, sizeof_msg,
            " -- Allow or disallow location services \n"
            "access by cores.");
   }*/
 /*  else if (!strcmp(label, "savestate_auto_save"))
   {
      snprintf(msg, sizeof_msg,
            " -- Automatically writes a savestate on exit. \n"
            " \n"
            "This will automatically load a state \n"
            "with the '.auto' extension on startup \n"
            "if 'Auto Load State' is set.");
   }*/
  /* else if (!strcmp(label, "shader_apply_changes"))
   {
      snprintf(msg, sizeof_msg,
            " -- Apply Shader Changes. \n"
            " \n"
            "After changing shader settings, use this to \n"
            "apply changes. \n"
            " \n"
            "Changing shader settings is a somewhat \n"
            "expensive operation so it has to be \n"
            "done explicitly. \n"
            " \n"
            "When you apply shaders, the menu shader \n"
            "settings are saved to a temporary file (either \n"
            "menu.cgp or menu.glslp) and loaded. The file \n"
            "persists after RetroArch exits. The file is \n"
            "saved to Shader Directory."
            );

   }
   else if (!strcmp(label, "video_shader_preset")) 
   {
      snprintf(msg, sizeof_msg,
            " -- Load Shader Preset. \n"
            " \n"
            " Load a "
#ifdef HAVE_CG
            "Cg"
#endif
#ifdef HAVE_GLSL
#ifdef HAVE_CG
            "/"
#endif
            "GLSL"
#endif
#ifdef HAVE_HLSL
#if defined(HAVE_CG) || defined(HAVE_HLSL)
            "/"
#endif
            "HLSL"
#endif
            " preset directly. \n"
            "The menu shader menu is updated accordingly. \n"
            " \n"
            "If the CGP uses scaling methods which are not \n"
            "simple, (i.e. source scaling, same scaling \n"
            "factor for X/Y), the scaling factor displayed \n"
            "in the menu might not be correct."
            );
   }
   else if (!strcmp(label, "video_shader_num_passes")) 
   {
      snprintf(msg, sizeof_msg,
            " -- Shader Passes. \n"
            " \n"
            "RetroArch allows you to mix and match various \n"
            "shaders with arbitrary shader passes, with \n"
            "custom hardware filters and scale factors. \n"
            " \n"
            "This option specifies the number of shader \n"
            "passes to use. If you set this to 0, and use \n"
            "Apply Shader Changes, you use a 'blank' shader. \n"
            " \n"
            "The Default Filter option will affect the \n"
            "stretching filter.");
   }
   else if (!strcmp(label, "video_shader_parameters"))
   {
      snprintf(msg, sizeof_msg,
            "-- Shader Parameters. \n"
            " \n"
            "Modifies current shader directly. Will not be \n"
            "saved to CGP/GLSLP preset file.");
   }
   else if (!strcmp(label, "video_shader_preset_parameters"))
   {
      snprintf(msg, sizeof_msg,
            "-- Shader Preset Parameters. \n"
            " \n"
            "Modifies shader preset currently in menu."
            );
   }
   else if (!strcmp(label, "video_shader_pass"))
   {
      snprintf(msg, sizeof_msg,
            " -- Path to shader. \n"
            " \n"
            "All shaders must be of the same \n"
            "type (i.e. CG, GLSL or HLSL). \n"
            " \n"
            "Set Shader Directory to set where \n"
            "the browser starts to look for \n"
            "shaders."
            );
   }
   else if (!strcmp(label, "video_shader_filter_pass"))
   {
      snprintf(msg, sizeof_msg,
            " -- Hardware filter for this pass. \n"
            " \n"
            "If 'Don't Care' is set, 'Default \n"
            "Filter' will be used."
            );
   }
   else if (!strcmp(label, "video_shader_scale_pass"))
   {
      snprintf(msg, sizeof_msg,
            " -- Scale for this pass. \n"
            " \n"
            "The scale factor accumulates, i.e. 2x \n"
            "for first pass and 2x for second pass \n"
            "will give you a 4x total scale. \n"
            " \n"
            "If there is a scale factor for last \n"
            "pass, the result is stretched to \n"
            "screen with the filter specified in \n"
            "'Default Filter'. \n"
            " \n"
            "If 'Don't Care' is set, either 1x \n"
            "scale or stretch to fullscreen will \n"
            "be used depending if it's not the last \n"
            "pass or not."
            );
   }*/
 /*  else if (
         !strcmp(label, "l_x_plus")  ||
         !strcmp(label, "l_x_minus") ||
         !strcmp(label, "l_y_plus")  ||
         !strcmp(label, "l_y_minus")
         )
      snprintf(msg, sizeof_msg,
            " -- Axis for analog stick.\n"
            " \n"
            "Bound as usual, however, if a real analog \n"
            "axis is bound, it can be read as a true analog.\n"
            " \n"
            "Positive X axis is right. \n"
            "Positive Y axis is down.");
   else if (!strcmp(label, "turbo"))
      snprintf(msg, sizeof_msg,
            " -- Turbo enable.\n"
            " \n"
            "Holding turbo while pressing another \n"
            "button will let the button enter \n"
            "a turbo mode where the button state \n"
            "is modulated with a periodic signal. \n"
            " \n"
            "The modulation stops when the button \n"
            "itself (not turbo button) is released.");
   else if (!strcmp(label, "exit_emulator"))
      snprintf(msg, sizeof_msg,
            " -- Key to exit cleanly."*/
#if !defined(RARCH_MOBILE) && !defined(RARCH_CONSOLE)
            "\nKilling it in any hard way (SIGKILL, \n"
            "etc) will terminate without saving\n"
            "RAM, etc. On Unix-likes,\n"
            "SIGINT/SIGTERM allows\n"
            "a clean deinitialization."
#endif
         //   );
   /*else if (!strcmp(label, "rewind"))
      snprintf(msg, sizeof_msg,
            " -- Hold button down to rewind.\n"
            " \n"
            "Rewind must be enabled.");*/
 /*  else if (!strcmp(label, "load_state"))
      snprintf(msg, sizeof_msg,
            " -- Loads state.");
   else if (!strcmp(label, "save_state"))
      snprintf(msg, sizeof_msg,
            " -- Saves state.");
   else if (!strcmp(label, "state_slot_increase") ||
         !strcmp(label, "state_slot_decrease"))
      snprintf(msg, sizeof_msg,
            " -- State slots.\n"
            " \n"
            "With slot set to 0, save state name \n"
            "is *.state. Otherwise the slot will \n"
            "be appended to the end of the filename."); */
  /* else if (!strcmp(label, "netplay_flip_players"))
      snprintf(msg, sizeof_msg,
            " -- Netplay flip players.");*/
 //  else if (!strcmp(label, "frame_advance"))
   //   snprintf(msg, sizeof_msg,
     //       " -- Frame advance when content is paused.");
  /* else if (!strcmp(label, "enable_hotkey"))
      snprintf(msg, sizeof_msg,
            " -- Enable other hotkeys.\n"
            " \n"
            " If this hotkey is bound to either keyboard, \n"
            "joybutton or joyaxis, all other hotkeys will \n"
            "be disabled unless this hotkey is also held \n"
            "at the same time. \n"
            " \n"
            "This is useful for RETRO_KEYBOARD centric \n"
            "implementations which query a large area of \n"
            "the keyboard, where it is not desirable that \n"
            "hotkeys get in the way.");*/
  /* else if (!strcmp(label, "slowmotion"))
      snprintf(msg, sizeof_msg,
            " -- Hold for slowmotion.");
   else if (!strcmp(label, "pause_toggle"))
      snprintf(msg, sizeof_msg,
            " -- Toggle between paused and non-paused state.");
   else if (!strcmp(label, "hold_fast_forward"))
      snprintf(msg, sizeof_msg,
            " -- Hold for fast-forward. Releasing button \n"
            "disables fast-forward."); */
  /* else if (!strcmp(label, "shader_next"))
      snprintf(msg, sizeof_msg,
            " -- Applies next shader in directory.");*/
 //  else if (!strcmp(label, "reset"))
   //   snprintf(msg, sizeof_msg,
     //       " -- Reset the content.\n");
  /* else if (!strcmp(label, "cheat_index_plus"))
      snprintf(msg, sizeof_msg,
            " -- Increment cheat index.\n");
   else if (!strcmp(label, "cheat_index_minus"))
      snprintf(msg, sizeof_msg,
            " -- Decrement cheat index.\n");
   else if (!strcmp(label, "cheat_toggle"))
      snprintf(msg, sizeof_msg,
            " -- Toggle cheat index.\n");
   else if (!strcmp(label, "shader_prev"))
      snprintf(msg, sizeof_msg,
            " -- Applies previous shader in directory."); */
 /*  else if (!strcmp(label, "audio_mute"))
      snprintf(msg, sizeof_msg,
            " -- Mute/unmute audio.");
   else if (!strcmp(label, "screenshot"))
      snprintf(msg, sizeof_msg,
            " -- Take screenshot.");
   else if (!strcmp(label, "volume_up"))
      snprintf(msg, sizeof_msg,
            " -- Increases audio volume.");
   else if (!strcmp(label, "volume_down"))
      snprintf(msg, sizeof_msg,
            " -- Decreases audio volume.");
   else if (!strcmp(label, "overlay_next"))
      snprintf(msg, sizeof_msg,
            " -- Toggles to next overlay.\n"
            " \n"
            "Wraps around.");
   else if (!strcmp(label, "disk_eject_toggle"))
      snprintf(msg, sizeof_msg,
            " -- Toggles eject for disks.\n"
            " \n"
            "Used for multiple-disk content.");
   else if (!strcmp(label, "disk_next"))
      snprintf(msg, sizeof_msg,
            " -- Cycles through disk images. Use after \n"
            "ejecting. \n"
            " \n"
            " Complete by toggling eject again."); */
  /* else if (!strcmp(label, "grab_mouse_toggle"))
      snprintf(msg, sizeof_msg,
            " -- Toggles mouse grab.\n"
            " \n"
            "When mouse is grabbed, mouse is \n"
            "hidden, and keeps the mouse pointer inside \n"
            "the window to allow relative mouse input to \n"
            "work better."); */
   /*else if (!strcmp(label, "menu_toggle"))
      snprintf(msg, sizeof_msg,
            " -- Toggles menu.");
   else if (!strcmp(label, "input_bind_device_id"))
      snprintf(msg, sizeof_msg,
            " -- Input Device. \n"
            " \n"
            "Which controller to use for each player. \n"
            );
   else if (!strcmp(label, "input_bind_device_type"))
      snprintf(msg, sizeof_msg,
            " -- Input Device Type. \n"
            " \n"
            "Picks which device type to use. This is \n"
            "relevant for the libretro core itself."
            );*/
   else
      snprintf(msg, sizeof_msg,
            "-- No hay info disponible. --\n");

   return 0;
}

#ifdef HAVE_MENU
static void menu_common_setting_set_label_perf(char *type_str,
      size_t type_str_size, unsigned *w, unsigned type,
      const struct retro_perf_counter **counters, unsigned offset)
{
   if (counters[offset] && counters[offset]->call_cnt)
   {
      snprintf(type_str, type_str_size,
#ifdef _WIN32
            "%I64u ticks, %I64u runs.",
#else
            "%llu ticks, %llu runs.",
#endif
            ((unsigned long long)counters[offset]->total /
             (unsigned long long)counters[offset]->call_cnt),
            (unsigned long long)counters[offset]->call_cnt);
      return;
   }

   *type_str = '\0';
   *w = 0;
}

#if defined(GEKKO)
extern unsigned menu_gx_resolutions[][2];
extern unsigned menu_current_gx_resolution;
#endif

static int get_fallback_label(char *type_str,
      size_t type_str_size, unsigned *w, unsigned type, 
      const char *menu_label, const char *label, unsigned idx)
{
   int ret = 0;

   switch (type)
   {
#if defined(GEKKO)
      case MENU_SETTINGS_VIDEO_RESOLUTION:
         snprintf(type_str, type_str_size, "%.3ux%.3u%c",
               menu_gx_resolutions[menu_current_gx_resolution][0],
               menu_gx_resolutions[menu_current_gx_resolution][1],
               menu_gx_resolutions[menu_current_gx_resolution][1] > 300 ? ' ' : 'p');
         break;
#elif defined(__CELLOS_LV2__)
      case MENU_SETTINGS_VIDEO_RESOLUTION:
         {
            unsigned width = gfx_ctx_get_resolution_width(
                  g_extern.console.screen.resolutions.list
                  [g_extern.console.screen.resolutions.current.idx]);
            unsigned height = gfx_ctx_get_resolution_height(
                  g_extern.console.screen.resolutions.list
                  [g_extern.console.screen.resolutions.current.idx]);
            snprintf(type_str, type_str_size, "%ux%u", width, height);
         }
         break;
#endif
      case MENU_SETTINGS_CUSTOM_VIEWPORT:
      case MENU_SETTINGS_CUSTOM_BIND_ALL:
      case MENU_SETTINGS_CUSTOM_BIND_DEFAULT_ALL:
         strlcpy(type_str, "...", type_str_size);
         break;
      case MENU_SETTINGS_CORE_DISK_OPTIONS_DISK_INDEX:
         {
            const struct retro_disk_control_callback *control =
               (const struct retro_disk_control_callback*)
               &g_extern.system.disk_control;
            unsigned images = control->get_num_images();
            unsigned current = control->get_image_index();
            if (current >= images)
               strlcpy(type_str, "No Disk", type_str_size);
            else
               snprintf(type_str, type_str_size, "%u", current + 1);
         }
         break;
      default:
         ret = -1;
         break;
   }

   return ret;
}

static void get_string_representation_bind_device(void * data, char *type_str,
      size_t type_str_size)
{
   unsigned map = 0;
   rarch_setting_t *setting = (rarch_setting_t*)data;

  // if (!setting || !type_str || !type_str_size)
    //  return;

   map = g_settings.input.joypad_map[setting->index_offset];

   if (map < MAX_PLAYERS - 12)
   {
     // const char *device_name = 
       //  g_settings.input.device_names[map];

      char cnt_label[64];
      snprintf(cnt_label, sizeof(cnt_label),
               "Puerto #%d", map + 1);
     // if (*device_name)
         strlcpy(type_str, cnt_label, type_str_size);
     // else
       //  snprintf(type_str, type_str_size,
         //      "Nada (puerto #%d)", map);
   }
   else
      strlcpy(type_str, "Nada", type_str_size);
}
/*
static void get_string_representation_savestate(void * data, char *type_str,
      size_t type_str_size)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting || !type_str || !type_str_size)
      return;

   snprintf(type_str, type_str_size, "%d", g_settings.state_slot);
   if (g_settings.state_slot == -1)
      strlcat(type_str, " (Auto)", type_str_size);
} */

void setting_data_get_label(char *type_str,
      size_t type_str_size, unsigned *w, unsigned type, 
      const char *menu_label, const char *label, unsigned idx)
{
   rarch_setting_t *setting_data = NULL;
   rarch_setting_t *setting      = NULL;

   if (!driver.menu || !driver.menu->menu_list)
      return;

   setting_data = (rarch_setting_t*)driver.menu->list_settings;
   setting = (rarch_setting_t*)setting_data_find_setting(setting_data,
         driver.menu->menu_list->selection_buf->list[idx].label);

   if ((get_fallback_label(type_str, type_str_size, w, type, menu_label,
         label, idx)) == 0)
      return;

 /*  if ((!strcmp(menu_label, "Shader Options") ||
            !strcmp(menu_label, "video_shader_parameters") ||
            !strcmp(menu_label, "video_shader_preset_parameters"))
      )
   {
      menu_shader_manager_get_str(driver.menu->shader, type_str, type_str_size,
            menu_label, label, type);
   }
   else */if (type >= MENU_SETTINGS_PERF_COUNTERS_BEGIN
         && type <= MENU_SETTINGS_PERF_COUNTERS_END)
      menu_common_setting_set_label_perf(type_str, type_str_size, w, type,
            perf_counters_rarch,
            type - MENU_SETTINGS_PERF_COUNTERS_BEGIN);
   else if (type >= MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_BEGIN
         && type <= MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_END)
      menu_common_setting_set_label_perf(type_str, type_str_size, w, type,
            perf_counters_libretro,
            type - MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_BEGIN);
   else if (setting)
      setting_data_get_string_representation(setting, type_str, type_str_size);
   else
   {
      if (!driver.menu || !driver.menu->menu_list)
         return;

      setting_data = (rarch_setting_t*)driver.menu->list_mainmenu;

      setting = (rarch_setting_t*)setting_data_find_setting(setting_data,
            driver.menu->menu_list->selection_buf->list[idx].label);

      if (setting)
      {
         if (!strcmp(setting->name, "configurations"))
         {
            if (*g_extern.config_path)
               fill_pathname_base(type_str, g_extern.config_path,
                     type_str_size);
            else
               strlcpy(type_str, "<default>", type_str_size);
         }
         else
            setting_data_get_string_representation(setting, type_str, type_str_size);
      }
      else
      {
      }
   }
}
#endif

static void general_read_handler(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return;

   if (!strcmp(setting->name, "audio_rate_control_delta"))
   {
      *setting->value.fraction = g_settings.audio.rate_control_delta;
      if (*setting->value.fraction < 0.0005)
      {
         g_settings.audio.rate_control = false;
         g_settings.audio.rate_control_delta = 0.0;
      }
      else
      {
         g_settings.audio.rate_control = true;
         g_settings.audio.rate_control_delta = *setting->value.fraction;
      }
   }
   else if (!strcmp(setting->name, "video_refresh_rate_auto"))
      *setting->value.fraction = g_settings.video.refresh_rate;
   else if (!strcmp(setting->name, "input_player1_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[0];
   else if (!strcmp(setting->name, "input_player2_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[1];
   else if (!strcmp(setting->name, "input_player3_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[2];
   else if (!strcmp(setting->name, "input_player4_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[3];
   else if (!strcmp(setting->name, "input_player5_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[4];
   else if (!strcmp(setting->name, "input_player6_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[5];
   else if (!strcmp(setting->name, "input_player7_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[6];
   else if (!strcmp(setting->name, "input_player8_joypad_index"))
      *setting->value.integer = g_settings.input.joypad_map[7];
}

static void general_write_handler(void *data)
{
   unsigned rarch_cmd = RARCH_CMD_NONE;
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return;

   if (setting->cmd_trigger.idx != RARCH_CMD_NONE)
   {
      if (setting->flags & SD_FLAG_EXIT)
      {
         if (*setting->value.boolean)
            *setting->value.boolean = false;
      }
      if (setting->cmd_trigger.triggered ||
            (setting->flags & SD_FLAG_CMD_APPLY_AUTO))
         rarch_cmd = setting->cmd_trigger.idx;
   }

   if (!strcmp(setting->name, "help"))
   {
      if (!driver.menu || !driver.menu->menu_list)
         return;

      if (*setting->value.boolean)
      {
#ifdef HAVE_MENU
         menu_list_push_stack_refresh(
               driver.menu->menu_list,
               "",
               "help",
               0,
               0);
#endif
         *setting->value.boolean = false;
      }
   }
   else if (!strcmp(setting->name, "video_viwidth"))
   {
      if (driver.video_data)
        update_screen_width();
   }
   else if (!strcmp(setting->name, "video_vfilter") || !strcmp(setting->name, "video_vbright"))
   {
      // solo se cambia gx_mode.vfilter[3], asi que el limite es diferente dependiendo del deflicker
      if (g_settings.video.vfilter && g_settings.video.vbright < -12)
        g_settings.video.vbright = -12;
      if (driver.video_data)
        update_deflicker();
   }
     // Needs a better way of updating
   else if (!strcmp(setting->name, "video_dither") || !strcmp(setting->name, "video_force_288p"))
   {
      if (driver.video_data)
      gx_set_video_mode(driver.video_data, menu_gx_resolutions
            [g_settings.video.vres][0],
            menu_gx_resolutions[g_settings.video.vres][1]);
   }
   else if (!strcmp(setting->name, "video_smooth"))
   {
      if (driver.video_data && driver.video_poke
            && driver.video_poke->set_filtering)
         driver.video_poke->set_filtering(driver.video_data,
               1, g_settings.video.smooth);
   }
   else if (!strcmp(setting->name, "pal60_enable"))
   {
      if (*setting->value.boolean && g_extern.console.screen.pal_enable)
         rarch_cmd = RARCH_CMD_REINIT;
      else
         *setting->value.boolean = false;
   }
   else if (!strcmp(setting->name, "video_rotation"))
   {
      if (driver.video && driver.video->set_rotation)
         driver.video->set_rotation(driver.video_data,
               (*setting->value.unsigned_integer +
                g_extern.system.rotation) % 4);
   }
   else if (!strcmp(setting->name, "system_bgm_enable"))
   {
      if (*setting->value.boolean)
      {
#if defined(__CELLOS_LV2__) && (CELL_SDK_VERSION > 0x340000)
         cellSysutilEnableBgmPlayback();
#endif         
      }
      else
      {
#if defined(__CELLOS_LV2__) && (CELL_SDK_VERSION > 0x340000)
         cellSysutilDisableBgmPlayback();
#endif
      }
   }
   else if (!strcmp(setting->name, "audio_volume"))
      g_extern.audio_data.volume_gain = db_to_gain(*setting->value.fraction);
   else if (!strcmp(setting->name, "audio_latency"))
      rarch_cmd = RARCH_CMD_AUDIO_REINIT;
   else if (!strcmp(setting->name, "audio_rate_control_delta"))
   {
      if (*setting->value.fraction < 0.0005)
      {
         g_settings.audio.rate_control = false;
         g_settings.audio.rate_control_delta = 0.0;
      }
      else
      {
         g_settings.audio.rate_control = true;
         g_settings.audio.rate_control_delta = *setting->value.fraction;
      }
   }
   else if (!strcmp(setting->name, "video_refresh_rate_auto"))
   {
      if (driver.video && driver.video_data)
      {
         driver_set_monitor_refresh_rate(*setting->value.fraction);

         /* In case refresh rate update forced non-block video. */
         rarch_cmd = RARCH_CMD_VIDEO_SET_BLOCKING_STATE;
      }
   }
  /* else if (!strcmp(setting->name, "video_scale"))
   {
      g_settings.video.scale = roundf(*setting->value.fraction);

      if (!g_settings.video.fullscreen)
         rarch_cmd = RARCH_CMD_REINIT;
   } */
   else if (!strcmp(setting->name, "input_player1_joypad_index"))
      g_settings.input.joypad_map[0] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player2_joypad_index"))
      g_settings.input.joypad_map[1] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player3_joypad_index"))
      g_settings.input.joypad_map[2] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player4_joypad_index"))
      g_settings.input.joypad_map[3] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player5_joypad_index"))
      g_settings.input.joypad_map[4] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player6_joypad_index"))
      g_settings.input.joypad_map[5] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player7_joypad_index"))
      g_settings.input.joypad_map[6] = *setting->value.integer;
   else if (!strcmp(setting->name, "input_player8_joypad_index"))
      g_settings.input.joypad_map[7] = *setting->value.integer;
#ifdef HAVE_NETPLAY
   else if (!strcmp(setting->name, "netplay_ip_address"))
      g_extern.has_set_netplay_ip_address = (setting->value.string[0] != '\0');
   else if (!strcmp(setting->name, "netplay_mode"))
   {
      if (!g_extern.netplay_is_client)
         *g_extern.netplay_server = '\0';
      g_extern.has_set_netplay_mode = true;
   }
   else if (!strcmp(setting->name, "netplay_spectator_mode_enable"))
   {
      if (g_extern.netplay_is_spectate)
         *g_extern.netplay_server = '\0';
   }
   else if (!strcmp(setting->name, "netplay_delay_frames"))
      g_extern.has_set_netplay_delay_frames = (g_extern.netplay_sync_frames > 0);
#endif
   else if (!strcmp(setting->name, "log_verbosity"))
   {
      g_extern.verbosity         = *setting->value.boolean;
      g_extern.has_set_verbosity = *setting->value.boolean;
   }

   if (rarch_cmd || setting->cmd_trigger.triggered)
      rarch_main_command(rarch_cmd);
}

#define START_GROUP(group_info, NAME) \
{ \
   group_info.name = NAME; \
   if (!(settings_list_append(list, list_info, setting_data_group_setting (ST_GROUP, NAME)))) return false; \
}

#define END_GROUP(list, list_info) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_group_setting (ST_END_GROUP, 0)))) return false; \
}

#define START_SUB_GROUP(list, list_info, NAME, group_info, subgroup_info) \
{ \
   subgroup_info.name = NAME; \
   if (!(settings_list_append(list, list_info, setting_data_subgroup_setting (ST_SUB_GROUP, NAME, group_info)))) return false; \
}

#define END_SUB_GROUP(list, list_info) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_group_setting (ST_END_SUB_GROUP, 0)))) return false; \
}

#define CONFIG_ACTION(NAME, SHORT, group_info, subgroup_info) \
{ \
   if (!settings_list_append(list, list_info, setting_data_action_setting  (NAME, SHORT, group_info, subgroup_info))) return false; \
}

#define CONFIG_BOOL(TARGET, NAME, SHORT, DEF, OFF, ON, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!settings_list_append(list, list_info, setting_data_bool_setting  (NAME, SHORT, &TARGET, DEF, OFF, ON, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))return false; \
}

#define CONFIG_INT(TARGET, NAME, SHORT, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_int_setting   (NAME, SHORT, &TARGET, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_UINT(TARGET, NAME, SHORT, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_uint_setting  (NAME, SHORT, &TARGET, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_FLOAT(TARGET, NAME, SHORT, DEF, ROUNDING, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_float_setting (NAME, SHORT, &TARGET, DEF, ROUNDING, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_PATH(TARGET, NAME, SHORT, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_string_setting(ST_PATH, NAME, SHORT, TARGET, sizeof(TARGET), DEF, "", group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_DIR(TARGET, NAME, SHORT, DEF, EMPTY, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_string_setting(ST_DIR, NAME, SHORT, TARGET, sizeof(TARGET), DEF, EMPTY, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_STRING(TARGET, NAME, SHORT, DEF, group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_string_setting(ST_STRING, NAME, SHORT, TARGET, sizeof(TARGET), DEF, "", group_info, subgroup_info, CHANGE_HANDLER, READ_HANDLER)))) return false; \
}

#define CONFIG_HEX(TARGET, NAME, SHORT, group_info, subgroup_info)

#define CONFIG_BIND(TARGET, PLAYER, PLAYER_OFFSET, NAME, SHORT, DEF, group_info, subgroup_info) \
{ \
   if (!(settings_list_append(list, list_info, setting_data_bind_setting  (NAME, SHORT, &TARGET, PLAYER, PLAYER_OFFSET, DEF, group_info, subgroup_info)))) return false; \
}


#ifdef GEKKO
#define MAX_GAMMA_SETTING 2
#else
#define MAX_GAMMA_SETTING 1
#endif

static int setting_data_string_action_toggle_driver(void *data,
      unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;


   switch (action)
   {
      case MENU_ACTION_LEFT:
         find_prev_driver(setting->name, setting->value.string, setting->size);
         break;
      case MENU_ACTION_RIGHT:
         find_next_driver(setting->name, setting->value.string, setting->size);
         break;
   }

   return 0;
}

static int setting_data_string_action_start_allow_input(void *data)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting || !driver.menu)
      return -1;

   *setting->value.string = '\0';

   return 0;
}

static int setting_data_string_action_ok_allow_input(void *data,
      unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting || !driver.menu)
      return -1;

   menu_key_start_line(driver.menu, setting->short_description,
         setting->name, st_string_callback);

   return 0;
}

static int setting_data_string_action_toggle_audio_resampler(void *data,
      unsigned action)
{
   rarch_setting_t *setting = (rarch_setting_t*)data;

   if (!setting)
      return -1;

   switch (action)
   {
      case MENU_ACTION_LEFT:
         find_prev_resampler_driver();
         break;
      case MENU_ACTION_RIGHT:
         find_next_resampler_driver();
         break;
   }

   return 0;
}

static void setting_data_add_special_callbacks(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info,
      unsigned values)
{
   unsigned idx = list_info->index - 1;

   if (values & SD_FLAG_ALLOW_INPUT)
   {
      switch ((*list)[idx].type)
      {
         case ST_UINT:
            (*list)[idx].action_start  = setting_data_uint_action_start_linefeed;
            (*list)[idx].action_ok     = setting_data_uint_action_ok_linefeed;
            break;
         case ST_STRING:
            (*list)[idx].action_start  = setting_data_string_action_start_allow_input;
            (*list)[idx].action_ok     = setting_data_string_action_ok_allow_input;
            break;
         default:
            break;
      }
   }
   else if (values & SD_FLAG_IS_DRIVER)
      (*list)[idx].action_toggle = setting_data_string_action_toggle_driver;
}

static void settings_data_list_current_add_flags(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info,
      unsigned values)
{
   settings_list_current_add_flags(
         list,
         list_info,
         values);
   setting_data_add_special_callbacks(list, list_info, values);
}

static bool setting_data_append_list_main_menu_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;
   
   /* Something around here is causing an annoying bug,
    * sometimes it hides the number for Load State,
    * toggling through device index causes the gui to exit to game.
	* No idea. Move slot number to its own setting in General. */

   START_GROUP(group_info, "Main Menu");
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);
   
   if (!g_settings.single_mode) {
#if defined(HAVE_DYNAMIC) || defined(HAVE_LIBRETRO_MANAGEMENT)
   CONFIG_ACTION(
         "core_list",
         nucleo,
         group_info.name,
         subgroup_info.name);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_PUSH_ACTION);
#endif
  /* if (g_defaults.history)
   {
      CONFIG_ACTION(
            "history_list",
            "Load Content (History)",
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   } */
  /* if (
         driver.menu 
         && g_extern.core_info 
         && core_info_list_num_info_files(g_extern.core_info))
   {
      CONFIG_ACTION(
            "detect_core_list",
            "Load Content (Detect Core)",
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   } */
   CONFIG_ACTION(
         "load_content",
         "Selecciona el juego",
         group_info.name,
         subgroup_info.name);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);

   if (!g_settings.hide_core) {
      CONFIG_ACTION(
            "core_options",
            nucleo_opciones,
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
	 }
   } else {
   if (!g_settings.hide_core) {
      CONFIG_ACTION(
            "core_options",
            "Opciones del sistema",
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
     }
   }

  /* CONFIG_ACTION(
         "core_information",
         "Core Information",
         group_info.name,
         subgroup_info.name);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);*/

   if (g_extern.main_is_init
         && !g_extern.libretro_dummy
         && g_extern.system.disk_control.get_num_images)
   {
      CONFIG_ACTION(
            "disk_options",
            "Disk Options",
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   }

   /*if (g_extern.perfcnt_enable)
   {
      CONFIG_ACTION(
            "performance_counters",
            "Performance Counters",
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   }*/

   if (g_extern.main_is_init && !g_extern.libretro_dummy)
   {
     if (!g_settings.hide_states) {
      CONFIG_ACTION(
            "savestate_nonsense_long_name",
            "Salvar estado",
            group_info.name,
            subgroup_info.name);
     // (*list)[list_info->index - 1].action_toggle = &setting_data_bool_action_toggle_savestates;
  //    (*list)[list_info->index - 1].action_start = &setting_data_bool_action_start_savestates;
      (*list)[list_info->index - 1].action_ok = &setting_data_bool_action_ok_exit;
     // (*list)[list_info->index - 1].get_string_representation = &get_string_representation_savestate;
      settings_list_current_add_cmd  (list, list_info, RARCH_CMD_SAVE_STATE);

      CONFIG_ACTION(
            "loadstate_nonsense_long_name",
            "Cargar estado",
            group_info.name,
            subgroup_info.name);
     // (*list)[list_info->index - 1].action_toggle = &setting_data_bool_action_toggle_savestates;
    //  (*list)[list_info->index - 1].action_start = &setting_data_bool_action_start_savestates;
      (*list)[list_info->index - 1].action_ok = &setting_data_bool_action_ok_exit;
     // (*list)[list_info->index - 1].get_string_representation = &get_string_representation_loadstate;
      settings_list_current_add_cmd  (list, list_info, RARCH_CMD_LOAD_STATE);
	 }
     if (!g_settings.hide_curr_state) {
         CONFIG_UINT(
             g_settings.state_slot,
             "state_slot",
             "Estado actual",
             state_slot,
             group_info.name,
             subgroup_info.name,
             general_write_handler,
             general_read_handler);
         settings_list_current_add_range(list, list_info, 1, 10, 1, true, true);
     }
	}

   if (!g_settings.hide_settings) {
         CONFIG_ACTION(
               "settings",
               "Configurar",
               group_info.name,
               subgroup_info.name);
         settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   }

   if (g_settings.single_mode && g_settings.show_manuals) {
         CONFIG_ACTION(
               "core_list",
               "Manual original",
               group_info.name,
               subgroup_info.name);
         settings_data_list_current_add_flags(
               list,
               list_info,
               SD_FLAG_PUSH_ACTION);
   }

   if (g_extern.main_is_init && !g_extern.libretro_dummy)
   {
     if (!g_settings.hide_screenshot) {
      CONFIG_ACTION(
            "take_screenshot",
            "Tomar foto",
            group_info.name,
            subgroup_info.name);
      settings_list_current_add_cmd  (list, list_info, RARCH_CMD_TAKE_SCREENSHOT);
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
     }

     if (!g_settings.hide_resume) {
      CONFIG_ACTION(
            "resume_content",
            "Regresar al juego",
            group_info.name,
            subgroup_info.name);
      settings_list_current_add_cmd  (list, list_info, RARCH_CMD_RESUME);
      (*list)[list_info->index - 1].action_ok = &setting_data_bool_action_ok_exit;
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
     }

     if (!g_settings.hide_reset) {
      CONFIG_ACTION(
            "restart_content",
            "Reiniciar",
            group_info.name,
            subgroup_info.name);
      settings_list_current_add_cmd(list, list_info, RARCH_CMD_RESET);
      (*list)[list_info->index - 1].action_ok = &setting_data_bool_action_ok_exit;
      settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
     }
   }
/*
#ifndef HAVE_DYNAMIC
   CONFIG_ACTION(
         "restart_retroarch",
         "Restart RetroArch",
         group_info.name,
         subgroup_info.name);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_RESTART_RETROARCH);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
#endif

   CONFIG_ACTION(
         "configurations",
         "Configurations",
         group_info.name,
         subgroup_info.name);

   CONFIG_ACTION(
         "save_new_config",
         "Save New Config",
         group_info.name,
         subgroup_info.name);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_MENU_SAVE_CONFIG);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);

   CONFIG_ACTION(
         "help",
         "Help",
         group_info.name,
         subgroup_info.name);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
*/
   if (!g_settings.hide_exit) {
     CONFIG_ACTION(
           "quit_retroarch",
           "Salir",
           group_info.name,
           subgroup_info.name);
     settings_list_current_add_cmd(list, list_info, RARCH_CMD_QUIT_RETROARCH);
     settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);
   }

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_driver_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   /* Driver Options is now Menu */
   START_GROUP(group_info, "Menu");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);

   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

/* CONFIG_STRING(
         g_settings.input.driver,
         "input_driver",
         "Input Driver",
         config_get_default_input(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);

   CONFIG_STRING(
         g_settings.video.driver,
         "video_driver",
         "Video Driver",
         config_get_default_video(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);

   CONFIG_STRING(
         g_settings.video.context_driver,
         "video_context_driver",
         "Video Context Driver",
         "",
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);
   CONFIG_STRING(
         g_settings.audio.driver,
         "audio_driver",
         "Audio Driver",
         config_get_default_audio(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);*/

   CONFIG_BOOL(
         g_settings.input.rgui_reset,
         "rgui_reset",
         "Abrir con Reiniciar",
         rgui_reset,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.title_posx,
         "title_posx",
         title_pos,
         title_posx,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 250, 1, true, true);

   CONFIG_UINT(
         g_settings.item_posx,
         "item_posx",
         pos_generalx,
         item_posx,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 90, 1, true, true);

   CONFIG_UINT(
         g_settings.item_posy,
         "item_posy",
         pos_generaly,
         item_posy,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 90, 1, true, true);

   CONFIG_BOOL(
         g_settings.hide_core,
         "hide_core",
         "Esconder opciones",
         hide_core,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_states,
         "hide_states",
         "Esconder estados",
         hide_states,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_screenshot,
         "hide_screenshot",
         "Esconder Toma foto",
         hide_screenshot,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_resume,
         "hide_resume",
         "Esconder Regresar al juego",
         hide_resume,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_reset,
         "hide_reset",
         "Esconder Reiniciar",
         hide_reset,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_cursor,
         "hide_cursor",
         "Esconder cursor",
         hide_cursor,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.hide_curr_state,
         "hide_curr_state",
         "Esconder estado actual",
         hide_curr_state,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

  /* CONFIG_STRING(
         g_settings.camera.driver,
         "camera_driver",
         "Camera Driver",
         config_get_default_camera(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);

   CONFIG_STRING(
         g_settings.location.driver,
         "location_driver",
         "Location Driver",
         config_get_default_location(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);*/

#ifdef HAVE_MENU
/*   CONFIG_STRING(
         g_settings.menu.driver,
         "menu_driver",
         "Menu Driver",
         config_get_default_menu(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);
*/
   CONFIG_UINT(
         g_settings.theme_preset,
         "theme_preset",
         "Color",
         theme_preset,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 8, 1, true, true);

   CONFIG_BOOL(
         g_settings.menu_fullscreen,
         "menu_fullscreen",
         "Imagen completa",
         menu_fullscreen,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.menu_solid,
         "menu_solid",
         "Color del fondo",
         menu_solid,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.clock_show,
         "clock_show",
         "La hora",
         clock_show,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.clock_posx,
         "clock_posx",
         "Pos X del reloj",
         clock_posx,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 320, 1, true, true);

   CONFIG_BOOL(
         g_settings.fadein,
         "fadein",
         "Fundido de entrada",
         fadein,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.reset_fade,
         "reset_fade",
         "Fundido al reiniciar",
         reset_fade,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 2, 1, true, true);

   CONFIG_BOOL(
         g_settings.exit_fade,
         "exit_fade",
         "Fundido de salida",
         exit_fade,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif

   /*CONFIG_STRING(
         g_settings.input.joypad_driver,
         "input_joypad_driver",
         "Joypad Driver",
         config_get_default_joypad(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);

   CONFIG_STRING(
         g_settings.input.keyboard_layout,
         "input_keyboard_layout",
         "Keyboard Layout",
         "",
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DRIVER);*/

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_general_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;
//General Options is now just General
   START_GROUP(group_info, "General");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

  /* CONFIG_BOOL(
         g_extern.verbosity,
         "log_verbosity",
         "Logging Verbosity",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(g_settings.libretro_log_level,
         "libretro_log_level",
         "Libretro Logging Level",
         libretro_log_level,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 3, 1.0, true, true);

   CONFIG_BOOL(g_extern.perfcnt_enable,
         "perfcnt_enable",
         "Performance Counters",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

   CONFIG_BOOL(g_settings.config_save_on_exit,
         "config_save_on_exit",
         "Guardar ajustes",
         config_save_on_exit,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.core_specific_config,
         "core_specific_config",
         "Config por sistema",
         default_core_specific_config,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_ACTION(
           "game_specific_config_create",
           "Crear config para el juego",
           group_info.name,
           subgroup_info.name);
     settings_list_current_add_cmd(list, list_info, RARCH_CMD_PER_GAME_CFG);
     settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);

   CONFIG_ACTION(
           "game_specific_config_remove",
           "Remover config del juego",
           group_info.name,
           subgroup_info.name);
     settings_list_current_add_cmd(list, list_info, RARCH_CMD_PER_GAME_CFG_REMOVE);
     settings_data_list_current_add_flags(list, list_info, SD_FLAG_PUSH_ACTION);

  /* CONFIG_BOOL(
         g_settings.load_dummy_on_core_shutdown,
         "dummy_on_core_shutdown",
         "Dummy On Core Shutdown",
         load_dummy_on_core_shutdown,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

   if (!g_settings.video.drawdone) {
      CONFIG_BOOL(g_settings.fps_show,
            "fps_show",
            "Ver velocidad",
            fps_show,
            "No",
            sip,
            group_info.name,
            subgroup_info.name,
            general_write_handler,
            general_read_handler);
   }

  /* CONFIG_BOOL(
         g_settings.rewind_enable,
         "rewind_enable",
         "Rewind",
         rewind_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REWIND_TOGGLE);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
#if 0
   CONFIG_SIZE(
         g_settings.rewind_buffer_size,
         "rewind_buffer_size",
         "Rewind Buffer Size",
         rewind_buffer_size,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler)
#endif
      CONFIG_UINT(
            g_settings.rewind_granularity,
            "rewind_granularity",
            "Rewind Granularity",
            rewind_granularity,
            group_info.name,
            subgroup_info.name,
            general_write_handler,
            general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 32768, 1, true, false);*/

   CONFIG_BOOL(
         g_settings.block_sram_overwrite,
         "block_sram_overwrite",
         "Bloqueo de SRAM",
         block_sram_overwrite,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);;

#ifdef HAVE_THREADS
   CONFIG_UINT(
         g_settings.autosave_interval,
         "autosave_interval",
         "SRAM Autosalvar",
         autosave_interval,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_AUTOSAVE_INIT);
   settings_list_current_add_range(list, list_info, 0, 0, 10, true, false);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
#endif

  /* CONFIG_BOOL(
         g_settings.video.disable_composition,
         "video_disable_composition",
         "Window Compositing Disable",
         disable_composition,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   CONFIG_BOOL(
         g_settings.pause_nonactive,
         "pause_nonactive",
         "Window Unfocus Pause",
         pause_nonactive,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.fastforward_ratio_throttle_enable,
         "fastforward_ratio_throttle_enable",
         "Limit Maximum Run Speed",
         fastforward_ratio_throttle_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
*/
   CONFIG_FLOAT(
         g_settings.fastforward_ratio,
         "fastforward_ratio",
         "Velocidad del juego",
         fastforward_ratio,
         "%.1fx",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 10, 0.1, true, true);

   CONFIG_FLOAT(
         g_settings.slowmotion_ratio,
         "slowmotion_ratio",
         "Camara lenta",
         slowmotion_ratio,
         "%.1fx",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 10, 1.0, true, true);

   CONFIG_BOOL(
         g_settings.savestate_auto_index,
         "savestate_auto_index",
         "Auto indice de estados",
         savestate_auto_index,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.regular_state_pause,
         "regular_state_pause",
         "Pausa al cargar estado",
         regular_state_pause,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.stateload_pause,
         "stateload_pause",
         "Pausa al cargar auto estado",
         stateload_pause,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.savestate_auto_once,
         "savestate_auto_once",
         "Auto cargar una vez",
         savestate_auto_once,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.savestate_auto_save,
         "savestate_auto_save",
         "Auto salvar estado",
         savestate_auto_save,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
            general_read_handler);

   CONFIG_BOOL(
         g_settings.savestate_auto_load,
         "savestate_auto_load",
         "Auto cargar estado",
         savestate_auto_load,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.state_slot,
         "state_slot",
         "Estado actual",
         state_slot,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 10, 1, true, true);

   END_SUB_GROUP(list, list_info);
   START_SUB_GROUP(
         list,
         list_info,
         "Miscellaneous",
         group_info.name,
         subgroup_info);
#if defined(HAVE_NETWORK_CMD) && defined(HAVE_NETPLAY)
   CONFIG_BOOL(
         g_settings.network_cmd_enable,
         "network_cmd_enable",
         "Network Commands",
         network_cmd_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#if 0
   CONFIG_INT(
         g_settings.network_cmd_port,
         "network_cmd_port",
         "Network Command Port",
         network_cmd_port,
         group_info.name,
         subgroup_info.name,
         NULL);
#endif
   CONFIG_BOOL(
         g_settings.stdin_cmd_enable,
         "stdin_cmd_enable",
         "stdin command",
         stdin_cmd_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif
   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_video_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   /* Video Options is now Video */
   START_GROUP(group_info, "Video");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   /*CONFIG_BOOL(
         g_settings.video.shared_context,
         "video_shared_context",
         "HW Shared Context Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   END_SUB_GROUP(list, list_info);
   START_SUB_GROUP(list, list_info, "Monitor", group_info.name, subgroup_info);

   CONFIG_UINT(
         g_settings.video.monitor_index,
         "video_monitor_index",
         "Monitor Index",
         monitor_index,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
   settings_list_current_add_range(list, list_info, 0, 1, 1, true, false);*/

#if !defined(RARCH_CONSOLE) && !defined(RARCH_MOBILE)
   CONFIG_BOOL(
         g_settings.video.fullscreen,
         "video_fullscreen",
         "Use Fullscreen mode",
         fullscreen,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
#endif
  /* CONFIG_BOOL(
         g_settings.video.windowed_fullscreen,
         "video_windowed_fullscreen",
         "Windowed Fullscreen Mode",
         windowed_fullscreen,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.video.fullscreen_x,
         "video_fullscreen_x",
         "Fullscreen Width",
         fullscreen_x,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.video.fullscreen_y,
         "video_fullscreen_y",
         "Fullscreen Height",
         fullscreen_y,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

  /* CONFIG_FLOAT(
         g_settings.video.refresh_rate,
         "video_refresh_rate",
         "Refresh Rate",
         refresh_rate,
         "%.3f Hz",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 0, 0.001, true, false);*/

 /*  CONFIG_BOOL(
         g_settings.video.force_srgb_disable,
         "video_force_srgb_disable",
         "Force-disable sRGB FBO",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   END_SUB_GROUP(list, list_info);

   // START_SUB_GROUP(list, list_info, "Aspect", group_info.name, subgroup_info);
   CONFIG_BOOL(
         g_settings.video.force_aspect,
         "video_force_aspect",
         "Force aspect ratio",
         force_aspect,
         sip,
         "No",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_FLOAT(
         g_settings.video.aspect_ratio,
         "video_aspect_ratio",
         "Aspect Ratio",
         aspect_ratio,
         "%.2f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.aspect_ratio_auto,
         "video_aspect_ratio_auto",
         "Use Auto Aspect Ratio",
         aspect_ratio_auto,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

   if (CONF_GetVideo() != CONF_VIDEO_NTSC) {
       CONFIG_BOOL(
             g_settings.video.force_288p,
             "video_force_288p",
             "Forzar 288p",
             video_force_288p,
             "No",
             sip,
             group_info.name,
             subgroup_info.name,
             general_write_handler,
             general_read_handler);
   }

   CONFIG_UINT(
         g_settings.video.aspect_ratio_idx,
         "aspect_ratio_index",
         indice_aspect,
         aspect_ratio_idx,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(
         list,
         list_info,
         RARCH_CMD_VIDEO_SET_ASPECT_RATIO);
   settings_list_current_add_range(
         list,
         list_info,
         0,
         LAST_ASPECT_RATIO,
         1,
         true,
         true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   // END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(list, list_info, "Scaling", group_info.name, subgroup_info);

#if !defined(RARCH_CONSOLE) && !defined(RARCH_MOBILE)
   CONFIG_FLOAT(
         g_settings.video.scale,
         "video_scale",
         "Windowed Scale",
         scale,
         "%.1fx",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1.0, 10.0, 1.0, true, true);
#endif

   CONFIG_BOOL(
         g_settings.video.scale_integer,
         "video_scale_integer",
         por_int,
         scale_integer,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_INT(
         g_extern.console.screen.viewports.custom_vp.x,
         "custom_viewport_x",
         "Vista X",
         0,
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   
   CONFIG_INT(
         g_extern.console.screen.viewports.custom_vp.y,
         "custom_viewport_y",
         "Vista Y",
         0,
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);

   CONFIG_UINT(
         g_extern.console.screen.viewports.custom_vp.width,
         "custom_viewport_width",
         "Ancho actual",
         0,
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);

   CONFIG_UINT(
         g_extern.console.screen.viewports.custom_vp.height,
         "custom_viewport_height",
         "Altura actual",
         0,
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);

#ifdef GEKKO
/* Added for testing */
  /* CONFIG_UINT(
         g_settings.video.vres,
         "video_vres",
         "Current Resolution Index",
         video_vres,
         group_info.name,
         subgroup_info.name,
		 NULL,
		 NULL);
         //general_write_handler,
         //general_read_handler);
  // settings_list_current_add_range(list, list_info, 0, 38, 1, true, true);
  */

   CONFIG_BOOL(
         g_settings.video.autores,
         "video_autores",
         "Auto-cambio de res",
         video_autores,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.video.viwidth,
         "video_viwidth",
         "El ancho del VI",
         video_viwidth,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 640, 720, 2, true, true);

   CONFIG_BOOL(
         g_settings.video.vfilter,
         "video_vfilter",
         "Defliqueador",
         video_vfilter,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.dither,
         "video_dither",
         "Dither",
         video_dither,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.blendframe,
         "video_blendframe",
         "Mezclar imagen",
         video_blendframe,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

	CONFIG_BOOL(
         g_settings.video.prescale,
         "video_prescale",
         "Pre-escalar",
         video_prescale,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif

#if defined(_XBOX1) || defined(HW_RVL)
   CONFIG_BOOL(
         g_extern.console.softfilter_enable,
         "soft_filter",
         "Filtro Trampa",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(
         list,
         list_info,
         RARCH_CMD_VIDEO_APPLY_STATE_CHANGES);
#endif

   CONFIG_BOOL(
         g_settings.video.smooth,
         "video_smooth",
         "Filtro de textura",
         video_smooth,
         "Punto",
         "Bilineal",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.menu_smooth,
         "video_menu_smooth",
         "Filtro del interfaz",
         video_menu_smooth,
         "Punto",
         "Bilineal",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.blend_smooth,
         "video_blend_smooth",
         "Filtro de imagen mezclada",
         video_blend_smooth,
         "Punto",
         "Bilineal",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

#ifdef HAVE_RENDERSCALE
   CONFIG_UINT(
         g_settings.video.renderscale,
         "video_renderscale",
         "Render Scale",
         video_renderscale,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 2, 1, true, true);
#endif

#if defined(__CELLOS_LV2__)
   CONFIG_BOOL(
         g_extern.console.screen.pal60_enable,
         "pal60_enable",
         "Use PAL60 Mode",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif

   CONFIG_FLOAT(
         g_settings.video.vbright,
         "video_vbright",
         "Brillo",
         video_vbright,
		 "%.0f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, -22, 30, 1, true, true);

#if defined(HW_RVL) || defined(_XBOX360)
   CONFIG_UINT(
         g_extern.console.screen.gamma_correction,
         "video_gamma",
         "Gamma",
         0,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(
         list,
         list_info,
         RARCH_CMD_VIDEO_APPLY_STATE_CHANGES);
   settings_list_current_add_range(
         list,
         list_info,
         0,
         MAX_GAMMA_SETTING,
         1,
         true,
         true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
#endif
   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Synchronization",
         group_info.name,
         subgroup_info);

#if defined(HAVE_THREADS) && !defined(RARCH_CONSOLE)
   CONFIG_BOOL(
         g_settings.video.threaded,
         "video_threaded",
         "Threaded Video",
         video_threaded,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
#endif

   CONFIG_UINT(
         g_settings.video.rotation,
         "video_rotation",
         rotacion,
         0,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 3, 1, true, true);

   CONFIG_BOOL(
         g_settings.video.vsync,
         "video_vsync",
         "VSync",
         vsync,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

  /* CONFIG_UINT(
         g_settings.video.swap_interval,
         "video_swap_interval",
         "VSync Swap Interval",
         swap_interval,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_VIDEO_SET_BLOCKING_STATE);
   settings_list_current_add_range(list, list_info, 1, 4, 1, true, true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   CONFIG_BOOL(
         g_settings.video.hard_sync,
         "video_hard_sync",
         "Hard GPU Sync",
         hard_sync,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.video.hard_sync_frames,
         "video_hard_sync_frames",
         "Hard GPU Sync Frames",
         hard_sync_frames,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 3, 1, true, true);*/

   CONFIG_UINT(
         g_settings.video.frame_delay,
         "video_frame_delay",
         "Retraso de imagen",
         frame_delay,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 15, 1, true, true);
/*
#if !defined(RARCH_MOBILE)
   CONFIG_BOOL(
         g_settings.video.black_frame_insertion,
         "video_black_frame_insertion",
         "Black Frame Insertion",
         black_frame_insertion,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif
*/
   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Miscellaneous",
         group_info.name,
         subgroup_info);

  /* CONFIG_BOOL(
         g_settings.video.post_filter_record,
         "video_post_filter_record",
         "Post filter record Enable",
         post_filter_record,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.gpu_record,
         "video_gpu_record",
         "GPU Record Enable",
         gpu_record,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.video.gpu_screenshot,
         "video_gpu_screenshot",
         "GPU Screenshots",
         gpu_screenshot,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

  /* CONFIG_BOOL(
         g_settings.video.allow_rotate,
         "video_allow_rotate",
         "Allow Rotation",
         allow_rotate,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

  /* CONFIG_BOOL(
         g_settings.video.crop_overscan,
         "video_crop_overscan",
         "Crop Overscan (reload)",
         crop_overscan,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

#ifdef HAVE_FILTERS_BUILTIN
   CONFIG_PATH(
         g_settings.video.softfilter_plugin,
         "video_filter",
         "Filtro CPU",
         g_settings.video.filter_dir,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_values(list, list_info, "filt");
  // settings_list_current_add_cmd(list, list_info, RARCH_CMD_REINIT);
  // CPU filters always crash on the 16th switch.
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);
#endif

/*   CONFIG_FLOAT(
         g_settings.video.refresh_rate,
         "video_refresh_rate_auto",
         "Estimated Refresh Rate",
         refresh_rate,
         "%.3f Hz",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   (*list)[list_info->index - 1].action_start = 
      &setting_data_fraction_action_start_video_refresh_rate_auto;
   (*list)[list_info->index - 1].action_ok = 
      &setting_data_fraction_action_ok_video_refresh_rate_auto;
*/
#ifdef _XBOX1
   CONFIG_UINT(
         g_settings.video.swap_interval,
         "video_filter_flicker",
         "Flicker filter",
         0,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 5, 1, true, true);
#endif
   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_shader_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Shader Options");
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_settings.video.shader_enable,
         "video_shader_enable",
         "Enable Shaders",
         shader_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);

   CONFIG_PATH(
         g_settings.video.shader_path,
         "video_shader",
         "Shader",
         "",
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/

   return true;
}

static bool setting_data_append_list_font_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Font Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "Messages", group_info.name, subgroup_info);

   CONFIG_PATH(
         g_settings.video.font_path,
         "video_font_path",
         "Font Path",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

   CONFIG_FLOAT(
         g_settings.video.font_size,
         "video_font_size",
         "OSD Font Size",
         font_size,
         "%.1f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1.00, 100.00, 1.0, true, true);

   CONFIG_BOOL(
         g_settings.video.font_enable,
         "video_font_enable",
         "OSD Font Enable",
         font_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_FLOAT(
         g_settings.video.msg_pos_x,
         "video_message_pos_x",
         "Message X Position",
         message_pos_offset_x,
         "%.3f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 1, 0.01, true, true);

   CONFIG_FLOAT(
         g_settings.video.msg_pos_y,
         "video_message_pos_y",
         "Message Y Position",
         message_pos_offset_y,
         "%.3f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 1, 0.01, true, true);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/

   return true;
}

static bool setting_data_append_list_audio_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   /* Audio Options is now Audio */
   START_GROUP(group_info, "Audio");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   /* Why would a user EVER need to disable audio?? */

  /* CONFIG_BOOL(
         g_settings.audio.enable,
         "audio_enable",
         "Audio Enable (Restart)",
         audio_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

   CONFIG_STRING(
         g_settings.audio.resampler,
         "audio_resampler_driver",
         "Remuestreo",
         config_get_default_audio_resampler(),
         group_info.name,
         subgroup_info.name,
         NULL,
         NULL);
   (*list)[list_info->index - 1].action_toggle = &setting_data_string_action_toggle_audio_resampler;

   CONFIG_UINT(
         g_settings.audio.sinc_taps,
         "audio_sinc_taps",
         "SINC Taps",
         sinc_taps,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
	settings_list_current_add_range(list, list_info, 2, 256, 2, true, true);

   CONFIG_BOOL(
         g_extern.audio_data.mute,
         "audio_mute_enable",
         "Mute",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_FLOAT(
         g_settings.audio.volume,
         "audio_volume",
         "Nivel del volumen",
         audio_volume,
         "%.1f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, -80, 12, 1.0, true, true);

#ifdef __CELLOS_LV2__
   CONFIG_BOOL(
         g_extern.console.sound.system_bgm_enable,
         "system_bgm_enable",
         "System BGM Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#endif

   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Synchronization",
         group_info.name,
         subgroup_info);

   CONFIG_BOOL(
         g_settings.audio.sync,
         "audio_sync",
         "Sincronizar",
         audio_sync,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

  /* CONFIG_UINT(
         g_settings.audio.latency,
         "audio_latency",
         "Audio Latency",
         g_defaults.settings.out_latency ? 
         g_defaults.settings.out_latency : out_latency,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 256, 1.0, true, true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_DEFERRED);*/

   CONFIG_FLOAT(
         g_settings.audio.rate_control_delta,
         "audio_rate_control_delta",
         "Velocidad control delta",
         rate_control_delta,
         "%.3f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(
         list,
         list_info,
         0,
         0,
         0.001,
         true,
         false);

  /* CONFIG_UINT(
         g_settings.audio.block_frames,
         "audio_block_frames",
         "Block Frames",
         0,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/

   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Miscellaneous",
         group_info.name,
         subgroup_info);

  /* CONFIG_STRING(
         g_settings.audio.device,
         "audio_device",
         "Device",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_INPUT);*/

   CONFIG_UINT(
         g_settings.audio.out_rate,
         "audio_out_rate",
         "Output Rate",
         out_rate,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
	settings_list_current_add_range(list, list_info, 32000, 48000, 16000, true, true);

   CONFIG_PATH(
         g_settings.audio.dsp_plugin,
         "audio_dsp_plugin",
         "Plugin DSP",
         g_settings.audio.filter_dir,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_values(list, list_info, "dsp");
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_DSP_FILTER_INIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_input_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;
   unsigned i, player;

   /* Input Options is now Input */
   START_GROUP(group_info, "Controles");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_settings.input.autodetect_enable,
         "input_autodetect_enable",
         "Autodetectar",
         input_autodetect_enable,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Joypad Mapping",
         group_info.name,
         subgroup_info);

  /* CONFIG_BOOL(
         g_extern.menu.bind_mode_keyboard,
         "input_bind_mode",
         "Bind Mode",
         false,
         "RetroPad",
         "RetroKeyboard",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);*/
#ifdef HAVE_5PLAY
   for (player = 0; player < (MAX_PLAYERS - 11); player ++) // 5 players
#elif HAVE_6PLAY
   for (player = 0; player < (MAX_PLAYERS - 10); player ++) // 6 players
#elif HAVE_8PLAY
   for (player = 0; player < (MAX_PLAYERS - 8); player ++) // 8 players
#else
   for (player = 0; player < (MAX_PLAYERS - 12); player ++)
#endif
   {
      /* These constants match the string lengths.
       * Keep them up to date or you'll get some really obvious bugs.
       * 2 is the length of '99'; we don't need more players than that.
       */
      /* FIXME/TODO - really need to clean up this mess in some way. */
      static char key[MAX_PLAYERS][64];
      static char key_type[MAX_PLAYERS][64];
      static char key_analog[MAX_PLAYERS][64];
      static char key_bind_all[MAX_PLAYERS][64];
      static char key_bind_defaults[MAX_PLAYERS][64];

      static char label[MAX_PLAYERS][64];
      static char label_type[MAX_PLAYERS][64];
      static char label_analog[MAX_PLAYERS][64];
      static char label_bind_all[MAX_PLAYERS][64];
      static char label_bind_defaults[MAX_PLAYERS][64];

      snprintf(key[player], sizeof(key[player]),
               "input_player%d_joypad_index", player + 1);
      snprintf(key_type[player], sizeof(key_type[player]),
               "input_libretro_device_p%u", player + 1);
      snprintf(key_analog[player], sizeof(key_analog[player]),
               "input_player%u_analog_dpad_mode", player + 1);
      snprintf(key_bind_all[player], sizeof(key_bind_all[player]),
               "input_player%u_bind_all", player + 1);
      snprintf(key_bind_defaults[player], sizeof(key_bind_defaults[player]),
               "input_player%u_bind_defaults", player + 1);

      snprintf(label[player], sizeof(label[player]),
               "User %d Device Index", player + 1);
      snprintf(label_type[player], sizeof(label_type[player]),
               "User %d Device Type", player + 1);
      snprintf(label_analog[player], sizeof(label_analog[player]),
               "User %d Analog To Digital Type", player + 1);
      snprintf(label_bind_all[player], sizeof(label_bind_all[player]),
               "User %d Bind All", player + 1);
      snprintf(label_bind_defaults[player], sizeof(label_bind_defaults[player]),
               "User %d Bind Default All", player + 1);

      CONFIG_UINT(
            g_settings.input.libretro_device[player],
            key_type[player],
            label_type[player],
            player,
            group_info.name,
            subgroup_info.name,
            general_write_handler,
            general_read_handler);
      (*list)[list_info->index - 1].index = player + 1;
      (*list)[list_info->index - 1].index_offset = player;
      (*list)[list_info->index - 1].action_toggle = &setting_data_uint_action_toggle_libretro_device_type;
      (*list)[list_info->index - 1].action_start = &setting_data_uint_action_start_libretro_device_type;

      CONFIG_UINT(
            g_settings.input.analog_dpad_mode[player],
            key_analog[player],
            label_analog[player],
            player,
            group_info.name,
            subgroup_info.name,
            general_write_handler,
            general_read_handler);
      (*list)[list_info->index - 1].index = player + 1;
      (*list)[list_info->index - 1].index_offset = player;
      (*list)[list_info->index - 1].action_toggle = &setting_data_uint_action_toggle_analog_dpad_mode;
      (*list)[list_info->index - 1].action_start = &setting_data_uint_action_start_analog_dpad_mode;

    if (player != 0) { // Let's exclude player 1 because it only locks us out
      CONFIG_ACTION(
            key[player],
            label[player],
            group_info.name,
            subgroup_info.name);
      (*list)[list_info->index - 1].index = player + 1;
      (*list)[list_info->index - 1].index_offset = player;
      (*list)[list_info->index - 1].action_start  = &setting_data_action_start_bind_device;
      (*list)[list_info->index - 1].action_toggle = &setting_data_action_toggle_bind_device;
      (*list)[list_info->index - 1].get_string_representation = &get_string_representation_bind_device;
	}

      CONFIG_ACTION(
            key_bind_all[player],
            label_bind_all[player],
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(
            list,
            list_info,
            SD_FLAG_PUSH_ACTION);
      (*list)[list_info->index - 1].index = player + 1;
      (*list)[list_info->index - 1].index_offset = player;
      (*list)[list_info->index - 1].action_ok    = &setting_data_action_ok_bind_all;

      CONFIG_ACTION(
            key_bind_defaults[player],
            label_bind_defaults[player],
            group_info.name,
            subgroup_info.name);
      settings_data_list_current_add_flags(
            list,
            list_info,
            SD_FLAG_PUSH_ACTION);
      (*list)[list_info->index - 1].index = player + 1;
      (*list)[list_info->index - 1].index_offset = player;
      (*list)[list_info->index - 1].action_ok    = &setting_data_action_ok_bind_defaults;
   }

   START_SUB_GROUP(
         list,
         list_info,
         "Turbo/Deadzone",
         group_info.name,
         subgroup_info);

   CONFIG_UINT(
         g_settings.input.key_profile,
         "input_key_profile",
         "Key Layout Profile",
         key_profile,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
#if defined HAVE_5PLAY || defined HAVE_6PLAY || defined HAVE_8PLAY
   settings_list_current_add_range(list, list_info, 0, 3, 1, true, true);
#else
   settings_list_current_add_range(list, list_info, 0, 2, 1, true, true);
#endif

   CONFIG_UINT(
         g_settings.input.poll_rate,
         "input_poll_rate",
         "Polling Rate",
         poll_rate,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 11, 1, true, true);

   CONFIG_UINT(
         g_settings.input.trigger_threshold,
         "input_trigger_threshold",
         "Trigger Threshold",
         trigger_threshold,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 200, 1, true, true);

   CONFIG_FLOAT(
         g_settings.input.axis_threshold,
         "input_axis_threshold",
         "Axis Threshold",
         axis_threshold,
         "%.3f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 1.00, 0.001, true, true);

   CONFIG_UINT(
         g_settings.input.turbo_period,
         "input_turbo_period",
         "Periodo turbo",
         turbo_period,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 0, 1, true, false);

   CONFIG_UINT(
         g_settings.input.turbo_duty_cycle,
         "input_duty_cycle",
         "Duty Cycle",
         turbo_duty_cycle,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 0, 1, true, false);

   CONFIG_UINT(
         g_settings.input.menu_combos,
         "input_menu_combos",
         "Combo del interfaz",
         menu_combos,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 2, 1, true, true);

   END_SUB_GROUP(list, list_info);

   /* The second argument to config bind is 1 
    * based for players and 0 only for meta keys. */
   START_SUB_GROUP(
         list,
         list_info,
         "Meta Keys",
         group_info.name,
         subgroup_info);

   for (i = 0; i < RARCH_BIND_LIST_END; i ++)
   {
      const struct input_bind_map* keybind = (const struct input_bind_map*)
         &input_config_bind_map[i];

      if (!keybind || !keybind->meta)
         continue;

      CONFIG_BIND(g_settings.input.binds[0][i], 0, 0,
            keybind->base, keybind->desc, &retro_keybinds_1[i],
            group_info.name, subgroup_info.name);
      settings_list_current_add_bind_type(list, list_info, i + MENU_SETTINGS_BIND_BEGIN);
   }
   END_SUB_GROUP(list, list_info);

#ifdef HAVE_5PLAY
   for (player = 0; player < (MAX_PLAYERS - 11); player ++) // 5 players
#elif HAVE_6PLAY
   for (player = 0; player < (MAX_PLAYERS - 10); player ++) // 6 players
#elif HAVE_8PLAY
   for (player = 0; player < (MAX_PLAYERS - 8); player ++) // 8 players
#else
   for (player = 0; player < (MAX_PLAYERS - 12); player ++)
#endif
   {
      /* This const matches the string length.
       * Keep it up to date or you'll get some really obvious bugs.
       * 2 is the length of '99'; we don't need more players than that.
       */
      static char buffer[MAX_PLAYERS][7+2+1];
      const struct retro_keybind* const defaults =
         (player == 0) ? retro_keybinds_1 : retro_keybinds_rest;

      snprintf(buffer[player], sizeof(buffer[player]), "User %d", player + 1);

      START_SUB_GROUP(
            list,
            list_info,
            buffer[player],
            group_info.name,
            subgroup_info);

      for (i = 0; i < RARCH_BIND_LIST_END; i ++)
      {
         char label[PATH_MAX];
         char name[PATH_MAX];
         const struct input_bind_map* keybind = 
            (const struct input_bind_map*)&input_config_bind_map[i];

         if (!keybind || keybind->meta)
            continue;

         snprintf(label, sizeof(label), "%s %s", buffer[player], keybind->desc);
         snprintf(name, sizeof(name), "p%u_%s", player + 1, keybind->base);

         CONFIG_BIND(
               g_settings.input.binds[player][i],
               player + 1,
               player,
               strdup(name), /* TODO: Find a way to fix these memleaks. */
               strdup(label),
               &defaults[i],
               group_info.name,
               subgroup_info.name);
         settings_list_current_add_bind_type(list, list_info, i + MENU_SETTINGS_BIND_BEGIN);
      }
      END_SUB_GROUP(list, list_info);
   }
  /* START_SUB_GROUP(
         list,
         list_info,
         "Onscreen Keyboard",
         group_info.name,
         subgroup_info);

   CONFIG_BOOL(
         g_settings.osk.enable,
         "osk_enable",
         "Onscreen Keyboard Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   END_SUB_GROUP(list, list_info);

   START_SUB_GROUP(
         list,
         list_info,
         "Miscellaneous",
         group_info.name,
         subgroup_info);

   CONFIG_BOOL(
         g_settings.input.netplay_client_swap_input,
         "netplay_client_swap_input",
         "Swap Netplay Input",
         netplay_client_swap_input,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   END_SUB_GROUP(list, list_info);*/
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_overlay_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
#ifdef HAVE_OVERLAY
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   /* Overlay Options is now Overlays */
   START_GROUP(group_info, "Sobreponer");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_PATH(
         g_settings.input.overlay,
         "input_overlay",
         "Sobreponer imagen",
         g_extern.overlay_dir,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_values(list, list_info, "cfg");
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_OVERLAY_INIT);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

   CONFIG_FLOAT(
         g_settings.input.overlay_opacity,
         "input_overlay_opacity",
         "Sobreponer opacidad",
         0.7f,
         "%.2f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_OVERLAY_SET_ALPHA_MOD);
   settings_list_current_add_range(list, list_info, 0, 1, 0.01, true, true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   CONFIG_FLOAT(
         g_settings.input.overlay_scale,
         "input_overlay_scale",
         "Sobreponer escala",
         1.0f,
         "%.2f",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_OVERLAY_SET_SCALE_FACTOR);
   settings_list_current_add_range(list, list_info, 0, 2, 0.01, true, true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);
#endif

   return true;
}

static bool setting_data_append_list_menu_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
#ifdef HAVE_MENU
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Menu Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_settings.menu_show_start_screen,
         "rgui_show_start_screen",
         "Show Start Screen",
         menu_show_start_screen,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.menu.pause_libretro,
         "menu_pause_libretro",
         "Pause Libretro",
         true,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_MENU_PAUSE_LIBRETRO);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_CMD_APPLY_AUTO);
   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/
#endif

   return true;
}

static bool setting_data_append_list_netplay_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{/*
#ifdef HAVE_NETPLAY
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Netplay Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_extern.netplay_enable,
         "netplay_enable",
         "Netplay Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_STRING(
         g_extern.netplay_server,
         "netplay_ip_address",
         "IP Address",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_INPUT);

   CONFIG_BOOL(
         g_extern.netplay_is_client,
         "netplay_mode",
         "Netplay Client Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_extern.netplay_is_spectate,
         "netplay_spectator_mode_enable",
         "Netplay Spectator Enable",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   
   CONFIG_UINT(
         g_extern.netplay_sync_frames,
         "netplay_delay_frames",
         "Netplay Delay Frames",
         0,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 10, 1, true, false);

   CONFIG_UINT(
         g_extern.netplay_port,
         "netplay_tcp_udp_port",
         "Netplay TCP/UDP Port",
         RARCH_DEFAULT_PORT,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 1, 99999, 1, true, true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_INPUT);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);
#endif
*/
   return true;
}

static bool setting_data_append_list_playlist_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Playlist Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "History", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_settings.history_list_enable,
         "history_list_enable",
         "History List Enable",
         true,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_UINT(
         g_settings.content_history_size,
         "game_history_size",
         "History List Size",
         default_content_history_size,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(list, list_info, 0, 0, 1.0, true, false);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/

   return true;
}

static bool setting_data_append_list_user_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "User Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_STRING(
         g_settings.username,
         "netplay_nickname",
         "Username",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_INPUT);

   CONFIG_UINT(
         g_settings.user_language,
         "user_language",
         "Language",
         def_user_language,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_range(
         list,
         list_info,
         0,
         RETRO_LANGUAGE_LAST-1,
         1,
         true,
         true);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_INPUT);

   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/

   return true;
}

static bool setting_data_append_list_path_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   /* Path Options is now Paths */
   START_GROUP(group_info, "Paths");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);


   END_SUB_GROUP(list, list_info);
   START_SUB_GROUP(list, list_info, "Paths", group_info.name, subgroup_info);
#ifdef HAVE_MENU
   CONFIG_DIR(
         g_settings.menu_content_directory,
         "rgui_browser_directory",
         "Buscador",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

/*   CONFIG_DIR(
         g_settings.content_directory,
         "content_directory",
         "Content Directory",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);
*/
  /* CONFIG_DIR(
         g_settings.assets_directory,
         "assets_directory",
         "Assets Directory",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);
*/

   CONFIG_DIR(
         g_settings.libretro_directory,
         "libretro_dir_path",
         "Sistema",
         g_defaults.core_dir,
         "<None>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_CORE_INFO_INIT);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

   CONFIG_DIR(
         g_settings.menu_config_directory,
         "rgui_config_directory",
         "Configuraciones",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

#endif

  /* CONFIG_DIR(
         g_settings.libretro_info_path,
         "libretro_info_path",
         "Core Info Directory",
         g_defaults.core_info_dir,
         "<None>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_list_current_add_cmd(list, list_info, RARCH_CMD_CORE_INFO_INIT);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);*/

   CONFIG_PATH(
         g_settings.core_options_path,
         "core_options_path",
         "Opciones del sistema",
         "",
         "Paths",
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

  /* CONFIG_PATH(
         g_settings.cheat_database,
         "cheat_database_path",
         "Cheat Database",
         "",
         "Paths",
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);

   CONFIG_PATH(
         g_settings.cheat_settings_path,
         "cheat_settings_path",
         "Cheat Settings",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler, general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);*/
/*
   CONFIG_PATH(
         g_settings.content_history_path,
         "game_history_path",
         "Content History Path",
         "",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_ALLOW_EMPTY);
*/
   CONFIG_DIR(
         g_settings.video.filter_dir,
         "video_filter_dir",
         "Filtros de video",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

   CONFIG_DIR(
         g_settings.audio.filter_dir,
         "audio_filter_dir",
         "Filtros de sonido",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

#if defined(HAVE_DYLIB) && defined(HAVE_SHADER_MANAGER)
   CONFIG_DIR(
         g_settings.video.shader_dir,
         "video_shader_dir",
         "Shader Directory",
         g_defaults.shader_dir,
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);
#endif

#ifdef HAVE_OVERLAY
   CONFIG_DIR(
         g_extern.overlay_dir,
         "overlay_directory",
         "Data de sobreponer",
         g_defaults.overlay_dir,
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);
#endif

  /* CONFIG_DIR(
         g_settings.resampler_directory,
         "resampler_directory",
         "Resampler Directory",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);*/

   CONFIG_DIR(
         g_settings.screenshot_directory,
         "screenshot_directory",
         "Fotos",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);

 /*  CONFIG_DIR(
         g_settings.input.autoconfig_dir,
         "joypad_autoconfig_dir",
         "Joypad Autoconfig Directory",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);*/

  /* CONFIG_DIR(
         g_settings.playlist_directory,
         "playlist_directory",
         "Playlist Directory",
         "",
         "<default>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   settings_data_list_current_add_flags(
         list,
         list_info,
         SD_FLAG_ALLOW_EMPTY | SD_FLAG_PATH_DIR);*/

   CONFIG_DIR(
         g_extern.savefile_dir,
         "savefile_directory",
         "Data de partidas",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_DIR(
         g_extern.savestate_dir,
         "savestate_directory",
         "Data de estados",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_DIR(
         g_settings.system_directory,
         "system_directory",
         "Data del sistema",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_DIR(
         g_settings.extraction_directory,
         "extraction_directory",
         "Data temporera",
         "",
         "<Dir del juego>",
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);

   return true;
}

static bool setting_data_append_list_privacy_options(
      rarch_setting_t **list,
      rarch_setting_info_t *list_info)
{
   /*rarch_setting_group_info_t group_info;
   rarch_setting_group_info_t subgroup_info;

   START_GROUP(group_info, "Privacy Options");
   settings_data_list_current_add_flags(list, list_info, SD_FLAG_IS_CATEGORY);
   START_SUB_GROUP(list, list_info, "State", group_info.name, subgroup_info);

   CONFIG_BOOL(
         g_settings.camera.allow,
         "camera_allow",
         "Allow Camera",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);

   CONFIG_BOOL(
         g_settings.location.allow,
         "location_allow",
         "Allow Location",
         false,
         "No",
         sip,
         group_info.name,
         subgroup_info.name,
         general_write_handler,
         general_read_handler);
   END_SUB_GROUP(list, list_info);
   END_GROUP(list, list_info);*/

   return true;
}


rarch_setting_t *setting_data_new(unsigned mask)
{
   rarch_setting_t terminator = { ST_NONE };
   rarch_setting_t* list = NULL;
   rarch_setting_info_t *list_info = (rarch_setting_info_t*)
      settings_info_list_new();
   if (!list_info)
      return NULL;

   list = (rarch_setting_t*)settings_list_new(list_info->size);
   if (!list)
      goto error;

   if (mask & SL_FLAG_MAIN_MENU)
   {
      if (!setting_data_append_list_main_menu_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_DRIVER_OPTIONS)
   {
      if (!setting_data_append_list_driver_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_GENERAL_OPTIONS)
   {
      if (!setting_data_append_list_general_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_GENERAL_OPTIONS)
   {
      if (!setting_data_append_list_video_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_SHADER_OPTIONS)
   {
      if (!setting_data_append_list_shader_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_FONT_OPTIONS)
   {
      if (!setting_data_append_list_font_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_AUDIO_OPTIONS)
   {
      if (!setting_data_append_list_audio_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_INPUT_OPTIONS)
   {
      if (!setting_data_append_list_input_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_OVERLAY_OPTIONS)
   {
      if (!setting_data_append_list_overlay_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_MENU_OPTIONS)
   {
      if (!setting_data_append_list_menu_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_PLAYLIST_OPTIONS)
   {
      if (!setting_data_append_list_playlist_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_NETPLAY_OPTIONS)
   {
      if (!setting_data_append_list_netplay_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_USER_OPTIONS)
   {
      if (!setting_data_append_list_user_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_PATH_OPTIONS)
   {
      if (!setting_data_append_list_path_options(&list, list_info))
         goto error;
   }

   if (mask & SL_FLAG_PRIVACY_OPTIONS)
   {
      if (!setting_data_append_list_privacy_options(&list, list_info))
         goto error;
   }

   if (!(settings_list_append(&list, list_info, terminator)))
      goto error;

   /* flatten this array to save ourselves some kilobytes. */
   if (!(list = (rarch_setting_t*)
            realloc(list, list_info->index * sizeof(rarch_setting_t))))
      goto error;

   settings_info_list_free(list_info);

   return list;

error:
   RARCH_ERR("Allocation failed.\n");
   settings_info_list_free(list_info);
   settings_list_free(list);

   return NULL;
}
