/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2013-2014 - pinumbernumber
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

#ifndef AUTOCONF_BUILTIN_H__
#define AUTOCONF_BUILTIN_H__

#include "../input_common.h"
#define DECL_BTN(btn, bind) "input_" #btn "_btn = " #bind "\n"
#define DECL_AXIS(axis, bind) "input_" #axis "_axis = " #bind "\n"
#define DECL_MENU(btn) "input_menu_toggle_btn = " #btn "\n"
#define DECL_MENU_AXIS(axis) "input_menu_toggle_axis = " #axis "\n"
#define DECL_AUTOCONF_DEVICE(device, driver, binds) "input_device = \"" #device "\" \ninput_driver = \"" #driver "\"                    \n" binds

#endif
