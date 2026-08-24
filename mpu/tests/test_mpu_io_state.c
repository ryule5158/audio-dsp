#include "aud_mpu_io_state.h"

#include <errno.h>
#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
    return 1; } } while (0)

int main(void)
{
    AudMpuIoDecision d;
    d = AudMpuIo_Classify(32, 64u, 0);
    CHECK(d.action == AUD_MPU_IO_TRANSFER && d.frames == 32u && d.short_transfer);
    CHECK(AudMpuIo_Classify(64, 64u, 0).short_transfer == 0);
    CHECK(AudMpuIo_Classify(0, 64u, 0).action == AUD_MPU_IO_RETRY);
    CHECK(AudMpuIo_Classify(-EINTR, 64u, 0).action == AUD_MPU_IO_RETRY);
    CHECK(AudMpuIo_Classify(-EAGAIN, 64u, 0).action == AUD_MPU_IO_RETRY);
    CHECK(AudMpuIo_Classify(-EPIPE, 64u, 0).action == AUD_MPU_IO_RESTART_PAIR);
    CHECK(AudMpuIo_Classify(-ESTRPIPE, 64u, 0).action == AUD_MPU_IO_RESTART_PAIR);
    CHECK(AudMpuIo_Classify(-EIO, 64u, 0).action == AUD_MPU_IO_FATAL);
    CHECK(AudMpuIo_Classify(12, 64u, 1).action == AUD_MPU_IO_STOP);
    puts("MPU_IO_STATE_TEST_OK");
    return 0;
}
