#include "aud_mpu_config.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

static int write_text(const char *path, const char *text)
{
    FILE *stream = fopen(path, "w");
    if (stream == NULL) return -1;
    if (fputs(text, stream) == EOF) {
        (void)fclose(stream);
        return -1;
    }
    return fclose(stream);
}

int main(void)
{
    const char *valid_path = "test_mpu_config_valid.ini";
    const char *invalid_path = "test_mpu_config_invalid.ini";
    AudMpuAppConfig config;
    char error[256];
    AudMpuAppConfig_Default(&config);
    if (write_text(valid_path,
            "[audio]\ncapture_device=hw:CARD=wm8960audio,DEV=0\n"
            "playback_device=hw:CARD=wm8960audio,DEV=0\n"
            "sample_rate=48000\nperiod_frames=128\nperiods=3\n"
            "format=s16\nmax_delay_ms=1500\nrequire_duplex_link=true\n"
            "[realtime]\npriority=55\nlock_memory=false\nallow_fallback=true\n"
            "[effects]\ninput_gain=1.25\nfilter_cutoff_hz=12000\n"
            "filter_resonance=0.2\nfilter_mix=0.4\ndrive=0.3\n"
            "delay_ms=375\ndelay_feedback=0.45\n"
            "delay_crossfeed=0.1\ndelay_mix=0.5\noutput_gain=0.75\n") != 0 ||
        AudMpuAppConfig_Load(&config, valid_path, error, sizeof(error)) != 0) {
        fprintf(stderr, "valid config rejected: %s\n", error);
        return 1;
    }
    if (config.period_frames != 128u || config.periods != 3u ||
        config.realtime_priority != 55 || config.lock_memory != 0 ||
        config.require_duplex_link != 1 || config.allow_realtime_fallback != 1 ||
        config.effects.delay_ms != 375.0f) {
        fprintf(stderr, "valid config values were not applied\n");
        return 1;
    }
    AudMpuAppConfig_Default(&config);
    if (write_text(invalid_path, "[audio]\nperiod_frames=-1\n") != 0 ||
        AudMpuAppConfig_Load(&config, invalid_path, error, sizeof(error)) == 0) {
        fprintf(stderr, "negative unsigned value was not rejected\n");
        return 1;
    }
    AudMpuAppConfig_Default(&config);
    config.effects.input_gain = NAN;
    if (AudMpuAppConfig_Validate(&config, error, sizeof(error)) == 0 ||
        strstr(error, "finite") == NULL) {
        fprintf(stderr, "programmatic NaN was not rejected: %s\n", error);
        return 1;
    }
    AudMpuAppConfig_Default(&config);
    if (write_text(invalid_path,
            "[audio]\nperiod_frames=8\nunknown=1\n") != 0 ||
        AudMpuAppConfig_Load(&config, invalid_path, error, sizeof(error)) == 0 ||
        strstr(error, "unknown key") == NULL) {
        fprintf(stderr, "invalid config was not rejected: %s\n", error);
        return 1;
    }
    (void)remove(valid_path);
    (void)remove(invalid_path);
    puts("MPU_CONFIG_TEST_OK");
    return 0;
}
