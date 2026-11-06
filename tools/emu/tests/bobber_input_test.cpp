// Include the generated firmware in this translation unit to inspect Bobber's
// private state while exercising the real setup/loop and touch-driver adapter.
#include "../sketch.cpp"
#include <cassert>

uint32_t g_seed = 7;
FakeSerial Serial;
FakeESP ESP;
FakeWire Wire;
volatile int g_touchX = 0, g_touchY = 0;
volatile bool g_touchDown = false;
void FakeESP::restart() { exit(0); }
int FakeSerial::available() { return 0; }
String FakeSerial::readStringUntil(char) { return String(""); }

int main() {
  nvs().clear();
  setup();
  auto sample = [&](bool down, int x, int y) {
    g_touchDown = down;
    g_touchX = x;
    g_touchY = y;
    emuFireInterrupt();
    loop();
  };
  auto click = [&](int x, int y) {
    sample(true, x, y);
    sample(false, x, y);
  };
  assert(bobberState == BOBBER_TITLE);
  sample(true, 233, 400);
  assert(bobberState == BOBBER_TITLE); // activation waits for release
  sample(false, 233, 400);
  assert(bobberState == BOBBER_READY);
  click(161, 326);
  assert(bobberState == BOBBER_JOURNAL);
  click(349, 398);
  assert(bobberJournalPage == 1);
  click(233, 400);
  assert(bobberState == BOBBER_READY);
  click(302, 326);
  assert(bobberState == BOBBER_TRAVEL);
  click(233, 170);
  assert(bobberState == BOBBER_READY);
  sample(true, 233, 400);
  sample(true, 10, 10);
  sample(false, 10, 10);
  assert(bobberState == BOBBER_READY); // dragged-off gestures still cancel
  click(233, 400);
  assert(bobberState == BOBBER_WAITING);
  click(233, 400);
  assert(bobberState == BOBBER_READY);
  puts("Bobber firmware input: PLAY, journal, paging, travel, cast/cancel and drag-off passed");
}
