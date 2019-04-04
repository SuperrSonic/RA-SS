/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
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

#include "overlay.h"
#include "../general.h"
#include "../driver.h"
#include "../libretro.h"
#include "../gfx/image/image.h"
#include <file/config_file.h>
#include <compat/posix_string.h>
#include "input_common.h"
#include <file/file_path.h>
#include <stddef.h>
#include <math.h>

enum overlay_hitbox
{
   OVERLAY_HITBOX_RADIAL = 0,
   OVERLAY_HITBOX_RECT
};

enum overlay_type
{
   OVERLAY_TYPE_BUTTONS = 0,
   OVERLAY_TYPE_ANALOG_LEFT,
   OVERLAY_TYPE_ANALOG_RIGHT,
   OVERLAY_TYPE_KEYBOARD
};

struct overlay_desc
{
   float x;
   float y;

   enum overlay_hitbox hitbox;
   float range_x, range_y;
   float range_x_mod, range_y_mod;
   float mod_x, mod_y, mod_w, mod_h;
   float delta_x, delta_y;

   enum overlay_type type;
   uint64_t key_mask;
   float analog_saturate_pct;

   unsigned next_index;
   char next_index_name[64];

   struct texture_image image;
   unsigned image_index;

   float alpha_mod;
   float range_mod;

   bool updated;
   bool movable;
};

struct overlay
{
   struct overlay_desc *descs;
   size_t size;

   struct texture_image image;

   bool block_scale;
   float mod_x, mod_y, mod_w, mod_h;
   float x, y, w, h;
   float scale;
   float center_x, center_y;

   bool full_screen;

   char name[64];

   struct texture_image *load_images;
   unsigned load_images_size;
};

struct input_overlay
{
   void *iface_data;
   const video_overlay_interface_t *iface;
   bool enable;

   bool blocked;

   struct overlay *overlays;
   const struct overlay *active;
   size_t index;
   size_t size;

   unsigned next_index;
   char *overlay_path;
};

static void input_overlay_scale(struct overlay *overlay, float scale)
{
   size_t i;
   if (overlay->block_scale)
      scale = 1.0f;

   overlay->scale = scale;
   overlay->mod_w = overlay->w * scale;
   overlay->mod_h = overlay->h * scale;
   overlay->mod_x = overlay->center_x +
      (overlay->x - overlay->center_x) * scale;
   overlay->mod_y = overlay->center_y +
      (overlay->y - overlay->center_y) * scale;

   for (i = 0; i < overlay->size; i++)
   {
      struct overlay_desc *desc = &overlay->descs[i];

      float scale_w = overlay->mod_w * desc->range_x;
      float scale_h = overlay->mod_h * desc->range_y;

      desc->mod_w = 2.0f * scale_w;
      desc->mod_h = 2.0f * scale_h;

      float adj_center_x = overlay->mod_x + desc->x * overlay->mod_w;
      float adj_center_y = overlay->mod_y + desc->y * overlay->mod_h;
      desc->mod_x = adj_center_x - scale_w;
      desc->mod_y = adj_center_y - scale_h;
   }
}

static void input_overlay_set_vertex_geom(input_overlay_t *ol)
{
   size_t i;
   if (ol->active->image.pixels)
      ol->iface->vertex_geom(ol->iface_data, 0,
            ol->active->mod_x, ol->active->mod_y,
            ol->active->mod_w, ol->active->mod_h);

   for (i = 0; i < ol->active->size; i++)
   {
      struct overlay_desc *desc = &ol->active->descs[i];
      if (desc->image.pixels)
         ol->iface->vertex_geom(ol->iface_data, desc->image_index,
               desc->mod_x, desc->mod_y, desc->mod_w, desc->mod_h);
   }
}

void input_overlay_set_scale_factor(input_overlay_t *ol, float scale)
{
   size_t i;

   if (!ol)
      return;

   for (i = 0; i < ol->size; i++)
      input_overlay_scale(&ol->overlays[i], scale);

   input_overlay_set_vertex_geom(ol);
}

static void input_overlay_free_overlay(struct overlay *overlay)
{
   size_t i;

   if (!overlay)
      return;

   for (i = 0; i < overlay->size; i++)
      texture_image_free(&overlay->descs[i].image);

   free(overlay->load_images);
   free(overlay->descs);
   texture_image_free(&overlay->image);
}

static void input_overlay_free_overlays(input_overlay_t *ol)
{
   size_t i;

   if (!ol)
      return;

   for (i = 0; i < ol->size; i++)
      input_overlay_free_overlay(&ol->overlays[i]);

   free(ol->overlays);
}

static bool input_overlay_load_desc(input_overlay_t *ol,
      config_file_t *conf, struct overlay_desc *desc,
      unsigned ol_idx, unsigned desc_idx,
      unsigned width, unsigned height,
      bool normalized, float alpha_mod, float range_mod)
{
   bool ret = true;
   char overlay_desc_key[64];
   snprintf(overlay_desc_key, sizeof(overlay_desc_key),
         "overlay%u_desc%u", ol_idx, desc_idx);

   char overlay_desc_image_key[64];
   snprintf(overlay_desc_image_key, sizeof(overlay_desc_image_key),
         "overlay%u_desc%u_overlay", ol_idx, desc_idx);

   char image_path[PATH_MAX];
   if (config_get_path(conf, overlay_desc_image_key,
            image_path, sizeof(image_path)))
   {
      char path[PATH_MAX];
      fill_pathname_resolve_relative(path, ol->overlay_path,
            image_path, sizeof(path));

      struct texture_image img = {0};
      if (texture_image_load(&img, path))
         desc->image = img;
   }

   char overlay_desc_normalized_key[64];
   snprintf(overlay_desc_normalized_key, sizeof(overlay_desc_normalized_key),
         "overlay%u_desc%u_normalized", ol_idx, desc_idx);
   config_get_bool(conf, overlay_desc_normalized_key, &normalized);
   bool by_pixel = !normalized;

   if (by_pixel && (width == 0 || height == 0))
   {
      RARCH_ERR("[Overlay]: Base overlay is not set and not using normalized coordinates.\n");
      return false;
   }

   char overlay[256];
   if (!config_get_array(conf, overlay_desc_key, overlay, sizeof(overlay)))
   {
      RARCH_ERR("[Overlay]: Didn't find key: %s.\n", overlay_desc_key);
      return false;
   }

   struct string_list *list = string_split(overlay, ", ");
   if (!list)
   {
      RARCH_ERR("[Overlay]: Failed to split overlay desc.\n");
      return false;
   }

   if (list->size < 6)
   {
      string_list_free(list);
      RARCH_ERR("[Overlay]: Overlay desc is invalid. Requires at least 6 tokens.\n");
      return false;
   }

   const char *x   = list->elems[1].data;
   const char *y   = list->elems[2].data;
   const char *box = list->elems[3].data;

   char *key = list->elems[0].data;
   char *save;
   desc->key_mask = 0;

   if (strcmp(key, "analog_left") == 0)
      desc->type = OVERLAY_TYPE_ANALOG_LEFT;
   else if (strcmp(key, "analog_right") == 0)
      desc->type = OVERLAY_TYPE_ANALOG_RIGHT;
   else if (strstr(key, "retrok_") == key)
   {
      desc->type = OVERLAY_TYPE_KEYBOARD;
      desc->key_mask = input_translate_str_to_rk(key + 7);
   }
   else
   {
      const char *tmp;
      desc->type = OVERLAY_TYPE_BUTTONS;
      for (tmp = strtok_r(key, "|", &save); tmp; tmp = strtok_r(NULL, "|", &save))
      {
         if (strcmp(tmp, "nul") != 0)
            desc->key_mask |= UINT64_C(1) << input_translate_str_to_bind_id(tmp);
      }

     /* if (desc->key_mask & (UINT64_C(1) << RARCH_OVERLAY_NEXT))
      {
         char overlay_target_key[64];
         snprintf(overlay_target_key, sizeof(overlay_target_key),
               "overlay%u_desc%u_next_target", ol_idx, desc_idx);
         config_get_array(conf, overlay_target_key,
               desc->next_index_name, sizeof(desc->next_index_name));
      }*/
   }

   float width_mod = by_pixel ? (1.0f / width) : 1.0f;
   float height_mod = by_pixel ? (1.0f / height) : 1.0f;

   desc->x = (float)strtod(x, NULL) * width_mod;
   desc->y = (float)strtod(y, NULL) * height_mod;

   if (!strcmp(box, "radial"))
      desc->hitbox = OVERLAY_HITBOX_RADIAL;
   else if (!strcmp(box, "rect"))
      desc->hitbox = OVERLAY_HITBOX_RECT;
   else
   {
      RARCH_ERR("[Overlay]: Hitbox type (%s) is invalid. Use \"radial\" or \"rect\".\n", box);
      ret = false;
      goto end;
   }

   if (
         desc->type == OVERLAY_TYPE_ANALOG_LEFT ||
         desc->type == OVERLAY_TYPE_ANALOG_RIGHT)
   {
      if (desc->hitbox != OVERLAY_HITBOX_RADIAL)
      {
         RARCH_ERR("[Overlay]: Analog hitbox type must be \"radial\".\n");
         ret = false;
         goto end;
      }

      char overlay_analog_saturate_key[64];
      snprintf(overlay_analog_saturate_key, sizeof(overlay_analog_saturate_key),
            "overlay%u_desc%u_saturate_pct", ol_idx, desc_idx);
      if (!config_get_float(conf, overlay_analog_saturate_key,
               &desc->analog_saturate_pct))
         desc->analog_saturate_pct = 1.0f;
   }

   desc->range_x = (float)strtod(list->elems[4].data, NULL) * width_mod;
   desc->range_y = (float)strtod(list->elems[5].data, NULL) * height_mod;

   desc->mod_x = desc->x - desc->range_x;
   desc->mod_w = 2.0f * desc->range_x;
   desc->mod_y = desc->y - desc->range_y;
   desc->mod_h = 2.0f * desc->range_y;

   char conf_key[64];
   snprintf(conf_key, sizeof(conf_key),
         "overlay%u_desc%u_alpha_mod", ol_idx, desc_idx);
   desc->alpha_mod = alpha_mod;
   config_get_float(conf, conf_key, &desc->alpha_mod);

   snprintf(conf_key, sizeof(conf_key),
         "overlay%u_desc%u_range_mod", ol_idx, desc_idx);
   desc->range_mod = range_mod;
   config_get_float(conf, conf_key, &desc->range_mod);

   snprintf(conf_key, sizeof(conf_key),
         "overlay%u_desc%u_movable", ol_idx, desc_idx);
   desc->movable = false;
   desc->delta_x = 0.0f;
   desc->delta_y = 0.0f;
   config_get_bool(conf, conf_key, &desc->movable);

   desc->range_x_mod = desc->range_x;
   desc->range_y_mod = desc->range_y;

end:
   if (list)
      string_list_free(list);
   return ret;
}

static bool input_overlay_load_overlay(input_overlay_t *ol,
      config_file_t *conf, const char *config_path,
      struct overlay *overlay, unsigned idx)
{
   size_t i;
   char overlay_path_key[64];
   char overlay_name_key[64];
   char overlay_path[PATH_MAX];
   char overlay_resolved_path[PATH_MAX];

   snprintf(overlay_path_key, sizeof(overlay_path_key),
         "overlay%u_overlay", idx);
    
   if (config_get_path(conf, overlay_path_key,
            overlay_path, sizeof(overlay_path)))
   {
      struct texture_image img = {0};

      fill_pathname_resolve_relative(overlay_resolved_path, config_path,
            overlay_path, sizeof(overlay_resolved_path));

      if (texture_image_load(&img, overlay_resolved_path))
         overlay->image = img;
      else
      {
         RARCH_ERR("[Overlay]: Failed to load image: %s.\n",
               overlay_resolved_path);
         return false;
      }
   }

   snprintf(overlay_name_key, sizeof(overlay_name_key),
         "overlay%u_name", idx);
   config_get_array(conf, overlay_name_key,
         overlay->name, sizeof(overlay->name));

   /* By default, we stretch the overlay out in full. */
   overlay->x = overlay->y = 0.0f;
   overlay->w = overlay->h = 1.0f;

   char overlay_rect_key[64];
   snprintf(overlay_rect_key, sizeof(overlay_rect_key),
         "overlay%u_rect", idx);
   char overlay_rect[256];

   if (config_get_array(conf, overlay_rect_key,
            overlay_rect, sizeof(overlay_rect)))
   {
      struct string_list *list = string_split(overlay_rect, ", ");
      if (list->size < 4)
      {
         RARCH_ERR("[Overlay]: Failed to split rect \"%s\" into at least four tokens.\n",
               overlay_rect);
         return false;
      }

      overlay->x = (float)strtod(list->elems[0].data, NULL);
      overlay->y = (float)strtod(list->elems[1].data, NULL);
      overlay->w = (float)strtod(list->elems[2].data, NULL);
      overlay->h = (float)strtod(list->elems[3].data, NULL);
      string_list_free(list);
   }

   char overlay_full_screen_key[64];
   snprintf(overlay_full_screen_key, sizeof(overlay_full_screen_key),
         "overlay%u_full_screen", idx);
   overlay->full_screen = false;
   config_get_bool(conf, overlay_full_screen_key, &overlay->full_screen);

   char overlay_descs_key[64];
   snprintf(overlay_descs_key, sizeof(overlay_descs_key), "overlay%u_descs", idx);

   unsigned descs = 0;
   if (!config_get_uint(conf, overlay_descs_key, &descs))
   {
      RARCH_ERR("[Overlay]: Failed to read number of descs from config key: %s.\n",
            overlay_descs_key);
      return false;
   }

   overlay->descs = (struct overlay_desc*)
      calloc(descs, sizeof(*overlay->descs));

   if (!overlay->descs)
   {
      RARCH_ERR("[Overlay]: Failed to allocate descs.\n");
      return false;
   }

   overlay->size = descs;

   char conf_key[64];
   bool normalized = false;
   snprintf(conf_key, sizeof(conf_key),
         "overlay%u_normalized", idx);
   config_get_bool(conf, conf_key, &normalized);

   float alpha_mod = 1.0f;
   snprintf(conf_key, sizeof(conf_key), "overlay%u_alpha_mod", idx);
   config_get_float(conf, conf_key, &alpha_mod);

   float range_mod = 1.0f;
   snprintf(conf_key, sizeof(conf_key), "overlay%u_range_mod", idx);
   config_get_float(conf, conf_key, &range_mod);

   for (i = 0; i < overlay->size; i++)
   {
      if (!input_overlay_load_desc(ol, conf, &overlay->descs[i], idx, i,
               overlay->image.width, overlay->image.height,
               normalized, alpha_mod, range_mod))
      {
         RARCH_ERR("[Overlay]: Failed to load overlay descs for overlay #%u.\n",
               (unsigned)i);
         return false;
      }
   }

   /* Precache load image array for simplicity. */
   overlay->load_images = (struct texture_image*)
      calloc(1 + overlay->size, sizeof(struct texture_image));

   if (!overlay->load_images)
   {
      RARCH_ERR("[Overlay]: Failed to allocate load_images.\n");
      return false;
   }

   if (overlay->image.pixels)
      overlay->load_images[overlay->load_images_size++] = overlay->image;

   for (i = 0; i < overlay->size; i++)
   {
      if (overlay->descs[i].image.pixels)
      {
         overlay->descs[i].image_index = overlay->load_images_size;
         overlay->load_images[overlay->load_images_size++] = overlay->descs[i].image;
      }
   }

   /* Assume for now that scaling center is in the middle.
    * TODO: Make this configurable. */
   overlay->block_scale = false;
   overlay->center_x = overlay->x + 0.5f * overlay->w;
   overlay->center_y = overlay->y + 0.5f * overlay->h;

   return true;
}

static ssize_t input_overlay_find_index(const struct overlay *ol,
      const char *name, size_t size)
{
   size_t i;
   for (i = 0; i < size; i++)
   {
      if (strcmp(ol[i].name, name) == 0)
         return i;
   }

   return -1;
}

static bool input_overlay_resolve_targets(struct overlay *ol,
      size_t idx, size_t size)
{
   size_t i;
   struct overlay *current = (struct overlay*)&ol[idx];

   for (i = 0; i < current->size; i++)
   {
      const char *next = current->descs[i].next_index_name;
      if (*next)
      {
         ssize_t next_idx = input_overlay_find_index(ol, next, size);
         if (next_idx < 0)
         {
            RARCH_ERR("[Overlay]: Couldn't find overlay called: \"%s\".\n",
                  next);
            return false;
         }

         current->descs[i].next_index = next_idx;
      }
      else
         current->descs[i].next_index = (idx + 1) % size;
   }

   return true;
}

static bool input_overlay_load_overlays(input_overlay_t *ol, const char *path)
{
   size_t i;
   bool ret = true;
   config_file_t *conf = config_file_new(path);
   if (!conf)
   {
      RARCH_ERR("Failed to load config file: %s.\n", path);
      return false;
   }

   unsigned overlays = 0;
   if (!config_get_uint(conf, "overlays", &overlays))
   {
      RARCH_ERR("overlays variable not defined in config.\n");
      ret = false;
      goto end;
   }

   if (!overlays)
   {
      ret = false;
      goto end;
   }

   ol->overlays = (struct overlay*)calloc(overlays, sizeof(*ol->overlays));
   if (!ol->overlays)
   {
      ret = false;
      goto end;
   }

   ol->size = overlays;

   for (i = 0; i < ol->size; i++)
   {
      if (!input_overlay_load_overlay(ol, conf, path, &ol->overlays[i], i))
      {
         RARCH_ERR("[Overlay]: Failed to load overlay #%u.\n", (unsigned)i);
         ret = false;
         goto end;
      }
   }

   for (i = 0; i < ol->size; i++)
   {
      if (!input_overlay_resolve_targets(ol->overlays, i, ol->size))
      {
         RARCH_ERR("[Overlay]: Failed to resolve next targets.\n");
         ret = false;
         goto end;
      }
   }

end:
   config_file_free(conf);
   return ret;
}

static void input_overlay_load_active(input_overlay_t *ol)
{
   ol->iface->load(ol->iface_data, ol->active->load_images,
         ol->active->load_images_size);

   input_overlay_set_alpha_mod(ol, g_settings.input.overlay_opacity);
   input_overlay_set_vertex_geom(ol);
   ol->iface->full_screen(ol->iface_data, ol->active->full_screen);
}

input_overlay_t *input_overlay_new(const char *overlay)
{
   input_overlay_t *ol = (input_overlay_t*)calloc(1, sizeof(*ol));

   if (!ol)
      goto error;

   ol->overlay_path = strdup(overlay);
   if (!ol->overlay_path)
   {
      free(ol);
      return NULL;
   }

   if (!driver.video->overlay_interface)
   {
      RARCH_ERR("Overlay interface is not present in video driver.\n");
      goto error;
   }

   if (driver.video && driver.video->overlay_interface)
      driver.video->overlay_interface(driver.video_data, &ol->iface);
   ol->iface_data = driver.video_data;

   if (!ol->iface)
      goto error;

   if (!input_overlay_load_overlays(ol, overlay))
      goto error;

   ol->active = &ol->overlays[0];

   input_overlay_load_active(ol);
   ol->iface->enable(ol->iface_data, true);
   ol->enable = true;

   input_overlay_set_alpha_mod(ol, g_settings.input.overlay_opacity);
   input_overlay_set_scale_factor(ol, g_settings.input.overlay_scale);
   ol->next_index = (ol->index + 1) % ol->size;

   return ol;

error:
   input_overlay_free(ol);
   return NULL;
}

void input_overlay_enable(input_overlay_t *ol, bool enable)
{
   ol->enable = enable;
   ol->iface->enable(ol->iface_data, enable);
}

static bool inside_hitbox(const struct overlay_desc *desc, float x, float y)
{
   switch (desc->hitbox)
   {
      case OVERLAY_HITBOX_RADIAL:
      {
         /* Ellipsis. */
         float x_dist = (x - desc->x) / desc->range_x_mod;
         float y_dist = (y - desc->y) / desc->range_y_mod;
         float sq_dist = x_dist * x_dist + y_dist * y_dist;
         return sq_dist <= 1.0f;
      }

      case OVERLAY_HITBOX_RECT:
         return (fabs(x - desc->x) <= desc->range_x_mod) &&
            (fabs(y - desc->y) <= desc->range_y_mod);
   }

   return false;
}

static inline float clamp(float val, float lower, float upper)
{
   if (val < lower)
      return lower;
   else if (val > upper)
      return upper;
   return val;
}

void input_overlay_poll(input_overlay_t *ol, input_overlay_state_t *out,
      int16_t norm_x, int16_t norm_y)
{
   size_t i;
   memset(out, 0, sizeof(*out));

   if (!ol->enable)
   {
      ol->blocked = false;
      return;
   }

   /* norm_x and norm_y is in [-0x7fff, 0x7fff] range,
    * like RETRO_DEVICE_POINTER. */
   float x = (float)(norm_x + 0x7fff) / 0xffff;
   float y = (float)(norm_y + 0x7fff) / 0xffff;

   x -= ol->active->mod_x;
   y -= ol->active->mod_y;
   x /= ol->active->mod_w;
   y /= ol->active->mod_h;

   for (i = 0; i < ol->active->size; i++)
   {
      struct overlay_desc *desc = &ol->active->descs[i];
      if (!inside_hitbox(desc, x, y))
         continue;

      desc->updated = true;

      if (desc->type == OVERLAY_TYPE_BUTTONS)
      {
         uint64_t mask = desc->key_mask;
         out->buttons |= mask;

        // if (mask & (UINT64_C(1) << RARCH_OVERLAY_NEXT))
          //  ol->next_index = desc->next_index;
      }
      else if (desc->type == OVERLAY_TYPE_KEYBOARD)
      {
         if (desc->key_mask < RETROK_LAST)
            OVERLAY_SET_KEY(out, desc->key_mask);
      }
      else
      {
         float x_dist = x - desc->x;
         float y_dist = y - desc->y;
         float x_val = x_dist / desc->range_x;
         float y_val = y_dist / desc->range_y;
         float x_val_sat = x_val / desc->analog_saturate_pct;
         float y_val_sat = y_val / desc->analog_saturate_pct;

         unsigned int base = (desc->type == OVERLAY_TYPE_ANALOG_RIGHT) ? 2 : 0;
         out->analog[base + 0] = clamp(x_val_sat, -1.0f, 1.0f) * 32767.0f;
         out->analog[base + 1] = clamp(y_val_sat, -1.0f, 1.0f) * 32767.0f;
      }

      if (desc->movable)
      {
         float x_dist = x - desc->x;
         float y_dist = y - desc->y;
         desc->delta_x = clamp(x_dist, -desc->range_x, desc->range_x)
            * ol->active->mod_w;
         desc->delta_y = clamp(y_dist, -desc->range_y, desc->range_y)
            * ol->active->mod_h;
      }
   }

   if (!out->buttons)
      ol->blocked = false;
   else if (ol->blocked)
      memset(out, 0, sizeof(*out));
}

static void input_overlay_update_desc_geom(input_overlay_t *ol,
      struct overlay_desc *desc)
{
   if (desc->image.pixels && desc->movable)
   {
      ol->iface->vertex_geom(ol->iface_data, desc->image_index,
            desc->mod_x + desc->delta_x, desc->mod_y + desc->delta_y,
            desc->mod_w, desc->mod_h);

      desc->delta_x = 0.0f;
      desc->delta_y = 0.0f;
   }
}

void input_overlay_post_poll(input_overlay_t *ol)
{
   size_t i;

   input_overlay_set_alpha_mod(ol, g_settings.input.overlay_opacity);

   for (i = 0; i < ol->active->size; i++)
   {
      struct overlay_desc *desc = &ol->active->descs[i];

      if (desc->updated)
      {
         /* If pressed this frame, change the hitbox. */
         desc->range_x_mod = desc->range_x * desc->range_mod;
         desc->range_y_mod = desc->range_y * desc->range_mod;

         if (desc->image.pixels)
            ol->iface->set_alpha(ol->iface_data, desc->image_index,
                  desc->alpha_mod * g_settings.input.overlay_opacity);
      }
      else
      {
         desc->range_x_mod = desc->range_x;
         desc->range_y_mod = desc->range_y;
      }

      input_overlay_update_desc_geom(ol, desc);
      desc->updated = false;
   }
}

void input_overlay_poll_clear(input_overlay_t *ol)
{
   size_t i;
   ol->blocked = false;
   input_overlay_set_alpha_mod(ol, g_settings.input.overlay_opacity);

   for (i = 0; i < ol->active->size; i++)
   {
      struct overlay_desc *desc = &ol->active->descs[i];
      desc->range_x_mod = desc->range_x;
      desc->range_y_mod = desc->range_y;
      desc->updated = false;

      desc->delta_x = 0.0f;
      desc->delta_y = 0.0f;
      input_overlay_update_desc_geom(ol, desc);
   }
}

void input_overlay_next(input_overlay_t *ol)
{
   if (!ol)
      return;

   ol->index = ol->next_index;
   ol->active = &ol->overlays[ol->index];

   input_overlay_load_active(ol);

   ol->blocked = true;
   ol->next_index = (ol->index + 1) % ol->size;
}

bool input_overlay_full_screen(input_overlay_t *ol)
{
   if (!ol)
      return false;
   return ol->active->full_screen;
}

void input_overlay_free(input_overlay_t *ol)
{
   if (!ol)
      return;

   input_overlay_free_overlays(ol);

   if (ol->iface)
      ol->iface->enable(ol->iface_data, false);

   free(ol->overlay_path);
   free(ol);
}

void input_overlay_set_alpha_mod(input_overlay_t *ol, float mod)
{
   unsigned i;

   if (!ol)
      return;

   for (i = 0; i < ol->active->load_images_size; i++)
      ol->iface->set_alpha(ol->iface_data, i,
            g_settings.input.overlay_opacity);
}

