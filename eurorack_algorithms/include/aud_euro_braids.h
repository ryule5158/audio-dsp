/* Braids macro oscillator, 96 kHz, internally rendered in blocks of 24. */
#ifndef AUD_EURO_BRAIDS_H_
#define AUD_EURO_BRAIDS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_BRAIDS_CONTEXT_BYTES 17152u
#define AUD_EURO_BRAIDS_SHAPE_COUNT 48u

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_BRAIDS_CONTEXT_BYTES];
} aud_euro_braids_t;

aud_euro_result_t aud_euro_braids_init(aud_euro_braids_t* context);
void aud_euro_braids_deinit(aud_euro_braids_t* context);
aud_euro_result_t aud_euro_braids_set_shape(
    aud_euro_braids_t* context, uint8_t shape);
void aud_euro_braids_set_pitch_q7(
    aud_euro_braids_t* context, int16_t midi_note_q7);
aud_euro_result_t aud_euro_braids_set_frequency_hz(
    aud_euro_braids_t* context, float frequency_hz);
void aud_euro_braids_set_parameters(
    aud_euro_braids_t* context, float timbre, float color);
void aud_euro_braids_trigger(aud_euro_braids_t* context);
void aud_euro_braids_process(
    aud_euro_braids_t* context,
    const uint8_t* sync,
    int16_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_BRAIDS_H_ */
