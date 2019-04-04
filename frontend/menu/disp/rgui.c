/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2014 - Daniel De Matteis
 *  Copyright (C) 2012-2014 - Michael Lelli
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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "../menu_driver.h"
#include "../menu_list.h"
#include "../menu_common.h"
#include "../../../general.h"
#include "../../../config.def.h"
#include "../../../dynamic.h"
#include <compat/posix_string.h>
#include <file/file_path.h>

#include "../../../settings_data.h"
#include "../../../gfx/fonts/bitmap.h"

#include "shared.h"

typedef struct rgui_handle
{
   uint16_t *menu_framebuf;
} rgui_handle_t;

#define RGUI_TERM_START_X (driver.menu->width / 21)
#define RGUI_TERM_START_Y (driver.menu->height / 9)
#define RGUI_TERM_WIDTH (((driver.menu->width - RGUI_TERM_START_X - RGUI_TERM_START_X) / (FONT_WIDTH_STRIDE)))
#define RGUI_TERM_HEIGHT (((driver.menu->height - RGUI_TERM_START_Y - RGUI_TERM_START_X) / (FONT_HEIGHT_STRIDE)) - 1)

static void rgui_copy_glyph(uint8_t *glyph, const uint8_t *buf)
{
   int y, x;
   for (y = 0; y < FONT_HEIGHT; y++)
   {
      for (x = 0; x < FONT_WIDTH; x++)
      {
         uint32_t col =
            ((uint32_t)buf[3 * (-y * 256 + x) + 0] << 0) |
            ((uint32_t)buf[3 * (-y * 256 + x) + 1] << 8) |
            ((uint32_t)buf[3 * (-y * 256 + x) + 2] << 16);

         uint8_t rem = 1 << ((x + y * FONT_WIDTH) & 7);
         unsigned offset = (x + y * FONT_WIDTH) >> 3;

         if (col != 0xff)
            glyph[offset] |= rem;
      }
   }
}

static uint16_t gray_filler(unsigned x, unsigned y)
{
   x >>= 1;
   y >>= 1;
   unsigned col = ((x + y) & 1) + 1;
#if defined(GEKKO) || defined(PSP)
   return (6 << 12) | (col << 8) | (col << 4) | (col << 0);
#else
   return (col << 13) | (col << 9) | (col << 5) | (12 << 0);
#endif
}

static uint16_t green_filler(unsigned x, unsigned y)
{
   x >>= 1;
   y >>= 1;
   unsigned col = ((x + y) & 1) + 1;
#if defined(GEKKO) || defined(PSP)
   return (6 << 12) | (col << 8) | (col << 4) | (col << 1);
#else
   return (col << 13) | (col << 10) | (col << 5) | (12 << 0);
#endif
}

static uint16_t custom_filler(unsigned x, unsigned y)
{
   return g_settings.menu_bg_clr;

}

static uint16_t custom_msg_filler(unsigned x, unsigned y)
{
   return g_settings.menu_msg_clr;
}

static void fill_rect(uint16_t *buf, unsigned pitch,
      unsigned x, unsigned y,
      unsigned width, unsigned height,
      uint16_t (*col)(unsigned x, unsigned y))
{
   unsigned j, i;
   for (j = y; j < y + height; j++)
      for (i = x; i < x + width; i++)
         buf[j * (pitch >> 1) + i] = col(i, j);
}

static void blit_line(int x, int y, const char *message, bool green)
{
   int j, i;
   unsigned hov_col = 32767;
   unsigned tex_col = 54965;

   if (!driver.menu)
      return;

   if (g_settings.theme_preset == 0) {
	  /* Type 0 is default, custom values */
      hov_col = g_settings.hover_color;
      tex_col = g_settings.text_color;
   } else if (g_settings.theme_preset == 1) {
	   /* Type 1 is green/white */
	   hov_col = 46060;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 2) {
	   /* Type 2 is mute red/white */
	   hov_col = 60647;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 3) {
       /* Type 3 is yellow/white */
	   hov_col = 64450;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 4) {
       /* Type 4 is pink/white */
	   hov_col = 64927;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 5) {
	   /* Type 5 is white/gray */
	   hov_col = 32767;
	   tex_col = 54965;
   } else if (g_settings.theme_preset == 6) {
	   /* Type 6 is cyan/gray */
	   hov_col = 46046;
	   tex_col = 35351;
   } else if (g_settings.theme_preset == 7) {
	   /* Type 7 is purple/white */
	   hov_col = 56383;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 8) {
	   /* Type 8 is red/gold */
	   hov_col = 64512;
	   tex_col = 60071;
   }

   while (*message)
   {
      for (j = 0; j < FONT_HEIGHT; j++)
      {
         for (i = 0; i < FONT_WIDTH; i++)
         {
            uint8_t rem = 1 << ((i + j * FONT_WIDTH) & 7);
            int offset = (i + j * FONT_WIDTH) >> 3;
            bool col = (driver.menu->font[FONT_OFFSET
                  ((unsigned char)*message) + offset] & rem);

            if (col)
            {
               driver.menu->frame_buf[(y + j) *
                  (driver.menu->frame_buf_pitch >> 1) + (x + i)] = green ?
#if defined(GEKKO)|| defined(PSP)
               hov_col : tex_col;
#else
               (15 << 0) | (7 << 4) | (15 << 8) | (7 << 12) : 0xFFFF;
#endif
            }
         }
      }

      x += FONT_WIDTH_STRIDE;
      message++;
   }
}

static bool init_font(menu_handle_t *menu, const uint8_t *font_bmp_buf)
{
   unsigned i;
   uint8_t *font = (uint8_t *) calloc(1, FONT_OFFSET(256));

   if (!font)
   {
      RARCH_ERR("Font memory allocation failed.\n");
      return false;
   }

   menu->alloc_font = true;
   for (i = 0; i < 256; i++)
   {
      unsigned y = i / 16;
      unsigned x = i % 16;
      rgui_copy_glyph(&font[FONT_OFFSET(i)],
            font_bmp_buf + 54 + 3 * (256 * (255 - 16 * y) + 16 * x));
   }

   menu->font = font;
   return true;
}

static bool rguidisp_init_font(void *data)
{
   menu_handle_t *menu = (menu_handle_t*)data;

   const uint8_t *font_bmp_buf = NULL;
   const uint8_t *font_bin_buf = bitmap_bin;

   if (font_bmp_buf)
      return init_font(menu, font_bmp_buf);
   else if (font_bin_buf)
      menu->font = font_bin_buf;
   else
      return false;

   return true;
}

static void rgui_render_background(void)
{
   if (!driver.menu)
      return;

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         0, 0, driver.menu->width, driver.menu->height, g_settings.menu_solid ? custom_filler : gray_filler);

  /* fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, 5, driver.menu->width - 10, 5, green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, driver.menu->height - 10, driver.menu->width - 10, 5,
         green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, 5, 5, driver.menu->height - 10, green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         driver.menu->width - 10, 5, 5, driver.menu->height - 10,
         green_filler);*/
}

static void rgui_render_messagebox(const char *message)
{
   size_t i;

   if (!driver.menu || !message || !*message)
      return;

   struct string_list *list = string_split(message, "\n");
   if (!list)
      return;
   if (list->elems == 0)
   {
      string_list_free(list);
      return;
   }

   unsigned width = 0;
   unsigned glyphs_width = 0;
   for (i = 0; i < list->size; i++)
   {
      char *msg = list->elems[i].data;
      unsigned msglen = strlen(msg);
      if (msglen > RGUI_TERM_WIDTH)
      {
         msg[RGUI_TERM_WIDTH - 2] = '.';
         msg[RGUI_TERM_WIDTH - 1] = '.';
         msg[RGUI_TERM_WIDTH - 0] = '.';
         msg[RGUI_TERM_WIDTH + 1] = '\0';
         msglen = RGUI_TERM_WIDTH;
      }

      unsigned line_width = msglen * FONT_WIDTH_STRIDE - 1 + 6 + 10;
      width = max(width, line_width);
      glyphs_width = max(glyphs_width, msglen);
   }

   unsigned height = FONT_HEIGHT_STRIDE * list->size + 6 + 10;
   int x = (driver.menu->width - width) / 2;
   int y = (driver.menu->height - height) / 2;

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + 5, y + 5, width - 10, height - 10, g_settings.menu_solid ? custom_filler : gray_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x, y, width - 5, 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + width - 5, y, 5, height - 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + 5, y + height - 5, width - 5, 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x, y + 5, 5, height - 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   for (i = 0; i < list->size; i++)
   {
      const char *msg = list->elems[i].data;
      int offset_x = FONT_WIDTH_STRIDE * (glyphs_width - strlen(msg)) / 2;
      int offset_y = FONT_HEIGHT_STRIDE * i;
      blit_line(x + 8 + offset_x, y + 8 + offset_y, msg, false);
   }

   string_list_free(list);
}

static void rgui_render(void)
{
   size_t begin = 0;
   size_t end;

   if (driver.menu->need_refresh 
         && g_extern.is_menu
         && !driver.menu->msg_force)
      return;

   if (driver.menu->selection_ptr >= RGUI_TERM_HEIGHT / 2)
      begin = driver.menu->selection_ptr - RGUI_TERM_HEIGHT / 2;
   end   = (driver.menu->selection_ptr + RGUI_TERM_HEIGHT <=
         menu_list_get_size(driver.menu->menu_list)) ?
      driver.menu->selection_ptr + RGUI_TERM_HEIGHT :
      menu_list_get_size(driver.menu->menu_list);

   /* Do not scroll if all items are visible. */
   if (menu_list_get_size(driver.menu->menu_list) <= RGUI_TERM_HEIGHT)
      begin = 0;

   if (end - begin > RGUI_TERM_HEIGHT)
      end = begin + RGUI_TERM_HEIGHT;

   rgui_render_background();

   char title[256];
   const char *dir = NULL;
   const char *label = NULL;
   unsigned menu_type = 0;
   menu_list_get_last_stack(driver.menu->menu_list,
         &dir, &label, &menu_type);

#if 0
   RARCH_LOG("Dir is: %s\n", label);
#endif

   get_title(label, dir, menu_type, title, sizeof(title));

   char title_buf[256];
   menu_ticker_line(title_buf, RGUI_TERM_WIDTH - 3,
         g_extern.frame_count / RGUI_TERM_START_X, title, true);
   // Current TITLE
   blit_line(g_settings.title_posx, g_settings.title_posy, title_buf, true);

   char title_msg[64];
   const char *core_name = g_extern.menu.info.library_name;
   if (!core_name)
      core_name = g_extern.system.info.library_name;
   if (!core_name)
      core_name = "No Core";

   const char *core_version = g_extern.menu.info.library_version;
   if (!core_version)
      core_version = g_extern.system.info.library_version;
   if (!core_version)
      core_version = "";

   if (!g_settings.single_mode) {
         //snprintf(title_msg, sizeof(title_msg), "%s - %s %s", PACKAGE_VERSION,
           //  core_name, core_version);
         snprintf(title_msg, sizeof(title_msg), "%s %s",
                  core_name, core_version);
         blit_line(
             RGUI_TERM_START_X + RGUI_TERM_START_X,
             (RGUI_TERM_HEIGHT * FONT_HEIGHT_STRIDE) +
             RGUI_TERM_START_Y + 2, title_msg, true);
	}

    if (g_settings.clock_show) {
        char timetxt[10];
        time_t currenttime = time(0);
        struct tm * timeinfo = localtime(&currenttime);

     // strftime(timetxt, sizeof(timetxt),
     //          "%H:%M:%S", timeinfo);
        strftime(timetxt, sizeof(timetxt),
                    "%I:%M %p", timeinfo);
        blit_line(
             RGUI_TERM_START_X + RGUI_TERM_START_X + 180,
             (RGUI_TERM_HEIGHT * FONT_HEIGHT_STRIDE) +
             RGUI_TERM_START_Y + 2, timetxt, true);
    }

   unsigned x, y;
   size_t i;

   x = RGUI_TERM_START_X;
   y = RGUI_TERM_START_Y;

   for (i = begin; i < end; i++, y += FONT_HEIGHT_STRIDE)
   {
      char message[PATH_MAX], type_str[PATH_MAX],
           entry_title_buf[PATH_MAX], type_str_buf[PATH_MAX],
           path_buf[PATH_MAX];
      const char *path = NULL, *entry_label = NULL;
      unsigned type = 0, w = 0;
      bool selected = false;

      menu_list_get_at_offset(driver.menu->menu_list->selection_buf, i, &path,
            &entry_label, &type);
      rarch_setting_t *setting = (rarch_setting_t*)setting_data_find_setting(
            driver.menu->list_settings,
            driver.menu->menu_list->selection_buf->list[i].label);
      (void)setting;

      disp_set_label(driver.menu->menu_list->selection_buf, &w, type, i, label,
            type_str, sizeof(type_str), 
            entry_label, path,
            path_buf, sizeof(path_buf));

      selected = (i == driver.menu->selection_ptr);

      menu_ticker_line(entry_title_buf, RGUI_TERM_WIDTH - (w + 1 + 2),
            g_extern.frame_count / RGUI_TERM_START_X, path_buf, selected);
      menu_ticker_line(type_str_buf, w, g_extern.frame_count / RGUI_TERM_START_X,
            type_str, selected);

      snprintf(message, sizeof(message), "%c %-*.*s %-*s",
            selected ? '>' : ' ',
            RGUI_TERM_WIDTH - (w + 1 + 2),
            RGUI_TERM_WIDTH - (w + 1 + 2),
            entry_title_buf,
            w,
            type_str_buf);

      blit_line(x + g_settings.item_posx, y + g_settings.item_posy, message, selected);
   }

#ifdef GEKKO
   const char *message_queue;

   if (driver.menu->msg_force)
   {
      message_queue = msg_queue_pull(g_extern.msg_queue);
      driver.menu->msg_force = false;
   }
   else
      message_queue = driver.current_msg;

   rgui_render_messagebox(message_queue);
#endif
/*
   if (driver.menu->keyboard.display)
   {
      char msg[PATH_MAX];
      const char *str = *driver.menu->keyboard.buffer;
      if (!str)
         str = "";
      snprintf(msg, sizeof(msg), "%s\n%s", driver.menu->keyboard.label, str);
      rgui_render_messagebox(msg);
   }*/
}

static void *rgui_init(void)
{
   menu_handle_t *menu = (menu_handle_t*)calloc(1, sizeof(*menu));

   if (!menu)
      return NULL;

   menu->userdata = (rgui_handle_t*)calloc(1, sizeof(rgui_handle_t));
   
   if (!menu->userdata)
   {
      free(menu);
      return NULL;
   }

   menu->frame_buf = (uint16_t*)malloc(400 * 240 * sizeof(uint16_t)); 

   if (!menu->frame_buf)
   {
      free(menu->userdata);
      free(menu);
      return NULL;
   }

   menu->width = 320;
   menu->height = 240;
   menu->frame_buf_pitch = menu->width * sizeof(uint16_t);

   bool ret = rguidisp_init_font(menu);

   if (!ret)
   {
      RARCH_ERR("No font bitmap or binary, abort");

      rarch_main_command(RARCH_CMD_QUIT_RETROARCH);

      free(menu->userdata);
      free(menu);
      return NULL;
   }

   return menu;
}

static void rgui_free(void *data)
{
   rgui_handle_t *rgui = NULL;
   menu_handle_t *menu = (menu_handle_t*)data;

   if (!menu)
      return;

   rgui = (rgui_handle_t*)menu->userdata;

   if (!rgui)
      return;

   if (menu->frame_buf)
      free(menu->frame_buf);

   if (menu->userdata)
      free(menu->userdata);
   driver.menu->userdata = NULL;

   if (menu->alloc_font)
      free((uint8_t*)menu->font);
}

static void rgui_set_texture(void *data)
{
   menu_handle_t *menu = (menu_handle_t*)data;

   if (driver.video_data && driver.video_poke &&
         driver.video_poke->set_texture_frame)
      driver.video_poke->set_texture_frame(driver.video_data,
            menu->frame_buf, false, menu->width, menu->height, 1.0f);
}

menu_ctx_driver_t menu_ctx_rgui = {
   rgui_set_texture,
   rgui_render_messagebox,
   rgui_render,
   NULL,
   rgui_init,
   NULL,
   rgui_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   &menu_ctx_backend_common,
   "rgui",
};
