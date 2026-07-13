#include "aud_euro_braids.h"

#include <math.h>
#include <new>

#include "braids/macro_oscillator.h"

namespace {

braids::MacroOscillator* Impl(aud_euro_braids_t* context) {
  return reinterpret_cast<braids::MacroOscillator*>(context->storage);
}

float Clamp01(float value) {
  return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

}  // namespace

static_assert(
    sizeof(braids::MacroOscillator) <= AUD_EURO_BRAIDS_CONTEXT_BYTES,
    "AUD_EURO_BRAIDS_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_braids_init(
    aud_euro_braids_t* context) {
  if (!context) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  braids::MacroOscillator* oscillator = new (context->storage)
      braids::MacroOscillator();
  oscillator->Init();
  oscillator->set_shape(braids::MACRO_OSC_SHAPE_CSAW);
  oscillator->set_pitch(60 << 7);
  oscillator->set_parameters(16384, 16384);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_braids_deinit(aud_euro_braids_t* context) {
  if (context) {
    Impl(context)->~MacroOscillator();
  }
}

extern "C" aud_euro_result_t aud_euro_braids_set_shape(
    aud_euro_braids_t* context, uint8_t shape) {
  if (!context) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  if (shape >= braids::MACRO_OSC_SHAPE_LAST) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_shape(static_cast<braids::MacroOscillatorShape>(shape));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_braids_set_pitch_q7(
    aud_euro_braids_t* context, int16_t midi_note_q7) {
  if (context) {
    Impl(context)->set_pitch(midi_note_q7);
  }
}

extern "C" aud_euro_result_t aud_euro_braids_set_frequency_hz(
    aud_euro_braids_t* context, float frequency_hz) {
  if (!context || !(frequency_hz > 0.0f)) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  float note = 69.0f + 12.0f * log2f(frequency_hz / 440.0f);
  if (note < -128.0f) note = -128.0f;
  if (note > 127.0f) note = 127.0f;
  Impl(context)->set_pitch(static_cast<int16_t>(note * 128.0f));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_braids_set_parameters(
    aud_euro_braids_t* context, float timbre, float color) {
  if (!context) return;
  Impl(context)->set_parameters(
      static_cast<int16_t>(Clamp01(timbre) * 32767.0f),
      static_cast<int16_t>(Clamp01(color) * 32767.0f));
}

extern "C" void aud_euro_braids_trigger(aud_euro_braids_t* context) {
  if (context) {
    Impl(context)->Strike();
  }
}

extern "C" void aud_euro_braids_process(
    aud_euro_braids_t* context,
    const uint8_t* sync,
    int16_t* output,
    size_t frames) {
  if (!context || !output) return;
  uint8_t no_sync[24] = { 0 };
  while (frames) {
    const size_t block = frames > 24 ? 24 : frames;
    Impl(context)->Render(sync ? sync : no_sync, output, block);
    if (sync) sync += block;
    output += block;
    frames -= block;
  }
}
