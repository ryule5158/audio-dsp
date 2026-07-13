#include "aud_euro_elements.h"

#include <new>

#include "elements/dsp/part.h"

namespace {

elements::Part* Impl(aud_euro_elements_t* context) {
  return reinterpret_cast<elements::Part*>(context->storage);
}

void CopyPatch(elements::Patch* d, const aud_euro_elements_patch_t& s) {
  d->exciter_envelope_shape = s.exciter_envelope_shape;
  d->exciter_bow_level = s.exciter_bow_level;
  d->exciter_bow_timbre = s.exciter_bow_timbre;
  d->exciter_blow_level = s.exciter_blow_level;
  d->exciter_blow_meta = s.exciter_blow_meta;
  d->exciter_blow_timbre = s.exciter_blow_timbre;
  d->exciter_strike_level = s.exciter_strike_level;
  d->exciter_strike_meta = s.exciter_strike_meta;
  d->exciter_strike_timbre = s.exciter_strike_timbre;
  d->exciter_signature = s.exciter_signature;
  d->resonator_geometry = s.resonator_geometry;
  d->resonator_brightness = s.resonator_brightness;
  d->resonator_damping = s.resonator_damping;
  d->resonator_position = s.resonator_position;
  d->resonator_modulation_frequency = s.resonator_modulation_frequency;
  d->resonator_modulation_offset = s.resonator_modulation_offset;
  d->reverb_diffusion = s.reverb_diffusion;
  d->reverb_lp = s.reverb_lowpass;
  d->space = s.space;
  d->modulation_frequency = s.modulation_frequency;
}

}  // namespace

static_assert(sizeof(elements::Part) <= AUD_EURO_ELEMENTS_CONTEXT_BYTES,
    "AUD_EURO_ELEMENTS_CONTEXT_BYTES is too small");

extern "C" void aud_euro_elements_default_patch(
    aud_euro_elements_patch_t* p) {
  if (!p) return;
  p->exciter_envelope_shape = 0.0f;
  p->exciter_bow_level = 0.0f;
  p->exciter_bow_timbre = 0.5f;
  p->exciter_blow_level = 0.0f;
  p->exciter_blow_meta = 0.5f;
  p->exciter_blow_timbre = 0.5f;
  p->exciter_strike_level = 0.5f;
  p->exciter_strike_meta = 0.5f;
  p->exciter_strike_timbre = 0.5f;
  p->exciter_signature = 0.0f;
  p->resonator_geometry = 0.4f;
  p->resonator_brightness = 0.7f;
  p->resonator_damping = 0.8f;
  p->resonator_position = 0.3f;
  p->resonator_modulation_frequency = 0.0f;
  p->resonator_modulation_offset = 0.0f;
  p->reverb_diffusion = 0.625f;
  p->reverb_lowpass = 0.7f;
  p->space = 0.1f;
  p->modulation_frequency = 0.0f;
}

extern "C" aud_euro_result_t aud_euro_elements_init(
    aud_euro_elements_t* context,
    uint16_t* reverb_buffer,
    size_t reverb_samples) {
  if (!context || !reverb_buffer) return AUD_EURO_ERROR_ARGUMENT;
  if (reverb_samples < AUD_EURO_ELEMENTS_REVERB_SAMPLES) {
    return AUD_EURO_ERROR_MEMORY;
  }
  elements::Part* part = new (context->storage) elements::Part();
  part->Init(reverb_buffer);
  part->set_resonator_model(elements::RESONATOR_MODEL_MODAL);
  aud_euro_elements_patch_t defaults;
  aud_euro_elements_default_patch(&defaults);
  CopyPatch(part->mutable_patch(), defaults);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_elements_deinit(aud_euro_elements_t* context) {
  if (context) Impl(context)->~Part();
}

extern "C" aud_euro_result_t aud_euro_elements_set_model(
    aud_euro_elements_t* context, aud_euro_elements_model_t model) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (model < AUD_EURO_ELEMENTS_MODAL ||
      model > AUD_EURO_ELEMENTS_STRING_ENSEMBLE) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_resonator_model(
      static_cast<elements::ResonatorModel>(model));
  return AUD_EURO_OK;
}

extern "C" void aud_euro_elements_set_patch(
    aud_euro_elements_t* context, const aud_euro_elements_patch_t* patch) {
  if (context && patch) CopyPatch(Impl(context)->mutable_patch(), *patch);
}

extern "C" void aud_euro_elements_process_f32(
    aud_euro_elements_t* context,
    const aud_euro_elements_performance_t* performance,
    const float* blow_input,
    const float* strike_input,
    float* main_output,
    float* aux_output,
    size_t frames) {
  if (!context || !performance || !main_output || !aux_output) return;
  elements::PerformanceState state;
  state.gate = performance->gate != 0;
  state.note = performance->note;
  state.modulation = performance->modulation;
  state.strength = performance->strength;
  const float silence[16] = { 0.0f };
  while (frames) {
    const size_t block = frames > 16 ? 16 : frames;
    Impl(context)->Process(
        state,
        blow_input ? blow_input : silence,
        strike_input ? strike_input : silence,
        main_output,
        aux_output,
        block);
    if (blow_input) blow_input += block;
    if (strike_input) strike_input += block;
    main_output += block;
    aux_output += block;
    frames -= block;
  }
}
