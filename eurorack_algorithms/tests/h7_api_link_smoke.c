#include <stdint.h>

#include "aud_eurorack.h"

/*
 * Taking one address from every wrapper forces all modules into one image.
 * This is intentionally a C translation unit: it verifies the public ABI is
 * usable without exposing C++ types to an STM32 application.
 */
static const uintptr_t api_references[] = {
  (uintptr_t)&aud_euro_branches_init,
  (uintptr_t)&aud_euro_braids_init,
  (uintptr_t)&aud_euro_clouds_init,
  (uintptr_t)&aud_euro_edges_init,
  (uintptr_t)&aud_euro_elements_init,
  (uintptr_t)&aud_euro_fsk_init,
  (uintptr_t)&aud_euro_frames_keyframer_init,
  (uintptr_t)&aud_euro_grids_init,
  (uintptr_t)&aud_euro_marbles_init,
  (uintptr_t)&aud_euro_peaks_init,
  (uintptr_t)&aud_euro_plaits_init,
  (uintptr_t)&aud_euro_plaits_set_user_data,
  (uintptr_t)&aud_euro_rings_init,
  (uintptr_t)&aud_euro_stages_init,
  (uintptr_t)&aud_euro_streams_init,
  (uintptr_t)&aud_euro_tides_init,
  (uintptr_t)&aud_euro_tides2_init,
  (uintptr_t)&aud_euro_warps_init,
  (uintptr_t)&aud_euro_yarns_init
};

int main(void) {
  uintptr_t checksum = 0;
  for (uint32_t i = 0;
       i < sizeof(api_references) / sizeof(api_references[0]);
       ++i) {
    checksum ^= api_references[i];
  }
  return checksum == 0;
}
