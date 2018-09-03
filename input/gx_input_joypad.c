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

#ifdef HW_RVL
#include <gccore.h>
#include <ogc/pad.h>
#include <wiiuse/wpad.h>
#else
#include <cafe/pads/wpad/wpad.h>
#endif

#ifdef GEKKO
#define WPADInit WPAD_Init
#define WPADDisconnect WPAD_Disconnect
#define WPADProbe WPAD_Probe
#endif

#define WPAD_EXP_SICKSAXIS 253
#define WPAD_EXP_GAMECUBE  254

#ifdef HW_RVL
#ifdef HAVE_LIBSICKSAXIS
#define NUM_DEVICES 5
#else
#define NUM_DEVICES 4
#endif
#else
#define NUM_DEVICES 1
#endif

#ifndef MAX_PADS
#define MAX_PADS 4
#endif

enum
{
   GX_GC_A                 = 0,
   GX_GC_B                 = 1,
   GX_GC_X                 = 2,
   GX_GC_Y                 = 3,
   GX_GC_START             = 4,
   GX_GC_Z_TRIGGER         = 5,
   GX_GC_L_TRIGGER         = 6,
   GX_GC_R_TRIGGER         = 7,
   GX_GC_UP                = 8,
   GX_GC_DOWN              = 9,
   GX_GC_LEFT              = 10,
   GX_GC_RIGHT             = 11,
#ifdef HW_RVL
   GX_CLASSIC_A            = 20,
   GX_CLASSIC_B            = 21,
   GX_CLASSIC_X            = 22,
   GX_CLASSIC_Y            = 23,
   GX_CLASSIC_PLUS         = 24,
   GX_CLASSIC_MINUS        = 25,
   GX_CLASSIC_HOME         = 26,
   GX_CLASSIC_L_TRIGGER    = 27,
   GX_CLASSIC_R_TRIGGER    = 28,
   GX_CLASSIC_ZL_TRIGGER   = 29,
   GX_CLASSIC_ZR_TRIGGER   = 30,
   GX_CLASSIC_UP           = 31,
   GX_CLASSIC_DOWN         = 32,
   GX_CLASSIC_LEFT         = 33,
   GX_CLASSIC_RIGHT        = 34,
   GX_WIIMOTE_A            = 43,
   GX_WIIMOTE_B            = 44,
   GX_WIIMOTE_1            = 45,
   GX_WIIMOTE_2            = 46,
   GX_WIIMOTE_PLUS         = 47,
   GX_WIIMOTE_MINUS        = 48,
#if 0
   GX_WIIMOTE_HOME         = 49,
#endif
   GX_WIIMOTE_UP           = 50,
   GX_WIIMOTE_DOWN         = 51,
   GX_WIIMOTE_LEFT         = 52,
   GX_WIIMOTE_RIGHT        = 53,
   GX_NUNCHUK_Z            = 54,
   GX_NUNCHUK_C            = 55,
   GX_NUNCHUK_UP           = 56,
   GX_NUNCHUK_DOWN         = 57,
   GX_NUNCHUK_LEFT         = 58,
   GX_NUNCHUK_RIGHT        = 59,
#endif
   GX_WIIMOTE_HOME         = 49, /* needed on GameCube as "fake" menu button. */
   GX_QUIT_KEY             = 60,
};

#define GC_JOYSTICK_THRESHOLD (48 * 256)
#define WII_JOYSTICK_THRESHOLD (40 * 256)

static uint64_t pad_state[MAX_PADS];
static uint32_t pad_type[MAX_PADS];
static int16_t analog_state[MAX_PADS][2][2];
static bool g_menu;
#ifdef HW_RVL
static bool g_quit;
static bool g_reset;

static void reset_cb(void)
{
   if(g_settings.video.rgui_reset) {
   g_menu = true;
   }
   else {
   g_reset = true;
   }
}

static void power_callback(void)
{
   g_quit = true;
}

#ifdef HAVE_LIBSICKSAXIS
volatile int lol = 0;
struct ss_device dev[MAX_PADS];

int change_cb(int result, void *usrdata)
{
    (*(volatile int*)usrdata)++;
    return result;
}

void removal_cb(void *usrdata)
{
   RARCH_LOG("Device %d disconnected\n", (int)usrdata);
}
#endif

#endif

/*static void reset_cb(void)
{
   g_menu = true;
}*/

static const char *gx_joypad_name(unsigned pad)
{
   switch (pad_type[pad])
   {
#ifdef HW_RVL
      case WPAD_EXP_NONE:
         return "Wiimote Controller";
      case WPAD_EXP_NUNCHUK:
         return "Nunchuk Controller";
      case WPAD_EXP_CLASSIC:
         return "Classic Controller";
#ifdef HAVE_LIBSICKSAXIS
      case WPAD_EXP_SICKSAXIS:
         return "Sixaxis Controller";
#endif
#endif
      case WPAD_EXP_GAMECUBE:
         return "GameCube Controller";
   }

   return NULL;
}

static const char *gx_joypad_name_static(unsigned pad)
{
   switch (pad_type[pad])
   {
#ifdef HW_RVL
      case WPAD_EXP_NONE:
         return "Wiimote Controller";
      case WPAD_EXP_NUNCHUK:
         return "Nunchuk Controller";
      case WPAD_EXP_CLASSIC:
         return "Classic Controller";
#ifdef HAVE_LIBSICKSAXIS
      case WPAD_EXP_SICKSAXIS:
         return "Sixaxis Controller";
#endif
#endif
      case WPAD_EXP_GAMECUBE:
         return "GameCube Controller";
   }

   return NULL;
}

static void handle_hotplug(unsigned port, uint32_t ptype)
{
   pad_type[port] = ptype;

   if (!g_settings.input.autodetect_enable)
      return;

   strlcpy(g_settings.input.device_names[port],
         gx_joypad_name(port),
         sizeof(g_settings.input.device_names[port]));
   /* TODO - implement VID/PID? */
   input_config_autoconfigure_joypad(port,
         gx_joypad_name(port),
         0, 0,
         gx_joypad.ident);
}

static bool gx_joypad_init(void)
{
   int autoconf_pad;

   SYS_SetResetCallback(reset_cb);
#ifdef HW_RVL
   SYS_SetPowerCallback(power_callback);
#endif

   PAD_Init();
#ifdef HW_RVL
   WPADInit();
#endif
#ifdef HAVE_LIBSICKSAXIS
   int i;
   USB_Initialize();
   ss_init();
   for (i = 0; i < MAX_PADS; i++)
      ss_initialize(&dev[i]);
#endif

   for (autoconf_pad = 0; autoconf_pad < MAX_PADS; autoconf_pad++)
   {
      pad_type[autoconf_pad] = WPAD_EXP_GAMECUBE;
      strlcpy(g_settings.input.device_names[autoconf_pad],
            gx_joypad_name_static(autoconf_pad),
            sizeof(g_settings.input.device_names[autoconf_pad]));
      /* TODO - implement VID/PID? */
      input_config_autoconfigure_joypad(autoconf_pad,
            gx_joypad_name_static(autoconf_pad),
            0, 0,
            gx_joypad.ident);
   }

   return true;
}

static bool gx_joypad_button(unsigned port, uint16_t joykey)
{
   if (port >= MAX_PADS)
      return false;
   return pad_state[port] & (1ULL << joykey);
}

static int16_t gx_joypad_axis(unsigned port, uint32_t joyaxis)
{
   int val = 0, axis = -1;
   bool is_neg = false, is_pos = false;
   if (joyaxis == AXIS_NONE || port >= MAX_PADS)
      return 0;

   if (AXIS_NEG_GET(joyaxis) < 4)
   {
      axis = AXIS_NEG_GET(joyaxis);
      is_neg = true;
   }
   else if (AXIS_POS_GET(joyaxis) < 4)
   {
      axis = AXIS_POS_GET(joyaxis);
      is_pos = true;
   }

   switch (axis)
   {
      case 0:
         val = analog_state[port][0][0];
         break;
      case 1:
         val = analog_state[port][0][1];
         break;
      case 2:
         val = analog_state[port][1][0];
         break;
      case 3:
         val = analog_state[port][1][1];
         break;
   }

   if (is_neg && val > 0)
      val = 0;
   else if (is_pos && val < 0)
      val = 0;

   return val;
}

static void gx_joypad_poll(void)
{
   unsigned i, j, port;
   uint8_t gcpad = 0;

   pad_state[0] = 0;
   pad_state[1] = 0;
   pad_state[2] = 0;
   pad_state[3] = 0;

   gcpad = PAD_ScanPads();

#ifdef HW_RVL
   WPAD_ReadPending(WPAD_CHAN_ALL, NULL);
#endif

   for (port = 0; port < MAX_PADS; port++)
   {
      uint32_t down = 0, ptype = 0;
      uint64_t *state_cur = &pad_state[port];

#ifdef HW_RVL
      if (WPADProbe(port, &ptype) == WPAD_ERR_NONE)
      {
         WPADData *wpaddata = (WPADData*)WPAD_Data(port);
         expansion_t *exp = NULL;

         down = wpaddata->btns_h;

         exp = (expansion_t*)&wpaddata->exp;

         *state_cur |= (down & WPAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_A) : 0;
         *state_cur |= (down & WPAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_B) : 0;
         *state_cur |= (down & WPAD_BUTTON_1) ? (1ULL << GX_WIIMOTE_1) : 0;
         *state_cur |= (down & WPAD_BUTTON_2) ? (1ULL << GX_WIIMOTE_2) : 0;
         *state_cur |= (down & WPAD_BUTTON_PLUS) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
         *state_cur |= (down & WPAD_BUTTON_MINUS) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
         *state_cur |= (down & WPAD_BUTTON_HOME) ? (1ULL << GX_WIIMOTE_HOME) : 0;

         if (ptype != WPAD_EXP_NUNCHUK)
         {
            /* Rotated d-pad on Wiimote. */
            *state_cur |= (down & WPAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & WPAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & WPAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & WPAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_UP) : 0;
         }


         if (ptype == WPAD_EXP_CLASSIC)
         {
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_A) ? (1ULL << GX_CLASSIC_A) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_B) ? (1ULL << GX_CLASSIC_B) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_X) ? (1ULL << GX_CLASSIC_X) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_Y) ? (1ULL << GX_CLASSIC_Y) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_UP) ? (1ULL << GX_CLASSIC_UP) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_DOWN) ? (1ULL << GX_CLASSIC_DOWN) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_LEFT) ? (1ULL << GX_CLASSIC_LEFT) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_RIGHT) ? (1ULL << GX_CLASSIC_RIGHT) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_PLUS) ? (1ULL << GX_CLASSIC_PLUS) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_MINUS) ? (1ULL << GX_CLASSIC_MINUS) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_HOME) ? (1ULL << GX_CLASSIC_HOME) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_FULL_L) ? (1ULL << GX_CLASSIC_L_TRIGGER) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_FULL_R) ? (1ULL << GX_CLASSIC_R_TRIGGER) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_ZL) ? (1ULL << GX_CLASSIC_ZL_TRIGGER) : 0;
            *state_cur |= (down & WPAD_CLASSIC_BUTTON_ZR) ? (1ULL << GX_CLASSIC_ZR_TRIGGER) : 0;

            float ljs_mag = exp->classic.ljs.mag;
            float ljs_ang = exp->classic.ljs.ang;

            float rjs_mag = exp->classic.rjs.mag;
            float rjs_ang = exp->classic.rjs.ang;

            if (ljs_mag > 1.0f)
               ljs_mag = 1.0f;
            else if (ljs_mag < -1.0f)
               ljs_mag = -1.0f;

            if (rjs_mag > 1.0f)
               rjs_mag = 1.0f;
            else if (rjs_mag < -1.0f)
               rjs_mag = -1.0f;

            double ljs_val_x = ljs_mag * sin(M_PI * ljs_ang / 180.0);
            double ljs_val_y = -ljs_mag * cos(M_PI * ljs_ang / 180.0);

            double rjs_val_x = rjs_mag * sin(M_PI * rjs_ang / 180.0);
            double rjs_val_y = -rjs_mag * cos(M_PI * rjs_ang / 180.0);

            int16_t ls_x = (int16_t)(ljs_val_x * 32767.0f);
            int16_t ls_y = (int16_t)(ljs_val_y * 32767.0f);

            int16_t rs_x = (int16_t)(rjs_val_x * 32767.0f);
            int16_t rs_y = (int16_t)(rjs_val_y * 32767.0f);

            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] = ls_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] = ls_y;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X] = rs_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y] = rs_y;
         }
         else if (ptype == WPAD_EXP_NUNCHUK)
         {
            /* Wiimote is held upright with nunchuk, 
             * do not change d-pad orientation. */
            *state_cur |= (down & WPAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & WPAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & WPAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & WPAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;

            *state_cur |= (down & WPAD_NUNCHUK_BUTTON_Z) ? (1ULL << GX_NUNCHUK_Z) : 0;
            *state_cur |= (down & WPAD_NUNCHUK_BUTTON_C) ? (1ULL << GX_NUNCHUK_C) : 0;

            float js_mag = exp->nunchuk.js.mag;
            float js_ang = exp->nunchuk.js.ang;

            if (js_mag > 1.0f)
               js_mag = 1.0f;
            else if (js_mag < -1.0f)
               js_mag = -1.0f;

            double js_val_x = js_mag * sin(M_PI * js_ang / 180.0);
            double js_val_y = -js_mag * cos(M_PI * js_ang / 180.0);

            int16_t x = (int16_t)(js_val_x * 32767.0f);
            int16_t y = (int16_t)(js_val_y * 32767.0f);

            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] = x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] = y;

         }

		 if (gcpad & (1 << port))
         {
            int16_t ls_x, ls_y, rs_x, rs_y;

            down = PAD_ButtonsHeld(port);

//Default
            *state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_WIIMOTE_A) : 0;

//SNES direct
            /**state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_A) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_GC_L_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_GC_R_TRIGGER) : 0;*/

//SNES alt
            /**state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_A) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_GC_L_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_GC_R_TRIGGER) : 0;*/

            ls_x = (int16_t)PAD_StickX(port) * 256;
            ls_y = (int16_t)PAD_StickY(port) * -256;
            rs_x = (int16_t)PAD_SubStickX(port) * 256;
            rs_y = (int16_t)PAD_SubStickY(port) * -256;

            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] = ls_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] = ls_y;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X] = rs_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y] = rs_y;
         }
      }
#else
      ptype = WPAD_EXP_GAMECUBE;
#endif
      else
      {
         if (gcpad & (1 << port))
         {
            int16_t ls_x, ls_y, rs_x, rs_y;
            //uint64_t menu_combo = 0;

            down = PAD_ButtonsHeld(port);

//Default
            *state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_WIIMOTE_A) : 0;

//SNES direct
			/**state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_A) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_GC_L_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_GC_R_TRIGGER) : 0;*/

//SNES alt
            /**state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_WIIMOTE_2) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_WIIMOTE_1) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_WIIMOTE_A) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_WIIMOTE_B) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_WIIMOTE_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_WIIMOTE_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_WIIMOTE_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_WIIMOTE_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_WIIMOTE_PLUS) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_WIIMOTE_MINUS) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_GC_L_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_GC_R_TRIGGER) : 0;*/

            /**state_cur |= (down & PAD_BUTTON_A) ? (1ULL << GX_GC_A) : 0;
            *state_cur |= (down & PAD_BUTTON_B) ? (1ULL << GX_GC_B) : 0;
            *state_cur |= (down & PAD_BUTTON_X) ? (1ULL << GX_GC_X) : 0;
            *state_cur |= (down & PAD_BUTTON_Y) ? (1ULL << GX_GC_Y) : 0;
            *state_cur |= (down & PAD_BUTTON_UP) ? (1ULL << GX_GC_UP) : 0;
            *state_cur |= (down & PAD_BUTTON_DOWN) ? (1ULL << GX_GC_DOWN) : 0;
            *state_cur |= (down & PAD_BUTTON_LEFT) ? (1ULL << GX_GC_LEFT) : 0;
            *state_cur |= (down & PAD_BUTTON_RIGHT) ? (1ULL << GX_GC_RIGHT) : 0;
            *state_cur |= (down & PAD_BUTTON_START) ? (1ULL << GX_GC_START) : 0;
            *state_cur |= (down & PAD_TRIGGER_Z) ? (1ULL << GX_GC_Z_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_L) || PAD_TriggerL(port) > 80) ? (1ULL << GX_GC_L_TRIGGER) : 0;
            *state_cur |= ((down & PAD_TRIGGER_R) || PAD_TriggerR(port) > 80) ? (1ULL << GX_GC_R_TRIGGER) : 0;*/

            ls_x = (int16_t)PAD_StickX(port) * 256;
            ls_y = (int16_t)PAD_StickY(port) * -256;
            rs_x = (int16_t)PAD_SubStickX(port) * 256;
            rs_y = (int16_t)PAD_SubStickY(port) * -256;

            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X] = ls_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y] = ls_y;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X] = rs_x;
            analog_state[port][RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y] = rs_y;

            /*menu_combo = (1ULL << GX_GC_START) | (1ULL << GX_GC_Z_TRIGGER) | 
               (1ULL << GX_GC_L_TRIGGER) | (1ULL << GX_GC_R_TRIGGER);

            if ((*state_cur & menu_combo) == menu_combo)
               *state_cur |= (1ULL << GX_WIIMOTE_HOME);*/

            ptype = WPAD_EXP_GAMECUBE;
         }
#ifdef HAVE_LIBSICKSAXIS
         else
         {
            USB_DeviceChangeNotifyAsync(USB_CLASS_HID, change_cb, (void*)&lol);

            if (ss_is_connected(&dev[port]))
            {
               ptype = WPAD_EXP_SICKSAXIS;
               *state_cur |= (dev[port].pad.buttons.PS)       ? (1ULL << RARCH_MENU_TOGGLE) : 0;
               *state_cur |= (dev[port].pad.buttons.cross)    ? (1ULL << RETRO_DEVICE_ID_JOYPAD_B) : 0;
               *state_cur |= (dev[port].pad.buttons.square)   ? (1ULL << RETRO_DEVICE_ID_JOYPAD_Y) : 0;
               *state_cur |= (dev[port].pad.buttons.select)   ? (1ULL << RETRO_DEVICE_ID_JOYPAD_SELECT) : 0;
               *state_cur |= (dev[port].pad.buttons.start)    ? (1ULL << RETRO_DEVICE_ID_JOYPAD_START) : 0;
               *state_cur |= (dev[port].pad.buttons.up)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_UP) : 0;
               *state_cur |= (dev[port].pad.buttons.down)     ? (1ULL << RETRO_DEVICE_ID_JOYPAD_DOWN) : 0;
               *state_cur |= (dev[port].pad.buttons.left)     ? (1ULL << RETRO_DEVICE_ID_JOYPAD_LEFT) : 0;
               *state_cur |= (dev[port].pad.buttons.right)    ? (1ULL << RETRO_DEVICE_ID_JOYPAD_RIGHT) : 0;
               *state_cur |= (dev[port].pad.buttons.circle)   ? (1ULL << RETRO_DEVICE_ID_JOYPAD_A) : 0;
               *state_cur |= (dev[port].pad.buttons.triangle) ? (1ULL << RETRO_DEVICE_ID_JOYPAD_X) : 0;
               *state_cur |= (dev[port].pad.buttons.L1)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_L) : 0;
               *state_cur |= (dev[port].pad.buttons.R1)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_R) : 0;
               *state_cur |= (dev[port].pad.buttons.L2)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_L2) : 0;
               *state_cur |= (dev[port].pad.buttons.R2)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_R2) : 0;
               *state_cur |= (dev[port].pad.buttons.L3)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_L3) : 0;
               *state_cur |= (dev[port].pad.buttons.R3)       ? (1ULL << RETRO_DEVICE_ID_JOYPAD_R3) : 0;
            }
            else
            {
               if (ss_open(&dev[port]) > 0)
               {
                  ptype = WPAD_EXP_SICKSAXIS;
                  ss_start_reading(&dev[port]);
                  ss_set_removal_cb(&dev[port], removal_cb, (void*)1);
               }
            }
         }
#endif
      }

      if (ptype != pad_type[port])
         handle_hotplug(port, ptype);

      for (i = 0; i < 2; i++)
         for (j = 0; j < 2; j++)
            if (analog_state[port][i][j] == -0x8000)
               analog_state[port][i][j] = -0x7fff;
   }

   uint64_t *state_p1 = &pad_state[0];
   uint64_t *lifecycle_state = &g_extern.lifecycle_state;

   *lifecycle_state &= ~((1ULL << RARCH_RESET));

   if (g_reset)
   {
      *state_p1 |= (1ULL << RARCH_RESET);
      g_reset = false;
   }
   
   if (*state_p1 & ((1ULL << RARCH_RESET)))
   
   *lifecycle_state |= (1ULL << RARCH_RESET);

   *lifecycle_state &= ~((1ULL << RARCH_QUIT_KEY)); // RARCH_QUIT_KEY

   if (g_quit)
   {
      *state_p1 |= (1ULL << GX_QUIT_KEY);
      g_quit = false;
   }
   
   if (*state_p1 & ((1ULL << GX_QUIT_KEY)))
   
   *lifecycle_state |= (1ULL << RARCH_QUIT_KEY);

   *lifecycle_state &= ~((1ULL << RARCH_MENU_TOGGLE));

   if (g_menu)
   {
      *state_p1 |= (1ULL << GX_WIIMOTE_HOME);
      g_menu = false;
   }

   if (*state_p1 & ((1ULL << GX_WIIMOTE_HOME)
#ifdef HW_RVL
            | (1ULL << GX_CLASSIC_HOME)
#endif
            ))
      *lifecycle_state |= (1ULL << RARCH_MENU_TOGGLE);
}

static bool gx_joypad_query_pad(unsigned pad)
{
   return pad < MAX_PLAYERS && pad_state[pad];
}

static void gx_joypad_destroy(void)
{
   int i;
   for (i = 0; i < MAX_PADS; i++)
   {
#ifdef HAVE_LIBSICKSAXIS
      ss_close(&dev[i]);
      USB_Deinitialize();
#endif

#ifdef HW_RVL
     // WPAD_Flush(i);
     // WPADDisconnect(i);
#endif
   }
}

rarch_joypad_driver_t gx_joypad = {
   gx_joypad_init,
   gx_joypad_query_pad,
   gx_joypad_destroy,
   gx_joypad_button,
   gx_joypad_axis,
   gx_joypad_poll,
   NULL,
   gx_joypad_name,
   "gx",
};
