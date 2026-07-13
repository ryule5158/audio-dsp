/* Plaits 24-engine macro oscillator. Native rate: 48 kHz, block <= 24. */
#ifndef AUD_EURO_PLAITS_H_
#define AUD_EURO_PLAITS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_PLAITS_CONTEXT_BYTES 9024u
#define AUD_EURO_PLAITS_ENGINE_MEMORY_BYTES 16384u
#define AUD_EURO_PLAITS_USER_DATA_BYTES 4096u
#define AUD_EURO_PLAITS_ENGINE_COUNT 24u

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_PLAITS_CONTEXT_BYTES];
} aud_euro_plaits_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_PLAITS_ENGINE_MEMORY_BYTES];
} aud_euro_plaits_engine_memory_t;

typedef struct {
  float note;
  float harmonics;
  float timbre;
  float morph;
  float frequency_modulation_amount;
  float timbre_modulation_amount;
  float morph_modulation_amount;
  int32_t engine;
  float decay;
  float lpg_colour;
} aud_euro_plaits_patch_t;

typedef struct {
  float engine;
  float note;
  float frequency;
  float harmonics;
  float timbre;
  float morph;
  /* Gate level, not a one-shot event. Keep high for the full note duration. */
  float trigger;
  float level;
  uint8_t frequency_patched;
  uint8_t timbre_patched;
  uint8_t morph_patched;
  uint8_t trigger_patched;
  uint8_t level_patched;
} aud_euro_plaits_modulations_t;

void aud_euro_plaits_default_patch(aud_euro_plaits_patch_t* patch);
void aud_euro_plaits_default_modulations(aud_euro_plaits_modulations_t* modulations);
aud_euro_result_t aud_euro_plaits_init(
    aud_euro_plaits_t* context,
    void* engine_memory,
    size_t engine_memory_bytes);
void aud_euro_plaits_deinit(aud_euro_plaits_t* context);
/*
 * Associates one Plaits user-data page with an engine. The 4096-byte data
 * buffer must remain valid while the context is used. Pass NULL and zero to
 * disable the page. Storage can reside in internal Flash, memory-mapped QSPI,
 * or static RAM; this library never erases or programs it.
 */
aud_euro_result_t aud_euro_plaits_set_user_data(
    aud_euro_plaits_t* context,
    int32_t engine,
    const void* data,
    size_t data_bytes);
void aud_euro_plaits_process(
    aud_euro_plaits_t* context,
    const aud_euro_plaits_patch_t* patch,
    const aud_euro_plaits_modulations_t* modulations,
    aud_euro_stereo_i16_t* output,
    size_t frames);
int32_t aud_euro_plaits_active_engine(const aud_euro_plaits_t* context);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_PLAITS_H_ */
