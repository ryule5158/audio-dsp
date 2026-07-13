#include "aud_euro_yarns.h"

static aud_euro_yarns_t yarns;

int yarns_example_init(void) {
  if (aud_euro_yarns_init(&yarns, 1000.0f) != AUD_EURO_OK) return -1;
  return aud_euro_yarns_clock_start(&yarns, 120.0f, 54.0f);
}

/* Call at exactly 1 kHz. A return value of 1 is one outgoing MIDI clock tick. */
uint8_t yarns_example_control_tick(void) {
  return aud_euro_yarns_clock_process(&yarns);
}

/* Return value is the adaptive pitch offset in the upstream fixed-point unit. */
int16_t yarns_example_note_on(uint8_t midi_note) {
  return aud_euro_yarns_note_on(&yarns, midi_note);
}
