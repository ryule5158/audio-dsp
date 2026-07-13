#include "aud_euro_stages.h"

#include <new>

#include "stages/segment_generator.h"

namespace {
stages::SegmentGenerator* Impl(aud_euro_stages_t* context) {
  return reinterpret_cast<stages::SegmentGenerator*>(context->storage);
}
}  // namespace

static_assert(sizeof(stages::SegmentGenerator) <= AUD_EURO_STAGES_CONTEXT_BYTES,
    "AUD_EURO_STAGES_CONTEXT_BYTES is too small");
static_assert(sizeof(stages::SegmentGenerator::Output) ==
    sizeof(aud_euro_stages_output_t), "Stages output ABI mismatch");

extern "C" aud_euro_result_t aud_euro_stages_init(aud_euro_stages_t* context) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  stages::SegmentGenerator* generator = new (context->storage)
      stages::SegmentGenerator();
  generator->Init();
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_stages_configure(
    aud_euro_stages_t* context,
    uint8_t has_trigger,
    const aud_euro_stages_segment_t* segments,
    size_t segment_count,
    uint8_t sequencer_mode) {
  if (!context || !segments) return AUD_EURO_ERROR_ARGUMENT;
  if (segment_count == 0 || segment_count > AUD_EURO_STAGES_MAX_SEGMENTS) {
    return AUD_EURO_ERROR_RANGE;
  }
  stages::segment::Configuration config[AUD_EURO_STAGES_MAX_SEGMENTS];
  for (size_t i = 0; i < segment_count; ++i) {
    if (segments[i].type > AUD_EURO_STAGES_ALTERNATING) {
      return AUD_EURO_ERROR_RANGE;
    }
    config[i].type = static_cast<stages::segment::Type>(segments[i].type);
    config[i].loop = segments[i].loop != 0;
    Impl(context)->set_segment_parameters(
        static_cast<int>(i), segments[i].primary, segments[i].secondary);
  }
  if (sequencer_mode) {
    Impl(context)->ConfigureSequencer(config, static_cast<int>(segment_count));
  } else {
    Impl(context)->Configure(
        has_trigger != 0, config, static_cast<int>(segment_count));
  }
  return AUD_EURO_OK;
}

extern "C" uint8_t aud_euro_stages_process(
    aud_euro_stages_t* context,
    const uint8_t* gate_flags,
    aud_euro_stages_output_t* output,
    size_t frames) {
  if (!context || !gate_flags || !output) return 0;
  uint8_t active = 0;
  while (frames) {
    const size_t block = frames > 32 ? 32 : frames;
    active |= Impl(context)->Process(
        gate_flags,
        reinterpret_cast<stages::SegmentGenerator::Output*>(output),
        block);
    gate_flags += block;
    output += block;
    frames -= block;
  }
  return active;
}
