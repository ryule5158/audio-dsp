/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef AUD_EURO_GRIDS_HARDWARE_CONFIG_H_
#define AUD_EURO_GRIDS_HARDWARE_CONFIG_H_

namespace grids {
enum LedBits {
  LED_CLOCK = 1,
  LED_HH = 2,
  LED_SD = 4,
  LED_BD = 8,
  LED_ALL = 15
};
}  // namespace grids

#endif  /* AUD_EURO_GRIDS_HARDWARE_CONFIG_H_ */
