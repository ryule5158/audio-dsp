#include "aud_euro_branches.h"

static aud_euro_branches_t branches;

void branches_example_init(void) {
  aud_euro_branches_init(&branches, 0x12345678u);
}

/* Call once per debounced gate sample. probability_a is 0.0 to 1.0. */
aud_euro_branches_output_t branches_example_control_tick(
    uint8_t input_gate, float probability_a) {
  return aud_euro_branches_process(
      &branches, 0, input_gate, probability_a, 0, 0);
}
