#include "aud_euro_tides.h"

static aud_euro_tides_t tides;

int tides_example_init(void) {
  if (aud_euro_tides_init(&tides) != AUD_EURO_OK) return -1;
  aud_euro_tides_set_range(&tides, AUD_EURO_TIDES_RANGE_HIGH);
  aud_euro_tides_set_mode(&tides, AUD_EURO_TIDES_LOOPING);
  aud_euro_tides_set_parameters(&tides, 60 * 128, 0, 0, 0, 0);
  return 0;
}

void tides_example_audio(
    const uint8_t* gate_flags,
    aud_euro_tides_sample_t* output,
    size_t frames) {
  aud_euro_tides_process(&tides, gate_flags, output, frames);
}
