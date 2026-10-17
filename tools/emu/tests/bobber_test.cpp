// Bobber-specific touch/state checks; no upstream --shot preparation.
#include "bobber_welcome.h"
#include <cassert>
#include <cstdio>

// Standalone harness for the real Bobber header and emulator canvas.
uint32_t g_seed = 1;
uint32_t millis() { return 100; }

int main() {
  Arduino_Canvas canvas(466, 466, nullptr);
  auto tap = [&]() {
    updateBobberTouch(&canvas, true, 233, 400);
    updateBobberTouch(&canvas, false, 233, 400);
  };
  assert(bobberBestOz == 0);
  bool sawSmaller = false, sawLarger = false, sawTie = false;
  for (int i = 0; i < 1000; ++i) {
    const auto previousBest = bobberBestOz;
    tap();
    assert(bobberState == BOBBER_WAITING);
    updateBobberFishing(&canvas, bobberCastAt + 2999);
    assert(bobberState == BOBBER_WAITING);
    updateBobberFishing(&canvas, bobberCastAt + 3000);
    assert(bobberState == BOBBER_BITE);
    tap();
    assert(bobberState == BOBBER_CAUGHT);
    assert(bobberCatchOz >= 8 && bobberCatchOz <= 128);
    assert(bobberBestOz == (bobberCatchOz > previousBest ? bobberCatchOz : previousBest));
    assert(bobberNewBest == (bobberCatchOz > previousBest));
    sawSmaller |= bobberCatchOz < previousBest;
    sawLarger |= bobberCatchOz > previousBest;
    sawTie |= bobberCatchOz == previousBest;
    const auto weight = bobberCatchOz;
    const auto seed = g_seed;
    renderBobberWelcome(&canvas);
    assert(bobberCatchOz == weight && g_seed == seed);
  }
  assert(sawSmaller && sawLarger && sawTie);
  const auto best = bobberBestOz;
  const auto seed = g_seed;
  tap(); // cast
  tap(); // cancel
  assert(bobberState == BOBBER_READY && bobberBestOz == best && g_seed == seed);
  tap();
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberFishing(&canvas, bobberCastAt + 3000);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_BITE && g_seed == seed);
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberTouch(&canvas, true, 0, 0);
  updateBobberTouch(&canvas, false, 0, 0);
  assert(bobberState == BOBBER_BITE && bobberBestOz == best && g_seed == seed);
  puts("Bobber: 1000 catches, records/ties, redraws, cancel and touch guards passed");
}
