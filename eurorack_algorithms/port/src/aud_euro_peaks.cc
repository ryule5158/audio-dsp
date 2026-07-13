#include "aud_euro_peaks.h"

#include <new>

#include "peaks/processors.h"

namespace {
peaks::Processors* Impl(aud_euro_peaks_t* context) {
  return reinterpret_cast<peaks::Processors*>(context->storage);
}
}  // namespace

static_assert(sizeof(peaks::Processors) <= AUD_EURO_PEAKS_CONTEXT_BYTES,
    "AUD_EURO_PEAKS_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_peaks_init(
    aud_euro_peaks_t* context, uint8_t channel_index) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  peaks::Processors* processors = new (context->storage) peaks::Processors();
  processors->Init(channel_index);
  processors->set_control_mode(peaks::CONTROL_MODE_FULL);
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_peaks_set_function(
    aud_euro_peaks_t* context, aud_euro_peaks_function_t function) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (function < AUD_EURO_PEAKS_ENVELOPE ||
      function > AUD_EURO_PEAKS_NUMBER_STATION) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_function(static_cast<peaks::ProcessorFunction>(function));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_peaks_set_full_control(
    aud_euro_peaks_t* context, uint8_t full) {
  if (context) Impl(context)->set_control_mode(
      full ? peaks::CONTROL_MODE_FULL : peaks::CONTROL_MODE_HALF);
}

extern "C" aud_euro_result_t aud_euro_peaks_set_parameter(
    aud_euro_peaks_t* context, uint8_t index, uint16_t value) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (index >= 4) return AUD_EURO_ERROR_RANGE;
  Impl(context)->set_parameter(index, value);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_peaks_process(
    aud_euro_peaks_t* context,
    const uint8_t* gate_flags,
    int16_t* output,
    size_t frames) {
  if (context && gate_flags && output) {
    Impl(context)->Process(gate_flags, output, frames);
  }
}
