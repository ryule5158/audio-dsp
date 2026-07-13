// STM32H7 library port: timing pins are optional and disabled by default.
#ifndef AUD_EURO_WARPS_DEBUG_PIN_H_
#define AUD_EURO_WARPS_DEBUG_PIN_H_

namespace warps {
class DebugPin {
 public:
  static void Init() { }
  static void High() { }
  static void Low() { }
};
}  // namespace warps

#define TIC do { } while (0)
#define TOC do { } while (0)

#endif  // AUD_EURO_WARPS_DEBUG_PIN_H_
