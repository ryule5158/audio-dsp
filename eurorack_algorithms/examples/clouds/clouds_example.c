#include "aud_euro_clouds.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_FAST_RAM aud_euro_clouds_t clouds;
static AUD_EXAMPLE_LARGE_RAM aud_euro_clouds_large_buffer_t clouds_large;
static AUD_EXAMPLE_LARGE_RAM aud_euro_clouds_small_buffer_t clouds_small;
static aud_euro_clouds_parameters_t clouds_parameters;

int clouds_example_init(void) {
  aud_euro_clouds_default_parameters(&clouds_parameters);
  clouds_parameters.dry_wet = 0.75f;
  return aud_euro_clouds_init(
      &clouds, clouds_large.storage, sizeof(clouds_large.storage),
      clouds_small.storage, sizeof(clouds_small.storage));
}

/* Input/output are interleaved signed 16-bit stereo at native 32 kHz. */
void clouds_example_audio(
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t frames) {
  aud_euro_clouds_set_parameters(&clouds, &clouds_parameters);
  aud_euro_clouds_process(&clouds, input, output, frames);
  clouds_parameters.trigger = 0;
}
