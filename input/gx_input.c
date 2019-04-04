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
#define MAX_PADS 4
#endif

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
} gx_input_t;

static int16_t gx_mouse_state(gx_input_t *gx, unsigned id)
{
   switch (id)
   {
      case RETRO_DEVICE_ID_MOUSE_X:
         return gx->ml_x - gx->ml_lastx;
      case RETRO_DEVICE_ID_MOUSE_Y:
         return gx->ml_y - gx->ml_lasty;
      case RETRO_DEVICE_ID_MOUSE_LEFT:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_MOUSE_LEFT);
      case RETRO_DEVICE_ID_MOUSE_RIGHT:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
      default:
         return 0;
   }
}

static int16_t gx_lightgun_state(gx_input_t *gx, unsigned id)
{
   switch (id)
   {
      case RETRO_DEVICE_ID_LIGHTGUN_X:
         return gx->ml_x - gx->ml_lastx;
      case RETRO_DEVICE_ID_LIGHTGUN_Y:
         return gx->ml_y - gx->ml_lasty;
      case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_LIGHTGUN_TRIGGER);
      case RETRO_DEVICE_ID_LIGHTGUN_CURSOR:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_LIGHTGUN_CURSOR);
      case RETRO_DEVICE_ID_LIGHTGUN_TURBO:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_LIGHTGUN_TURBO);
      case RETRO_DEVICE_ID_LIGHTGUN_PAUSE:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_LIGHTGUN_PAUSE);
      case RETRO_DEVICE_ID_LIGHTGUN_START:
         return gx->ml_state & (1 << RETRO_DEVICE_ID_LIGHTGUN_START);
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
         return gx_mouse_state(gx, id);
	  case RETRO_DEVICE_LIGHTGUN:
         return gx_lightgun_state(gx, id);
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

   free(gx);
}

static void *gx_input_init(void)
{
   gx_input_t *gx = (gx_input_t*)calloc(1, sizeof(*gx));
   if (!gx)
      return NULL;

   gx->joypad = input_joypad_init_driver(g_settings.input.joypad_driver);

   return gx;
}

static void gx_input_poll_mouse(gx_input_t *gx)
{
   gx->ml_lastx = gx->ml_x;
   gx->ml_lasty = gx->ml_y;
   gx->ml_x = gxpad_mlposx();
   gx->ml_y = gxpad_mlposy();
   gx->ml_state = gxpad_mlbuttons();
}

static void gx_input_poll(void *data)
{
   gx_input_t *gx = (gx_input_t*)data;

   if (gx && gx->joypad)
      gx->joypad->poll();

	/* poll mouse data */
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

   return (1 << RETRO_DEVICE_JOYPAD) |  (1 << RETRO_DEVICE_ANALOG);
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
