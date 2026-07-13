#include "aud_euro_peaks.h"

static aud_euro_peaks_t peaks;

int peaks_example_init(void) {
  if (aud_euro_peaks_init(&peaks, 0) != AUD_EURO_OK) return -1;
  aud_euro_peaks_set_parameter(&peaks, 0, 32768);
  aud_euro_peaks_set_parameter(&peaks, 1, 32768);
  return aud_euro_peaks_set_function(&peaks, AUD_EURO_PEAKS_BASS_DRUM);
}

/* gate_flags uses AUD_EURO_GATE_* bits for every 48 kHz sample. */
void peaks_example_audio(
    const uint8_t* gate_flags, int16_t* output, size_t frames) {
  aud_euro_peaks_process(&peaks, gate_flags, output, frames);
}
