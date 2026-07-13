/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVRLIB_RANDOM_H_
#define AUD_EURO_AVRLIB_RANDOM_H_

#include "avrlib/base.h"

namespace avrlib {
class Random {
 public:
  static void Update() {
    rng_state_ = (rng_state_ >> 1) ^ (-(rng_state_ & 1u) & 0xb400u);
  }
  static uint16_t state() { return rng_state_; }
  static void Seed(uint16_t seed) { rng_state_ = seed ? seed : 1u; }
  static uint8_t GetByte() {
    Update();
    return static_cast<uint8_t>(rng_state_ >> 8);
  }
 private:
  static uint16_t rng_state_;
};
}  // namespace avrlib

#endif  /* AUD_EURO_AVRLIB_RANDOM_H_ */
