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

#include "filter.h"
#include "filters/softfilter.h"
#include "../dynamic.h"
#include <file/config_file_userdata.h>
#include <file/file_path.h>
#include "../file_ext.h"
#include <file/dir_list.h>
#include "../performance.h"
#include <stdlib.h>

struct rarch_soft_plug
{
#ifdef HAVE_DYLIB
   dylib_t lib;
#endif
   const struct softfilter_implementation *impl;
};

struct rarch_softfilter
{
   config_file_t *conf;

   const struct softfilter_implementation *impl;
   void *impl_data;

   struct rarch_soft_plug *plugs;
   unsigned num_plugs;

   unsigned max_width, max_height;
   enum retro_pixel_format pix_fmt, out_pix_fmt;

   struct softfilter_work_packet *packets;
};

static const struct softfilter_implementation *
softfilter_find_implementation(rarch_softfilter_t *filt, const char *ident)
{
   unsigned i;
   for (i = 0; i < filt->num_plugs; i++)
   {
      if (!strcmp(filt->plugs[i].impl->short_ident, ident))
         return filt->plugs[i].impl;
   }

   return NULL;
}

static const struct softfilter_config softfilter_config = {
   config_userdata_get_float,
   config_userdata_get_int,
   config_userdata_get_float_array,
   config_userdata_get_int_array,
   config_userdata_get_string,
   config_userdata_free,
};

static bool create_softfilter_graph(rarch_softfilter_t *filt,
      enum retro_pixel_format in_pixel_format,
      unsigned max_width, unsigned max_height,
      softfilter_simd_mask_t cpu_features,
      unsigned threads)
{
   unsigned input_fmts, input_fmt, output_fmts;
   char key[64];
   struct config_file_userdata userdata;

   snprintf(key, sizeof(key), "filter");

   char name[64];
   if (!config_get_array(filt->conf, key, name, sizeof(name)))
      return false;

   filt->impl = softfilter_find_implementation(filt, name);
   if (!filt->impl)
      return false;

   userdata.conf = filt->conf;
   /* Index-specific configs take priority over ident-specific. */
   userdata.prefix[0] = key; 
   userdata.prefix[1] = filt->impl->short_ident;

   /* Simple assumptions. */
   filt->pix_fmt = in_pixel_format;
   input_fmts = filt->impl->query_input_formats();

   switch (in_pixel_format)
   {
      case RETRO_PIXEL_FORMAT_XRGB8888:
         input_fmt = SOFTFILTER_FMT_XRGB8888;
         break;
      case RETRO_PIXEL_FORMAT_RGB565:
         input_fmt = SOFTFILTER_FMT_RGB565;
         break;
      default:
         return false;
   }

   if (!(input_fmt & input_fmts))
   {
      RARCH_ERR("Softfilter does not support input format.\n");
      return false;
   }

   output_fmts = filt->impl->query_output_formats(input_fmt);
   /* If we have a match of input/output formats, use that. */
   if (output_fmts & input_fmt)
      filt->out_pix_fmt = in_pixel_format;
   else if (output_fmts & SOFTFILTER_FMT_XRGB8888)
      filt->out_pix_fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   else if (output_fmts & SOFTFILTER_FMT_RGB565)
      filt->out_pix_fmt = RETRO_PIXEL_FORMAT_RGB565;
   else
   {
      RARCH_ERR("Did not find suitable output format for softfilter.\n");
      return false;
   }

   filt->max_width = max_width;
   filt->max_height = max_height;

   filt->impl_data = filt->impl->create(
         &softfilter_config, input_fmt, input_fmt, max_width, max_height, 1, cpu_features, &userdata);
   //filt->impl_data = filt->impl->create(input_fmt, input_fmt, max_width, max_height, 1, cpu_features);
   if (!filt->impl_data)
   {
      RARCH_ERR("Failed to create softfilter state.\n");
      return false;
   }

   filt->packets = (struct softfilter_work_packet*)calloc(1, sizeof(*filt->packets));
   if (!filt->packets)
   {
      RARCH_ERR("Failed to allocate softfilter packets.\n");
      return false;
   }

   return true;
}

#ifdef HAVE_DYLIB
static bool append_softfilter_plugs(rarch_softfilter_t *filt,
      struct string_list *list)
{
   unsigned i;
   softfilter_simd_mask_t mask = rarch_get_cpu_features();

   for (i = 0; i < list->size; i++)
   {
      dylib_t lib = dylib_load(list->elems[i].data);
      if (!lib)
         continue;

      softfilter_get_implementation_t cb = (softfilter_get_implementation_t)
         dylib_proc(lib, "softfilter_get_implementation");
      if (!cb)
      {
         dylib_close(lib);
         continue;
      }

      const struct softfilter_implementation *impl = cb(mask);
      if (!impl)
      {
         dylib_close(lib);
         continue;
      }

      if (impl->api_version != SOFTFILTER_API_VERSION)
      {
         dylib_close(lib);
         continue;
      }

      struct rarch_soft_plug *new_plugs = (struct rarch_soft_plug*)
         realloc(filt->plugs, sizeof(*filt->plugs) * (filt->num_plugs + 1));
      if (!new_plugs)
      {
         dylib_close(lib);
         return false;
      }

      RARCH_LOG("[SoftFilter]: Found plug: %s (%s).\n",
            impl->ident, impl->short_ident);
      
      filt->plugs = new_plugs;
      filt->plugs[filt->num_plugs].lib = lib;
      filt->plugs[filt->num_plugs].impl = impl;
      filt->num_plugs++;
   }

   return true;
}
#else
extern const struct softfilter_implementation *blargg_ntsc_snes_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *lq2x_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *phosphor2x_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *twoxbr_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *epx_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *twoxsai_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *supereagle_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *supertwoxsai_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *twoxbr_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *darken_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *scale2x_get_implementation(softfilter_simd_mask_t simd);
extern const struct softfilter_implementation *normal2x_get_implementation(softfilter_simd_mask_t simd);

static const softfilter_get_implementation_t soft_plugs_builtin[] = {
   blargg_ntsc_snes_get_implementation,
   lq2x_get_implementation,
   phosphor2x_get_implementation,
   twoxbr_get_implementation,
   darken_get_implementation,
   twoxsai_get_implementation,
   supertwoxsai_get_implementation,
   supereagle_get_implementation,
   epx_get_implementation,
   scale2x_get_implementation,
   normal2x_get_implementation,
};

static bool append_softfilter_plugs(rarch_softfilter_t *filt)
{
   unsigned i;
   softfilter_simd_mask_t mask = rarch_get_cpu_features();

   filt->plugs = (struct rarch_soft_plug*)
      calloc(ARRAY_SIZE(soft_plugs_builtin), sizeof(*filt->plugs));
   if (!filt->plugs)
      return false;
   filt->num_plugs = ARRAY_SIZE(soft_plugs_builtin);

   for (i = 0; i < ARRAY_SIZE(soft_plugs_builtin); i++)
   {
      filt->plugs[i].impl = soft_plugs_builtin[i](mask);
      if (!filt->plugs[i].impl)
         return false;
   }

   return true;
}
#endif

rarch_softfilter_t *rarch_softfilter_new(const char *filter_config,
     // unsigned threads,
      enum retro_pixel_format in_pixel_format,
      unsigned max_width, unsigned max_height)
{
#if defined(HAVE_DYLIB)
   char basedir[PATH_MAX];
#endif
   softfilter_simd_mask_t cpu_features = rarch_get_cpu_features();
   struct string_list *plugs = NULL;

   rarch_softfilter_t *filt = (rarch_softfilter_t*)calloc(1, sizeof(*filt));
   if (!filt)
      return NULL;

   filt->conf = config_file_new(filter_config);
   if (!filt->conf)
   {
      RARCH_ERR("[SoftFilter]: Did not find config: %s\n", filter_config);
      goto error;
   }

#if defined(HAVE_DYLIB)
   fill_pathname_basedir(basedir, filter_config, sizeof(basedir));

   plugs = dir_list_new(basedir, EXT_EXECUTABLES, false);
   if (!plugs)
      goto error;

   if (!append_softfilter_plugs(filt, plugs))
      goto error;

   string_list_free(plugs);
   plugs = NULL;
#else
   if (!append_softfilter_plugs(filt))
      goto error;
#endif

   if (!create_softfilter_graph(filt, in_pixel_format,
            max_width, max_height, cpu_features, 1))
      goto error;

   return filt;

error:
   string_list_free(plugs);
   rarch_softfilter_free(filt);
   return NULL;
}

void rarch_softfilter_free(rarch_softfilter_t *filt)
{
 //  unsigned i = 0;
//   (void)i;

   if (!filt)
      return;

   free(filt->packets);
   if (filt->impl && filt->impl_data)
      filt->impl->destroy(filt->impl_data);

#ifdef HAVE_DYLIB
   for (i = 0; i < filt->num_plugs; i++)
   {
      if (filt->plugs[i].lib)
         dylib_close(filt->plugs[i].lib);
   }
   free(filt->plugs);
#endif

   free(filt);
}

void rarch_softfilter_get_max_output_size(rarch_softfilter_t *filt,
      unsigned *width, unsigned *height)
{
   rarch_softfilter_get_output_size(filt, width, height,
         filt->max_width, filt->max_height);
}

void rarch_softfilter_get_output_size(rarch_softfilter_t *filt,
      unsigned *out_width, unsigned *out_height,
      unsigned width, unsigned height)
{
   if (filt && filt->impl && filt->impl->query_output_size)
      filt->impl->query_output_size(filt->impl_data, out_width,
            out_height, width, height);
}

enum retro_pixel_format rarch_softfilter_get_output_format(
      rarch_softfilter_t *filt)
{
   return filt->out_pix_fmt;
}

void rarch_softfilter_process(rarch_softfilter_t *filt,
      void *output, size_t output_stride,
      const void *input, unsigned width, unsigned height, size_t input_stride)
{
   //unsigned i;

   if (filt && filt->impl && filt->impl->get_work_packets)
   {
      filt->impl->get_work_packets(filt->impl_data, filt->packets,
            output, output_stride, input, width, height, input_stride);

      filt->packets->work(filt->impl_data, filt->packets->thread_data);
   }
}

