/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVRLIB_OP_H_
#define AUD_EURO_AVRLIB_OP_H_

#include "avrlib/base.h"

namespace avrlib {
static inline uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance) {
  return static_cast<uint8_t>(
      (static_cast<uint16_t>(a) * (255u - balance) +
       static_cast<uint16_t>(b) * balance) >> 8);
}
static inline uint8_t U8U8MulShift8(uint8_t a, uint8_t b) {
  return static_cast<uint8_t>((static_cast<uint16_t>(a) * b) >> 8);
}
static inline uint16_t U8U8Mul(uint8_t a, uint8_t b) {
  return static_cast<uint16_t>(a) * b;
}
}  // namespace avrlib

#endif  /* AUD_EURO_AVRLIB_OP_H_ */
