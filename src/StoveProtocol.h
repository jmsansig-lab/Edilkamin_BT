#ifndef EDILKAMIN_STOVE_PROTOCOL_H
#define EDILKAMIN_STOVE_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

namespace StoveProtocol {
inline uint16_t word(const uint8_t *data) {
  return (uint16_t(data[0]) << 8) | data[1];
}
inline void temperature(uint16_t tenths, char *out, size_t size) {
  snprintf(out, size, "%u.%u", unsigned(tenths / 10), unsigned(tenths % 10));
}
// Decimal Celsius only. Extra fractional digits are allowed only if zero:
// 24, 24.0 and 24.50 are exact; 24.55 is not representable in this register.
inline bool parseTemperature(const char *text, uint16_t &tenths) {
  if (!text || *text < '0' || *text > '9') return false;
  unsigned whole = 0;
  while (*text >= '0' && *text <= '9') {
    whole = whole * 10 + (*text++ - '0');
    if (whole > 35) return false;
  }
  unsigned fraction = 0;
  if (*text == '.') {
    ++text;
    if (*text < '0' || *text > '9') return false;
    fraction = *text++ - '0';
    while (*text == '0') ++text;
  }
  const unsigned value = whole * 10 + fraction;
  if (*text || value < 50 || value > 350) return false;
  tenths = value;
  return true;
}
// Pellet quantity (remote menu 1111 "Q.te granule"): signed percent byte.
inline bool parsePelletQuantity(const char *text, uint8_t &level) {
  if (!text) return false;
  if (!strcmp(text, "Bas")) level = 0xf1;        // -15
  else if (!strcmp(text, "Moyen")) level = 0x00; // 0
  else if (!strcmp(text, "Haut")) level = 0x0f;  // +15
  else return false;
  return true;
}
// Standby delay in seconds: 60..1800, multiple of 30. "300" and "300.0" ok.
inline bool parseStandbyDelay(const char *text, uint16_t &seconds) {
  if (!text || *text < '0' || *text > '9') return false;
  unsigned value = 0;
  while (*text >= '0' && *text <= '9') {
    value = value * 10 + (*text++ - '0');
    if (value > 1800) return false;
  }
  if (*text == '.') {
    ++text;
    if (!*text) return false;
    while (*text == '0') ++text;
  }
  if (*text || value < 60 || value % 30) return false;
  seconds = static_cast<uint16_t>(value);
  return true;
}
inline bool pelletReserve(const uint8_t *data) { return (data[1] & 4) != 0; }
inline bool maintenanceDue(const uint8_t *data) { return (data[1] & 1) != 0; }
// Maintenance counters zeroed by the remote "RESET HEURES" (0x0624..0x0628).
static const uint16_t MaintenanceFirst = 0x0624;
static const uint8_t MaintenanceCount = 5;
inline void statusText(uint8_t sub, uint8_t main, char *out, size_t size) {
  const char *text = nullptr;
  if (main == 0 && sub == 0) text = "Off";
  else if (main == 1) {
    switch (sub) {
    case 0: case 1: case 2: text = "Cleaning"; break;
    case 3: text = "Loading Pellets"; break;
    case 4: text = "Ignition - charge suspend"; break;
    case 5: text = "Check exhaust gas temperature"; break;
    case 6: text = "Threshold temperature check"; break;
    case 7: text = "Warmup"; break;
    }
  } else if (main == 2 && sub == 2) text = "Ok";
  else if (main == 3 && sub == 0) text = "Shutdown";
  else if (main == 4 && sub == 0) text = "Extinction";
  else if (main == 5) text = "Final cleaning";
  if (text) snprintf(out, size, "%s", text);
  else snprintf(out, size, "Unknown (main=%u, sub=%u)", main, sub);
}

// A serialized read-before-write transaction. The caller validates the Modbus
// response against the prerequisite request before calling acknowledge().
// Never reuse a cached shared register across user commands or reconnects.
class ControlTransaction {
public:
  bool pending() const { return state_ != Idle; }
  void cancel() { state_ = Idle; }
  // For the shared command register 0x031C only the bits in `mask` are taken
  // from `command`; every other bit is copied from a fresh read.
  // Bit 0 = ON/OFF (default), bit 10 (0x0400) = AirKare.
  static constexpr uint16_t OnOffBit = 0x0001;
  static constexpr uint16_t AirKareBit = 0x0400;
  // 0x0546 (pellet quantity): only the high byte is written, the low byte is
  // copied from a fresh read, exactly as the remote does.
  static constexpr uint16_t HighByte = 0xff00;
  bool begin(uint8_t command[6], uint16_t mask = OnOffBit) {
    if (pending() || command[1] != 6) return false;
    const uint16_t reg = word(command + 2);
    if (reg != 0x031c && reg != 0x0522 && reg != 0x0529 && reg != 0x0546) return false;
    if (reg == 0x031c && mask != OnOffBit && mask != AirKareBit) return false;
    if (reg == 0x0546 && mask != HighByte) return false;
    memcpy(desired_, command, 6);
    mask_ = mask;
    state_ = Waiting;
    if (reg == 0x0529) {
      // Manual power: confirm Automatic OFF before changing the power level.
      const uint8_t automaticOff[] = {1, 6, 5, 35, 0, 1};
      memcpy(command, automaticOff, 6);
    } else {
      command[1] = 3;
      command[4] = 0;
      command[5] = 1;
    }
    return true;
  }
  void acknowledge(const uint8_t *frame) {
    if (state_ != Waiting) return;
    const uint16_t reg = word(desired_ + 2);
    if (reg == 0x0529) {
      const uint8_t expected[] = {1, 6, 5, 35, 0, 1};
      if (memcmp(frame, expected, 6)) { cancel(); return; }
    } else {
      if (frame[1] != 3 || frame[2] != 2) { cancel(); return; }
      if (reg == 0x031c || reg == 0x0546) {
        const uint16_t current = word(frame + 3);
        const uint16_t value = static_cast<uint16_t>((current & ~mask_) |
                                                     (word(desired_ + 4) & mask_));
        desired_[4] = static_cast<uint8_t>(value >> 8);
        desired_[5] = static_cast<uint8_t>(value);
      } else {
        desired_[4] = frame[3];
      }
    }
    state_ = Ready;
  }
  bool take(uint8_t command[6]) {
    if (state_ != Ready) return false;
    memcpy(command, desired_, 6);
    cancel();
    return true;
  }
private:
  enum State { Idle, Waiting, Ready } state_ = Idle;
  uint8_t desired_[6] = {};
  uint16_t mask_ = OnOffBit;
};
} // namespace StoveProtocol
#endif
