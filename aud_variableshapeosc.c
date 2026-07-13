#include "aud_variableshapeosc.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

static float aud_varshape_ComputeNaive(float phase, float pw, float slope_up,
    float slope_down, float tri_amt, float sq_amt)
{
    float saw = phase, square = (phase < pw) ? 0.0f : 1.0f;
    float triangle = (phase < pw) ? (phase * slope_up)
                                  : (1.0f - (phase - pw) * slope_down);
    saw += (square - saw) * sq_amt;
    saw += (triangle - saw) * tri_amt;
    return saw;
}

void Aud_VariableShapeOscillator_Init(Aud_VariableShapeOscillator *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr; self->enable_sync = false;
    self->master_phase = self->slave_phase = self->next_sample = 0.0f;
    self->previous_pw = 0.5f; self->high = false;
    self->master_frequency = 440.0f / sr;
    self->slave_frequency  = 440.0f / sr;
    self->pw = 0.5f; self->waveshape = 0.0f;
}

float Aud_VariableShapeOscillator_Process(Aud_VariableShapeOscillator *self)
{
    if (self == NULL) return 0.0f;
    float ns = self->next_sample;
    bool reset = false, trans = false; float rt = 0.0f;
    float ts = ns; ns = 0.0f;
    const float sq_amt = aud_fmax(self->waveshape - 0.5f, 0.0f) * 2.0f;
    const float tri_amt= aud_fmax(1.0f - self->waveshape * 2.0f, 0.0f);
    const float su = 1.0f / self->pw, sd = 1.0f / (1.0f - self->pw);

    if (self->enable_sync) {
        self->master_phase += self->master_frequency;
        if (self->master_phase >= 1.0f) {
            self->master_phase -= 1.0f;
            rt = self->master_phase / self->master_frequency;
            float sp = self->slave_phase + (1.0f - rt) * self->slave_frequency;
            reset = true;
            if (sp >= 1.0f) { sp -= 1.0f; trans = true; }
            if (!self->high && sp >= self->pw) trans = true;
            float v = aud_varshape_ComputeNaive(sp, self->pw, su, sd, tri_amt, sq_amt);
            ts -= v * aud_this_blep(rt); ns -= v * aud_next_blep(rt);
        }
    }
    self->slave_phase += self->slave_frequency;
    while (trans || !reset) {
        if (!self->high) {
            if (self->slave_phase < self->pw) break;
            float t = (self->slave_phase - self->pw)
                / (self->previous_pw - self->pw + self->slave_frequency);
            float tri_step = (su + sd) * self->slave_frequency * tri_amt;
            ts += sq_amt * aud_this_blep(t); ns += sq_amt * aud_next_blep(t);
            ts -= tri_step * aud_this_integrated_blep(t);
            ns -= tri_step * aud_next_integrated_blep(t);
            self->high = true;
        }
        if (self->high) {
            if (self->slave_phase < 1.0f) break;
            self->slave_phase -= 1.0f;
            float t = self->slave_phase / self->slave_frequency;
            float tri_step = (su + sd) * self->slave_frequency * tri_amt;
            ts -= (1.0f - tri_amt) * aud_this_blep(t);
            ns -= (1.0f - tri_amt) * aud_next_blep(t);
            ts += tri_step * aud_this_integrated_blep(t);
            ns += tri_step * aud_next_integrated_blep(t);
            self->high = false;
        }
    }
    if (self->enable_sync && reset) {
        self->slave_phase = rt * self->slave_frequency; self->high = false;
    }
    ns += aud_varshape_ComputeNaive(self->slave_phase, self->pw, su, sd, tri_amt, sq_amt);
    self->previous_pw = self->pw; self->next_sample = ns;
    return 2.0f * ts - 1.0f;
}

void Aud_VariableShapeOscillator_SetFreq(Aud_VariableShapeOscillator *self, float f)
    { if (self) { f /= self->sample_rate; self->master_frequency = (f>=0.25f)?0.25f:f; } }
void Aud_VariableShapeOscillator_SetPW(Aud_VariableShapeOscillator *self, float pw)
    { if (self) { if (self->slave_frequency>=0.25f) self->pw=0.5f;
    else self->pw=aud_fclamp(pw,self->slave_frequency*2.0f,1.0f-2.0f*self->slave_frequency); } }
void Aud_VariableShapeOscillator_SetWaveshape(Aud_VariableShapeOscillator *self, float ws)
    { if (self) self->waveshape = ws; }
void Aud_VariableShapeOscillator_SetSync(Aud_VariableShapeOscillator *self, bool en)
    { if (self) self->enable_sync = en; }
void Aud_VariableShapeOscillator_SetSyncFreq(Aud_VariableShapeOscillator *self, float f)
    { if (self) { f/=self->sample_rate; if(f>=0.25f){self->pw=0.5f;f=0.25f;} self->slave_frequency=f; } }
