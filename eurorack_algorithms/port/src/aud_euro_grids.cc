/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "aud_euro_grids.h"

#include "avrlib/random.h"
#include "grids/pattern_generator.h"

extern "C" void aud_euro_grids_default_settings(
    aud_euro_grids_settings_t* s) {
  if (!s) return;
  s->x = 128;
  s->y = 128;
  s->randomness = 0;
  s->density[0] = 128;
  s->density[1] = 128;
  s->density[2] = 128;
  s->euclidean_length[0] = 127;
  s->euclidean_length[1] = 127;
  s->euclidean_length[2] = 127;
  s->mode = AUD_EURO_GRIDS_DRUMS;
  s->output_clock = 0;
  s->gate_mode = 0;
  s->swing = 0;
}

extern "C" void aud_euro_grids_init(uint16_t seed) {
  avrlib::Random::Seed(seed);
  grids::PatternGenerator::Init();
  grids::PatternGenerator::set_clock_resolution(grids::CLOCK_RESOLUTION_24_PPQN);
}

extern "C" void aud_euro_grids_configure(
    const aud_euro_grids_settings_t* s) {
  if (!s) return;
  grids::PatternGeneratorSettings* native =
      grids::PatternGenerator::mutable_settings();
  native->options.drums.x = s->x;
  native->options.drums.y = s->y;
  native->options.drums.randomness = s->randomness;
  for (size_t i = 0; i < 3; ++i) {
    native->density[i] = s->density[i];
    if (s->mode == AUD_EURO_GRIDS_EUCLIDEAN) {
      native->options.euclidean_length[i] = s->euclidean_length[i];
    }
  }
  grids::PatternGenerator::set_output_mode(s->mode);
  grids::PatternGenerator::set_output_clock(s->output_clock);
  grids::PatternGenerator::set_gate_mode(s->gate_mode != 0);
  grids::PatternGenerator::set_swing(s->swing);
}

extern "C" uint8_t aud_euro_grids_tick(uint8_t pulses) {
  grids::PatternGenerator::TickClock(pulses);
  return grids::PatternGenerator::state();
}

extern "C" void aud_euro_grids_retrigger(void) {
  grids::PatternGenerator::Retrigger();
}

extern "C" void aud_euro_grids_clock_falling_edge(void) {
  grids::PatternGenerator::ClockFallingEdge();
}

extern "C" void aud_euro_grids_pulse_timer_tick(void) {
  grids::PatternGenerator::IncrementPulseCounter();
}

extern "C" uint8_t aud_euro_grids_step(void) {
  return grids::PatternGenerator::step();
}
