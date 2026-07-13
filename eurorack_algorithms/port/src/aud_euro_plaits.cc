#include "aud_euro_plaits.h"

#include <cstring>
#include <new>

#include "plaits/dsp/voice.h"
#include "stmlib/utils/buffer_allocator.h"

namespace {

struct Context {
  plaits::Voice voice;
  const uint8_t* user_data;
  int32_t user_engine;
  plaits::Voice::Frame pending[plaits::kMaxBlockSize];
  size_t pending_read;
  size_t pending_count;
};

Context* State(aud_euro_plaits_t* context) {
  return reinterpret_cast<Context*>(context->storage);
}

const Context* State(const aud_euro_plaits_t* context) {
  return reinterpret_cast<const Context*>(context->storage);
}

plaits::Voice* Impl(aud_euro_plaits_t* context) {
  return &State(context)->voice;
}

const plaits::Voice* Impl(const aud_euro_plaits_t* context) {
  return &State(context)->voice;
}

plaits::Patch Convert(const aud_euro_plaits_patch_t& p) {
  plaits::Patch result;
  result.note = p.note;
  result.harmonics = p.harmonics;
  result.timbre = p.timbre;
  result.morph = p.morph;
  result.frequency_modulation_amount = p.frequency_modulation_amount;
  result.timbre_modulation_amount = p.timbre_modulation_amount;
  result.morph_modulation_amount = p.morph_modulation_amount;
  result.engine = p.engine;
  result.decay = p.decay;
  result.lpg_colour = p.lpg_colour;
  return result;
}

plaits::Modulations Convert(const aud_euro_plaits_modulations_t& m) {
  plaits::Modulations result;
  result.engine = m.engine;
  result.note = m.note;
  result.frequency = m.frequency;
  result.harmonics = m.harmonics;
  result.timbre = m.timbre;
  result.morph = m.morph;
  result.trigger = m.trigger;
  result.level = m.level;
  result.frequency_patched = m.frequency_patched != 0;
  result.timbre_patched = m.timbre_patched != 0;
  result.morph_patched = m.morph_patched != 0;
  result.trigger_patched = m.trigger_patched != 0;
  result.level_patched = m.level_patched != 0;
  return result;
}

}  // namespace

namespace plaits {
namespace port {
const uint8_t* user_data = NULL;
int user_engine = -1;
}  // namespace port
}  // namespace plaits

static_assert(sizeof(Context) <= AUD_EURO_PLAITS_CONTEXT_BYTES,
    "AUD_EURO_PLAITS_CONTEXT_BYTES is too small");
static_assert(sizeof(plaits::Voice::Frame) == sizeof(aud_euro_stereo_i16_t),
    "Stereo frame ABI mismatch");

extern "C" void aud_euro_plaits_default_patch(
    aud_euro_plaits_patch_t* p) {
  if (!p) return;
  p->note = 48.0f;
  p->harmonics = 0.5f;
  p->timbre = 0.5f;
  p->morph = 0.5f;
  p->frequency_modulation_amount = 0.0f;
  p->timbre_modulation_amount = 0.0f;
  p->morph_modulation_amount = 0.0f;
  p->engine = 0;
  p->decay = 0.5f;
  p->lpg_colour = 0.5f;
}

extern "C" void aud_euro_plaits_default_modulations(
    aud_euro_plaits_modulations_t* m) {
  if (!m) return;
  m->engine = 0.0f;
  m->note = 0.0f;
  m->frequency = 0.0f;
  m->harmonics = 0.0f;
  m->timbre = 0.0f;
  m->morph = 0.0f;
  m->trigger = 0.0f;
  m->level = 1.0f;
  m->frequency_patched = 0;
  m->timbre_patched = 0;
  m->morph_patched = 0;
  m->trigger_patched = 0;
  m->level_patched = 0;
}

extern "C" aud_euro_result_t aud_euro_plaits_init(
    aud_euro_plaits_t* context,
    void* engine_memory,
    size_t engine_memory_bytes) {
  if (!context || !engine_memory) return AUD_EURO_ERROR_ARGUMENT;
  if (reinterpret_cast<uintptr_t>(engine_memory) & 7u) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  if (engine_memory_bytes < AUD_EURO_PLAITS_ENGINE_MEMORY_BYTES) {
    return AUD_EURO_ERROR_MEMORY;
  }
  Context* state = new (context->storage) Context();
  state->user_data = NULL;
  state->user_engine = -1;
  state->pending_read = 0;
  state->pending_count = 0;
  stmlib::BufferAllocator allocator(engine_memory, engine_memory_bytes);
  state->voice.Init(&allocator);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_plaits_deinit(aud_euro_plaits_t* context) {
  if (context) State(context)->~Context();
}

extern "C" aud_euro_result_t aud_euro_plaits_set_user_data(
    aud_euro_plaits_t* context,
    int32_t engine,
    const void* data,
    size_t data_bytes) {
  if (!context) return AUD_EURO_ERROR_ARGUMENT;
  if (engine < 0 || engine >= AUD_EURO_PLAITS_ENGINE_COUNT) {
    return AUD_EURO_ERROR_RANGE;
  }
  if (data && data_bytes < AUD_EURO_PLAITS_USER_DATA_BYTES) {
    return AUD_EURO_ERROR_MEMORY;
  }
  if (!data && data_bytes != 0) return AUD_EURO_ERROR_ARGUMENT;
  Context* state = State(context);
  state->user_data = static_cast<const uint8_t*>(data);
  state->user_engine = data ? engine : -1;
  state->voice.ReloadUserData();
  return AUD_EURO_OK;
}

extern "C" void aud_euro_plaits_process(
    aud_euro_plaits_t* context,
    const aud_euro_plaits_patch_t* patch,
    const aud_euro_plaits_modulations_t* modulations,
    aud_euro_stereo_i16_t* output,
    size_t frames) {
  if (!context || !patch || !modulations || !output) return;
  plaits::port::user_data = State(context)->user_data;
  plaits::port::user_engine = State(context)->user_engine;
  const plaits::Patch native_patch = Convert(*patch);
  plaits::Modulations native_modulations = Convert(*modulations);
  while (frames) {
    if (!State(context)->pending_count) {
      Impl(context)->Render(
          native_patch,
          native_modulations,
          State(context)->pending,
          plaits::kMaxBlockSize);
      State(context)->pending_read = 0;
      State(context)->pending_count = plaits::kMaxBlockSize;
    }
    const size_t count = frames < State(context)->pending_count
        ? frames : State(context)->pending_count;
    std::memcpy(
        output,
        State(context)->pending + State(context)->pending_read,
        count * sizeof(*output));
    State(context)->pending_read += count;
    State(context)->pending_count -= count;
    output += count;
    frames -= count;
  }
}

extern "C" int32_t aud_euro_plaits_active_engine(
    const aud_euro_plaits_t* context) {
  return context ? Impl(context)->active_engine() : -1;
}
