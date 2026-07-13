#include "aud_euro_braids.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_FAST_RAM aud_euro_braids_t braids;

int braids_example_init(void) {
  if (aud_euro_braids_init(&braids) != AUD_EURO_OK) return -1;
  aud_euro_braids_set_shape(&braids, 0);
  aud_euro_braids_set_frequency_hz(&braids, 440.0f);
  aud_euro_braids_set_parameters(&braids, 0.5f, 0.5f);
  return 0;
}

/* Braids is native at 96 kHz. Resample when the codec runs at 48 kHz. */
void braids_example_audio(int16_t* mono_output, size_t frames) {
  aud_euro_braids_process(&braids, NULL, mono_output, frames);
}
