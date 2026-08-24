#include "aud_mpu_alsa.h"
#include "aud_mpu_config.h"
#include "aud_mpu_fx_chain.h"
#include "aud_mpu_pcm.h"

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef AUD_MPU_VERSION
#define AUD_MPU_VERSION "development"
#endif

static volatile sig_atomic_t stop_requested;

static void on_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static void usage(FILE *stream, const char *program)
{
    (void)fprintf(stream,
        "Usage: %s [options]\n"
        "  --config FILE       load strict INI configuration\n"
        "  --capture DEVICE    override capture PCM\n"
        "  --playback DEVICE   override playback PCM\n"
        "  --rate HZ           override sample rate\n"
        "  --period FRAMES     override period size\n"
        "  --periods COUNT     override ALSA buffer periods\n"
        "  --format s16|s32    override PCM format\n"
        "  --rt-priority N     SCHED_FIFO priority, 0 disables it\n"
        "  --no-realtime       disable SCHED_FIFO and mlockall\n"
        "  --allow-realtime-fallback  continue if realtime setup fails\n"
        "  --allow-unlinked    permit unsynchronized PCM fallback (test only)\n"
        "  --run-seconds N     stop automatically (smoke testing)\n"
        "  --check-config      validate and print effective config\n"
        "  --probe             open/configure/close both PCM streams\n"
        "  --version           print version\n"
        "  --help              show this help\n",
        program);
}

static int copy_text(char *destination, size_t size, const char *source)
{
    size_t length = strlen(source);
    if (length == 0u || length >= size) return -1;
    memcpy(destination, source, length + 1u);
    return 0;
}

static int parse_u32(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;
    if (text == NULL || text[0] < '0' || text[0] > '9') return -1;
    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || text == end || end == NULL || *end != '\0' ||
        parsed > UINT32_MAX) return -1;
    *value = (uint32_t)parsed;
    return 0;
}

static int configure_realtime(const AudMpuAppConfig *config, int *locked)
{
    struct sched_param parameter;
    int status = 0;
    *locked = 0;
    if (config->lock_memory) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
            (void)fprintf(stderr, "warning: mlockall failed: %s\n",
                          strerror(errno));
            status = -1;
        } else {
            *locked = 1;
        }
    }
    if (config->realtime_priority > 0) {
        memset(&parameter, 0, sizeof(parameter));
        parameter.sched_priority = config->realtime_priority;
        int error_number = pthread_setschedparam(
            pthread_self(), SCHED_FIFO, &parameter);
        if (error_number != 0) {
            (void)fprintf(stderr,
                          "warning: SCHED_FIFO unavailable: %s\n",
                          strerror(error_number));
            status = -1;
        }
    }
    return status;
}

static int install_signal_handlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = on_signal;
    if (sigemptyset(&action.sa_mask) != 0) return -1;
    action.sa_flags = 0; /* Deliberately omit SA_RESTART: unblock ALSA I/O. */
    if (sigaction(SIGINT, &action, NULL) != 0 ||
        sigaction(SIGTERM, &action, NULL) != 0 ||
        sigaction(SIGALRM, &action, NULL) != 0) return -1;
    return 0;
}

static int load_requested_config(int argc,
                                 char **argv,
                                 AudMpuAppConfig *config,
                                 char *error,
                                 size_t error_size)
{
    int index;
    AudMpuAppConfig_Default(config);
    for (index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--config") == 0) {
            if (index + 1 >= argc) {
                (void)snprintf(error, error_size, "--config requires FILE");
                return -1;
            }
            if (AudMpuAppConfig_Load(config, argv[index + 1],
                                     error, error_size) != 0) return -1;
            ++index;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    AudMpuAppConfig app_config;
    AudMpuAlsaConfig alsa_config;
    AudMpuPcm pcm;
    AudMpuFxChain effects;
    AudMpuAlsa alsa;
    char error[256];
    float *work = NULL;
    float *delay_left = NULL;
    float *delay_right = NULL;
    void *capture_buffer = NULL;
    void *playback_buffer = NULL;
    uint32_t run_seconds = 0u;
    uint32_t delay_samples;
    uint32_t frame_bytes;
    uint64_t delay_calculation;
    int check_config = 0;
    int probe = 0;
    int locked = 0;
    int status = 1;
    int index;

    if (load_requested_config(argc, argv, &app_config,
                              error, sizeof(error)) != 0) {
        (void)fprintf(stderr, "configuration error: %s\n", error);
        return 2;
    }

    for (index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--config") == 0 && index + 1 < argc) {
            ++index;
        } else if (strcmp(argv[index], "--capture") == 0 && index + 1 < argc) {
            if (copy_text(app_config.capture_device,
                          sizeof(app_config.capture_device), argv[++index]) != 0)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--playback") == 0 && index + 1 < argc) {
            if (copy_text(app_config.playback_device,
                          sizeof(app_config.playback_device), argv[++index]) != 0)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--rate") == 0 && index + 1 < argc) {
            if (parse_u32(argv[++index], &app_config.sample_rate) != 0)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--period") == 0 && index + 1 < argc) {
            if (parse_u32(argv[++index], &app_config.period_frames) != 0)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--periods") == 0 && index + 1 < argc) {
            if (parse_u32(argv[++index], &app_config.periods) != 0)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--format") == 0 && index + 1 < argc) {
            const char *format = argv[++index];
            if (strcmp(format, "s16") == 0)
                app_config.format = AUD_MPU_PCM_S16_LE;
            else if (strcmp(format, "s32") == 0)
                app_config.format = AUD_MPU_PCM_S32_LE;
            else goto bad_arguments;
        } else if (strcmp(argv[index], "--rt-priority") == 0 &&
                   index + 1 < argc) {
            uint32_t priority;
            if (parse_u32(argv[++index], &priority) != 0 || priority > 95u)
                goto bad_arguments;
            app_config.realtime_priority = (int)priority;
        } else if (strcmp(argv[index], "--run-seconds") == 0 &&
                   index + 1 < argc) {
            if (parse_u32(argv[++index], &run_seconds) != 0 ||
                run_seconds > 604800u)
                goto bad_arguments;
        } else if (strcmp(argv[index], "--no-realtime") == 0) {
            app_config.realtime_priority = 0;
            app_config.lock_memory = 0;
        } else if (strcmp(argv[index], "--allow-realtime-fallback") == 0) {
            app_config.allow_realtime_fallback = 1;
        } else if (strcmp(argv[index], "--allow-unlinked") == 0) {
            app_config.require_duplex_link = 0;
        } else if (strcmp(argv[index], "--check-config") == 0) {
            check_config = 1;
        } else if (strcmp(argv[index], "--probe") == 0) {
            probe = 1;
        } else if (strcmp(argv[index], "--version") == 0) {
            (void)printf("audio_dsp_imx6ull %s\n", AUD_MPU_VERSION);
            return 0;
        } else if (strcmp(argv[index], "--help") == 0) {
            usage(stdout, argv[0]);
            return 0;
        } else {
            goto bad_arguments;
        }
    }

    if (AudMpuAppConfig_Validate(&app_config, error, sizeof(error)) != 0) {
        (void)fprintf(stderr, "configuration error: %s\n", error);
        return 2;
    }
    if (check_config) {
        AudMpuAppConfig_Print(&app_config, stdout);
        return 0;
    }

    delay_calculation = (uint64_t)app_config.sample_rate *
                        app_config.max_delay_ms / 1000u + 2u;
    if (delay_calculation > UINT32_MAX) {
        (void)fprintf(stderr, "delay allocation is too large\n");
        return 2;
    }
    delay_samples = (uint32_t)delay_calculation;
    frame_bytes = app_config.format == AUD_MPU_PCM_S16_LE ? 4u : 8u;

    work = (float *)calloc((size_t)app_config.period_frames * 2u,
                           sizeof(float));
    delay_left = (float *)calloc(delay_samples, sizeof(float));
    delay_right = (float *)calloc(delay_samples, sizeof(float));
    capture_buffer = calloc(app_config.period_frames, frame_bytes);
    playback_buffer = calloc(app_config.period_frames, frame_bytes);
    if (work == NULL || delay_left == NULL || delay_right == NULL ||
        capture_buffer == NULL || playback_buffer == NULL) {
        (void)fprintf(stderr, "startup buffer allocation failed\n");
        goto cleanup;
    }

    if (AudMpuFxChain_Init(&effects, (float)app_config.sample_rate,
                           delay_left, delay_right, delay_samples) != 0 ||
        AudMpuPcm_Init(&pcm, work, app_config.period_frames * 2u,
                       app_config.period_frames, app_config.format,
                       AudMpuFxChain_Process, &effects) != 0) {
        (void)fprintf(stderr, "DSP initialization failed\n");
        goto cleanup;
    }
    AudMpuFxChain_SetParams(&effects, &app_config.effects);

    alsa_config.capture_device = app_config.capture_device;
    alsa_config.playback_device = app_config.playback_device;
    alsa_config.sample_rate = app_config.sample_rate;
    alsa_config.period_frames = app_config.period_frames;
    alsa_config.periods = app_config.periods;
    alsa_config.format = app_config.format;
    alsa_config.require_duplex_link = app_config.require_duplex_link;
    if (install_signal_handlers() != 0) {
        (void)fprintf(stderr, "cannot install signal handlers: %s\n",
                      strerror(errno));
        status = 1;
        goto cleanup;
    }
    status = AudMpuAlsa_Open(&alsa, &alsa_config, &pcm,
                             capture_buffer, playback_buffer,
                             app_config.period_frames * frame_bytes);
    if (status < 0) {
        (void)fprintf(stderr, "ALSA open/config failed: %s\n",
                      snd_strerror(status));
        status = 1;
        goto cleanup;
    }
    if (probe) {
        (void)printf("MPU_ALSA_PROBE_OK capture=%s playback=%s rate=%" PRIu32
                     " period=%" PRIu32 " periods=%" PRIu32
                     " buffer=%" PRIu32 " linked=%s format=%s\n",
                     app_config.capture_device, app_config.playback_device,
                     app_config.sample_rate, app_config.period_frames,
                     alsa.actual_periods, alsa.actual_buffer_frames,
                     alsa.streams_linked ? "yes" : "no",
                     app_config.format == AUD_MPU_PCM_S16_LE ? "s16" : "s32");
        status = 0;
        goto close_alsa;
    }

    if (run_seconds > 0u) (void)alarm(run_seconds);
    if (configure_realtime(&app_config, &locked) != 0 &&
        !app_config.allow_realtime_fallback) {
        (void)fprintf(stderr,
                      "realtime setup failed; set realtime.allow_fallback=true "
                      "only after accepting non-realtime operation\n");
        status = 1;
        goto close_alsa;
    }
    (void)fprintf(stderr,
                  "audio running: %s -> %s, %" PRIu32 " Hz, %" PRIu32
                  " frames; "
                  "SIGINT/SIGTERM to stop\n",
                  app_config.capture_device, app_config.playback_device,
                  app_config.sample_rate, app_config.period_frames);
    status = AudMpuAlsa_Run(&alsa, &stop_requested);
    (void)fprintf(stderr,
        "frames=%llu capture_xruns=%u playback_xruns=%u "
        "pair_restarts=%u short_reads=%u short_writes=%u\n",
        (unsigned long long)alsa.stats.frames,
        alsa.stats.capture_xruns, alsa.stats.playback_xruns,
        alsa.stats.pair_restarts,
        alsa.stats.short_reads, alsa.stats.short_writes);
    if (status < 0) {
        (void)fprintf(stderr, "audio loop failed: %s\n", snd_strerror(status));
        status = 1;
    } else {
        status = 0;
    }

close_alsa:
    AudMpuAlsa_Close(&alsa);
cleanup:
    if (locked) (void)munlockall();
    free(playback_buffer);
    free(capture_buffer);
    free(delay_right);
    free(delay_left);
    free(work);
    return status;

bad_arguments:
    usage(stderr, argv[0]);
    return 2;
}
