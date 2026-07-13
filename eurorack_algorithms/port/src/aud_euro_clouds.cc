#include "aud_euro_clouds.h"

#include <new>

#include "clouds/dsp/frame.h"
#include "clouds/dsp/granular_processor.h"

namespace {

clouds::GranularProcessor* Impl(aud_euro_clouds_t* context) {
  return reinterpret_cast<clouds::GranularProcessor*>(context->storage);
}

void CopyParameters(
    clouds::Parameters* destination,
    const aud_euro_clouds_parameters_t& source) {
  destination->position = source.position;
  destination->size = source.size;
  destination->pitch = source.pitch_semitones;
  destination->density = source.density;
  destination->texture = source.texture;
  destination->dry_wet = source.dry_wet;
  destination->stereo_spread = source.stereo_spread;
  destination->feedback = source.feedback;
  destination->reverb = source.reverb;
  destination->freeze = source.freeze != 0;
  destination->trigger = source.trigger != 0;
  destination->gate = source.gate != 0;
}

}  // namespace

static_assert(
    sizeof(clouds::GranularProcessor) <= AUD_EURO_CLOUDS_CONTEXT_BYTES,
    "AUD_EURO_CLOUDS_CONTEXT_BYTES is too small");
static_assert(
    sizeof(clouds::ShortFrame) == sizeof(aud_euro_stereo_i16_t),
    "Stereo frame ABI mismatch");

extern "C" void aud_euro_clouds_default_parameters(
    aud_euro_clouds_parameters_t* p) {
  if (!p) return;
  p->position = 0.0f;
  p->size = 0.5f;
  p->pitch_semitones = 0.0f;
  p->density = 0.5f;
  p->texture = 0.5f;
  p->dry_wet = 1.0f;
  p->stereo_spread = 0.0f;
  p->feedback = 0.0f;
  p->reverb = 0.0f;
  p->freeze = 0;
  p->trigger = 0;
  p->gate = 0;
}

extern "C" aud_euro_result_t aud_euro_clouds_init(
    aud_euro_clouds_t* context,
    void* large_buffer,
    size_t large_buffer_bytes,
    void* small_buffer,
    size_t small_buffer_bytes) {
  if (!context || !large_buffer || !small_buffer) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  if ((reinterpret_cast<uintptr_t>(large_buffer) & 7u) ||
      (reinterpret_cast<uintptr_t>(small_buffer) & 7u)) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  if (large_buffer_bytes < AUD_EURO_CLOUDS_LARGE_BUFFER_BYTES ||
      small_buffer_bytes < AUD_EURO_CLOUDS_SMALL_BUFFER_BYTES) {
    return AUD_EURO_ERROR_MEMORY;
  }
  clouds::GranularProcessor* processor = new (context->storage)
      clouds::GranularProcessor();
  processor->Init(
      large_buffer, large_buffer_bytes, small_buffer, small_buffer_bytes);
  processor->set_num_channels(2);
  processor->set_low_fidelity(false);
  processor->set_playback_mode(clouds::PLAYBACK_MODE_GRANULAR);
  aud_euro_clouds_parameters_t defaults;
  aud_euro_clouds_default_parameters(&defaults);
  CopyParameters(processor->mutable_parameters(), defaults);
  processor->Prepare();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_clouds_deinit(aud_euro_clouds_t* context) {
  if (context) Impl(context)->~GranularProcessor();
}

extern "C" aud_euro_result_t aud_euro_clouds_set_mode(
    aud_euro_clouds_t* context, aud_euro_clouds_mode_t mode) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (mode < AUD_EURO_CLOUDS_GRANULAR || mode > AUD_EURO_CLOUDS_SPECTRAL) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_playback_mode(static_cast<clouds::PlaybackMode>(mode));
  Impl(context)->Prepare();
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_clouds_set_quality(
    aud_euro_clouds_t* context, uint8_t mono, uint8_t low_fidelity) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  Impl(context)->set_num_channels(mono ? 1 : 2);
  Impl(context)->set_low_fidelity(low_fidelity != 0);
  Impl(context)->Prepare();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_clouds_set_parameters(
    aud_euro_clouds_t* context,
    const aud_euro_clouds_parameters_t* parameters) {
  if (context && parameters) {
    CopyParameters(Impl(context)->mutable_parameters(), *parameters);
  }
}

extern "C" void aud_euro_clouds_process(
    aud_euro_clouds_t* context,
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t frames) {
  if (!context || !input || !output) return;
  clouds::GranularProcessor* processor = Impl(context);
  while (frames) {
    const size_t block = frames > clouds::kMaxBlockSize
        ? clouds::kMaxBlockSize : frames;
    processor->Process(
        reinterpret_cast<clouds::ShortFrame*>(
            const_cast<aud_euro_stereo_i16_t*>(input)),
        reinterpret_cast<clouds::ShortFrame*>(output),
        block);
    processor->Prepare();
    processor->mutable_parameters()->trigger = false;
    input += block;
    output += block;
    frames -= block;
  }
}
