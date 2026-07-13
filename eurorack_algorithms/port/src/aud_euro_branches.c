/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "aud_euro_branches.h"

#include <string.h>

void aud_euro_branches_init(aud_euro_branches_t* context, uint32_t seed) {
  if (!context) return;
  memset(context, 0, sizeof(*context));
  context->rng_state = seed ? seed : 1u;
}

aud_euro_branches_output_t aud_euro_branches_process(
    aud_euro_branches_t* context,
    uint8_t channel,
    uint8_t gate,
    float probability_a,
    uint8_t toggle_mode,
    uint8_t latch_mode) {
  aud_euro_branches_output_t output = { 0, 0, 0 };
  if (!context || channel >= 2) return output;
  if (probability_a < 0.0f) probability_a = 0.0f;
  if (probability_a > 1.0f) probability_a = 1.0f;
  gate = gate != 0;
  if (gate && !context->input[channel]) {
    const uint16_t random = (uint16_t)(context->rng_state >> (channel * 16));
    uint8_t outcome;
    if (probability_a <= 0.0f) {
      outcome = 0;
    } else if (probability_a >= 1.0f) {
      outcome = 1;
    } else {
      const uint32_t threshold = (uint32_t)(probability_a * 65536.0f);
      outcome = random < threshold;
    }
    if (toggle_mode) outcome ^= context->previous_outcome[channel];
    context->previous_outcome[channel] = outcome;
    context->output_a[channel] = outcome;
    context->output_b[channel] = !outcome;
    output.new_decision = 1;
  } else if (!gate && context->input[channel] && !latch_mode) {
    context->output_a[channel] = 0;
    context->output_b[channel] = 0;
  }
  context->input[channel] = gate;
  context->rng_state = (context->rng_state >> 1) ^
      ((uint32_t)-(int32_t)(context->rng_state & 1u) & 0xd0000001u);
  output.output_a = context->output_a[channel];
  output.output_b = context->output_b[channel];
  return output;
}
