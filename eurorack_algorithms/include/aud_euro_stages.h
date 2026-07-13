/* Stages multi-segment generator. Native rate: 31.25 kHz. */
#ifndef AUD_EURO_STAGES_H_
#define AUD_EURO_STAGES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_STAGES_CONTEXT_BYTES 3096u
#define AUD_EURO_STAGES_MAX_SEGMENTS 36u

typedef enum {
  AUD_EURO_STAGES_RAMP = 0,
  AUD_EURO_STAGES_STEP = 1,
  AUD_EURO_STAGES_HOLD = 2,
  AUD_EURO_STAGES_ALTERNATING = 3
} aud_euro_stages_segment_type_t;

typedef struct {
  aud_euro_stages_segment_type_t type;
  uint8_t loop;
  float primary;
  float secondary;
} aud_euro_stages_segment_t;

typedef struct {
  float value;
  float phase;
  int32_t segment;
} aud_euro_stages_output_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_STAGES_CONTEXT_BYTES];
} aud_euro_stages_t;

aud_euro_result_t aud_euro_stages_init(aud_euro_stages_t* context);
aud_euro_result_t aud_euro_stages_configure(
    aud_euro_stages_t* context,
    uint8_t has_trigger,
    const aud_euro_stages_segment_t* segments,
    size_t segment_count,
    uint8_t sequencer_mode);
uint8_t aud_euro_stages_process(
    aud_euro_stages_t* context,
    const uint8_t* gate_flags,
    aud_euro_stages_output_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_STAGES_H_ */
