#include "aud_euro_frames.h"

static aud_euro_frames_keyframer_t keyframer;

int frames_example_init(void) {
  const uint16_t start[4] = { 0, 0, 0, 0 };
  const uint16_t end[4] = { 65535, 32768, 16384, 49152 };
  if (aud_euro_frames_keyframer_init(&keyframer) != AUD_EURO_OK) return -1;
  aud_euro_frames_keyframer_set_channel(
      &keyframer, 0, AUD_EURO_FRAMES_EASE_SINE, 128);
  aud_euro_frames_keyframer_add(&keyframer, 0, start);
  return aud_euro_frames_keyframer_add(&keyframer, 65535, end);
}

void frames_example_control(uint16_t position, uint16_t levels[4]) {
  aud_euro_frames_keyframer_evaluate(&keyframer, position, levels);
}
