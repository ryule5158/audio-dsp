#include "aud_euro_tides.h"

#include <new>

#include "tides/generator.h"

namespace {
tides::Generator* Impl(aud_euro_tides_t* context) {
  return reinterpret_cast<tides::Generator*>(context->storage);
}
}  // namespace

static_assert(sizeof(tides::Generator) <= AUD_EURO_TIDES_CONTEXT_BYTES,
    "AUD_EURO_TIDES_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_tides_init(aud_euro_tides_t* context) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  tides::Generator* generator = new (context->storage) tides::Generator();
  generator->Init();
  generator->Process();
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_tides_set_range(
    aud_euro_tides_t* context, aud_euro_tides_range_t range) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (range < AUD_EURO_TIDES_RANGE_HIGH || range > AUD_EURO_TIDES_RANGE_LOW) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_range(static_cast<tides::GeneratorRange>(range));
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_tides_set_mode(
    aud_euro_tides_t* context, aud_euro_tides_mode_t mode) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (mode < AUD_EURO_TIDES_AD || mode > AUD_EURO_TIDES_AR) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_mode(static_cast<tides::GeneratorMode>(mode));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_tides_set_parameters(
    aud_euro_tides_t* context,
    int16_t pitch_q7,
    int16_t slope,
    int16_t shape,
    int16_t smoothness,
    uint8_t clock_sync) {
  if (!context) return;
  Impl(context)->set_pitch(pitch_q7);
  Impl(context)->set_slope(slope);
  Impl(context)->set_shape(shape);
  Impl(context)->set_smoothness(smoothness);
  Impl(context)->set_sync(clock_sync != 0);
}

extern "C" void aud_euro_tides_process(
    aud_euro_tides_t* context,
    const uint8_t* controls,
    aud_euro_tides_sample_t* output,
    size_t frames) {
  if (!context || !controls || !output) return;
  tides::Generator* generator = Impl(context);
  for (size_t i = 0; i < frames; ++i) {
    if (generator->writable_block()) generator->Process();
    const tides::GeneratorSample& sample = generator->Process(controls[i]);
    output[i].unipolar = sample.unipolar;
    output[i].bipolar = sample.bipolar;
    output[i].flags = sample.flags;
  }
  if (generator->writable_block()) generator->Process();
}
