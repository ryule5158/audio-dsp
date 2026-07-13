#include "aud_euro_edges.h"

static aud_euro_edges_t edges;

int edges_example_init(float codec_sample_rate_hz) {
  if (aud_euro_edges_init(&edges, codec_sample_rate_hz) != AUD_EURO_OK) return -1;
  aud_euro_edges_set_bitcrush(&edges, 96);
  return aud_euro_edges_set(
      &edges, 220.0f, AUD_EURO_EDGES_NES_TRIANGLE, 1);
}

void edges_example_audio(int16_t* mono_output, size_t frames) {
  aud_euro_edges_process(&edges, mono_output, frames);
}
