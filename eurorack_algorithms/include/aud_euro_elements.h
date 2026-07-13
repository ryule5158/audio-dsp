/* Elements physical-modeling voice. Native rate: 32 kHz, block <= 16. */
#ifndef AUD_EURO_ELEMENTS_H_
#define AUD_EURO_ELEMENTS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_ELEMENTS_CONTEXT_BYTES 113664u
#define AUD_EURO_ELEMENTS_REVERB_SAMPLES 32768u

typedef enum {
  AUD_EURO_ELEMENTS_MODAL = 0,
  AUD_EURO_ELEMENTS_STRING = 1,
  AUD_EURO_ELEMENTS_STRING_ENSEMBLE = 2
} aud_euro_elements_model_t;

typedef struct {
  float exciter_envelope_shape;
  float exciter_bow_level;
  float exciter_bow_timbre;
  float exciter_blow_level;
  float exciter_blow_meta;
  float exciter_blow_timbre;
  float exciter_strike_level;
  float exciter_strike_meta;
  float exciter_strike_timbre;
  float exciter_signature;
  float resonator_geometry;
  float resonator_brightness;
  float resonator_damping;
  float resonator_position;
  float resonator_modulation_frequency;
  float resonator_modulation_offset;
  float reverb_diffusion;
  float reverb_lowpass;
  float space;
  float modulation_frequency;
} aud_euro_elements_patch_t;

typedef struct {
  uint8_t gate;
  float note;
  float modulation;
  float strength;
} aud_euro_elements_performance_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_ELEMENTS_CONTEXT_BYTES];
} aud_euro_elements_t;

void aud_euro_elements_default_patch(aud_euro_elements_patch_t* patch);
aud_euro_result_t aud_euro_elements_init(
    aud_euro_elements_t* context,
    uint16_t* reverb_buffer,
    size_t reverb_samples);
void aud_euro_elements_deinit(aud_euro_elements_t* context);
aud_euro_result_t aud_euro_elements_set_model(
    aud_euro_elements_t* context, aud_euro_elements_model_t model);
void aud_euro_elements_set_patch(
    aud_euro_elements_t* context, const aud_euro_elements_patch_t* patch);
void aud_euro_elements_process_f32(
    aud_euro_elements_t* context,
    const aud_euro_elements_performance_t* performance,
    const float* blow_input,
    const float* strike_input,
    float* main_output,
    float* aux_output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_ELEMENTS_H_ */
