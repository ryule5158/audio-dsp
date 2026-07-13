#include "aud_euro_warps.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_LARGE_RAM aud_euro_warps_t warps;
static aud_euro_warps_parameters_t warps_parameters;

int warps_example_init(float sample_rate_hz) {
  aud_euro_warps_default_parameters(&warps_parameters);
  warps_parameters.algorithm = 0.5f;
  warps_parameters.timbre = 0.5f;
  if (aud_euro_warps_init(&warps, sample_rate_hz) != AUD_EURO_OK) return -1;
  aud_euro_warps_set_parameters(&warps, &warps_parameters);
  return 0;
}

void warps_example_audio(
    const aud_euro_stereo_i16_t* two_inputs,
    aud_euro_stereo_i16_t* main_and_aux,
    size_t frames) {
  aud_euro_warps_process(&warps, two_inputs, main_and_aux, frames);
}
