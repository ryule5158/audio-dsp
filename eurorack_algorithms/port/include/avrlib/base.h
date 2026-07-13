/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVRLIB_BASE_H_
#define AUD_EURO_AVRLIB_BASE_H_

#include <stddef.h>
#include <stdint.h>

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif

namespace avrlib {
union LongWord {
  uint32_t value;
  uint16_t words[2];
  uint8_t bytes[4];
};
}  // namespace avrlib

typedef avrlib::LongWord LongWord;

#endif  /* AUD_EURO_AVRLIB_BASE_H_ */
