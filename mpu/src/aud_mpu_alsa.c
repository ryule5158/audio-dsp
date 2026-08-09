#include "aud_mpu_alsa.h"

#include <alloca.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static snd_pcm_format_t aud_mpu_alsa_format(AudMpuPcmFormat format)
{
    return format == AUD_MPU_PCM_S16_LE
        ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_S32_LE;
}

static int aud_mpu_configure_pcm(snd_pcm_t *handle,
                                 snd_pcm_stream_t stream,
                                 AudMpuAlsaConfig *config)
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
        (status = snd_pcm_hw_params(handle, hw)) < 0) {
        return status;
    }

    if (rate != config->sample_rate || period != config->period_frames) {
        return -EINVAL;
    }

    if ((status = snd_pcm_sw_params_current(handle, sw)) < 0 ||
        (status = snd_pcm_sw_params_set_avail_min(handle, sw, period)) < 0) {
        return status;
    }
    if (stream == SND_PCM_STREAM_PLAYBACK) {
        snd_pcm_uframes_t threshold = buffer > period ? buffer - period : period;
        if ((status = snd_pcm_sw_params_set_start_threshold(
                 handle, sw, threshold)) < 0) {
            return status;
        }
    } else if ((status = snd_pcm_sw_params_set_start_threshold(
                    handle, sw, 1u)) < 0) {
        return status;
    }
    return snd_pcm_sw_params(handle, sw);
}

static int aud_mpu_recover(snd_pcm_t *handle, int error, uint32_t *counter)
{
    int status;
    if (counter != NULL && (error == -EPIPE || error == -ESTRPIPE)) {
        (*counter)++;
    }
    status = snd_pcm_recover(handle, error, 1);
    return status < 0 ? status : 0;
}

int AudMpuAlsa_Open(AudMpuAlsa *alsa,
                    const AudMpuAlsaConfig *config,
                    AudMpuPcm *pcm,
                    void *capture_buffer,
                    void *playback_buffer,
                    uint32_t buffer_bytes)
{
    uint32_t required_bytes;
    int status;

    if (alsa == NULL || config == NULL || pcm == NULL ||
        capture_buffer == NULL || playback_buffer == NULL ||
        config->capture_device == NULL || config->playback_device == NULL ||
        config->period_frames == 0u || config->periods < 2u ||
        config->period_frames > pcm->max_frames || config->format != pcm->format) {
        return -EINVAL;
    }
    required_bytes = config->period_frames * AudMpuPcm_FrameBytes(pcm);
    if (buffer_bytes < required_bytes) {
        return -ENOMEM;
    }

    memset(alsa, 0, sizeof(*alsa));
    alsa->config = *config;
    alsa->pcm = pcm;
    alsa->capture_buffer = capture_buffer;
    alsa->playback_buffer = playback_buffer;
    alsa->buffer_bytes = buffer_bytes;

    status = snd_pcm_open(&alsa->capture, config->capture_device,
                          SND_PCM_STREAM_CAPTURE, 0);
    if (status < 0) {
        goto fail;
    }
    status = snd_pcm_open(&alsa->playback, config->playback_device,
                          SND_PCM_STREAM_PLAYBACK, 0);
    if (status < 0) {
        goto fail;
    }
    status = aud_mpu_configure_pcm(alsa->capture, SND_PCM_STREAM_CAPTURE,
                                   &alsa->config);
    if (status < 0) {
        goto fail;
    }
    status = aud_mpu_configure_pcm(alsa->playback, SND_PCM_STREAM_PLAYBACK,
                                   &alsa->config);
    if (status < 0) {
        goto fail;
    }
    if ((status = snd_pcm_prepare(alsa->capture)) < 0 ||
        (status = snd_pcm_prepare(alsa->playback)) < 0) {
        goto fail;
    }
    return 0;

fail:
    AudMpuAlsa_Close(alsa);
    return status;
}

int AudMpuAlsa_Run(AudMpuAlsa *alsa, volatile sig_atomic_t *stop_requested)
{
    uint32_t frame_bytes;

    if (alsa == NULL || alsa->capture == NULL || alsa->playback == NULL ||
        alsa->pcm == NULL || stop_requested == NULL) {
        return -EINVAL;
    }
    frame_bytes = AudMpuPcm_FrameBytes(alsa->pcm);

    while (!*stop_requested) {
        snd_pcm_sframes_t captured = snd_pcm_readi(
            alsa->capture, alsa->capture_buffer, alsa->config.period_frames);
        if (captured < 0) {
            int status = aud_mpu_recover(alsa->capture, (int)captured,
                                         &alsa->stats.capture_xruns);
            if (status < 0) {
                return status;
            }
            continue;
        }
        if (captured == 0) {
            continue;
        }
        if ((uint32_t)captured != alsa->config.period_frames) {
            alsa->stats.short_reads++;
        }
        if (AudMpuPcm_Process(alsa->pcm, alsa->capture_buffer,
                             alsa->playback_buffer, (uint32_t)captured) != 0) {
            return -EINVAL;
        }

        snd_pcm_sframes_t offset = 0;
        while (offset < captured && !*stop_requested) {
            uint8_t *output = (uint8_t *)alsa->playback_buffer +
                (uint32_t)offset * frame_bytes;
            snd_pcm_sframes_t written = snd_pcm_writei(
                alsa->playback, output, (snd_pcm_uframes_t)(captured - offset));
            if (written < 0) {
                int status = aud_mpu_recover(alsa->playback, (int)written,
                                             &alsa->stats.playback_xruns);
                if (status < 0) {
                    return status;
                }
                continue;
            }
            if (written == 0) {
                continue;
            }
            if (written < captured - offset) {
                alsa->stats.short_writes++;
            }
            offset += written;
        }
        alsa->stats.frames += (uint64_t)captured;
    }
    return 0;
}

void AudMpuAlsa_Close(AudMpuAlsa *alsa)
{
    if (alsa == NULL) {
        return;
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
