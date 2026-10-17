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
  nvs().clear();
  loadBobberRecord();
  assert(bobberBestOz == 0);
  assert(bobberSessionCatches == 0);
  // The emulator shim ignores namespaces: use a unique Bobber key, too.
  Preferences prefs;
  prefs.putUInt("upstreamSentinel", 42);
  bool sawSmaller = false, sawLarger = false, sawTie = false;
  for (int i = 0; i < 1000; ++i) {
    const auto previousBest = bobberBestOz;
    tap();
    assert(bobberSessionCatches == (uint32_t)i);
    assert(bobberState == BOBBER_WAITING);
    updateBobberFishing(&canvas, bobberCastAt + 2999);
    assert(bobberState == BOBBER_WAITING);
    updateBobberFishing(&canvas, bobberCastAt + 3000);
    assert(bobberState == BOBBER_BITE);
    tap();
    assert(bobberState == BOBBER_CAUGHT);
    assert(bobberSessionCatches == (uint32_t)i + 1);
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
    assert(bobberSessionCatches == (uint32_t)i + 1);
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
  assert(bobberSessionCatches == 1000);
  assert(prefs.getUInt("upstreamSentinel") == 42);
  const auto savedBest = bobberBestOz;
  loadBobberRecord(); // simulate reinitializing gameplay over retained NVS
  assert(bobberBestOz == savedBest && bobberCatchOz == 0 && !bobberNewBest);
  assert(bobberSessionCatches == 0);
  nvsFailWritesAfter(0);
  // Already saved records must not attempt another write.
  saveBobberRecord();
  assert(nvsWriteBudget() == 0);
  nvsResumeWrites();
  prefs.putUShort("bobberBestOz", 999);
  loadBobberRecord();
  assert(bobberBestOz == 0);
  prefs.putUShort("bobberBestOz", 7);
  loadBobberRecord();
  assert(bobberBestOz == 0);
  // Failed saves retain the in-memory record, and retry without replacing it.
  bobberBestOz = 100;
  nvsFailWritesAfter(0);
  saveBobberRecord();
  assert(bobberBestOz == 100 && bobberSavedBestOz == 0);
  nvsResumeWrites();
  saveBobberRecord();
  loadBobberRecord();
  assert(bobberBestOz == 100 && bobberSavedBestOz == 100);
  assert(prefs.getUInt("upstreamSentinel") == 42);
  // No writes for a smaller catch or a tie once the record is saved.
  nvsFailWritesAfter(1);
  saveBobberRecord();
  assert(nvsWriteBudget() == 1);
  nvsResumeWrites();
  puts("Bobber persistence: reload, invalid data, failed writes/retry, upstream isolation passed");
  puts("Bobber: 1000 catches, records/ties, redraws, cancel and touch guards passed");
}
