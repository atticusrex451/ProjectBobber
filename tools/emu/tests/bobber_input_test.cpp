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
  assert(bobberState == BOBBER_DOCK);
  click(233, 303);
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
  click(233, 400);
  assert(bobberState == BOBBER_WAITING);
  bobberCastAt = millis() - 3000;
  loop();
  assert(bobberState == BOBBER_BITE);
  click(233, 400);
  assert(bobberState == BOBBER_CAUGHT && bobberEconomy.count == 1);
  click(233, 400); click(233, 400); // recast/cancel back to ready
  click(302, 326); // dock
  click(233, 166); // cooler
  assert(bobberState == BOBBER_COOLER);
  click(161, 302);
  assert(bobberEconomy.fish[0].reserved);
  click(233, 400); // dock
  click(233, 234); // shop
  click(233, 166); // sell fish
  assert(bobberState == BOBBER_SELL_FISH);
  click(161, 302); // unreserve
  click(302, 302); // quote
  assert(bobberState == BOBBER_CONFIRM);
  const auto quote = bobberSaleQuote(0);
  click(233, 400); // confirm sale through real touch release
  assert(!bobberEconomy.count && bobberEconomy.coins == quote);
  puts("Bobber firmware economy input: auto-keep, dock/cooler/shop, reservation and sale confirmation passed");
  puts("Bobber firmware input: PLAY, journal, paging, travel, cast/cancel and drag-off passed");
}
