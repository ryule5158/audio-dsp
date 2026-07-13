/* Peaks envelopes, LFOs, drums, pulse processors and sequencer. 48 kHz. */
#ifndef AUD_EURO_PEAKS_H_
#define AUD_EURO_PEAKS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_PEAKS_CONTEXT_BYTES 1408u

typedef enum {
  AUD_EURO_PEAKS_ENVELOPE = 0,
  AUD_EURO_PEAKS_LFO = 1,
  AUD_EURO_PEAKS_TAP_LFO = 2,
  AUD_EURO_PEAKS_BASS_DRUM = 3,
  AUD_EURO_PEAKS_SNARE_DRUM = 4,
  AUD_EURO_PEAKS_HIGH_HAT = 5,
  AUD_EURO_PEAKS_FM_DRUM = 6,
  AUD_EURO_PEAKS_PULSE_SHAPER = 7,
  AUD_EURO_PEAKS_PULSE_RANDOMIZER = 8,
  AUD_EURO_PEAKS_BOUNCING_BALL = 9,
  AUD_EURO_PEAKS_MINI_SEQUENCER = 10,
  AUD_EURO_PEAKS_NUMBER_STATION = 11
} aud_euro_peaks_function_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_PEAKS_CONTEXT_BYTES];
} aud_euro_peaks_t;

aud_euro_result_t aud_euro_peaks_init(
    aud_euro_peaks_t* context, uint8_t channel_index);
aud_euro_result_t aud_euro_peaks_set_function(
    aud_euro_peaks_t* context, aud_euro_peaks_function_t function);
void aud_euro_peaks_set_full_control(aud_euro_peaks_t* context, uint8_t full);
aud_euro_result_t aud_euro_peaks_set_parameter(
    aud_euro_peaks_t* context, uint8_t index, uint16_t value);
void aud_euro_peaks_process(
    aud_euro_peaks_t* context,
    const uint8_t* gate_flags,
    int16_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_PEAKS_H_ */
