#include "aud_euro_warps.h"

#include <new>

#include "warps/dsp/modulator.h"

namespace {

warps::Modulator* Impl(aud_euro_warps_t* context) {
  return reinterpret_cast<warps::Modulator*>(context->storage);
}

void ProcessBlock(
    warps::Modulator* modulator,
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t size) {
  modulator->Process(
      reinterpret_cast<warps::ShortFrame*>(
          const_cast<aud_euro_stereo_i16_t*>(input)),
      reinterpret_cast<warps::ShortFrame*>(output),
      size);
}

}  // namespace

static_assert(sizeof(warps::Modulator) <= AUD_EURO_WARPS_CONTEXT_BYTES,
    "AUD_EURO_WARPS_CONTEXT_BYTES is too small");
static_assert(sizeof(warps::ShortFrame) == sizeof(aud_euro_stereo_i16_t),
    "Stereo frame ABI mismatch");

extern "C" void aud_euro_warps_default_parameters(
    aud_euro_warps_parameters_t* p) {
  if (!p) return;
  p->carrier_drive = 1.0f;
  p->modulator_drive = 1.0f;
  p->algorithm = 0.0f;
  p->timbre = 0.5f;
  p->frequency_shift = 0.5f;
  p->frequency_shift_cv = 0.0f;
  p->phase_shift = 0.0f;
  p->note = 48.0f;
  p->carrier_shape = 0;
}

extern "C" aud_euro_result_t aud_euro_warps_init(
    aud_euro_warps_t* context, float sample_rate_hz) {
  if (!context || !(sample_rate_hz > 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  warps::Modulator* modulator = new (context->storage) warps::Modulator();
  modulator->Init(sample_rate_hz);
  aud_euro_warps_parameters_t defaults;
  aud_euro_warps_default_parameters(&defaults);
  aud_euro_warps_set_parameters(context, &defaults);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_warps_deinit(aud_euro_warps_t* context) {
  if (context) Impl(context)->~Modulator();
}

extern "C" void aud_euro_warps_set_parameters(
    aud_euro_warps_t* context, const aud_euro_warps_parameters_t* p) {
  if (!context || !p) return;
  warps::Parameters* d = Impl(context)->mutable_parameters();
  d->channel_drive[0] = p->carrier_drive;
  d->channel_drive[1] = p->modulator_drive;
  d->modulation_algorithm = p->algorithm;
  d->modulation_parameter = p->timbre;
  d->frequency_shift_pot = p->frequency_shift;
  d->frequency_shift_cv = p->frequency_shift_cv;
  d->phase_shift = p->phase_shift;
  d->note = p->note;
  d->carrier_shape = p->carrier_shape;
}

extern "C" void aud_euro_warps_set_frequency_shifter(
    aud_euro_warps_t* context, uint8_t enabled) {
  if (context) Impl(context)->set_easter_egg(enabled != 0);
}

extern "C" aud_euro_result_t aud_euro_warps_process(
    aud_euro_warps_t* context,
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t frames) {
  if (!context || !input || !output) return AUD_EURO_ERROR_ARGUMENT;
  if (frames % 3) return AUD_EURO_ERROR_RANGE;
  warps::Modulator* modulator = Impl(context);
  while (frames) {
    size_t block = frames > warps::kMaxBlockSize ? warps::kMaxBlockSize : frames;
    block -= block % 3;
    ProcessBlock(modulator, input, output, block);
    input += block;
    output += block;
    frames -= block;
  }
  return AUD_EURO_OK;
}
