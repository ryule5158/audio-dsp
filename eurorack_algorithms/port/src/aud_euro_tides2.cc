#include "aud_euro_tides2.h"

#include <new>

#include "tides2/poly_slope_generator.h"
#include "tides2/ramp_generator.h"

namespace {
struct Context {
  float sample_rate;
  tides::PolySlopeGenerator generator;
};
Context* Impl(aud_euro_tides2_t* context) {
  return reinterpret_cast<Context*>(context->storage);
}
}  // namespace

static_assert(sizeof(Context) <= AUD_EURO_TIDES2_CONTEXT_BYTES,
    "AUD_EURO_TIDES2_CONTEXT_BYTES is too small");
static_assert(sizeof(tides::PolySlopeGenerator::OutputSample) ==
    sizeof(aud_euro_tides2_sample_t), "Tides 2 sample ABI mismatch");

extern "C" aud_euro_result_t aud_euro_tides2_init(
    aud_euro_tides2_t* context, float sample_rate_hz) {
  if (!context || !(sample_rate_hz > 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  Context* state = new (context->storage) Context();
  state->sample_rate = sample_rate_hz;
  state->generator.Init();
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_tides2_process(
    aud_euro_tides2_t* context,
    aud_euro_tides2_ramp_mode_t ramp_mode,
    aud_euro_tides2_output_mode_t output_mode,
    uint8_t audio_range,
    float frequency_hz,
    float slope,
    float shape,
    float smoothness,
    float shift,
    const uint8_t* gate_flags,
    const float* external_ramp,
    aud_euro_tides2_sample_t* output,
    size_t frames) {
  if (!context || !gate_flags || !output) return AUD_EURO_ERROR_ARGUMENT;
  if (ramp_mode > AUD_EURO_TIDES2_AR ||
      output_mode > AUD_EURO_TIDES2_FREQUENCY) return AUD_EURO_ERROR_RANGE;
  Context* state = Impl(context);
  state->generator.Render(
      static_cast<tides::RampMode>(ramp_mode),
      static_cast<tides::OutputMode>(output_mode),
      audio_range ? tides::RANGE_AUDIO : tides::RANGE_CONTROL,
      frequency_hz / state->sample_rate,
      slope,
      shape,
      smoothness,
      shift,
      gate_flags,
      external_ramp,
      reinterpret_cast<tides::PolySlopeGenerator::OutputSample*>(output),
      frames);
  return AUD_EURO_OK;
}
