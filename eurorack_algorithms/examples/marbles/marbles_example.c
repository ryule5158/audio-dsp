#include "aud_euro_marbles.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_FAST_RAM aud_euro_marbles_t marbles;
static aud_euro_marbles_t_parameters_t t_parameters;

int marbles_example_init(float sample_rate_hz) {
  aud_euro_marbles_default_t_parameters(&t_parameters);
  t_parameters.bias = 0.6f;
  return aud_euro_marbles_init(&marbles, sample_rate_hz, 0x1234abcdu);
}

void marbles_example_random_gates(
    const uint8_t* external_clock,
    aud_euro_marbles_ramp_sample_t* ramps,
    uint8_t* two_interleaved_gates,
    size_t frames) {
  aud_euro_marbles_process_t(
      &marbles, &t_parameters, 1, external_clock,
      ramps, two_interleaved_gates, frames);
}
