#include "aud_euro_marbles.h"

#include <new>

#include "marbles/random/random_generator.h"
#include "marbles/random/random_stream.h"
#include "marbles/random/t_generator.h"
#include "marbles/random/x_y_generator.h"

namespace {

struct Context {
  marbles::RandomGenerator random_generator;
  marbles::RandomStream random_stream;
  marbles::TGenerator t_generator;
  marbles::XYGenerator xy_generator;
};

Context* Impl(aud_euro_marbles_t* context) {
  return reinterpret_cast<Context*>(context->storage);
}

marbles::GroupSettings Convert(const aud_euro_marbles_group_t& source) {
  marbles::GroupSettings result;
  result.control_mode = static_cast<marbles::ControlMode>(source.control_mode);
  result.voltage_range = static_cast<marbles::VoltageRange>(source.voltage_range);
  result.register_mode = source.register_mode != 0;
  result.register_value = source.register_value;
  result.spread = source.spread;
  result.bias = source.bias;
  result.steps = source.steps;
  result.deja_vu = source.deja_vu;
  result.scale_index = source.scale_index;
  result.length = source.length;
  result.ratio.q = 1000;
  result.ratio.p = static_cast<int>(source.ratio * 1000.0f);
  if (result.ratio.p < 1) result.ratio.p = 1;
  return result;
}

}  // namespace

static_assert(sizeof(Context) <= AUD_EURO_MARBLES_CONTEXT_BYTES,
    "AUD_EURO_MARBLES_CONTEXT_BYTES is too small");

extern "C" aud_euro_result_t aud_euro_marbles_init(
    aud_euro_marbles_t* context, float sample_rate_hz, uint32_t seed) {
  if (!context || !(sample_rate_hz > 0.0f)) return AUD_EURO_ERROR_ARGUMENT;
  Context* state = new (context->storage) Context();
  state->random_generator.Init(seed ? seed : 1u);
  state->random_stream.Init(&state->random_generator);
  state->t_generator.Init(&state->random_stream, sample_rate_hz);
  state->xy_generator.Init(&state->random_stream, sample_rate_hz);
  return AUD_EURO_OK;
}

extern "C" void aud_euro_marbles_add_entropy(
    aud_euro_marbles_t* context, uint32_t word) {
  if (context) Impl(context)->random_stream.Write(word);
}

extern "C" void aud_euro_marbles_default_t_parameters(
    aud_euro_marbles_t_parameters_t* p) {
  if (!p) return;
  p->model = marbles::T_GENERATOR_MODEL_COMPLEMENTARY_BERNOULLI;
  p->range = marbles::T_GENERATOR_RANGE_1X;
  p->rate = 0.0f;
  p->bias = 0.5f;
  p->jitter = 0.0f;
  p->deja_vu = 0.0f;
  p->length = 8;
  p->pulse_width_mean = 0.5f;
  p->pulse_width_spread = 0.0f;
}

extern "C" void aud_euro_marbles_default_group(aud_euro_marbles_group_t* p) {
  if (!p) return;
  p->control_mode = marbles::CONTROL_MODE_IDENTICAL;
  p->voltage_range = marbles::VOLTAGE_RANGE_FULL;
  p->register_mode = 0;
  p->register_value = 0.5f;
  p->spread = 0.5f;
  p->bias = 0.5f;
  p->steps = 0.0f;
  p->deja_vu = 0.0f;
  p->scale_index = 0;
  p->length = 8;
  p->ratio = 1.0f;
}

extern "C" aud_euro_result_t aud_euro_marbles_process_t(
    aud_euro_marbles_t* context,
    const aud_euro_marbles_t_parameters_t* p,
    uint8_t use_external_clock,
    const uint8_t* external_clock,
    aud_euro_marbles_ramp_sample_t* ramps,
    uint8_t* gates,
    size_t frames) {
  if (!context || !p || !external_clock || !ramps || !gates) {
    return AUD_EURO_ERROR_ARGUMENT;
  }
  if (p->model < 0 || p->model > marbles::T_GENERATOR_MODEL_MARKOV ||
      p->range < 0 || p->range > marbles::T_GENERATOR_RANGE_4X) {
    return AUD_EURO_ERROR_RANGE;
  }
  marbles::TGenerator& generator = Impl(context)->t_generator;
  generator.set_model(static_cast<marbles::TGeneratorModel>(p->model));
  generator.set_range(static_cast<marbles::TGeneratorRange>(p->range));
  generator.set_rate(p->rate);
  generator.set_bias(p->bias);
  generator.set_jitter(p->jitter);
  generator.set_deja_vu(p->deja_vu);
  generator.set_length(p->length);
  generator.set_pulse_width_mean(p->pulse_width_mean);
  generator.set_pulse_width_std(p->pulse_width_spread);

  const size_t block_limit = 64;
  size_t offset = 0;
  while (offset < frames) {
    const size_t block = (frames - offset) > block_limit
        ? block_limit : (frames - offset);
    float external[block_limit];
    float master[block_limit];
    float slave_1[block_limit];
    float slave_2[block_limit];
    bool gate[block_limit * 2];
    for (size_t i = 0; i < block; ++i) {
      external[i] = ramps[offset + i].external;
      master[i] = ramps[offset + i].master;
      slave_1[i] = ramps[offset + i].slave_1;
      slave_2[i] = ramps[offset + i].slave_2;
    }
    marbles::Ramps native_ramps;
    native_ramps.external = external;
    native_ramps.master = master;
    native_ramps.slave[0] = slave_1;
    native_ramps.slave[1] = slave_2;
    generator.Process(
        use_external_clock != 0,
        external_clock + offset,
        native_ramps,
        gate,
        block);
    for (size_t i = 0; i < block; ++i) {
      ramps[offset + i].external = external[i];
      ramps[offset + i].master = master[i];
      ramps[offset + i].slave_1 = slave_1[i];
      ramps[offset + i].slave_2 = slave_2[i];
      gates[(offset + i) * 2] = gate[i * 2] ? 1 : 0;
      gates[(offset + i) * 2 + 1] = gate[i * 2 + 1] ? 1 : 0;
    }
    offset += block;
  }
  return AUD_EURO_OK;
}

extern "C" aud_euro_result_t aud_euro_marbles_process_xy(
    aud_euro_marbles_t* context,
    int32_t clock_source,
    const aud_euro_marbles_group_t* x_parameters,
    const aud_euro_marbles_group_t* y_parameters,
    const uint8_t* external_clock,
    const aud_euro_marbles_ramp_sample_t* ramps,
    float* output,
    size_t frames) {
  if (!context || !x_parameters || !y_parameters || !external_clock ||
      !ramps || !output) return AUD_EURO_ERROR_ARGUMENT;
  if (clock_source < marbles::CLOCK_SOURCE_INTERNAL_T1_T2_T3 ||
      clock_source > marbles::CLOCK_SOURCE_EXTERNAL) return AUD_EURO_ERROR_RANGE;
  const marbles::GroupSettings x = Convert(*x_parameters);
  const marbles::GroupSettings y = Convert(*y_parameters);
  const size_t block_limit = 64;
  size_t offset = 0;
  while (offset < frames) {
    const size_t block = (frames - offset) > block_limit
        ? block_limit : (frames - offset);
    float external[block_limit];
    float master[block_limit];
    float slave_1[block_limit];
    float slave_2[block_limit];
    for (size_t i = 0; i < block; ++i) {
      external[i] = ramps[offset + i].external;
      master[i] = ramps[offset + i].master;
      slave_1[i] = ramps[offset + i].slave_1;
      slave_2[i] = ramps[offset + i].slave_2;
    }
    marbles::Ramps native_ramps;
    native_ramps.external = external;
    native_ramps.master = master;
    native_ramps.slave[0] = slave_1;
    native_ramps.slave[1] = slave_2;
    Impl(context)->xy_generator.Process(
        static_cast<marbles::ClockSource>(clock_source),
        x,
        y,
        external_clock + offset,
        native_ramps,
        output + offset * 4,
        block);
    offset += block;
  }
  return AUD_EURO_OK;
}
