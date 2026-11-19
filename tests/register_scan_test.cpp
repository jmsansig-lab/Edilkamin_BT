#include "RegisterScan.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

using namespace RegisterScan;

int main() {
  // Ranges are chunk aligned and their total fits the scheduler state.
  for (size_t i = 0; i < RangeCount; ++i) {
    assert(ranges[i].first % ChunkRegisters == 0);
    assert(ranges[i].last % ChunkRegisters == 0);
    assert(ranges[i].first < ranges[i].last);
    if (i) assert(ranges[i - 1].last <= ranges[i].first);
  }
  assert(chunkCount() == 112);
  assert(chunkCount() <= 128);

  // Every request is an FC03 read of 16 registers; round trip of indexes.
  for (unsigned i = 0; i < chunkCount(); ++i) {
    uint8_t cmd[6];
    assert(request(cmd, i));
    assert(cmd[0] == 1 && cmd[1] == 3 && cmd[4] == 0 && cmd[5] == ChunkRegisters);
    assert(isScanRequest(cmd));
    const uint16_t reg = static_cast<uint16_t>((cmd[2] << 8) | cmd[3]);
    assert(chunkIndex(reg) == static_cast<int>(i));
  }
  uint8_t cmd[6];
  assert(!request(cmd, chunkCount()));
  uint16_t reg;
  assert(chunkStart(0, reg) && reg == 0x0000);
  assert(chunkStart(0x2b, reg) && reg == 0x02b0);
  assert(chunkStart(chunkCount() - 1, reg) && reg == 0x06f0);
  assert(chunkIndex(0x02b1) == -1 && chunkIndex(0x0700) == -1);

  const uint8_t write[6] = {1, 6, 2, 0xb0, 0, 16};
  const uint8_t wrongCount[6] = {1, 3, 2, 0xb0, 0, 8};
  const uint8_t unknown[6] = {1, 3, 0x07, 0x00, 0, 16};
  assert(!isScanRequest(write) && !isScanRequest(wrongCount) && !isScanRequest(unknown));

  char text[8];
  const uint8_t bytes[3] = {0x00, 0xab, 0x5f};
  assert(hex(bytes, 3, text, sizeof(text)) && strcmp(text, "00AB5F") == 0);
  assert(!hex(bytes, 3, text, 6));
  char k[16];
  key(k, sizeof(k), 0x2b);
  assert(strcmp(k, "scan_02b0") == 0);

  // Scheduler: inactive, MTU gate, alternation, budget, pass wait, failures.
  Scanner s;
  unsigned index = 99;
  assert(!s.takeSlot(0, 300, cmd, index));
  s.start(1000);
  assert(s.active() && s.pass() == 1);
  assert(!s.takeSlot(1000, 23, cmd, index) || false);
  Scanner fresh;
  fresh.start(0);
  assert(fresh.takeSlot(0, 300, cmd, index) && index == 0);
  assert(!fresh.takeSlot(0, 300, cmd, index)); // alternate slot -> normal poll
  assert(fresh.takeSlot(0, 300, cmd, index) && index == 1);

  Scanner b;
  b.start(0);
  unsigned taken = 0;
  for (int i = 0; i < 100; ++i) if (b.takeSlot(0, 300, cmd, index)) ++taken;
  assert(taken == Scanner::ChunksPerWindow);

  // Complete a pass, then wait PassInterval before restarting.
  Scanner p;
  p.start(0);
  unsigned long now = 0;
  unsigned seen = 0;
  for (int guard = 0; guard < 10000 && !p.waiting(); ++guard) {
    if (p.takeSlot(now, 300, cmd, index)) { assert(index == seen); ++seen; }
    p.newWindow();
  }
  assert(seen == chunkCount() && p.waiting());
  assert(!p.takeSlot(now + Scanner::PassInterval - 1, 300, cmd, index));
  bool restarted = false;
  for (int i = 0; i < 4 && !restarted; ++i)
    restarted = p.takeSlot(now + Scanner::PassInterval, 300, cmd, index);
  assert(restarted && index == 0 && p.pass() == 2);

  // A chunk is skipped only after two failures.
  Scanner f;
  f.markFailed(0);
  assert(!f.failed(0));
  f.markFailed(0);
  assert(f.failed(0));
  f.start(0);
  assert(f.takeSlot(0, 300, cmd, index) && index == 1);
  f.markFailed(1000); // out of range: ignored
  f.markFailed(127);
  f.markFailed(127);
  assert(f.failed(127));
  assert(!f.failed(1000));

  // stop/start resets progress.
  f.stop();
  assert(!f.active() && !f.takeSlot(0, 300, cmd, index));
  f.start(5);
  assert(f.next() == 0 && f.pass() == 1);

  puts("Register scan tests passed");
}
