/*
 * Edges-inspired chiptune oscillator and timer helper. This portable renderer
 * preserves the useful waveform/noise/bitcrush behavior, but it is not a
 * cycle-exact emulation of the original AVR timer and GPIO implementation.
 * GPL-3.0-or-later module.
 */
#ifndef AUD_EURO_EDGES_H_
#define AUD_EURO_EDGES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AUD_EURO_EDGES_TRIANGLE = 0,
  AUD_EURO_EDGES_NES_TRIANGLE = 1,
  AUD_EURO_EDGES_PITCHED_NOISE = 2,
  AUD_EURO_EDGES_NES_NOISE_LONG = 3,
  AUD_EURO_EDGES_NES_NOISE_SHORT = 4,
  AUD_EURO_EDGES_SINE = 5
} aud_euro_edges_shape_t;

typedef struct {
  float sample_rate;
  float frequency;
  float phase;
  float held_sample;
  uint32_t lfsr;
  uint32_t decimation_counter;
  uint32_t decimation_period;
  aud_euro_edges_shape_t shape;
  uint8_t gate;
} aud_euro_edges_t;

aud_euro_result_t aud_euro_edges_init(
    aud_euro_edges_t* context, float sample_rate_hz);
aud_euro_result_t aud_euro_edges_set(
    aud_euro_edges_t* context,
    float frequency_hz,
    aud_euro_edges_shape_t shape,
    uint8_t gate);
void aud_euro_edges_set_bitcrush(
    aud_euro_edges_t* context, uint8_t amount);
void aud_euro_edges_process(
    aud_euro_edges_t* context, int16_t* output, size_t frames);
aud_euro_result_t aud_euro_edges_timer_parameters(
    float timer_clock_hz,
    float frequency_hz,
    float pulse_width,
    uint32_t* period,
    uint32_t* compare);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_EDGES_H_ */
