#include "aud_euro_streams.h"

#include <new>

#include "streams/processor.h"

namespace {
streams::Processor* Impl(aud_euro_streams_t* context) {
  return reinterpret_cast<streams::Processor*>(context->storage);
}
const streams::Processor* Impl(const aud_euro_streams_t* context) {
  return reinterpret_cast<const streams::Processor*>(context->storage);
}
}  // namespace

static_assert(sizeof(streams::Processor) <= AUD_EURO_STREAMS_CONTEXT_BYTES,
    "AUD_EURO_STREAMS_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_streams_init(
    aud_euro_streams_t* context, uint8_t channel_index) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  streams::Processor* processor = new (context->storage) streams::Processor();
  processor->Init(channel_index);
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_streams_set_function(
    aud_euro_streams_t* context, aud_euro_streams_function_t function) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (function < AUD_EURO_STREAMS_ENVELOPE || function > AUD_EURO_STREAMS_LORENZ) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_function(static_cast<streams::ProcessorFunction>(function));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_streams_set_alternate(
    aud_euro_streams_t* context, uint8_t enabled) {
  if (context) Impl(context)->set_alternate(enabled != 0);
}

extern "C" aud_euro_result_t aud_euro_streams_set_parameter(
    aud_euro_streams_t* context, uint8_t index, uint16_t value) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (index >= 2) return AUD_EURO_ERROR_RANGE;
  Impl(context)->set_parameter(index, value);
  Impl(context)->Configure();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_streams_process(
    aud_euro_streams_t* context,
    const int16_t* audio,
    const int16_t* excite,
    uint16_t* gain,
    uint16_t* filter_frequency,
    size_t frames) {
  if (!context || !audio || !excite || !gain || !filter_frequency) return;
  streams::Processor* processor = Impl(context);
  processor->Configure();
  for (size_t i = 0; i < frames; ++i) {
    processor->Process(audio[i], excite[i], &gain[i], &filter_frequency[i]);
  }
}

extern "C" int32_t aud_euro_streams_gain_reduction(
    const aud_euro_streams_t* context) {
  return context ? Impl(context)->gain_reduction() : 0;
}
