#include "aud_euro_plaits.h"

#include "../stm32h7_memory.h"

static AUD_EXAMPLE_FAST_RAM aud_euro_plaits_t plaits;
static AUD_EXAMPLE_FAST_RAM aud_euro_plaits_engine_memory_t plaits_engine_memory;
static aud_euro_plaits_patch_t plaits_patch;
static aud_euro_plaits_modulations_t plaits_modulations;

int plaits_example_init(void) {
  aud_euro_plaits_default_patch(&plaits_patch);
  aud_euro_plaits_default_modulations(&plaits_modulations);
  plaits_patch.note = 60.0f;
  plaits_patch.engine = 8;
  plaits_modulations.trigger_patched = 1;
  return aud_euro_plaits_init(
      &plaits, plaits_engine_memory.storage, sizeof(plaits_engine_memory.storage));
}

/*
 * Call from the 48 kHz SAI/I2S DMA half/full callback. output is interleaved
 * signed 16-bit stereo. gate_level must stay high for the full note duration.
 */
void plaits_example_sai_dma(
    uint8_t gate_level,
    aud_euro_stereo_i16_t* output,
    size_t stereo_frames) {
  plaits_modulations.trigger = gate_level ? 1.0f : 0.0f;
  aud_euro_plaits_process(
      &plaits, &plaits_patch, &plaits_modulations, output, stereo_frames);
}
