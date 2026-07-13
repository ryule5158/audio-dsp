#include "aud_euro_streams.h"

static aud_euro_streams_t streams;

int streams_example_init(void) {
  if (aud_euro_streams_init(&streams, 0) != AUD_EURO_OK) return -1;
  aud_euro_streams_set_parameter(&streams, 0, 32768);
  aud_euro_streams_set_parameter(&streams, 1, 32768);
  return aud_euro_streams_set_function(&streams, AUD_EURO_STREAMS_COMPRESSOR);
}

void streams_example_control(
    const int16_t* audio,
    const int16_t* sidechain,
    uint16_t* gain,
    uint16_t* filter_frequency,
    size_t frames) {
  aud_euro_streams_process(
      &streams, audio, sidechain, gain, filter_frequency, frames);
}
