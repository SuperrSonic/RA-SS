#ifndef _DISP_SHARED_H
#define _DISP_SHARED_H

// CORE SELECTION %s = SELECCIONAR NUCLEO %s
static const char selec_nucl[22] = { 0x53, 0x45, 0x4C, 0x45, 0x43, 0x43, 0x49, 0x4F, 0x4E, 0x41, 0x52, 0x20, 0x4E, 0xDA, 0x43, 0x4C, 0x45, 0x4F, 0x20, 0x25, 0x73, 0x00 };

// Menu = Menú
//static const char menu_simpl_shr[5] = { 0x4D, 0x65, 0x6E, 0xFA, 0x00 };

// MENU OPTIONS = Opciones del menu
static const char opciones_del_menu[18] = { 0x4F, 0x50, 0x43, 0x49, 0x4F, 0x4E, 0x45, 0x53, 0x20, 0x44, 0x45, 0x4C, 0x20, 0x4D, 0x45, 0x4E, 0xDA, 0x00 };

static void get_title(const char *label, const char *dir,
      unsigned menu_type, char *title, size_t sizeof_title)
{
   if (!strcmp(label, "core_list"))
      snprintf(title, sizeof_title, g_settings.single_mode && g_settings.show_manuals ? "MANUAL" : selec_nucl, dir);
   else if (!strcmp(label, "deferred_core_list"))
      snprintf(title, sizeof_title, "DETECTADO %s", dir);
   else if (!strcmp(label, "configurations"))
      snprintf(title, sizeof_title, "CONFIG %s", dir);
   else if (!strcmp(label, "disk_image_append"))
      snprintf(title, sizeof_title, "DISK EXTRA %s", dir);
   else if (!strcmp(label, "Video"))
      strlcpy(title, "VIDEO", sizeof_title);
   else if (!strcmp(label, "Controles") ||
         menu_type == MENU_SETTINGS_CUSTOM_BIND ||
         menu_type == MENU_SETTINGS_CUSTOM_BIND_KEYBOARD)
      strlcpy(title, "CONTROLES", sizeof_title);
   else if (!strcmp(label, "Sobreponer"))
      strlcpy(title, "SOBREPONER", sizeof_title);
  /* else if (!strcmp(label, "Playlist Options"))
      strlcpy(title, "PLAYLIST OPTIONS", sizeof_title);
   else if (!strcmp(label, "Netplay Options"))
      strlcpy(title, "NETPLAY OPTIONS", sizeof_title);
   else if (!strcmp(label, "User Options"))
      strlcpy(title, "USER OPTIONS", sizeof_title); */
   else if (!strcmp(label, "Paths"))
      strlcpy(title, "RUTAS", sizeof_title);
   else if (!strcmp(label, "settings"))
      strlcpy(title, "CONFIGURACIONES", sizeof_title);
   else if (!strcmp(label, "Menu"))
      strlcpy(title, opciones_del_menu, sizeof_title);
  /* else if (!strcmp(label, "performance_counters"))
      strlcpy(title, "PERFORMANCE COUNTERS", sizeof_title);
   else if (!strcmp(label, "frontend_counters"))
      strlcpy(title, "FRONTEND PERFORMANCE COUNTERS", sizeof_title);
   else if (!strcmp(label, "core_counters"))
      strlcpy(title, "CORE PERFORMANCE COUNTERS", sizeof_title);
   else if (!strcmp(label, "Shader Options"))
      strlcpy(title, "SHADER OPTIONS", sizeof_title);
   else if (!strcmp(label, "video_shader_parameters"))
      strlcpy(title, "SHADER PARAMETERS (CURRENT)", sizeof_title);
   else if (!strcmp(label, "video_shader_preset_parameters"))
      strlcpy(title, "SHADER PARAMETERS (MENU PRESET)", sizeof_title);
   else if (!strcmp(label, "Font Options"))
      strlcpy(title, "FONT OPTIONS", sizeof_title);*/
   else if (!strcmp(label, "General"))
      strlcpy(title, "GENERAL", sizeof_title);
   else if (!strcmp(label, "Audio"))
      strlcpy(title, "AUDIO", sizeof_title);
   else if (!strcmp(label, "disk_options"))
      strlcpy(title, "DISK", sizeof_title);
   else if (!strcmp(label, "core_options"))
      strlcpy(title, "OPCIONES", sizeof_title);
  /* else if (!strcmp(label, "core_information"))
      strlcpy(title, "CORE INFO", sizeof_title);
   else if (!strcmp(label, "Privacy Options"))
      strlcpy(title, "PRIVACY OPTIONS", sizeof_title);
   else if (!strcmp(label, "video_shader_pass"))
      snprintf(title, sizeof_title, "SHADER %s", dir);
   else if (!strcmp(label, "video_shader_preset"))
      snprintf(title, sizeof_title, "SHADER PRESET %s", dir); */
   else if (menu_type == MENU_SETTINGS_CUSTOM_VIEWPORT ||
         !strcmp(label, "custom_viewport_2") ||
         !strcmp(label, "help") ||
         menu_type == MENU_SETTINGS)
      snprintf(title, sizeof_title, "MENU %s", dir);
   else if (!strcmp(label, "history_list"))
      strlcpy(title, " ", sizeof_title);
   else if (!strcmp(label, "info_screen"))
      strlcpy(title, "INFO", sizeof_title);
   else if (!strcmp(label, "input_overlay"))
      snprintf(title, sizeof_title, "SOBREPONER %s", dir);
 //  else if (!strcmp(label, "video_font_path"))
   //   snprintf(title, sizeof_title, "FONT %s", dir);
   else if (!strcmp(label, "video_filter"))
      snprintf(title, sizeof_title, "FILTRO %s", dir);
   else if (!strcmp(label, "audio_dsp_plugin"))
      snprintf(title, sizeof_title, "FILTRO DSP %s", dir);
   else if (!strcmp(label, "rgui_browser_directory"))
      snprintf(title, sizeof_title, "RUTA DEL DIR %s", dir);
   else if (!strcmp(label, "playlist_directory"))
      snprintf(title, sizeof_title, " %s", dir);
   else if (!strcmp(label, "content_directory"))
      snprintf(title, sizeof_title, "RUTA DE JUEGOS %s", dir);
   else if (!strcmp(label, "screenshot_directory"))
      snprintf(title, sizeof_title, "RUTA DE FOTOS %s", dir);
  // else if (!strcmp(label, "video_shader_dir"))
    //  snprintf(title, sizeof_title, "SHADER DIR %s", dir);
   else if (!strcmp(label, "video_filter_dir"))
      snprintf(title, sizeof_title, "RUTA DE FILTROS %s", dir);
   else if (!strcmp(label, "audio_filter_dir"))
      snprintf(title, sizeof_title, "RUTA DE FILTROS DSP %s", dir);
   else if (!strcmp(label, "savestate_directory"))
      snprintf(title, sizeof_title, "RUTA DE ESTADOS %s", dir);
  /* else if (!strcmp(label, "libretro_dir_path"))
      snprintf(title, sizeof_title, "LIBRETRO DIR %s", dir);
   else if (!strcmp(label, "libretro_info_path"))
      snprintf(title, sizeof_title, "LIBRETRO INFO DIR %s", dir); */
   else if (!strcmp(label, "rgui_config_directory"))
      snprintf(title, sizeof_title, "RUTA DE CONFIG %s", dir);
   else if (!strcmp(label, "savefile_directory"))
      snprintf(title, sizeof_title, "RUTA DE PARTIDAS %s", dir);
   else if (!strcmp(label, "overlay_directory"))
      snprintf(title, sizeof_title, "RUTA PARA SOBREPONER %s", dir);
   else if (!strcmp(label, "system_directory"))
      snprintf(title, sizeof_title, "RUTA SISTEMA %s", dir);
   else if (!strcmp(label, "assets_directory"))
      snprintf(title, sizeof_title, " %s", dir);
   else if (!strcmp(label, "extraction_directory"))
      snprintf(title, sizeof_title, "RUTA DE EXTRACCIONES %s", dir);
   else if (!strcmp(label, "joypad_autoconfig_dir"))
      snprintf(title, sizeof_title, " %s", dir);
   else
   {
      if (driver.menu->defer_core)
         snprintf(title, sizeof_title, "BUSCAR %s", dir);
      else
      {
         const char *core_name = g_extern.menu.info.library_name;
         if (!core_name)
            core_name = g_extern.system.info.library_name;
         if (!core_name)
            core_name = "No Core";
         snprintf(title, sizeof_title, "BUSCAR (%s) %s", core_name, dir);
      }
   }
}

static void disp_set_label(file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *type_str, size_t type_str_size,
      const char *entry_label,
      const char *path,
      char *path_buf, size_t path_buf_size)
{
   *type_str = '\0';
   *w = 19;

   if (!strcmp(label, "performance_counters"))
      *w = 28;

   if (!strcmp(label, "history_list"))
      *w = 6;

   if (type == MENU_FILE_CORE)
   {
      strlcpy(type_str, " ", type_str_size);
      menu_list_get_alt_at_offset(list, i, &path);
      *w = 6;
   }
   else if (type == MENU_FILE_PLAIN)
   {
      strlcpy(type_str, "(FILE)", type_str_size);
      *w = 6;
   }
   else if (type == MENU_FILE_USE_DIRECTORY)
   {
      *type_str = '\0';
      *w = 0;
   }
   else if (type == MENU_FILE_DIRECTORY)
   {
      strlcpy(type_str, "(DIR)", type_str_size);
      *w = 5;
   }
   else if (type == MENU_FILE_CARCHIVE)
   {
      strlcpy(type_str, "(COMP)", type_str_size);
      *w = 6;
   }
   else if (type == MENU_FILE_IN_CARCHIVE)
   {
      strlcpy(type_str, "(CFILE)", type_str_size);
      *w = 7;
   }
   else if (
         type == MENU_FILE_VIDEOFILTER ||
         type == MENU_FILE_AUDIOFILTER)
   {
      strlcpy(type_str, "(FILTRO)", type_str_size);
      *w = 8;
   }
   else if (type == MENU_FILE_CONFIG)
   {
      strlcpy(type_str, "(CONFIG)", type_str_size);
      *w = 8;
   }
   else if (type == MENU_FILE_OVERLAY)
   {
      strlcpy(type_str, "(SOBREPONER)", type_str_size);
      *w = 9;
   }
   else if (type >= MENU_SETTINGS_CORE_OPTION_START)
      strlcpy(
            type_str,
            core_option_get_val(g_extern.system.core_options,
               type - MENU_SETTINGS_CORE_OPTION_START),
            type_str_size);
   else if (type == MENU_FILE_PUSH || type == MENU_FILE_LINEFEED_SWITCH)
      strlcpy(type_str, " ", type_str_size);
   else if (driver.menu_ctx && driver.menu_ctx->backend &&
         driver.menu_ctx->backend->setting_set_label)
      driver.menu_ctx->backend->setting_set_label(type_str,
            type_str_size, w, type, label, entry_label, i);

   strlcpy(path_buf, path, path_buf_size);
}

#endif
