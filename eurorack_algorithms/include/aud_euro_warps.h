/* Warps cross-modulator, wavefolder, ring modulator and vocoder. */
#ifndef AUD_EURO_WARPS_H_
#define AUD_EURO_WARPS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_WARPS_CONTEXT_BYTES 72660u

typedef struct {
  float carrier_drive;
  float modulator_drive;
  float algorithm;
  float timbre;
  float frequency_shift;
  float frequency_shift_cv;
  float phase_shift;
  float note;
  int32_t carrier_shape;
} aud_euro_warps_parameters_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_WARPS_CONTEXT_BYTES];
} aud_euro_warps_t;

void aud_euro_warps_default_parameters(aud_euro_warps_parameters_t* parameters);
aud_euro_result_t aud_euro_warps_init(
    aud_euro_warps_t* context, float sample_rate_hz);
void aud_euro_warps_deinit(aud_euro_warps_t* context);
void aud_euro_warps_set_parameters(
    aud_euro_warps_t* context, const aud_euro_warps_parameters_t* parameters);
void aud_euro_warps_set_frequency_shifter(
    aud_euro_warps_t* context, uint8_t enabled);
/* frames must be a multiple of 3, as required by the native oversampler. */
aud_euro_result_t aud_euro_warps_process(
    aud_euro_warps_t* context,
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_WARPS_H_ */
