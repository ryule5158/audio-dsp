#include "aud_euro_rings.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_LARGE_RAM aud_euro_rings_t rings;
static AUD_EXAMPLE_LARGE_RAM uint16_t rings_reverb[AUD_EURO_RINGS_REVERB_SAMPLES];
static aud_euro_rings_patch_t rings_patch;
static aud_euro_rings_performance_t rings_performance;

int rings_example_init(void) {
  aud_euro_rings_default_patch(&rings_patch);
  rings_performance.strum = 0;
  rings_performance.internal_exciter = 1;
  rings_performance.internal_strum = 0;
  rings_performance.internal_note = 0;
  rings_performance.tonic = 48.0f;
  rings_performance.note = 12.0f;
  rings_performance.frequency_modulation = 0.0f;
  rings_performance.chord = 0;
  return aud_euro_rings_init(
      &rings, rings_reverb, AUD_EURO_RINGS_REVERB_SAMPLES);
}

/* strum is consumed once even when frames exceeds the native 24-frame block. */
void rings_example_audio(
    uint8_t strum, float* main_output, float* aux_output, size_t frames) {
  rings_performance.strum = strum;
  aud_euro_rings_process_f32(
      &rings, &rings_patch, &rings_performance, NULL,
      main_output, aux_output, frames);
  rings_performance.strum = 0;
}
