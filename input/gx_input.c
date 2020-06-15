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

#include <stdint.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#include "../driver.h"
#include "../libretro.h"
#include <stdlib.h>

#ifndef MAX_PADS
#ifdef HAVE_5PLAY
#define MAX_PADS 5
#elif HAVE_6PLAY
#define MAX_PADS 6
#elif HAVE_8PLAY
#define MAX_PADS 8
#else
#define MAX_PADS 4
#endif
#endif

#define video_driver_translate_coord_viewport_wrap(vp, mouse_x, mouse_y, res_x, res_y, res_screen_x, res_screen_y) \
   (video_driver_get_viewport_info(vp) ? video_driver_translate_coord_viewport(vp, mouse_x, mouse_y, res_x, res_y, res_screen_x, res_screen_y) : false)

/* gx joypad functions */
bool gxpad_mousevalid(unsigned port);
void gx_joypad_read_mouse(unsigned port, int *irx, int *iry, uint32_t *button);

typedef struct
{
   int x_abs, y_abs;
   int x_last, y_last;
   uint32_t button;
} gx_input_mouse_t;

uint8_t gxpad_mlbuttons(void);
int32_t gxpad_mlposx(void);
int32_t gxpad_mlposy(void);

typedef struct gx_input
{
   const rarch_joypad_driver_t *joypad;
   uint8_t ml_state;
   int mouse_x, mouse_y;
   int ml_x, ml_y;
   int mouse_last_x, mouse_last_y;
   int ml_lastx, ml_lasty;
   int mouse_max;
   gx_input_mouse_t *mouse;
} gx_input_t;

static int16_t gx_lightgun_state(gx_input_t *gx, unsigned id, uint16_t idx)
{
   struct rarch_viewport vp = {0};
   video_driver_get_viewport_info(&vp);
   int16_t res_x               = 0;
   int16_t res_y               = 0;
   int16_t res_screen_x        = 0;
   int16_t res_screen_y        = 0;
   int16_t x = 0;
   int16_t y = 0;

   vp.x                        = 0;
   vp.y                        = 0;
   vp.width                    = 0;
   vp.height                   = 0;
   vp.full_width               = 0;
   vp.full_height              = 0;

   x = gx->mouse[idx].x_abs;
   y = gx->mouse[idx].y_abs;

   if (!(video_driver_translate_coord_viewport_wrap(&vp, x, y,
         &res_x, &res_y, &res_screen_x, &res_screen_y)))
      return 0;

   switch (id)
   {
      case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X:
         return res_screen_x;
      case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y:
         return res_screen_y;
      case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_TRIGGER);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_A:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_B:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
      case RETRO_DEVICE_ID_LIGHTGUN_AUX_C:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_AUX_C);
      case RETRO_DEVICE_ID_LIGHTGUN_START:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_START);
      case RETRO_DEVICE_ID_LIGHTGUN_SELECT:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_LIGHTGUN_SELECT);
      case RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
         return !gxpad_mousevalid(idx);
      default:
         return 0;
   }

   return 0;
}

static int16_t gx_mouse_state(gx_input_t *gx, unsigned id, uint16_t idx)
{
   int x = 0;
   int y = 0;

   int x_scale = 1;
   int y_scale = 1;

   x = (gx->mouse[idx].x_abs - gx->mouse[idx].x_last) * x_scale;
   y = (gx->mouse[idx].y_abs - gx->mouse[idx].y_last) * y_scale;

   switch (id)
   {
      case RETRO_DEVICE_ID_MOUSE_X:
         return x;
      case RETRO_DEVICE_ID_MOUSE_Y:
         return y;
      case RETRO_DEVICE_ID_MOUSE_LEFT:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_MOUSE_LEFT);
      case RETRO_DEVICE_ID_MOUSE_RIGHT:
         return gx->mouse[idx].button & (1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
      default:
         return 0;
   }
}

static int16_t gx_input_state(void *data, const struct retro_keybind **binds,
      unsigned port, unsigned device,
      unsigned idx, unsigned id)
{
   gx_input_t *gx = (gx_input_t*)data;
   if (port >= MAX_PADS || !gx)
      return 0;

   switch (device)
   {
      case RETRO_DEVICE_JOYPAD:
         return input_joypad_pressed(gx->joypad, port, binds[port], id);;
      case RETRO_DEVICE_ANALOG:
         return input_joypad_analog(gx->joypad, port, idx, id, binds[port]);
      case RETRO_DEVICE_MOUSE:
         return gx_mouse_state(gx, id, idx);

      case RETRO_DEVICE_LIGHTGUN:
         return gx_lightgun_state(gx, id, idx);
   }

   return 0;
}

static void gx_input_free_input(void *data)
{
   gx_input_t *gx = (gx_input_t*)data;

   if (!gx)
      return;

   if (gx->joypad)
      gx->joypad->destroy();
  
   if(gx->mouse)
      free(gx->mouse);

   free(gx);
}

static inline int gx_count_mouse(gx_input_t *gx)
{
   int count = 0;

   if(gx)
   {
      for(int i=0; i<4; i++)
      {
        // if(gx->joypad->name(i))
        // {
           // if(!strcmp(gx->joypad->name(i), "Port #1"))
           // {
               count++;
           // }
        // }
      }
	 // count=1;
   }

   return count;
}

static void *gx_input_init(void)
{
   gx_input_t *gx = (gx_input_t*)calloc(1, sizeof(*gx));
   if (!gx)
      return NULL;

   gx->joypad = input_joypad_init_driver(g_settings.input.joypad_driver);

   /* Allocate at least 1 mouse at startup */
   gx->mouse_max = 1;
   gx->mouse = (gx_input_mouse_t*) calloc(gx->mouse_max, sizeof(gx_input_mouse_t));

   return gx;
}

static void gx_input_poll_mouse(gx_input_t *gx)
{
   int count = 0;
   count = gx_count_mouse(gx);

   if(gx && count > 0)
   {
      if(count != gx->mouse_max)
      {
         gx_input_mouse_t* tmp = NULL;

         tmp = (gx_input_mouse_t*)realloc(gx->mouse, count * sizeof(gx_input_mouse_t));
         if(!tmp) 
         {
            free(gx->mouse);
         }
         else
         {
            gx->mouse = tmp;
            gx->mouse_max = count;

            for(int i=0; i<gx->mouse_max; i++)
            {
               gx->mouse[i].x_last = 0;
               gx->mouse[i].y_last = 0;
            }
         }
      }

      for(unsigned i=0; i<gx->mouse_max; i++)
      {
         gx->mouse[i].x_last = gx->mouse[i].x_abs;
         gx->mouse[i].y_last = gx->mouse[i].y_abs;
         gx_joypad_read_mouse(i, &gx->mouse[i].x_abs, &gx->mouse[i].y_abs, &gx->mouse[i].button);
      } 
   }
}

static void gx_input_poll(void *data)
{
   gx_input_t *gx = (gx_input_t*)data;

   if (gx && gx->joypad)
      gx->joypad->poll();

	/* poll mouse data */
   if(gx->mouse)
      gx_input_poll_mouse(gx);
}

static bool gx_input_key_pressed(void *data, int key)
{
   gx_input_t *gx = (gx_input_t*)data;
   return (g_extern.lifecycle_state & (1ULL << key)) || 
      input_joypad_pressed(gx->joypad, 0, g_settings.input.binds[0], key);
}

static uint64_t gx_input_get_capabilities(void *data)
{
   (void)data;

   return (1 << RETRO_DEVICE_JOYPAD) |
          (1 << RETRO_DEVICE_ANALOG) |
          (1 << RETRO_DEVICE_MOUSE) |
          (1 << RETRO_DEVICE_LIGHTGUN);
}

static const rarch_joypad_driver_t *gx_input_get_joypad_driver(void *data)
{
   gx_input_t *gx = (gx_input_t*)data;
   if (gx)
      return gx->joypad;
   return NULL;
}

input_driver_t input_gx = {
   gx_input_init,
   gx_input_poll,
   gx_input_state,
   gx_input_key_pressed,
   gx_input_free_input,
   NULL,
   NULL,
   gx_input_get_capabilities,
   "gx",

   NULL,
   NULL,
   gx_input_get_joypad_driver,
};
