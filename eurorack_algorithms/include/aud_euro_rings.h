/* Rings resonator/string voice. Native rate: 48 kHz, block <= 24. */
#ifndef AUD_EURO_RINGS_H_
#define AUD_EURO_RINGS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_RINGS_CONTEXT_BYTES 110592u
#define AUD_EURO_RINGS_REVERB_SAMPLES 65536u

typedef enum {
  AUD_EURO_RINGS_MODAL = 0,
  AUD_EURO_RINGS_SYMPATHETIC_STRING = 1,
  AUD_EURO_RINGS_STRING = 2,
  AUD_EURO_RINGS_FM_VOICE = 3,
  AUD_EURO_RINGS_QUANTIZED_STRING = 4,
  AUD_EURO_RINGS_STRING_REVERB = 5
} aud_euro_rings_model_t;

typedef struct {
  float structure;
  float brightness;
  float damping;
  float position;
} aud_euro_rings_patch_t;

typedef struct {
  uint8_t strum;
  uint8_t internal_exciter;
  uint8_t internal_strum;
  uint8_t internal_note;
  float tonic;
  float note;
  float frequency_modulation;
  int32_t chord;
} aud_euro_rings_performance_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_RINGS_CONTEXT_BYTES];
} aud_euro_rings_t;

void aud_euro_rings_default_patch(aud_euro_rings_patch_t* patch);
aud_euro_result_t aud_euro_rings_init(
    aud_euro_rings_t* context,
    uint16_t* reverb_buffer,
    size_t reverb_samples);
void aud_euro_rings_deinit(aud_euro_rings_t* context);
aud_euro_result_t aud_euro_rings_set_model(
    aud_euro_rings_t* context, aud_euro_rings_model_t model);
aud_euro_result_t aud_euro_rings_set_polyphony(
    aud_euro_rings_t* context, int32_t voices);
void aud_euro_rings_process_f32(
    aud_euro_rings_t* context,
    const aud_euro_rings_patch_t* patch,
    const aud_euro_rings_performance_t* performance,
    const float* input,
    float* main_output,
    float* aux_output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_RINGS_H_ */
