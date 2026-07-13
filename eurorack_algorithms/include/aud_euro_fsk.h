/* Audio/gate FSK demodulator and CRC-checked 256-byte packet decoder. */
#ifndef AUD_EURO_FSK_H_
#define AUD_EURO_FSK_H_

#include "aud_euro_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_EURO_FSK_CONTEXT_BYTES 512u
#define AUD_EURO_FSK_PACKET_BYTES 256u

typedef enum {
  AUD_EURO_FSK_SYNCING = 0,
  AUD_EURO_FSK_DECODING_PACKET = 1,
  AUD_EURO_FSK_PACKET_OK = 2,
  AUD_EURO_FSK_ERROR_SYNC = 3,
  AUD_EURO_FSK_ERROR_CRC = 4,
  AUD_EURO_FSK_END_OF_TRANSMISSION = 5
} aud_euro_fsk_state_t;

typedef union {
  uint64_t alignment;
  uint8_t storage[AUD_EURO_FSK_CONTEXT_BYTES];
} aud_euro_fsk_t;

/* Durations are measured in samples and must satisfy pause > one > zero. */
aud_euro_result_t aud_euro_fsk_init(
    aud_euro_fsk_t* context,
    uint32_t pause_duration,
    uint32_t one_duration,
    uint32_t zero_duration);
void aud_euro_fsk_sync(aud_euro_fsk_t* context);
aud_euro_fsk_state_t aud_euro_fsk_process(
    aud_euro_fsk_t* context,
    const uint8_t* sliced_input,
    size_t samples);
const uint8_t* aud_euro_fsk_packet_data(const aud_euro_fsk_t* context);

#ifdef __cplusplus
}
#endif

#endif  /* AUD_EURO_FSK_H_ */
