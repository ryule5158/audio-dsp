#ifndef AUD_MPU_IO_STATE_H
#define AUD_MPU_IO_STATE_H

#include <signal.h>
#include <stdint.h>

typedef enum {
    AUD_MPU_IO_TRANSFER = 0,
    AUD_MPU_IO_RETRY,
    AUD_MPU_IO_RESTART_PAIR,
    AUD_MPU_IO_STOP,
    AUD_MPU_IO_FATAL
} AudMpuIoAction;

typedef struct {
    AudMpuIoAction action;
    uint32_t frames;
    int short_transfer;
} AudMpuIoDecision;

AudMpuIoDecision AudMpuIo_Classify(long result,
                                   uint32_t requested_frames,
                                   sig_atomic_t stop_requested);

#endif
