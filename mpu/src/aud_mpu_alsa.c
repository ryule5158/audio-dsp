#include "aud_mpu_alsa.h"
#include "aud_mpu_io_state.h"

#include <alloca.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t rate;
    uint32_t period_frames;
    uint32_t periods;
    uint32_t buffer_frames;
} AudMpuAlsaParams;

static snd_pcm_format_t aud_mpu_alsa_format(AudMpuPcmFormat format)
{
    return format == AUD_MPU_PCM_S16_LE
        ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_S32_LE;
}

static int aud_mpu_configure_pcm(snd_pcm_t *handle,
                                 snd_pcm_stream_t stream,
                                 const AudMpuAlsaConfig *config,
                                 AudMpuAlsaParams *actual)
{
    snd_pcm_hw_params_t *hw;
    snd_pcm_sw_params_t *sw;
    unsigned int rate = config->sample_rate;
    unsigned int periods = config->periods;
    snd_pcm_uframes_t period = config->period_frames;
    snd_pcm_uframes_t buffer = period * periods;
    int direction = 0;
    int status;

    snd_pcm_hw_params_alloca(&hw);
    snd_pcm_sw_params_alloca(&sw);
    if ((status = snd_pcm_hw_params_any(handle, hw)) < 0 ||
        (status = snd_pcm_hw_params_set_access(
             handle, hw, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0 ||
        (status = snd_pcm_hw_params_set_format(
             handle, hw, aud_mpu_alsa_format(config->format))) < 0 ||
        (status = snd_pcm_hw_params_set_channels(handle, hw, 2u)) < 0 ||
        (status = snd_pcm_hw_params_set_rate_near(
             handle, hw, &rate, &direction)) < 0 ||
        (status = snd_pcm_hw_params_set_period_size_near(
             handle, hw, &period, &direction)) < 0 ||
        (status = snd_pcm_hw_params_set_periods_near(
             handle, hw, &periods, &direction)) < 0 ||
        (status = snd_pcm_hw_params_set_buffer_size_near(
             handle, hw, &buffer)) < 0 ||
        (status = snd_pcm_hw_params(handle, hw)) < 0 ||
        (status = snd_pcm_hw_params_get_rate(hw, &rate, &direction)) < 0 ||
        (status = snd_pcm_hw_params_get_period_size(hw, &period, &direction)) < 0 ||
        (status = snd_pcm_hw_params_get_periods(hw, &periods, &direction)) < 0 ||
        (status = snd_pcm_hw_params_get_buffer_size(hw, &buffer)) < 0) {
        return status;
    }
    if (rate != config->sample_rate || period != config->period_frames ||
        periods != config->periods ||
        buffer != (snd_pcm_uframes_t)config->period_frames * config->periods) {
        return -EINVAL;
    }
    actual->rate = rate;
    actual->period_frames = (uint32_t)period;
    actual->periods = periods;
    actual->buffer_frames = (uint32_t)buffer;
    if ((status = snd_pcm_sw_params_current(handle, sw)) < 0 ||
        (status = snd_pcm_sw_params_set_avail_min(handle, sw, period)) < 0 ||
        (status = snd_pcm_sw_params_set_start_threshold(
             handle, sw, stream == SND_PCM_STREAM_PLAYBACK ? buffer : 1u)) < 0) {
        return status;
    }
    return snd_pcm_sw_params(handle, sw);
}

static int aud_mpu_link(AudMpuAlsa *alsa)
{
    int status = snd_pcm_link(alsa->playback, alsa->capture);
    if (status < 0) {
        alsa->streams_linked = 0;
        return alsa->config.require_duplex_link ? status : 0;
    }
    alsa->streams_linked = 1;
    return 0;
}

static int aud_mpu_prime_playback(AudMpuAlsa *alsa)
{
    uint32_t queued = 0u;
    uint32_t target = alsa->actual_buffer_frames - alsa->actual_period_frames;
    memset(alsa->playback_buffer, 0, alsa->buffer_bytes);
    while (queued < target) {
        uint32_t request = target - queued;
        snd_pcm_sframes_t written;
        if (request > alsa->actual_period_frames) request = alsa->actual_period_frames;
        written = snd_pcm_writei(alsa->playback, alsa->playback_buffer, request);
        if (written == -EINTR || written == -EAGAIN) continue;
        if (written <= 0) return written == 0 ? -EIO : (int)written;
        queued += (uint32_t)written;
    }
    return 0;
}

static int aud_mpu_start_pair(AudMpuAlsa *alsa)
{
    int status;
    if (alsa->streams_linked) {
        (void)snd_pcm_unlink(alsa->playback);
        alsa->streams_linked = 0;
    }
    (void)snd_pcm_drop(alsa->capture);
    (void)snd_pcm_drop(alsa->playback);
    if ((status = snd_pcm_prepare(alsa->capture)) < 0 ||
        (status = snd_pcm_prepare(alsa->playback)) < 0 ||
        (status = aud_mpu_link(alsa)) < 0 ||
        (status = aud_mpu_prime_playback(alsa)) < 0) return status;
    if (alsa->streams_linked) return snd_pcm_start(alsa->capture);
    if ((status = snd_pcm_start(alsa->playback)) < 0) return status;
    return snd_pcm_start(alsa->capture);
}

static int aud_mpu_open_blocking(snd_pcm_t **handle, const char *device,
                                 snd_pcm_stream_t stream)
{
    int status = snd_pcm_open(handle, device, stream, SND_PCM_NONBLOCK);
    if (status < 0) return status;
    status = snd_pcm_nonblock(*handle, 0);
    if (status < 0) {
        (void)snd_pcm_close(*handle);
        *handle = NULL;
    }
    return status;
}

int AudMpuAlsa_Open(AudMpuAlsa *alsa, const AudMpuAlsaConfig *config,
                    AudMpuPcm *pcm, void *capture_buffer,
                    void *playback_buffer, uint32_t buffer_bytes)
{
    AudMpuAlsaParams capture_params;
    AudMpuAlsaParams playback_params;
    uint32_t required_bytes;
    int status;
    if (alsa == NULL || config == NULL || pcm == NULL ||
        capture_buffer == NULL || playback_buffer == NULL ||
        config->capture_device == NULL || config->playback_device == NULL ||
        config->period_frames == 0u || config->periods < 2u ||
        config->period_frames > pcm->max_frames || config->format != pcm->format)
        return -EINVAL;
    required_bytes = config->period_frames * AudMpuPcm_FrameBytes(pcm);
    if (buffer_bytes < required_bytes) return -ENOMEM;
    memset(alsa, 0, sizeof(*alsa));
    alsa->config = *config;
    alsa->pcm = pcm;
    alsa->capture_buffer = capture_buffer;
    alsa->playback_buffer = playback_buffer;
    alsa->buffer_bytes = buffer_bytes;
    status = aud_mpu_open_blocking(&alsa->capture, config->capture_device,
                                   SND_PCM_STREAM_CAPTURE);
    if (status < 0) goto fail;
    status = aud_mpu_open_blocking(&alsa->playback, config->playback_device,
                                   SND_PCM_STREAM_PLAYBACK);
    if (status < 0) goto fail;
    status = aud_mpu_configure_pcm(alsa->capture, SND_PCM_STREAM_CAPTURE,
                                   config, &capture_params);
    if (status < 0) goto fail;
    status = aud_mpu_configure_pcm(alsa->playback, SND_PCM_STREAM_PLAYBACK,
                                   config, &playback_params);
    if (status < 0) goto fail;
    if (memcmp(&capture_params, &playback_params, sizeof(capture_params)) != 0) {
        status = -EINVAL;
        goto fail;
    }
    alsa->actual_rate = capture_params.rate;
    alsa->actual_period_frames = capture_params.period_frames;
    alsa->actual_periods = capture_params.periods;
    alsa->actual_buffer_frames = capture_params.buffer_frames;
    status = aud_mpu_start_pair(alsa);
    if (status < 0) goto fail;
    return 0;
fail:
    AudMpuAlsa_Close(alsa);
    return status;
}

int AudMpuAlsa_Run(AudMpuAlsa *alsa, volatile sig_atomic_t *stop_requested)
{
    uint32_t frame_bytes;
    if (alsa == NULL || alsa->capture == NULL || alsa->playback == NULL ||
        alsa->pcm == NULL || stop_requested == NULL) return -EINVAL;
    frame_bytes = AudMpuPcm_FrameBytes(alsa->pcm);
    while (!*stop_requested) {
        uint32_t captured_total = 0u;
        int restart = 0;
        while (captured_total < alsa->config.period_frames) {
            uint8_t *input = (uint8_t *)alsa->capture_buffer +
                             captured_total * frame_bytes;
            uint32_t request = alsa->config.period_frames - captured_total;
            snd_pcm_sframes_t result = snd_pcm_readi(alsa->capture, input, request);
            AudMpuIoDecision decision = AudMpuIo_Classify(
                (long)result, request, *stop_requested);
            if (decision.action == AUD_MPU_IO_STOP) return 0;
            if (decision.action == AUD_MPU_IO_RETRY) continue;
            if (decision.action == AUD_MPU_IO_RESTART_PAIR) {
                alsa->stats.capture_xruns++;
                restart = 1;
                break;
            }
            if (decision.action == AUD_MPU_IO_FATAL) return (int)result;
            if (decision.short_transfer) alsa->stats.short_reads++;
            captured_total += decision.frames;
        }
        if (restart) {
            int status = aud_mpu_start_pair(alsa);
            if (status < 0) return status;
            alsa->stats.pair_restarts++;
            continue;
        }
        if (AudMpuPcm_Process(alsa->pcm, alsa->capture_buffer,
                             alsa->playback_buffer,
                             alsa->config.period_frames) != 0) return -EINVAL;
        {
            uint32_t written_total = 0u;
            while (written_total < alsa->config.period_frames) {
                uint8_t *output = (uint8_t *)alsa->playback_buffer +
                                  written_total * frame_bytes;
                uint32_t request = alsa->config.period_frames - written_total;
                snd_pcm_sframes_t result = snd_pcm_writei(alsa->playback,
                                                          output, request);
                AudMpuIoDecision decision = AudMpuIo_Classify(
                    (long)result, request, *stop_requested);
                if (decision.action == AUD_MPU_IO_STOP) return 0;
                if (decision.action == AUD_MPU_IO_RETRY) continue;
                if (decision.action == AUD_MPU_IO_RESTART_PAIR) {
                    int status;
                    alsa->stats.playback_xruns++;
                    status = aud_mpu_start_pair(alsa);
                    if (status < 0) return status;
                    alsa->stats.pair_restarts++;
                    restart = 1;
                    break;
                }
                if (decision.action == AUD_MPU_IO_FATAL) return (int)result;
                if (decision.short_transfer) alsa->stats.short_writes++;
                written_total += decision.frames;
            }
        }
        if (!restart) alsa->stats.frames += alsa->config.period_frames;
    }
    return 0;
}

void AudMpuAlsa_Close(AudMpuAlsa *alsa)
{
    if (alsa == NULL) return;
    if (alsa->streams_linked && alsa->playback != NULL) {
        (void)snd_pcm_unlink(alsa->playback);
        alsa->streams_linked = 0;
    }
    if (alsa->capture != NULL) {
        (void)snd_pcm_drop(alsa->capture);
        (void)snd_pcm_close(alsa->capture);
        alsa->capture = NULL;
    }
    if (alsa->playback != NULL) {
        (void)snd_pcm_drop(alsa->playback);
        (void)snd_pcm_close(alsa->playback);
        alsa->playback = NULL;
    }
}
