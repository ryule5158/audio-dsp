#include "aud_euro_stages.h"

static aud_euro_stages_t stages;

int stages_example_init(void) {
  const aud_euro_stages_segment_t ad[2] = {
    { AUD_EURO_STAGES_RAMP, 0, 0.15f, 0.5f },
    { AUD_EURO_STAGES_RAMP, 0, 0.65f, 0.5f }
  };
  if (aud_euro_stages_init(&stages) != AUD_EURO_OK) return -1;
  return aud_euro_stages_configure(&stages, 1, ad, 2, 0);
}

/* Process at the native 31.25 kHz rate, then resample for an audio codec. */
void stages_example_process(
    const uint8_t* gate_flags,
    aud_euro_stages_output_t* output,
    size_t frames) {
  aud_euro_stages_process(&stages, gate_flags, output, frames);
}
