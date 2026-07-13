#include "aud_euro_rings.h"

#include <new>

#include "rings/dsp/part.h"

namespace {

rings::Part* Impl(aud_euro_rings_t* context) {
  return reinterpret_cast<rings::Part*>(context->storage);
}

}  // namespace

static_assert(sizeof(rings::Part) <= AUD_EURO_RINGS_CONTEXT_BYTES,
    "AUD_EURO_RINGS_CONTEXT_BYTES is too small");

extern "C" void aud_euro_rings_default_patch(aud_euro_rings_patch_t* p) {
  if (!p) return;
  p->structure = 0.5f;
  p->brightness = 0.5f;
  p->damping = 0.5f;
  p->position = 0.3f;
}

extern "C" aud_euro_result_t aud_euro_rings_init(
    aud_euro_rings_t* context,
    uint16_t* reverb_buffer,
    size_t reverb_samples) {
  if (!context || !reverb_buffer) return AUD_EURO_ERROR_ARGUMENT;
  if (reverb_samples < AUD_EURO_RINGS_REVERB_SAMPLES) {
    return AUD_EURO_ERROR_MEMORY;
  }
  rings::Part* part = new (context->storage) rings::Part();
  part->Init(reverb_buffer);
  part->set_model(rings::RESONATOR_MODEL_MODAL);
  part->set_polyphony(1);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_rings_deinit(aud_euro_rings_t* context) {
  if (context) Impl(context)->~Part();
}

extern "C" aud_euro_result_t aud_euro_rings_set_model(
    aud_euro_rings_t* context, aud_euro_rings_model_t model) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (model < AUD_EURO_RINGS_MODAL || model > AUD_EURO_RINGS_STRING_REVERB) {
    return AUD_EURO_ERROR_RANGE;
  }
  Impl(context)->set_model(static_cast<rings::ResonatorModel>(model));
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_rings_set_polyphony(
    aud_euro_rings_t* context, int32_t voices) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (voices < 1 || voices > rings::kMaxPolyphony) return AUD_EURO_ERROR_RANGE;
  Impl(context)->set_polyphony(voices);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_rings_process_f32(
    aud_euro_rings_t* context,
    const aud_euro_rings_patch_t* patch,
    const aud_euro_rings_performance_t* performance,
    const float* input,
    float* main_output,
    float* aux_output,
    size_t frames) {
  if (!context || !patch || !performance || !main_output || !aux_output) return;
  rings::Patch native_patch;
  native_patch.structure = patch->structure;
  native_patch.brightness = patch->brightness;
  native_patch.damping = patch->damping;
  native_patch.position = patch->position;
  rings::PerformanceState state;
  state.strum = performance->strum != 0;
  state.internal_exciter = performance->internal_exciter != 0;
  state.internal_strum = performance->internal_strum != 0;
  state.internal_note = performance->internal_note != 0;
  state.tonic = performance->tonic;
  state.note = performance->note;
  state.fm = performance->frequency_modulation;
  state.chord = performance->chord;
  const float silence[24] = { 0.0f };
  while (frames) {
    const size_t block = frames > 24 ? 24 : frames;
    Impl(context)->Process(
        state,
        native_patch,
        input ? input : silence,
        main_output,
        aux_output,
        block);
    state.strum = false;
    if (input) input += block;
    main_output += block;
    aux_output += block;
    frames -= block;
  }
}
