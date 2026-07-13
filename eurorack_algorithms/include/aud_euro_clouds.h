/* Clouds granular processor. Its native processing rate is 32 kHz. */
#ifndef AUD_EURO_CLOUDS_H_
#define AUD_EURO_CLOUDS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_CLOUDS_CONTEXT_BYTES 8960u
#define AUD_EURO_CLOUDS_LARGE_BUFFER_BYTES 118784u
#define AUD_EURO_CLOUDS_SMALL_BUFFER_BYTES 65536u

typedef enum {
  AUD_EURO_CLOUDS_GRANULAR = 0,
  AUD_EURO_CLOUDS_STRETCH = 1,
  AUD_EURO_CLOUDS_LOOPING_DELAY = 2,
  AUD_EURO_CLOUDS_SPECTRAL = 3
} aud_euro_clouds_mode_t;

typedef struct {
  float position;
  float size;
  float pitch_semitones;
  float density;
  float texture;
  float dry_wet;
  float stereo_spread;
  float feedback;
  float reverb;
  uint8_t freeze;
  uint8_t trigger;
  uint8_t gate;
} aud_euro_clouds_parameters_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_CLOUDS_CONTEXT_BYTES];
} aud_euro_clouds_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_CLOUDS_LARGE_BUFFER_BYTES];
} aud_euro_clouds_large_buffer_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_CLOUDS_SMALL_BUFFER_BYTES];
} aud_euro_clouds_small_buffer_t;

void aud_euro_clouds_default_parameters(aud_euro_clouds_parameters_t* parameters);
aud_euro_result_t aud_euro_clouds_init(
    aud_euro_clouds_t* context,
    void* large_buffer,
    size_t large_buffer_bytes,
    void* small_buffer,
    size_t small_buffer_bytes);
void aud_euro_clouds_deinit(aud_euro_clouds_t* context);
aud_euro_result_t aud_euro_clouds_set_mode(
    aud_euro_clouds_t* context, aud_euro_clouds_mode_t mode);
aud_euro_result_t aud_euro_clouds_set_quality(
    aud_euro_clouds_t* context, uint8_t mono, uint8_t low_fidelity);
void aud_euro_clouds_set_parameters(
    aud_euro_clouds_t* context,
    const aud_euro_clouds_parameters_t* parameters);
void aud_euro_clouds_process(
    aud_euro_clouds_t* context,
    const aud_euro_stereo_i16_t* input,
    aud_euro_stereo_i16_t* output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_CLOUDS_H_ */
