/* Marbles correlated random gates and voltages. */
#ifndef AUD_EURO_MARBLES_H_
#define AUD_EURO_MARBLES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_MARBLES_CONTEXT_BYTES 8448u

typedef struct {
  float external;
  float master;
  float slave_1;
  float slave_2;
} aud_euro_marbles_ramp_sample_t;

typedef struct {
  int32_t model;
  int32_t range;
  float rate;
  float bias;
  float jitter;
  float deja_vu;
  int32_t length;
  float pulse_width_mean;
  float pulse_width_spread;
} aud_euro_marbles_t_parameters_t;

typedef struct {
  int32_t control_mode;
  int32_t voltage_range;
  uint8_t register_mode;
  float register_value;
  float spread;
  float bias;
  float steps;
  float deja_vu;
  int32_t scale_index;
  int32_t length;
  float ratio;
} aud_euro_marbles_group_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_MARBLES_CONTEXT_BYTES];
} aud_euro_marbles_t;

aud_euro_result_t aud_euro_marbles_init(
    aud_euro_marbles_t* context, float sample_rate_hz, uint32_t seed);
void aud_euro_marbles_add_entropy(aud_euro_marbles_t* context, uint32_t word);
void aud_euro_marbles_default_t_parameters(
    aud_euro_marbles_t_parameters_t* parameters);
void aud_euro_marbles_default_group(aud_euro_marbles_group_t* parameters);
aud_euro_result_t aud_euro_marbles_process_t(
    aud_euro_marbles_t* context,
    const aud_euro_marbles_t_parameters_t* parameters,
    uint8_t use_external_clock,
    const uint8_t* external_clock,
    aud_euro_marbles_ramp_sample_t* ramps,
    uint8_t* gates,
    size_t frames);
aud_euro_result_t aud_euro_marbles_process_xy(
    aud_euro_marbles_t* context,
    int32_t clock_source,
    const aud_euro_marbles_group_t* x_parameters,
    const aud_euro_marbles_group_t* y_parameters,
    const uint8_t* external_clock,
    const aud_euro_marbles_ramp_sample_t* ramps,
    float* interleaved_xy_output,
    size_t frames);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_MARBLES_H_ */
