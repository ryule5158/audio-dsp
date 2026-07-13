// STM32H7 library port: user wave data persistence is supplied by the host.
#ifndef AUD_EURO_PLAITS_USER_DATA_H_
#define AUD_EURO_PLAITS_USER_DATA_H_

#include <stddef.h>
#include <stdint.h>

namespace plaits {

namespace port {
extern const uint8_t* user_data;
extern int user_engine;
}  // namespace port

class UserData {
 public:
  enum {
    ADDRESS = 0,
    SIZE = 0x1000
  };

  UserData() { }
  ~UserData() { }

  const uint8_t* ptr(int slot) const {
    return slot == port::user_engine ? port::user_data : NULL;
  }

  bool Save(uint8_t* rx_buffer, int slot) {
    (void)rx_buffer;
    (void)slot;
    return false;
  }
};

}  // namespace plaits

#endif  // AUD_EURO_PLAITS_USER_DATA_H_
