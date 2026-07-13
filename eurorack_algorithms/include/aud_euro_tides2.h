/* Tides 2 four-output slope generator. Frequency is supplied in hertz. */
#ifndef AUD_EURO_TIDES2_H_
#define AUD_EURO_TIDES2_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_TIDES2_CONTEXT_BYTES 248u

typedef enum {
  AUD_EURO_TIDES2_AD = 0,
  AUD_EURO_TIDES2_LOOPING = 1,
  AUD_EURO_TIDES2_AR = 2
} aud_euro_tides2_ramp_mode_t;

typedef enum {
  AUD_EURO_TIDES2_GATES = 0,
  AUD_EURO_TIDES2_AMPLITUDE = 1,
  AUD_EURO_TIDES2_PHASE = 2,
  AUD_EURO_TIDES2_FREQUENCY = 3
} aud_euro_tides2_output_mode_t;

typedef struct {
  float channel[4];
} aud_euro_tides2_sample_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_TIDES2_CONTEXT_BYTES];
} aud_euro_tides2_t;

aud_euro_result_t aud_euro_tides2_init(
    aud_euro_tides2_t* context, float sample_rate_hz);
aud_euro_result_t aud_euro_tides2_process(
    aud_euro_tides2_t* context,
    aud_euro_tides2_ramp_mode_t ramp_mode,
    aud_euro_tides2_output_mode_t output_mode,
    uint8_t audio_range,
    float frequency_hz,
    float slope,
    float shape,
    float smoothness,
    float shift,
    const uint8_t* gate_flags,
    const float* external_ramp,
    aud_euro_tides2_sample_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_TIDES2_H_ */
