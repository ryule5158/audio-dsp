/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_AVR_PGMSPACE_H_
#define AUD_EURO_AVR_PGMSPACE_H_

#include <stdint.h>
#include <string.h>

#define PROGMEM
typedef char prog_char;
typedef uint8_t prog_uint8_t;
typedef uint16_t prog_uint16_t;
typedef uint32_t prog_uint32_t;

#define pgm_read_byte(address) (*(const uint8_t*)(address))
#define pgm_read_word(address) (*(const uint16_t*)(address))
#define pgm_read_dword(address) (*(const uint32_t*)(address))
#define memcpy_P(destination, source, size) memcpy((destination), (source), (size))
#define strncpy_P(destination, source, size) strncpy((destination), (source), (size))

#endif  /* AUD_EURO_AVR_PGMSPACE_H_ */
