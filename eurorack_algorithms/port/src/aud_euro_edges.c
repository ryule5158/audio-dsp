/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "aud_euro_edges.h"

#include <math.h>

static float next_noise(aud_euro_edges_t* context, uint8_t short_mode) {
  uint32_t tap = short_mode ? 6u : 1u;
  uint32_t bit = (context->lfsr ^ (context->lfsr >> tap)) & 1u;
  context->lfsr = (context->lfsr >> 1) | (bit << 14);
  return (context->lfsr & 1u) ? 0.8f : -0.8f;
}

aud_euro_result_t aud_euro_edges_init(
    aud_euro_edges_t* context, float sample_rate_hz) {
  if (!context || !(sample_rate_hz > 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  context->sample_rate = sample_rate_hz;
  context->frequency = 261.6256f;
  context->phase = 0.0f;
  context->held_sample = 0.0f;
  context->lfsr = 1u;
  context->decimation_counter = 0;
  context->decimation_period = 1;
  context->shape = AUD_EURO_EDGES_TRIANGLE;
  context->gate = 1;
  return AUD_EURO_OK;
}

aud_euro_result_t aud_euro_edges_set(
    aud_euro_edges_t* context,
    float frequency_hz,
    aud_euro_edges_shape_t shape,
    uint8_t gate) {
  if (!context || !(frequency_hz >= 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  if (shape > AUD_EURO_EDGES_SINE) {
    return AUD_EURO_ERROR_RANGE;
  }
  context->frequency = frequency_hz;
  context->shape = shape;
  context->gate = gate != 0;
  return AUD_EURO_OK;
}

void aud_euro_edges_set_bitcrush(aud_euro_edges_t* context, uint8_t amount) {
  if (!context) return;
  context->decimation_period = 1u + ((uint32_t)amount * amount >> 9);
}

void aud_euro_edges_process(
    aud_euro_edges_t* context, int16_t* output, size_t frames) {
  if (!context || !output) return;
  const float increment = context->frequency / context->sample_rate;
  for (size_t i = 0; i < frames; ++i) {
    if (!context->gate) {
      output[i] = 0;
      continue;
    }
    float previous = context->phase;
    context->phase += increment;
    if (context->phase >= 1.0f) context->phase -= floorf(context->phase);
    if (++context->decimation_counter >= context->decimation_period) {
      context->decimation_counter = 0;
      float sample;
      if (context->shape == AUD_EURO_EDGES_TRIANGLE) {
        sample = 1.0f - 4.0f * fabsf(context->phase - 0.5f);
      } else if (context->shape == AUD_EURO_EDGES_NES_TRIANGLE) {
        float triangle = 1.0f - 4.0f * fabsf(context->phase - 0.5f);
        sample = floorf((triangle + 1.0f) * 7.5f) / 7.5f - 1.0f;
      } else if (context->shape == AUD_EURO_EDGES_PITCHED_NOISE) {
        sample = context->phase < previous ? next_noise(context, 0) : context->held_sample;
      } else if (context->shape == AUD_EURO_EDGES_NES_NOISE_LONG) {
        sample = context->phase < previous ? next_noise(context, 0) : context->held_sample;
      } else if (context->shape == AUD_EURO_EDGES_NES_NOISE_SHORT) {
        sample = context->phase < previous ? next_noise(context, 1) : context->held_sample;
      } else {
        sample = sinf(context->phase * 6.2831853071795864769f);
      }
      context->held_sample = sample;
    }
    float sample = context->held_sample;
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;
    output[i] = (int16_t)(sample * 32767.0f);
  }
}

aud_euro_result_t aud_euro_edges_timer_parameters(
    float timer_clock_hz,
    float frequency_hz,
    float pulse_width,
    uint32_t* period,
    uint32_t* compare) {
  if (!(timer_clock_hz > 0.0f) || !(frequency_hz > 0.0f) ||
      !period || !compare) return AUD_EURO_ERROR_ARGUMENT;
  if (pulse_width < 0.01f) pulse_width = 0.01f;
  if (pulse_width > 0.99f) pulse_width = 0.99f;
  double ticks = timer_clock_hz / frequency_hz;
  if (ticks < 2.0 || ticks > 4294967295.0) return AUD_EURO_ERROR_RANGE;
  *period = (uint32_t)(ticks - 1.0);
  *compare = (uint32_t)(ticks * pulse_width);
  return AUD_EURO_OK;
}
