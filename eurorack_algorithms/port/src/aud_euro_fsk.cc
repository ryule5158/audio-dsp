#include "aud_euro_fsk.h"

#include <new>

#include "stm_audio_bootloader/fsk/demodulator.h"
#include "stm_audio_bootloader/fsk/packet_decoder.h"

namespace {

struct Context {
  stm_audio_bootloader::Demodulator demodulator;
  stm_audio_bootloader::PacketDecoder decoder;
  stm_audio_bootloader::PacketDecoderState state;
};

Context* Impl(aud_euro_fsk_t* context) {
  return reinterpret_cast<Context*>(context->storage);
}

const Context* Impl(const aud_euro_fsk_t* context) {
  return reinterpret_cast<const Context*>(context->storage);
}

}  // namespace

static_assert(sizeof(Context) <= AUD_EURO_FSK_CONTEXT_BYTES,
    "AUD_EURO_FSK_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_fsk_init(
    aud_euro_fsk_t* context,
    uint32_t pause_duration,
    uint32_t one_duration,
    uint32_t zero_duration) {
  if (!context || !zero_duration ||
      !(pause_duration > one_duration && one_duration > zero_duration)) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  Context* state = new (context->storage) Context();
  state->demodulator.Init(pause_duration, one_duration, zero_duration);
  state->decoder.Init();
  state->decoder.Reset();
  state->demodulator.Sync();
  state->state = stm_audio_bootloader::PACKET_DECODER_STATE_SYNCING;
  return AUD_EURO_OK;
}

extern "C" void aud_euro_fsk_sync(aud_euro_fsk_t* context) {
  if (!context) return;
  Impl(context)->demodulator.Sync();
  Impl(context)->decoder.Reset();
  Impl(context)->state = stm_audio_bootloader::PACKET_DECODER_STATE_SYNCING;
}

extern "C" aud_euro_fsk_state_t aud_euro_fsk_process(
    aud_euro_fsk_t* context,
    const uint8_t* sliced_input,
    size_t samples) {
  if (!context || (!sliced_input && samples)) return AUD_EURO_FSK_ERROR_SYNC;
  Context* state = Impl(context);
  for (size_t i = 0; i < samples; ++i) {
    state->demodulator.PushSample(sliced_input[i] != 0);
    while (state->demodulator.available()) {
      state->state = state->decoder.ProcessSymbol(
          state->demodulator.NextSymbol());
    }
  }
  return static_cast<aud_euro_fsk_state_t>(state->state);
}

extern "C" const uint8_t* aud_euro_fsk_packet_data(
    const aud_euro_fsk_t* context) {
  return context ? Impl(context)->decoder.packet_data() : NULL;
}
