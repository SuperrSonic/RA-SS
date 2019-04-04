#include "resampler.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
 
#if !defined(RESAMPLER_TEST) && defined(RARCH_INTERNAL)
#include "../../general.h"
#else
/* FIXME - variadic macros not supported for MSVC 2003 */
#define RARCH_LOG(...) fprintf(stderr, __VA_ARGS__)
#endif
 
typedef struct rarch_nearest_resampler
{
   float fraction;
} rarch_nearest_resampler_t;
 
static void resampler_nearest_process(
      void *re_, struct resampler_data *data)
{
   float ratio;
   rarch_nearest_resampler_t *re = (rarch_nearest_resampler_t*)re_;
   audio_frame_float_t *inp     = (audio_frame_float_t*)data->data_in;
   audio_frame_float_t *inp_max = (audio_frame_float_t*)inp + data->input_frames;
   audio_frame_float_t *outp    = (audio_frame_float_t*)data->data_out;

   ratio = 1.0 / data->ratio;
 
   while(inp != inp_max)
   {
      while(re->fraction > 1)
      {
         *outp++ = *inp;
         re->fraction -= ratio;
      }
      re->fraction++;
      inp++;      
   }
   
   data->output_frames = (outp - (audio_frame_float_t*)data->data_out);
}
 
static void resampler_nearest_free(void *re_)
{
   rarch_nearest_resampler_t *re = (rarch_nearest_resampler_t*)re_;
   if (re)
      free(re);
}
 
static void *resampler_nearest_init(const struct resampler_config *config,
      double bandwidth_mod, resampler_simd_mask_t mask)
{
   rarch_nearest_resampler_t *re = (rarch_nearest_resampler_t*)
      calloc(1, sizeof(rarch_nearest_resampler_t));

   (void)config;
   (void)mask;

   if (!re)
      return NULL;
   
   re->fraction = 0;
   
   RARCH_LOG("\nNearest resampler : \n");
 
   return re;
}
 
rarch_resampler_t nearest_resampler = {
   resampler_nearest_init,
   resampler_nearest_process,
   resampler_nearest_free,
   RESAMPLER_API_VERSION,
   "Nearest",
   "nearest"
};
