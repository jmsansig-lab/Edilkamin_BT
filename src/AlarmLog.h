#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// nvm.logAllarmi (register 0x033A): low byte = number of logged alarms,
// high byte = ring index of the next free slot. 100 slots of 4 registers
// start at 0x033C: timestamp (UNIX, UTC) low word, high word, alarm type,
// then a word observed as 0x00FF. Verified on a Celia (The Mind): the newest
// entry matched the cloud "last alarm" (code 21, 2026-05-11T22:06:53Z).
namespace AlarmLog {
constexpr uint16_t HeaderRegister = 0x033a;
constexpr uint16_t FirstEntry = 0x033c;
constexpr uint8_t Slots = 100;
constexpr uint8_t EntryRegisters = 4;

struct Entry {
  uint32_t timestamp;
  uint8_t code;
};

// Slot holding the newest alarm for a ring index (next free slot).
inline bool newestSlot(uint8_t index, uint8_t count, uint8_t &slot) {
  if (count == 0 || count > Slots || index >= Slots) return false;
  slot = index == 0 ? Slots - 1 : index - 1;
  return true;
}

// FC03 read of one 4-register entry. Read-only by construction.
inline bool request(uint8_t out[6], uint8_t slot) {
  if (slot >= Slots) return false;
  const uint16_t reg = FirstEntry + static_cast<uint16_t>(slot) * EntryRegisters;
  out[0] = 1;
  out[1] = 3;
  out[2] = static_cast<uint8_t>(reg >> 8);
  out[3] = static_cast<uint8_t>(reg);
  out[4] = 0;
  out[5] = EntryRegisters;
  return true;
}

// data: validated FC03 payload (8 bytes, big-endian registers).
inline bool decode(const uint8_t *data, size_t size, Entry &entry) {
  if (!data || size != EntryRegisters * 2) return false;
  const uint32_t low = (static_cast<uint32_t>(data[0]) << 8) | data[1];
  const uint32_t high = (static_cast<uint32_t>(data[2]) << 8) | data[3];
  const uint16_t type = static_cast<uint16_t>((data[4] << 8) | data[5]);
  entry.timestamp = (high << 16) | low;
  // Reject erased slots and values that cannot be an alarm type or a date
  // after 2015-01-01.
  if (type == 0 || type > 0xff || entry.timestamp < 1420070400UL) return false;
  entry.code = static_cast<uint8_t>(type);
  return true;
}

// Alarm names as used by the Edilkamin cloud integration; French labels for
// local display. Unknown codes are reported as "Code N".
struct Label {
  uint8_t code;
  const char *en;
  const char *fr;
};
static const Label labels[] = {
    {1, "Low incoming air", "Manque d'air comburant"},
    {2, "Wrong RPM exhaust fan", "Vitesse extracteur incorrecte"},
    {3, "No flame", "Pas de flamme / fin de pellets"},
    {4, "Failed ignition", "Échec d'allumage"},
    {5, "Failed airflow sensor", "Défaut capteur de débit d'air"},
    {6, "Failed thermocouple", "Défaut thermocouple"},
    {7, "Exhaust too hot", "Fumées trop chaudes"},
    {8, "Stove too hot", "Poêle en surchauffe"},
    {9, "Failed gear motor", "Défaut motoréducteur"},
    {10, "Circuit board too hot", "Carte électronique en surchauffe"},
    {11, "Chimney pressure", "Pression cheminée"},
    {12, "Room temperature sensor failed", "Défaut sonde ambiante"},
    {13, "Room temperature sensor 2 failed", "Défaut sonde ambiante 2"},
    {14, "Room temperature sensor 3 failed", "Défaut sonde ambiante 3"},
    {20, "Failed triac gear motor", "Défaut triac motoréducteur"},
    {21, "Power outage", "Coupure de courant"}};

inline const Label *label(uint8_t code) {
  for (size_t i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i)
    if (labels[i].code == code) return &labels[i];
  return nullptr;
}

inline void text(uint8_t code, bool french, char *out, size_t size) {
  const Label *l = label(code);
  if (l) snprintf(out, size, "%s", french ? l->fr : l->en);
  else snprintf(out, size, "Code %u", code);
}

// ISO 8601 UTC, e.g. 2026-05-11T22:06:53Z.
inline bool isoDate(uint32_t timestamp, char *out, size_t size) {
  const time_t t = static_cast<time_t>(timestamp);
  struct tm parts;
  if (!gmtime_r(&t, &parts)) return false;
  return strftime(out, size, "%Y-%m-%dT%H:%M:%SZ", &parts) > 0;
}
} // namespace AlarmLog
