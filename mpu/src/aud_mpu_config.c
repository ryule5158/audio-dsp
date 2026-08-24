#include "aud_mpu_config.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUD_MPU_CONFIG_LINE_MAX 512u

static void aud_mpu_error(char *error, size_t size, const char *format, ...)
{
    va_list args;
    if (error == NULL || size == 0u) return;
    va_start(args, format);
    (void)vsnprintf(error, size, format, args);
    va_end(args);
}

static char *aud_mpu_trim(char *text)
{
    char *end;
    while (*text != '\0' && isspace((unsigned char)*text)) ++text;
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return text;
}

static int aud_mpu_copy(char *destination, size_t size, const char *value)
{
    size_t length = strlen(value);
    if (length == 0u || length >= size) return -1;
    memcpy(destination, value, length + 1u);
    return 0;
}

static int aud_mpu_u32(const char *value, uint32_t *result)
{
    char *end = NULL;
    unsigned long parsed;
    if (value == NULL || !isdigit((unsigned char)value[0])) return -1;
    errno = 0;
    parsed = strtoul(value, &end, 10);
    if (errno != 0 || value == end || end == NULL || *end != '\0' ||
        parsed > UINT32_MAX) return -1;
    *result = (uint32_t)parsed;
    return 0;
}

static int aud_mpu_i32(const char *value, int *result)
{
    char *end = NULL;
    long parsed;
    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno != 0 || value == end || end == NULL || *end != '\0' ||
        parsed < -2147483647L - 1L || parsed > 2147483647L) return -1;
    *result = (int)parsed;
    return 0;
}

static int aud_mpu_float(const char *value, float *result)
{
    char *end = NULL;
    float parsed;
    errno = 0;
    parsed = strtof(value, &end);
    if (errno != 0 || value == end || end == NULL || *end != '\0' ||
        !isfinite(parsed)) return -1;
    *result = parsed;
    return 0;
}

static int aud_mpu_bool(const char *value, int *result)
{
    if (strcmp(value, "true") == 0 || strcmp(value, "yes") == 0 ||
        strcmp(value, "1") == 0) {
        *result = 1;
        return 0;
    }
    if (strcmp(value, "false") == 0 || strcmp(value, "no") == 0 ||
        strcmp(value, "0") == 0) {
        *result = 0;
        return 0;
    }
    return -1;
}

static int aud_mpu_set(AudMpuAppConfig *config,
                       const char *section,
                       const char *key,
                       const char *value)
{
    if (strcmp(section, "audio") == 0) {
        if (strcmp(key, "capture_device") == 0)
            return aud_mpu_copy(config->capture_device,
                                sizeof(config->capture_device), value);
        if (strcmp(key, "playback_device") == 0)
            return aud_mpu_copy(config->playback_device,
                                sizeof(config->playback_device), value);
        if (strcmp(key, "sample_rate") == 0)
            return aud_mpu_u32(value, &config->sample_rate);
        if (strcmp(key, "period_frames") == 0)
            return aud_mpu_u32(value, &config->period_frames);
        if (strcmp(key, "periods") == 0)
            return aud_mpu_u32(value, &config->periods);
        if (strcmp(key, "format") == 0) {
            if (strcmp(value, "s16") == 0) {
                config->format = AUD_MPU_PCM_S16_LE;
                return 0;
            }
            if (strcmp(value, "s32") == 0) {
                config->format = AUD_MPU_PCM_S32_LE;
                return 0;
            }
            return -1;
        }
        if (strcmp(key, "max_delay_ms") == 0)
            return aud_mpu_u32(value, &config->max_delay_ms);
        if (strcmp(key, "require_duplex_link") == 0)
            return aud_mpu_bool(value, &config->require_duplex_link);
        return -2;
    }
    if (strcmp(section, "realtime") == 0) {
        if (strcmp(key, "priority") == 0)
            return aud_mpu_i32(value, &config->realtime_priority);
        if (strcmp(key, "lock_memory") == 0)
            return aud_mpu_bool(value, &config->lock_memory);
        if (strcmp(key, "allow_fallback") == 0)
            return aud_mpu_bool(value, &config->allow_realtime_fallback);
        return -2;
    }
    if (strcmp(section, "effects") == 0) {
        if (strcmp(key, "input_gain") == 0)
            return aud_mpu_float(value, &config->effects.input_gain);
        if (strcmp(key, "filter_cutoff_hz") == 0)
            return aud_mpu_float(value, &config->effects.filter_cutoff_hz);
        if (strcmp(key, "filter_resonance") == 0)
            return aud_mpu_float(value, &config->effects.filter_resonance);
        if (strcmp(key, "filter_mix") == 0)
            return aud_mpu_float(value, &config->effects.filter_mix);
        if (strcmp(key, "drive") == 0)
            return aud_mpu_float(value, &config->effects.drive);
        if (strcmp(key, "delay_ms") == 0)
            return aud_mpu_float(value, &config->effects.delay_ms);
        if (strcmp(key, "delay_feedback") == 0)
            return aud_mpu_float(value, &config->effects.delay_feedback);
        if (strcmp(key, "delay_crossfeed") == 0)
            return aud_mpu_float(value, &config->effects.delay_crossfeed);
        if (strcmp(key, "delay_mix") == 0)
            return aud_mpu_float(value, &config->effects.delay_mix);
        if (strcmp(key, "output_gain") == 0)
            return aud_mpu_float(value, &config->effects.output_gain);
        return -2;
    }
    return -3;
}

void AudMpuAppConfig_Default(AudMpuAppConfig *config)
{
    if (config == NULL) return;
    memset(config, 0, sizeof(*config));
    (void)aud_mpu_copy(config->capture_device,
                       sizeof(config->capture_device),
                       "hw:CARD=wm8960audio,DEV=0");
    (void)aud_mpu_copy(config->playback_device,
                       sizeof(config->playback_device),
                       "hw:CARD=wm8960audio,DEV=0");
    config->sample_rate = 48000u;
    config->period_frames = 256u;
    config->periods = 4u;
    config->format = AUD_MPU_PCM_S16_LE;
    config->max_delay_ms = 1000u;
    config->require_duplex_link = 1;
    config->realtime_priority = 60;
    config->lock_memory = 1;
    config->allow_realtime_fallback = 0;
    AudMpuFxParams_Default(&config->effects);
}

int AudMpuAppConfig_Validate(const AudMpuAppConfig *config,
                             char *error,
                             size_t error_size)
{
#define RANGE(value, low, high, name) \
    do { \
        if ((value) < (low) || (value) > (high)) { \
            aud_mpu_error(error, error_size, "%s outside valid range", name); \
            return -1; \
        } \
    } while (0)
    if (config == NULL || config->capture_device[0] == '\0' ||
        config->playback_device[0] == '\0') {
        aud_mpu_error(error, error_size, "audio device is empty");
        return -1;
    }
    RANGE(config->sample_rate, 8000u, 192000u, "audio.sample_rate");
    RANGE(config->period_frames, 16u, 4096u, "audio.period_frames");
    RANGE(config->periods, 2u, 16u, "audio.periods");
    RANGE(config->max_delay_ms, 1u, 10000u, "audio.max_delay_ms");
    RANGE(config->realtime_priority, 0, 95, "realtime.priority");
    RANGE(config->require_duplex_link, 0, 1, "audio.require_duplex_link");
    RANGE(config->lock_memory, 0, 1, "realtime.lock_memory");
    RANGE(config->allow_realtime_fallback, 0, 1, "realtime.allow_fallback");
#define FINITE(value, name) \
    do { if (!isfinite(value)) { aud_mpu_error(error, error_size, \
        "%s must be finite", name); return -1; } } while (0)
    FINITE(config->effects.input_gain, "effects.input_gain");
    FINITE(config->effects.filter_cutoff_hz, "effects.filter_cutoff_hz");
    FINITE(config->effects.filter_resonance, "effects.filter_resonance");
    FINITE(config->effects.filter_mix, "effects.filter_mix");
    FINITE(config->effects.drive, "effects.drive");
    FINITE(config->effects.delay_ms, "effects.delay_ms");
    FINITE(config->effects.delay_feedback, "effects.delay_feedback");
    FINITE(config->effects.delay_crossfeed, "effects.delay_crossfeed");
    FINITE(config->effects.delay_mix, "effects.delay_mix");
    FINITE(config->effects.output_gain, "effects.output_gain");
    RANGE(config->effects.input_gain, 0.0f, 4.0f, "effects.input_gain");
    RANGE(config->effects.filter_cutoff_hz, 20.0f,
          (float)config->sample_rate / 3.0f, "effects.filter_cutoff_hz");
    RANGE(config->effects.filter_resonance, 0.0f, 1.0f,
          "effects.filter_resonance");
    RANGE(config->effects.filter_mix, 0.0f, 1.0f, "effects.filter_mix");
    RANGE(config->effects.drive, 0.0f, 1.0f, "effects.drive");
    RANGE(config->effects.delay_ms, 0.0f, (float)config->max_delay_ms,
          "effects.delay_ms");
    RANGE(config->effects.delay_feedback, -0.95f, 0.95f,
          "effects.delay_feedback");
    RANGE(config->effects.delay_crossfeed, 0.0f, 1.0f,
          "effects.delay_crossfeed");
    RANGE(config->effects.delay_mix, 0.0f, 1.0f, "effects.delay_mix");
    RANGE(config->effects.output_gain, 0.0f, 2.0f, "effects.output_gain");
    if (config->format != AUD_MPU_PCM_S16_LE &&
        config->format != AUD_MPU_PCM_S32_LE) {
        aud_mpu_error(error, error_size, "audio.format must be s16 or s32");
        return -1;
    }
    if (error != NULL && error_size > 0u) error[0] = '\0';
    return 0;
#undef RANGE
#undef FINITE
}

int AudMpuAppConfig_Load(AudMpuAppConfig *config,
                         const char *path,
                         char *error,
                         size_t error_size)
{
    FILE *stream;
    char line[AUD_MPU_CONFIG_LINE_MAX];
    char section[32] = "";
    unsigned int line_number = 0u;
    if (config == NULL || path == NULL || path[0] == '\0') {
        aud_mpu_error(error, error_size, "invalid config path");
        return -1;
    }
    stream = fopen(path, "r");
    if (stream == NULL) {
        aud_mpu_error(error, error_size, "%s: %s", path, strerror(errno));
        return -1;
    }
    while (fgets(line, sizeof(line), stream) != NULL) {
        char *text;
        char *equals;
        char *comment;
        int status;
        ++line_number;
        if (strchr(line, '\n') == NULL && !feof(stream)) {
            aud_mpu_error(error, error_size, "%s:%u: line too long",
                          path, line_number);
            (void)fclose(stream);
            return -1;
        }
        comment = strchr(line, '#');
        if (comment != NULL) *comment = '\0';
        text = aud_mpu_trim(line);
        if (*text == '\0') continue;
        if (*text == '[') {
            size_t length = strlen(text);
            if (length < 3u || text[length - 1u] != ']') {
                aud_mpu_error(error, error_size, "%s:%u: malformed section",
                              path, line_number);
                (void)fclose(stream);
                return -1;
            }
            text[length - 1u] = '\0';
            text = aud_mpu_trim(text + 1);
            if (strcmp(text, "audio") != 0 &&
                strcmp(text, "realtime") != 0 &&
                strcmp(text, "effects") != 0) {
                aud_mpu_error(error, error_size, "%s:%u: unknown section '%s'",
                              path, line_number, text);
                (void)fclose(stream);
                return -1;
            }
            if (aud_mpu_copy(section, sizeof(section), text) != 0) {
                (void)fclose(stream);
                return -1;
            }
            continue;
        }
        equals = strchr(text, '=');
        if (equals == NULL || section[0] == '\0') {
            aud_mpu_error(error, error_size, "%s:%u: expected section/key=value",
                          path, line_number);
            (void)fclose(stream);
            return -1;
        }
        *equals = '\0';
        text = aud_mpu_trim(text);
        status = aud_mpu_set(config, section, text, aud_mpu_trim(equals + 1));
        if (status != 0) {
            aud_mpu_error(error, error_size, "%s:%u: %s '%s.%s'",
                          path, line_number,
                          status == -2 ? "unknown key" : "invalid value for",
                          section, text);
            (void)fclose(stream);
            return -1;
        }
    }
    if (ferror(stream)) {
        aud_mpu_error(error, error_size, "%s: read failed", path);
        (void)fclose(stream);
        return -1;
    }
    (void)fclose(stream);
    return AudMpuAppConfig_Validate(config, error, error_size);
}

void AudMpuAppConfig_Print(const AudMpuAppConfig *config, FILE *stream)
{
    if (config == NULL || stream == NULL) return;
    (void)fprintf(stream,
        "[audio]\n"
        "capture_device=%s\nplayback_device=%s\n"
        "sample_rate=%" PRIu32 "\nperiod_frames=%" PRIu32
        "\nperiods=%" PRIu32 "\n"
        "format=%s\nmax_delay_ms=%" PRIu32 "\nrequire_duplex_link=%s\n\n"
        "[realtime]\npriority=%d\nlock_memory=%s\nallow_fallback=%s\n\n"
        "[effects]\ninput_gain=%.6g\nfilter_cutoff_hz=%.6g\n"
        "filter_resonance=%.6g\nfilter_mix=%.6g\ndrive=%.6g\n"
        "delay_ms=%.6g\ndelay_feedback=%.6g\n"
        "delay_crossfeed=%.6g\ndelay_mix=%.6g\noutput_gain=%.6g\n",
        config->capture_device, config->playback_device,
        config->sample_rate, config->period_frames, config->periods,
        config->format == AUD_MPU_PCM_S16_LE ? "s16" : "s32",
        config->max_delay_ms,
        config->require_duplex_link ? "true" : "false",
        config->realtime_priority,
        config->lock_memory ? "true" : "false",
        config->allow_realtime_fallback ? "true" : "false",
        config->effects.input_gain, config->effects.filter_cutoff_hz,
        config->effects.filter_resonance, config->effects.filter_mix,
        config->effects.drive, config->effects.delay_ms,
        config->effects.delay_feedback, config->effects.delay_crossfeed,
        config->effects.delay_mix, config->effects.output_gain);
}
