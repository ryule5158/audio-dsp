/* Frames keyframe interpolation and four-channel phase-coupled LFO. */
#ifndef AUD_EURO_FRAMES_H_
#define AUD_EURO_FRAMES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_FRAMES_KEYFRAMER_CONTEXT_BYTES 896u
#define AUD_EURO_FRAMES_LFO_CONTEXT_BYTES 52u
#define AUD_EURO_FRAMES_CHANNELS 4u
#define AUD_EURO_FRAMES_MAX_KEYFRAMES 64u

typedef enum {
  AUD_EURO_FRAMES_EASE_STEP = 0,
  AUD_EURO_FRAMES_EASE_LINEAR = 1,
  AUD_EURO_FRAMES_EASE_IN_QUARTIC = 2,
  AUD_EURO_FRAMES_EASE_OUT_QUARTIC = 3,
  AUD_EURO_FRAMES_EASE_SINE = 4,
  AUD_EURO_FRAMES_EASE_BOUNCE = 5
} aud_euro_frames_easing_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_FRAMES_KEYFRAMER_CONTEXT_BYTES];
} aud_euro_frames_keyframer_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_FRAMES_LFO_CONTEXT_BYTES];
} aud_euro_frames_lfo_t;

aud_euro_result_t aud_euro_frames_keyframer_init(
    aud_euro_frames_keyframer_t* context);
void aud_euro_frames_keyframer_clear(aud_euro_frames_keyframer_t* context);
aud_euro_result_t aud_euro_frames_keyframer_set_channel(
    aud_euro_frames_keyframer_t* context,
    uint8_t channel,
    aud_euro_frames_easing_t easing,
    uint8_t response);
aud_euro_result_t aud_euro_frames_keyframer_add(
    aud_euro_frames_keyframer_t* context,
    uint16_t timestamp,
    const uint16_t values[AUD_EURO_FRAMES_CHANNELS]);
uint8_t aud_euro_frames_keyframer_remove(
    aud_euro_frames_keyframer_t* context, uint16_t timestamp);
void aud_euro_frames_keyframer_evaluate(
    aud_euro_frames_keyframer_t* context,
    uint16_t timestamp,
    uint16_t levels[AUD_EURO_FRAMES_CHANNELS]);
uint16_t aud_euro_frames_keyframer_count(
    const aud_euro_frames_keyframer_t* context);

aud_euro_result_t aud_euro_frames_lfo_init(aud_euro_frames_lfo_t* context);
void aud_euro_frames_lfo_set_parameters(
    aud_euro_frames_lfo_t* context,
    uint16_t shape,
    uint16_t shape_spread,
    uint16_t phase_or_frequency_spread,
    uint16_t coupling);
void aud_euro_frames_lfo_render(
    aud_euro_frames_lfo_t* context,
    int32_t raw_frequency,
    uint16_t output[AUD_EURO_FRAMES_CHANNELS]);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_FRAMES_H_ */
