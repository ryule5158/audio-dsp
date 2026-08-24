#include "aud_mpu_io_state.h"

#include <errno.h>

AudMpuIoDecision AudMpuIo_Classify(long result,
                                   uint32_t requested_frames,
                                   sig_atomic_t stop_requested)
{
    AudMpuIoDecision decision = { AUD_MPU_IO_FATAL, 0u, 0 };
    if (stop_requested) {
        decision.action = AUD_MPU_IO_STOP;
    } else if (result > 0) {
        decision.action = AUD_MPU_IO_TRANSFER;
        decision.frames = (uint32_t)result;
        decision.short_transfer = decision.frames < requested_frames;
    } else if (result == 0 || result == -EAGAIN || result == -EINTR) {
        decision.action = AUD_MPU_IO_RETRY;
    } else if (result == -EPIPE || result == -ESTRPIPE) {
        decision.action = AUD_MPU_IO_RESTART_PAIR;
    }
    return decision;
}
