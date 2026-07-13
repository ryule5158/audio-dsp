/* Yarns adaptive just intonation and sample-rate-independent MIDI clock. */
#ifndef AUD_EURO_YARNS_H_
#define AUD_EURO_YARNS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_YARNS_CONTEXT_BYTES 192u

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_YARNS_CONTEXT_BYTES];
} aud_euro_yarns_t;

aud_euro_result_t aud_euro_yarns_init(
    aud_euro_yarns_t* context, float control_sample_rate_hz);
int16_t aud_euro_yarns_note_on(aud_euro_yarns_t* context, uint8_t midi_note);
void aud_euro_yarns_note_off(aud_euro_yarns_t* context, uint8_t midi_note);
aud_euro_result_t aud_euro_yarns_clock_start(
    aud_euro_yarns_t* context, float bpm, float swing_percent);
uint8_t aud_euro_yarns_clock_process(aud_euro_yarns_t* context);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_YARNS_H_ */
