#include "aud_euro_tides2.h"

static aud_euro_tides2_t tides2;

int tides2_example_init(float sample_rate_hz) {
  return aud_euro_tides2_init(&tides2, sample_rate_hz);
}

void tides2_example_audio(
    float frequency_hz,
    const uint8_t* gate_flags,
    aud_euro_tides2_sample_t* four_outputs,
    size_t frames) {
  aud_euro_tides2_process(
      &tides2, AUD_EURO_TIDES2_LOOPING, AUD_EURO_TIDES2_PHASE, 1,
      frequency_hz, 0.5f, 0.5f, 0.5f, 0.5f,
      gate_flags, NULL, four_outputs, frames);
}
