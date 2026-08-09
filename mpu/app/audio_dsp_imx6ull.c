#include "aud_mpu_alsa.h"
#include "aud_mpu_fx_chain.h"
#include "aud_mpu_pcm.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define AUD_MPU_MAX_PERIOD_FRAMES 1024u
#define AUD_MPU_DELAY_SAMPLES     48000u

static volatile sig_atomic_t stop_requested;
static float work[AUD_MPU_MAX_PERIOD_FRAMES * 2u];
static int32_t capture_buffer[AUD_MPU_MAX_PERIOD_FRAMES * 2u];
static int32_t playback_buffer[AUD_MPU_MAX_PERIOD_FRAMES * 2u];
static float delay_left[AUD_MPU_DELAY_SAMPLES];
static float delay_right[AUD_MPU_DELAY_SAMPLES];
static AudMpuPcm pcm;
static AudMpuFxChain fx;
static AudMpuAlsa alsa;

static void on_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static int parse_u32(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed = strtoul(text, &end, 10);
    if (text == end || end == NULL || *end != '\0' || parsed > UINT32_MAX) {
        return -1;
    }
    *value = (uint32_t)parsed;
    return 0;
}

static int enable_realtime(int priority)
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        fprintf(stderr, "warning: mlockall failed: %s\n", strerror(errno));
    }
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        fprintf(stderr, "warning: SCHED_FIFO unavailable; run with CAP_SYS_NICE\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    AudMpuAlsaConfig config = {
        "default", "default", 48000u, 128u, 4u, AUD_MPU_PCM_S32_LE
    };
    AudMpuFxParams params;
    int i;
    int status;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--capture") == 0 && i + 1 < argc) {
            config.capture_device = argv[++i];
        } else if (strcmp(argv[i], "--playback") == 0 && i + 1 < argc) {
            config.playback_device = argv[++i];
        } else if (strcmp(argv[i], "--rate") == 0 && i + 1 < argc) {
            if (parse_u32(argv[++i], &config.sample_rate) != 0) return 2;
        } else if (strcmp(argv[i], "--period") == 0 && i + 1 < argc) {
            if (parse_u32(argv[++i], &config.period_frames) != 0) return 2;
        } else if (strcmp(argv[i], "--format") == 0 && i + 1 < argc) {
            const char *format = argv[++i];
            if (strcmp(format, "s16") == 0) config.format = AUD_MPU_PCM_S16_LE;
            else if (strcmp(format, "s32") == 0) config.format = AUD_MPU_PCM_S32_LE;
            else return 2;
        } else {
            fprintf(stderr,
                "usage: %s [--capture DEV] [--playback DEV] "
                "[--rate HZ] [--period FRAMES] [--format s16|s32]\n",
                argv[0]);
            return 2;
        }
    }
    if (config.period_frames == 0u ||
        config.period_frames > AUD_MPU_MAX_PERIOD_FRAMES) {
        fprintf(stderr, "period must be 1..%u frames\n",
                AUD_MPU_MAX_PERIOD_FRAMES);
        return 2;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    AudMpuFxParams_Default(&params);
    if (AudMpuFxChain_Init(&fx, (float)config.sample_rate,
                           delay_left, delay_right,
                           AUD_MPU_DELAY_SAMPLES) != 0 ||
        AudMpuPcm_Init(&pcm, work, AUD_MPU_MAX_PERIOD_FRAMES * 2u,
                       AUD_MPU_MAX_PERIOD_FRAMES, config.format,
                       AudMpuFxChain_Process, &fx) != 0) {
        return 1;
    }
    AudMpuFxChain_SetParams(&fx, &params);

    status = AudMpuAlsa_Open(&alsa, &config, &pcm,
                             capture_buffer, playback_buffer,
                             (uint32_t)sizeof(capture_buffer));
    if (status < 0) {
        fprintf(stderr, "ALSA open/config failed: %s\n", snd_strerror(status));
        return 1;
    }
    (void)enable_realtime(60);
    fprintf(stderr, "audio running; Ctrl-C to stop\n");
    status = AudMpuAlsa_Run(&alsa, &stop_requested);
    AudMpuAlsa_Close(&alsa);
    fprintf(stderr,
        "frames=%llu capture_xruns=%u playback_xruns=%u "
        "short_reads=%u short_writes=%u\n",
        (unsigned long long)alsa.stats.frames,
        alsa.stats.capture_xruns, alsa.stats.playback_xruns,
        alsa.stats.short_reads, alsa.stats.short_writes);
    if (status < 0) {
        fprintf(stderr, "audio loop failed: %s\n", snd_strerror(status));
        return 1;
    }
    return 0;
}
