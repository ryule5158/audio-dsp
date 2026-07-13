/*
 * C ABI shared by the STM32H7 Eurorack algorithm ports.
 *
 * Context objects are intentionally caller-owned. Declare them static or
 * global; the larger physical-modeling contexts must never be put on a small
 * RTOS task stack. No wrapper allocates from the heap.
 */
#ifndef AUD_EURO_COMMON_H_
#define AUD_EURO_COMMON_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AUD_EURO_OK = 0,
  AUD_EURO_ERROR_ARGUMENT = -1,
  AUD_EURO_ERROR_MEMORY = -2,
  AUD_EURO_ERROR_RANGE = -3
} aud_euro_result_t;

typedef struct {
  int16_t left;
  int16_t right;
} aud_euro_stereo_i16_t;

typedef struct {
  float left;
  float right;
} aud_euro_stereo_f32_t;

enum {
  AUD_EURO_GATE_LOW = 0,
  AUD_EURO_GATE_HIGH = 1,
  AUD_EURO_GATE_RISING = 2,
  AUD_EURO_GATE_FALLING = 4
};

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_COMMON_H_ */
