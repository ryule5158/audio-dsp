#include "aud_euro_yarns.h"

#include <new>

#include "yarns/just_intonation_processor.h"

namespace {
struct Context {
  yarns::JustIntonationProcessor tuning;
  float sample_rate;
  uint32_t phase;
  uint32_t phase_increment;
  uint32_t swing_amount;
  uint8_t swing_step;
};
Context* Impl(aud_euro_yarns_t* context) {
  return reinterpret_cast<Context*>(context->storage);
}
}  // namespace

static_assert(sizeof(Context) <= AUD_EURO_YARNS_CONTEXT_BYTES,
    "AUD_EURO_YARNS_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_yarns_init(
    aud_euro_yarns_t* context, float control_sample_rate_hz) {
  if (!context || !(control_sample_rate_hz > 0.0f)) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  Context* state = new (context->storage) Context();
  state->tuning.Init();
  state->sample_rate = control_sample_rate_hz;
  state->phase = 0;
  state->phase_increment = 0;
  state->swing_amount = 0;
  state->swing_step = 11;
  return AUD_EURO_OK;
}

extern "C" int16_t aud_euro_yarns_note_on(
    aud_euro_yarns_t* context, uint8_t midi_note) {
  return context ? Impl(context)->tuning.NoteOn(midi_note) : 0;
}

extern "C" void aud_euro_yarns_note_off(
    aud_euro_yarns_t* context, uint8_t midi_note) {
  if (context) Impl(context)->tuning.NoteOff(midi_note);
}

extern "C" aud_euro_result_t aud_euro_yarns_clock_start(
    aud_euro_yarns_t* context, float bpm, float swing_percent) {
  if (!context || !(bpm > 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  if (swing_percent < 0.0f) swing_percent = 0.0f;
  if (swing_percent > 99.0f) swing_percent = 99.0f;
  Context* state = Impl(context);
  state->phase = 0;
  state->swing_step = 11;
  const double increment = 858993459.2 * bpm / state->sample_rate;
  if (increment > 4294967295.0) return AUD_EURO_ERROR_RANGE;
  state->phase_increment = static_cast<uint32_t>(increment);
  state->swing_amount = static_cast<uint32_t>(
      swing_percent * (2147483648.0 / 3.0 / 100.0));
  return AUD_EURO_OK;
}

extern "C" uint8_t aud_euro_yarns_clock_process(aud_euro_yarns_t* context) {
  if (!context) return 0;
  Context* state = Impl(context);
  uint32_t half_cycle = 0x80000000u;
  if (state->swing_step < 6) half_cycle += state->swing_amount;
  else half_cycle -= state->swing_amount;
  uint8_t tick = 0;
  if (state->phase >= half_cycle) {
    tick = 1;
    state->phase -= half_cycle;
    if (++state->swing_step >= 12) state->swing_step = 0;
  }
  state->phase += state->phase_increment;
  return tick;
}
