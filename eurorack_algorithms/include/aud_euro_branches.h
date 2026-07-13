/* Branches dual Bernoulli router. GPL-3.0-or-later module. */
#ifndef AUD_EURO_BRANCHES_H_
#define AUD_EURO_BRANCHES_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t rng_state;
  uint8_t input[2];
  uint8_t previous_outcome[2];
  uint8_t output_a[2];
  uint8_t output_b[2];
} aud_euro_branches_t;

typedef struct {
  uint8_t output_a;
  uint8_t output_b;
  uint8_t new_decision;
} aud_euro_branches_output_t;

void aud_euro_branches_init(aud_euro_branches_t* context, uint32_t seed);
/*
 * Makes one routing decision on each rising gate edge. probability_a is in
 * [0, 1]: 0 always selects B and 1 always selects A. In toggle mode the new
 * random decision toggles the previous route. In latch mode outputs remain
 * asserted after the input gate falls.
 */
aud_euro_branches_output_t aud_euro_branches_process(
    aud_euro_branches_t* context,
    uint8_t channel,
    uint8_t gate,
    float probability_a,
    uint8_t toggle_mode,
    uint8_t latch_mode);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_BRANCHES_H_ */
