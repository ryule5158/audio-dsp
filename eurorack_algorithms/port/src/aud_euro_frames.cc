#include "aud_euro_frames.h"

#include <new>

#include "frames/keyframer.h"
#include "frames/poly_lfo.h"

namespace {

frames::Keyframer* Keyframer(aud_euro_frames_keyframer_t* context) {
  return reinterpret_cast<frames::Keyframer*>(context->storage);
}

const frames::Keyframer* Keyframer(
    const aud_euro_frames_keyframer_t* context) {
  return reinterpret_cast<const frames::Keyframer*>(context->storage);
}

frames::PolyLfo* Lfo(aud_euro_frames_lfo_t* context) {
  return reinterpret_cast<frames::PolyLfo*>(context->storage);
}

}  // namespace

static_assert(sizeof(frames::Keyframer) <=
    AUD_EURO_FRAMES_KEYFRAMER_CONTEXT_BYTES, "Frames keyframer context too small");
static_assert(sizeof(frames::PolyLfo) <= AUD_EURO_FRAMES_LFO_CONTEXT_BYTES,
    "Frames LFO context too small");

extern "C" aud_euro_result_t aud_euro_frames_keyframer_init(
    aud_euro_frames_keyframer_t* context) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  frames::Keyframer* keyframer = new (context->storage) frames::Keyframer();
  keyframer->Init();
  keyframer->Clear();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_frames_keyframer_clear(
    aud_euro_frames_keyframer_t* context) {
  if (context) Keyframer(context)->Clear();
}

extern "C" aud_euro_result_t aud_euro_frames_keyframer_set_channel(
    aud_euro_frames_keyframer_t* context,
    uint8_t channel,
    aud_euro_frames_easing_t easing,
    uint8_t response) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (channel >= frames::kNumChannels || easing > AUD_EURO_FRAMES_EASE_BOUNCE) {
    return AUD_EURO_ERROR_RANGE;
  }
  frames::ChannelSettings* settings = Keyframer(context)->mutable_settings(channel);
  settings->easing_curve = static_cast<frames::EasingCurve>(easing);
  settings->response = response;
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_frames_keyframer_add(
    aud_euro_frames_keyframer_t* context,
    uint16_t timestamp,
    const uint16_t values[AUD_EURO_FRAMES_CHANNELS]) {
  if (!context || !values) return AUD_EURO_ERROR_ARGUMENT;
  uint16_t copy[frames::kNumChannels];
  for (size_t i = 0; i < frames::kNumChannels; ++i) copy[i] = values[i];
  return Keyframer(context)->AddKeyframe(timestamp, copy)
      ? AUD_EURO_OK : AUD_EURO_ERROR_MEMORY;
}

extern "C" uint8_t aud_euro_frames_keyframer_remove(
    aud_euro_frames_keyframer_t* context, uint16_t timestamp) {
  return context && Keyframer(context)->RemoveKeyframe(timestamp);
}

extern "C" void aud_euro_frames_keyframer_evaluate(
    aud_euro_frames_keyframer_t* context,
    uint16_t timestamp,
    uint16_t levels[AUD_EURO_FRAMES_CHANNELS]) {
  if (!context || !levels) return;
  Keyframer(context)->Evaluate(timestamp);
  for (size_t i = 0; i < frames::kNumChannels; ++i) {
    levels[i] = Keyframer(context)->level(i);
  }
}

extern "C" uint16_t aud_euro_frames_keyframer_count(
    const aud_euro_frames_keyframer_t* context) {
  return context ? Keyframer(context)->num_keyframes() : 0;
}

extern "C" aud_euro_result_t aud_euro_frames_lfo_init(
    aud_euro_frames_lfo_t* context) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  frames::PolyLfo* lfo = new (context->storage) frames::PolyLfo();
  lfo->Init();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_frames_lfo_set_parameters(
    aud_euro_frames_lfo_t* context,
    uint16_t shape,
    uint16_t shape_spread,
    uint16_t spread,
    uint16_t coupling) {
  if (!context) return;
  Lfo(context)->set_shape(shape);
  Lfo(context)->set_shape_spread(shape_spread);
  Lfo(context)->set_spread(spread);
  Lfo(context)->set_coupling(coupling);
}

extern "C" void aud_euro_frames_lfo_render(
    aud_euro_frames_lfo_t* context,
    int32_t raw_frequency,
    uint16_t output[AUD_EURO_FRAMES_CHANNELS]) {
  if (!context || !output) return;
  Lfo(context)->Render(raw_frequency);
  for (size_t i = 0; i < frames::kNumChannels; ++i) {
    output[i] = Lfo(context)->dac_code(i);
  }
}
