/**
 * @file    aud_dcblock.h
 * @brief   Online DC blocking filter (1st-order highpass with fixed corner).
 *          Ported from DaisySP Utility/dcblock (MIT)
 */
#ifndef AUD_DCBLOCK_H
#define AUD_DCBLOCK_H

typedef struct {
    float input;   /* Previous input  x[n-1] */
    float output;  /* Previous output y[n-1] */
    float gain;    /* Pole radius, typically 0.99 */
} Aud_DcBlock;

/**
 * @brief  Initialize DC blocker.
 * @param  dc           Instance
 * @param  sample_rate  Sample rate (unused, kept for API compatibility)
 * @note   gain defaults to 0.99 → cutoff ≈ 0.0016 * fs
 */
void Aud_DcBlock_Init(Aud_DcBlock *dc, float sample_rate);

/**
 * @brief  Process one sample:  y[n] = x[n] - x[n-1] + gain * y[n-1]
 */
float Aud_DcBlock_Process(Aud_DcBlock *dc, float in);

#endif /* AUD_DCBLOCK_H */
