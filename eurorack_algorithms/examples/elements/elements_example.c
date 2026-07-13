#include "aud_euro_elements.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_LARGE_RAM aud_euro_elements_t elements;
static AUD_EXAMPLE_LARGE_RAM uint16_t elements_reverb[AUD_EURO_ELEMENTS_REVERB_SAMPLES];
static aud_euro_elements_patch_t elements_patch;
static aud_euro_elements_performance_t elements_performance;

int elements_example_init(void) {
  aud_euro_elements_default_patch(&elements_patch);
  elements_performance.gate = 0;
  elements_performance.note = 48.0f;
  elements_performance.modulation = 0.0f;
  elements_performance.strength = 0.8f;
  if (aud_euro_elements_init(
          &elements, elements_reverb, AUD_EURO_ELEMENTS_REVERB_SAMPLES) !=
      AUD_EURO_OK) return -1;
  aud_euro_elements_set_patch(&elements, &elements_patch);
  return 0;
}

/* Native rate is 32 kHz. gate is a held level; note uses MIDI note units. */
void elements_example_audio(
    uint8_t gate, float* main_output, float* aux_output, size_t frames) {
  elements_performance.gate = gate;
  aud_euro_elements_process_f32(
      &elements, &elements_performance, NULL, NULL,
      main_output, aux_output, frames);
}
