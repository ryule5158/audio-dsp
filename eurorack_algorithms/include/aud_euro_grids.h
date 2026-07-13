/* Grids drum-map and Euclidean sequencer. GPL-3.0-or-later module. */
#ifndef AUD_EURO_GRIDS_H_
#define AUD_EURO_GRIDS_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AUD_EURO_GRIDS_EUCLIDEAN = 0,
  AUD_EURO_GRIDS_DRUMS = 1
} aud_euro_grids_mode_t;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t randomness;
  uint8_t density[3];
  uint8_t euclidean_length[3];
  aud_euro_grids_mode_t mode;
  uint8_t output_clock;
  uint8_t gate_mode;
  uint8_t swing;
} aud_euro_grids_settings_t;

void aud_euro_grids_default_settings(aud_euro_grids_settings_t* settings);
void aud_euro_grids_init(uint16_t seed);
void aud_euro_grids_configure(const aud_euro_grids_settings_t* settings);
uint8_t aud_euro_grids_tick(uint8_t pulses);
void aud_euro_grids_retrigger(void);
void aud_euro_grids_clock_falling_edge(void);
void aud_euro_grids_pulse_timer_tick(void);
uint8_t aud_euro_grids_step(void);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_GRIDS_H_ */
