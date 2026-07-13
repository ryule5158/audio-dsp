/* Original Tides slope generator. Native audio rate: 48 kHz. */
#ifndef AUD_EURO_TIDES_H_
#define AUD_EURO_TIDES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_TIDES_CONTEXT_BYTES 704u

typedef enum {
  AUD_EURO_TIDES_RANGE_HIGH = 0,
  AUD_EURO_TIDES_RANGE_MEDIUM = 1,
  AUD_EURO_TIDES_RANGE_LOW = 2
} aud_euro_tides_range_t;

typedef enum {
  AUD_EURO_TIDES_AD = 0,
  AUD_EURO_TIDES_LOOPING = 1,
  AUD_EURO_TIDES_AR = 2
} aud_euro_tides_mode_t;

typedef struct {
  uint16_t unipolar;
  int16_t bipolar;
  uint8_t flags;
} aud_euro_tides_sample_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_TIDES_CONTEXT_BYTES];
} aud_euro_tides_t;

aud_euro_result_t aud_euro_tides_init(aud_euro_tides_t* context);
aud_euro_result_t aud_euro_tides_set_range(
    aud_euro_tides_t* context, aud_euro_tides_range_t range);
aud_euro_result_t aud_euro_tides_set_mode(
    aud_euro_tides_t* context, aud_euro_tides_mode_t mode);
void aud_euro_tides_set_parameters(
    aud_euro_tides_t* context,
    int16_t pitch_q7,
    int16_t slope,
    int16_t shape,
    int16_t smoothness,
    uint8_t clock_sync);
void aud_euro_tides_process(
    aud_euro_tides_t* context,
    const uint8_t* controls,
    aud_euro_tides_sample_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_TIDES_H_ */
