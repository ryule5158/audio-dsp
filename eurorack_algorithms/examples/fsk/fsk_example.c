#include "aud_euro_fsk.h"

static aud_euro_fsk_t fsk;

int fsk_example_init(void) {
  /* Example run lengths after comparator slicing; tune for the sender rate. */
  return aud_euro_fsk_init(&fsk, 80, 40, 20);
}

aud_euro_fsk_state_t fsk_example_process_comparator_samples(
    const uint8_t* logic_samples, size_t samples) {
  return aud_euro_fsk_process(&fsk, logic_samples, samples);
}
