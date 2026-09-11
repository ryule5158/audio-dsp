#ifndef GENERIC_DSP_SELFTEST_H
#define GENERIC_DSP_SELFTEST_H

/*
 * Deterministic, non-real-time smoke tests for the optional Generic DSP
 * target. The function is called from AudioApp_Init() only when the target
 * define AUDIO_BOARD_ENABLE_GENERIC_DSP is enabled; it is never called
 * from a SAI/DMA interrupt.
 */
int GenericDsp_RunSelfTest(void);

#endif
