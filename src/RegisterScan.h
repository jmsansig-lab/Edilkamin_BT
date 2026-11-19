#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Read-only exploration of Modbus holding registers (function 0x03 only).
// Ranges are split into 16-register chunks: the largest read whose encrypted
// reply still fits BleFrame::MaxSize (20 + 5 + 32 bytes -> 64 bytes).
// A chunk never issues a write; a failed chunk is skipped for the rest of the
// boot after two failures, so an unreadable address cannot keep resetting BLE.
namespace RegisterScan {
constexpr uint8_t ChunkRegisters = 16;

struct Range {
  uint16_t first; // inclusive, multiple of ChunkRegisters
  uint16_t last;  // exclusive, multiple of ChunkRegisters
};

// Areas around the registers already known from The Mind / Service apps:
// machine status (0x02B0..), commands, counters and alarm log (0x0310..),
// chrono data (0x04C0..), user parameters (0x0500..) and the database code.
static const Range ranges[] = {
    {0x02b0, 0x0310}, {0x0310, 0x0470}, {0x04c0, 0x0500},
    {0x0500, 0x05b0}, {0x0600, 0x0630}};
constexpr size_t RangeCount = sizeof(ranges) / sizeof(ranges[0]);

inline unsigned chunkCount() {
  unsigned total = 0;
  for (size_t i = 0; i < RangeCount; ++i)
    total += (ranges[i].last - ranges[i].first) / ChunkRegisters;
  return total;
}

// Returns false for an index outside the configured ranges.
inline bool chunkStart(unsigned index, uint16_t &reg) {
  for (size_t i = 0; i < RangeCount; ++i) {
    const unsigned size = (ranges[i].last - ranges[i].first) / ChunkRegisters;
    if (index < size) {
      reg = static_cast<uint16_t>(ranges[i].first + index * ChunkRegisters);
      return true;
    }
    index -= size;
  }
  return false;
}

// Index of the chunk starting at reg, or -1 when reg is not a chunk start.
inline int chunkIndex(uint16_t reg) {
  const unsigned count = chunkCount();
  for (unsigned i = 0; i < count; ++i) {
    uint16_t start;
    if (chunkStart(i, start) && start == reg) return static_cast<int>(i);
  }
  return -1;
}

// Builds an FC03 read. Always function 3: this module cannot write.
inline bool request(uint8_t out[6], unsigned index) {
  uint16_t reg;
  if (!chunkStart(index, reg)) return false;
  out[0] = 1;
  out[1] = 3;
  out[2] = static_cast<uint8_t>(reg >> 8);
  out[3] = static_cast<uint8_t>(reg);
  out[4] = 0;
  out[5] = ChunkRegisters;
  return true;
}

// True for a read request produced by request() (used to route replies).
inline bool isScanRequest(const uint8_t cmd[6]) {
  if (cmd[0] != 1 || cmd[1] != 3 || cmd[4] != 0 || cmd[5] != ChunkRegisters)
    return false;
  return chunkIndex(static_cast<uint16_t>((cmd[2] << 8) | cmd[3])) >= 0;
}

// Upper-case hex, two characters per byte, no separators.
inline bool hex(const uint8_t *data, size_t size, char *out, size_t outsize) {
  static const char digits[] = "0123456789ABCDEF";
  if (!out || outsize < size * 2 + 1) return false;
  for (size_t i = 0; i < size; ++i) {
    out[2 * i] = digits[data[i] >> 4];
    out[2 * i + 1] = digits[data[i] & 0x0f];
  }
  out[size * 2] = '\0';
  return true;
}

inline void key(char *out, size_t size, unsigned index) {
  uint16_t reg = 0;
  chunkStart(index, reg);
  snprintf(out, size, "scan_%04x", reg);
}

// Scheduling state. Times are millis(); unsigned subtraction handles wrap.
class Scanner {
public:
  static constexpr unsigned long PassInterval = 30UL * 60 * 1000;
  static constexpr uint8_t ChunksPerWindow = 6;

  bool active() const { return active_; }
  unsigned next() const { return next_; }
  unsigned pass() const { return pass_; }
  bool waiting() const { return active_ && next_ >= chunkCount(); }

  void start(unsigned long now) {
    if (active_) return;
    active_ = true;
    next_ = 0;
    pass_ = 1;
    passStarted_ = now;
    budget_ = ChunksPerWindow;
    alternate_ = false;
  }
  void stop() { active_ = false; }
  void newWindow() { budget_ = ChunksPerWindow; alternate_ = false; }

  // Decides whether the next poll slot belongs to the scanner. Alternates
  // with the normal poll and respects a per-BLE-window budget.
  bool takeSlot(unsigned long now, uint16_t mtu, uint8_t cmd[6], unsigned &index) {
    if (!active_) return false;
    const unsigned count = chunkCount();
    if (next_ >= count) {
      if (now - passStarted_ < PassInterval) return false;
      next_ = 0;
      ++pass_;
      passStarted_ = now;
    }
    alternate_ = !alternate_;
    if (!alternate_ || budget_ == 0) return false;
    // 16 registers -> 64-byte notification; it needs a negotiated MTU >= 67.
    if (mtu < 67) return false;
    while (next_ < count && failed(next_)) ++next_;
    if (next_ >= count) return false;
    index = next_++;
    --budget_;
    return request(cmd, index);
  }

  // A chunk is skipped after MaxFailures failed attempts (exception or
  // timeout). One transient BLE timeout therefore does not hide a chunk.
  void markFailed(unsigned index) {
    if (index < MaxChunks && failures_[index] < MaxFailures) ++failures_[index];
  }
  bool failed(unsigned index) const {
    return index < MaxChunks && failures_[index] >= MaxFailures;
  }

private:
  static constexpr unsigned MaxChunks = 64;
  static constexpr uint8_t MaxFailures = 2;
  bool active_ = false;
  bool alternate_ = false;
  unsigned next_ = 0;
  unsigned pass_ = 0;
  unsigned long passStarted_ = 0;
  uint8_t budget_ = 0;
  uint8_t failures_[MaxChunks] = {};
};
} // namespace RegisterScan
