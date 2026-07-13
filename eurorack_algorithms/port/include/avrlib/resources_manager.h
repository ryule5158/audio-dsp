/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVRLIB_RESOURCES_MANAGER_H_
#define AUD_EURO_AVRLIB_RESOURCES_MANAGER_H_

#include <string.h>
#include "avr/pgmspace.h"
#include "avrlib/base.h"

namespace avrlib {
template<const prog_char* const* strings,
         const prog_uint16_t* const* lookup_tables>
struct ResourcesTables {
  static const prog_char* const* string_table() { return strings; }
  static const prog_uint16_t* const* lookup_table_table() {
    return lookup_tables;
  }
};

struct NoResourcesTables {
  static const prog_char* const* string_table() { return NULL; }
  static const prog_uint16_t* const* lookup_table_table() { return NULL; }
};

template<typename ResourceId = uint8_t, typename Tables = NoResourcesTables>
struct ResourcesManager {
  template<typename ResultType, typename IndexType, typename T>
  static ResultType Lookup(const T* data, IndexType index) {
    return static_cast<ResultType>(data[index]);
  }
  template<typename T, typename U>
  static void Load(const T* data, uint8_t index, U* destination) {
    memcpy(destination, data + index, sizeof(U));
  }
};

typedef ResourcesManager<> SimpleResourcesManager;
}  // namespace avrlib

#endif  /* AUD_EURO_AVRLIB_RESOURCES_MANAGER_H_ */
