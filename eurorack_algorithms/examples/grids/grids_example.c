#include "aud_euro_grids.h"

void grids_example_init(void) {
  aud_euro_grids_settings_t settings;
  aud_euro_grids_default_settings(&settings);
  settings.density[0] = 180;
  settings.density[1] = 120;
  settings.density[2] = 80;
  aud_euro_grids_init(0x5158u);
  aud_euro_grids_configure(&settings);
}

/* Call for each 24-PPQN clock pulse; output bits contain drum gates/accents. */
uint8_t grids_example_midi_clock(void) {
  return aud_euro_grids_tick(1);
}
