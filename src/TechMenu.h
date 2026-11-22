#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Remote technical menu (code 6446), read-only. Every entry was matched on a
// Celia against a photo of the remote screen (docs/registers.md). The block
// 0x0577..0x05BF is read in five FC03 requests; nothing here is ever written.
namespace TechMenu {
enum Part : uint8_t { Word, Low, High };
enum Scale : uint8_t { Plain, Tenths };
struct Param {
  const char *key;
  const char *name;
  uint16_t reg;
  Part part;
  Scale scale;
  const char *unit;
};

static const char *const C = "°C";
static const Param params[] = {
  {"tm_fumes_fan1_on", "T° fumées ventilateur 1 ON", 0x057d, Word, Tenths, C},
  {"tm_fan1_hysteresis", "Hystérésis ventilateur 1", 0x057e, Word, Tenths, C},
  {"tm_fumes_fan23_on", "T° fumées ventilateurs 2-3 ON", 0x057f, Word, Tenths, C},
  {"tm_fan23_hysteresis", "Hystérésis ventilateurs 2-3", 0x0580, Word, Tenths, C},
  {"tm_initial_cleaning_time", "Durée nettoyage initial", 0x0581, Word, Plain, "s"},
  {"tm_pellet_load_time", "Durée charge pellets", 0x0582, Word, Plain, "s"},
  {"tm_extra_load_time", "Durée charge supplémentaire", 0x0583, Word, Plain, "s"},
  {"tm_manual_load_time", "Durée charge manuelle", 0x0584, Word, Plain, "s"},
  {"tm_flame_wait", "Attente flamme", 0x0585, Word, Plain, "s"},
  {"tm_hot_cleaning_leonardo", "Leonardo nettoyage chaud", 0x0586, Word, Plain, nullptr},
  {"tm_hot_cleaning_extractor", "Extracteur nettoyage chaud", 0x0587, Word, Plain, "rpm"},
  {"tm_initial_cleaning_leonardo", "Leonardo nettoyage initial", 0x0589, Word, Plain, nullptr},
  {"tm_initial_cleaning_extractor", "Extracteur nettoyage initial", 0x058a, Word, Plain, "rpm"},
  {"tm_ignition1_time", "Durée allumage 1", 0x058c, Word, Plain, "s"},
  {"tm_ignition2_time", "Durée allumage 2", 0x058d, Word, Plain, "s"},
  {"tm_ignition1_leonardo", "Leonardo allumage 1", 0x058e, Word, Plain, nullptr},
  {"tm_ignition2_leonardo", "Leonardo allumage 2", 0x058f, Word, Plain, nullptr},
  {"tm_ignition1_extractor", "Extracteur allumage 1", 0x0590, Word, Plain, "rpm"},
  {"tm_ignition2_extractor", "Extracteur allumage 2", 0x0591, Word, Plain, "rpm"},
  {"tm_ignition_screw", "Vis allumage", 0x0593, Word, Plain, "rpm"},
  {"tm_ignition_delta", "DT allumage", 0x0594, Word, Tenths, C},
  {"tm_fumes_on", "T° fumées ON", 0x0595, Word, Tenths, C},
  {"tm_warmup_time", "Durée warmup", 0x0596, Word, Plain, "s"},
  {"tm_warmup_screw", "Vis warmup", 0x0597, Word, Plain, "rpm"},
  {"tm_warmup_leonardo", "Leonardo warmup", 0x0598, Word, Plain, nullptr},
  {"tm_warmup_extractor", "Extracteur warmup", 0x0599, Word, Plain, "rpm"},
  {"tm_cleaning_interval_p35", "Fréquence nettoyage P3-5", 0x059b, Word, Plain, "min"},
  {"tm_cleaning_interval_p12", "Fréquence nettoyage P1-2", 0x059c, Word, Plain, "min"},
  {"tm_cleaning_time_p35", "Durée nettoyage P3-5", 0x059d, Word, Plain, "s"},
  {"tm_cleaning_time_p12", "Durée nettoyage P1-2", 0x059e, Word, Plain, "s"},
  {"tm_cleaning_extractor_p35", "Extracteur nettoyage P3-5", 0x059f, Word, Plain, "rpm"},
  {"tm_cleaning_extractor_p12", "Extracteur nettoyage P1-2", 0x05a0, Word, Plain, "rpm"},
  {"tm_cleaning_screw_p35", "Vis nettoyage P3-5", 0x05a2, Word, Plain, "rpm"},
  {"tm_cleaning_screw_p12", "Vis nettoyage P1-2", 0x05a3, Word, Plain, "rpm"},
  {"tm_extinction_extractor", "Extracteur extinction", 0x05a4, Word, Plain, "rpm"},
  {"tm_extinction_extractor_limit", "Extracteur extinction limite", 0x05a5, Word, Plain, "rpm"},
  {"tm_extinction_time", "Durée extinction", 0x05a7, Word, Plain, "s"},
  {"tm_fumes_off", "T° fumées OFF", 0x05a8, Word, Tenths, C},
  {"tm_cooling_extractor", "Extracteur refroidissement", 0x05a9, Word, Plain, "rpm"},
  {"tm_screw_pmax", "Vis P.MAX", 0x05ab, Word, Plain, "rpm"},
  {"tm_fumes_pmin", "T° fumées P.MIN", 0x05ac, Word, Tenths, C},
  {"tm_fumes_pmax", "T° fumées P.MAX", 0x05ad, Word, Tenths, C},
  {"tm_leonardo_pmax", "Leonardo P.MAX", 0x05ae, Word, Plain, nullptr},
  {"tm_extractor_pmax", "Extracteur P.MAX", 0x05af, Word, Plain, "rpm"},
  {"tm_leonardo_pmin", "Leonardo P.MIN", 0x05b3, Word, Plain, nullptr},
  // Fan profiles V1..V5 and emergency extraction P1..P5: one byte each, low
  // byte first, in menu order.
  {"tm_fan1_v1", "Ventilateur 1 V1", 0x05b5, Low, Plain, "%"},
  {"tm_fan1_v2", "Ventilateur 1 V2", 0x05b5, High, Plain, "%"},
  {"tm_fan1_v3", "Ventilateur 1 V3", 0x05b6, Low, Plain, "%"},
  {"tm_fan1_v4", "Ventilateur 1 V4", 0x05b6, High, Plain, "%"},
  {"tm_fan1_v5", "Ventilateur 1 V5", 0x05b7, Low, Plain, "%"},
  {"tm_fan2_v1", "Ventilateur 2 V1", 0x05b7, High, Plain, "%"},
  {"tm_fan2_v2", "Ventilateur 2 V2", 0x05b8, Low, Plain, "%"},
  {"tm_fan2_v3", "Ventilateur 2 V3", 0x05b8, High, Plain, "%"},
  {"tm_fan2_v4", "Ventilateur 2 V4", 0x05b9, Low, Plain, "%"},
  {"tm_fan2_v5", "Ventilateur 2 V5", 0x05b9, High, Plain, "%"},
  {"tm_fan3_v1", "Ventilateur 3 V1", 0x05ba, Low, Plain, "%"},
  {"tm_fan3_v2", "Ventilateur 3 V2", 0x05ba, High, Plain, "%"},
  {"tm_fan3_v3", "Ventilateur 3 V3", 0x05bb, Low, Plain, "%"},
  {"tm_fan3_v4", "Ventilateur 3 V4", 0x05bb, High, Plain, "%"},
  {"tm_fan3_v5", "Ventilateur 3 V5", 0x05bc, Low, Plain, "%"},
  {"tm_emergency_p1", "Extraction urgence P1", 0x05bd, Low, Plain, "%"},
  {"tm_emergency_p2", "Extraction urgence P2", 0x05bd, High, Plain, "%"},
  {"tm_emergency_p3", "Extraction urgence P3", 0x05be, Low, Plain, "%"},
  {"tm_emergency_p4", "Extraction urgence P4", 0x05be, High, Plain, "%"},
  {"tm_emergency_p5", "Extraction urgence P5", 0x05bf, Low, Plain, "%"},
};
static const unsigned Count = sizeof(params) / sizeof(params[0]);

static const uint16_t First = 0x0577;
static const uint16_t Last = 0x05bf;
static const uint8_t BlockRegisters = 16;
static const unsigned Blocks = (Last - First + BlockRegisters) / BlockRegisters;

inline uint16_t blockStart(unsigned block) { return First + block * BlockRegisters; }
inline uint8_t blockCount(unsigned block) {
  const unsigned remaining = Last + 1u - blockStart(block);
  return remaining < BlockRegisters ? remaining : BlockRegisters;
}
inline bool request(uint8_t out[6], unsigned block) {
  memset(out, 0, 6);
  if (block >= Blocks) return false;
  const uint16_t start = blockStart(block);
  out[0] = 1;
  out[1] = 3;
  out[2] = static_cast<uint8_t>(start >> 8);
  out[3] = static_cast<uint8_t>(start);
  out[5] = blockCount(block);
  return true;
}
// Returns the block index of an FC03 request built by request(), or -1.
inline int blockOf(const uint8_t cmd[6]) {
  if (cmd[0] != 1 || cmd[1] != 3 || cmd[4] != 0) return -1;
  const uint16_t start = (uint16_t(cmd[2]) << 8) | cmd[3];
  for (unsigned b = 0; b < Blocks; ++b)
    if (start == blockStart(b) && cmd[5] == blockCount(b)) return static_cast<int>(b);
  return -1;
}
inline bool inBlock(unsigned index, unsigned block) {
  return index < Count && block < Blocks && params[index].reg >= blockStart(block) &&
         params[index].reg < blockStart(block) + blockCount(block);
}
// data/size: validated FC03 payload of the given block.
inline bool decode(unsigned index, unsigned block, const uint8_t *data, size_t size,
                   char *out, size_t outsize) {
  if (!out || !outsize) return false;
  out[0] = '\0';
  if (!data || !inBlock(index, block) || size != blockCount(block) * 2u) return false;
  const Param &p = params[index];
  const uint8_t *word = data + (p.reg - blockStart(block)) * 2;
  const unsigned value = p.part == Low ? word[1] : p.part == High ? word[0]
                                                                   : (unsigned(word[0]) << 8) | word[1];
  const int written = p.scale == Tenths
      ? snprintf(out, outsize, "%u.%u", value / 10, value % 10)
      : snprintf(out, outsize, "%u", value);
  if (written < 0 || static_cast<size_t>(written) >= outsize) {
    out[0] = '\0';
    return false;
  }
  return true;
}
} // namespace TechMenu
