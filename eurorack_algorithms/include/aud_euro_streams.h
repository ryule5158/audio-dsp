/* Streams envelope, vactrol, follower, compressor, filter and Lorenz control. */
#ifndef AUD_EURO_STREAMS_H_
#define AUD_EURO_STREAMS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_STREAMS_CONTEXT_BYTES 608u

typedef enum {
  AUD_EURO_STREAMS_ENVELOPE = 0,
  AUD_EURO_STREAMS_VACTROL = 1,
  AUD_EURO_STREAMS_FOLLOWER = 2,
  AUD_EURO_STREAMS_COMPRESSOR = 3,
  AUD_EURO_STREAMS_FILTER_CONTROLLER = 4,
  AUD_EURO_STREAMS_LORENZ = 5
} aud_euro_streams_function_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_STREAMS_CONTEXT_BYTES];
} aud_euro_streams_t;

aud_euro_result_t aud_euro_streams_init(
    aud_euro_streams_t* context, uint8_t channel_index);
aud_euro_result_t aud_euro_streams_set_function(
    aud_euro_streams_t* context, aud_euro_streams_function_t function);
void aud_euro_streams_set_alternate(aud_euro_streams_t* context, uint8_t enabled);
aud_euro_result_t aud_euro_streams_set_parameter(
    aud_euro_streams_t* context, uint8_t index, uint16_t value);
void aud_euro_streams_process(
    aud_euro_streams_t* context,
    const int16_t* audio,
    const int16_t* excite,
    uint16_t* gain,
    uint16_t* filter_frequency,
    size_t frames);
int32_t aud_euro_streams_gain_reduction(const aud_euro_streams_t* context);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_STREAMS_H_ */
