#ifndef AUDIO_APP_H
#define AUDIO_APP_H

#include <stdint.h>

typedef enum {
    AUDIO_APP_OK = 0,
    AUDIO_APP_BAD_CONFIG = -1,
    AUDIO_APP_DSP_SELF_TEST_FAILED = -2,
    AUDIO_APP_CODEC_FAILED = -3,
    AUDIO_APP_DMA_FAILED = -4
} AudioAppStatus;

extern volatile int32_t g_audio_app_status;
extern volatile uint32_t g_audio_callback_errors;
extern volatile uint32_t g_audio_restart_requested;

AudioAppStatus AudioApp_Init(void);
AudioAppStatus AudioApp_StartHardware(void);
AudioAppStatus AudioApp_Service(void);
int AudioApp_RunSelfTest(void);

#endif
