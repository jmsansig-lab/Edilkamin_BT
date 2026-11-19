#include "../src/AlarmLog.h"
#include <assert.h>
#include <string.h>

int main() {
  using namespace AlarmLog;
  uint8_t slot = 0;
  assert(!newestSlot(0, 0, slot));
  assert(!newestSlot(100, 10, slot));
  assert(!newestSlot(5, 101, slot));
  assert(newestSlot(0, 100, slot) && slot == 99);
  assert(newestSlot(81, 80, slot) && slot == 80);

  uint8_t cmd[6];
  assert(request(cmd, 0));
  const uint8_t first[] = {1, 3, 0x03, 0x3c, 0, 4};
  assert(!memcmp(cmd, first, sizeof(first)));
  assert(request(cmd, 99));
  assert(cmd[2] == 0x04 && cmd[3] == 0xc8);  // 0x033C + 396
  assert(!request(cmd, 100));

  // Real entry read on a Celia: 52FD 6A02 0015 00FF.
  const uint8_t real[] = {0x52, 0xfd, 0x6a, 0x02, 0x00, 0x15, 0x00, 0xff};
  Entry e{};
  assert(decode(real, sizeof(real), e));
  assert(e.code == 21 && e.timestamp == 0x6a0252fdUL);
  char date[24];
  assert(isoDate(e.timestamp, date, sizeof(date)));
  assert(!strcmp(date, "2026-05-11T22:06:53Z"));

  const uint8_t erased[8] = {0};
  assert(!decode(erased, sizeof(erased), e));
  uint8_t bad[8];
  memcpy(bad, real, 8);
  bad[4] = 0x01;  // type 0x0115 > 0xff
  assert(!decode(bad, sizeof(bad), e));
  assert(!decode(real, 6, e));
  assert(!decode(nullptr, 8, e));

  char out[48];
  text(21, false, out, sizeof(out));
  assert(!strcmp(out, "Power outage"));
  text(21, true, out, sizeof(out));
  assert(!strcmp(out, "Coupure de courant"));
  text(1, true, out, sizeof(out));
  assert(!strcmp(out, "Manque d'air comburant"));
  text(42, false, out, sizeof(out));
  assert(!strcmp(out, "Code 42"));
  return 0;
}
