/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVR_EEPROM_H_
#define AUD_EURO_AVR_EEPROM_H_

#include <stdint.h>

static inline uint8_t eeprom_read_byte(const uint8_t* address) {
  (void)address;
  return 0xff;
}

static inline void eeprom_write_byte(uint8_t* address, uint8_t value) {
  (void)address;
  (void)value;
}

#endif  /* AUD_EURO_AVR_EEPROM_H_ */
