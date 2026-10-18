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
  assert(bobberBests[bobberSpecies] == 0);
  assert(bobberSessionCatches == 0);
  assert(bobberState == BOBBER_TITLE);
  updateBobberFishing(&canvas, 10000);
  assert(bobberState == BOBBER_TITLE);
  // A cancelled PLAY gesture stays on the title.
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberTouch(&canvas, true, 0, 0);
  updateBobberTouch(&canvas, false, 0, 0);
  assert(bobberState == BOBBER_TITLE);
  updateBobberTouch(&canvas, true, 233, 400);
  assert(bobberState == BOBBER_TITLE);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_READY);
  // Releasing PLAY twice must not cast or change records/counters.
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_READY && bobberBests[bobberSpecies] == 0 && bobberSessionCatches == 0);
  renderBobberWelcome(&canvas);
  const auto initialFrame = canvas.fb;
  const auto animationSeed = g_seed;
  canvas.frameReady = false;
  updateBobberFishing(&canvas, 149);
  assert(canvas.fb == initialFrame && !canvas.frameReady);
  updateBobberFishing(&canvas, 150);
  assert(canvas.fb != initialFrame && canvas.frameReady);
  assert(bobberState == BOBBER_READY && g_seed == animationSeed);
  assert(bobberBests[bobberSpecies] == 0 && bobberSessionCatches == 0);
  // Only the scene changes; labels and controls remain untouched.
  for (int y = 0; y < 466; ++y) for (int x = 0; x < 466; ++x)
    if (x < 73 || x >= 393 || y < 145 || y >= 295)
      assert(canvas.fb[y * 466 + x] == initialFrame[y * 466 + x]);
  for (uint32_t now = 300; now <= 1200; now += 150)
    updateBobberFishing(&canvas, now);
  assert(canvas.fb == initialFrame); // eight gentle frames form a loop
  const auto journalSeed = g_seed;
  updateBobberTouch(&canvas, true, 233, 324);
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_READY); // dragging to CAST cannot activate either
  updateBobberTouch(&canvas, true, 233, 324);
  assert(bobberState == BOBBER_READY);
  updateBobberTouch(&canvas, false, 233, 324);
  assert(bobberState == BOBBER_JOURNAL);
  const auto journalFrame = canvas.fb;
  updateBobberFishing(&canvas, 10000);
  assert(canvas.fb == journalFrame && bobberState == BOBBER_JOURNAL);
  assert(g_seed == journalSeed && bobberSessionCatches == 0);
  tap();
  assert(bobberState == BOBBER_READY);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_READY); // BACK does not also cast
  // The emulator shim ignores namespaces: use a unique Bobber key, too.
  Preferences prefs;
  prefs.putUInt("upstreamSentinel", 42);
  bool sawSmaller = false, sawLarger = false, sawTie = false;
  for (int i = 0; i < 1000; ++i) {
    const auto previousBests = std::vector<uint16_t>(bobberBests, bobberBests + BOBBER_SPECIES_COUNT);
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
    const auto previousBest = previousBests[bobberSpecies];
    assert(bobberCatchOz >= bobberMinOz[bobberSpecies] && bobberCatchOz <= bobberMaxOz[bobberSpecies]);
    for (int j = 0; j < BOBBER_SPECIES_COUNT; ++j)
      if (j != bobberSpecies) assert(bobberBests[j] == previousBests[j]);
    assert(bobberBests[bobberSpecies] == (bobberCatchOz > previousBest ? bobberCatchOz : previousBest));
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
  const auto best = bobberBests[bobberSpecies];
  const auto seed = g_seed;
  tap(); // cast
  tap(); // cancel
  assert(bobberState == BOBBER_READY && bobberBests[bobberSpecies] == best && g_seed == seed);
  tap();
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberFishing(&canvas, bobberCastAt + 3000);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberState == BOBBER_BITE && g_seed == seed);
  updateBobberFishing(&canvas, bobberCastAt + 60000);
  assert(bobberState == BOBBER_BITE && g_seed == seed);
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberTouch(&canvas, true, 0, 0);
  updateBobberTouch(&canvas, false, 0, 0);
  assert(bobberState == BOBBER_BITE && bobberBests[bobberSpecies] == best && g_seed == seed);
  assert(bobberSessionCatches == 1000);
  assert(prefs.getUInt("upstreamSentinel") == 42);
  const auto savedBest = bobberBests[BOBBER_FLOUNDER];
  const auto savedCroaker = bobberBests[BOBBER_CROAKER];
  loadBobberRecord(); // simulate reinitializing gameplay over retained NVS
  assert(bobberBests[bobberSpecies] == savedBest && bobberCatchOz == 0 && !bobberNewBest);
  assert(bobberState == BOBBER_TITLE);
  assert(bobberBests[BOBBER_CROAKER] == savedCroaker && savedCroaker > 0);
  assert(bobberSessionCatches == 0);
  nvsFailWritesAfter(0);
  // Already saved records must not attempt another write.
  saveBobberRecord();
  assert(nvsWriteBudget() == 0);
  nvsResumeWrites();
  prefs.putUShort("bobberBestOz", 999);
  loadBobberRecord();
  assert(bobberBests[bobberSpecies] == 0);
  prefs.putUShort("bobberBestOz", 7);
  loadBobberRecord();
  assert(bobberBests[bobberSpecies] == 0);
  // Failed saves retain the in-memory record, and retry without replacing it.
  bobberBests[bobberSpecies] = 100;
  nvsFailWritesAfter(0);
  saveBobberRecord();
  assert(bobberBests[bobberSpecies] == 100 && bobberSavedBests[bobberSpecies] == 0);
  nvsResumeWrites();
  saveBobberRecord();
  loadBobberRecord();
  assert(bobberBests[bobberSpecies] == 100 && bobberSavedBests[bobberSpecies] == 100);
  assert(prefs.getUInt("upstreamSentinel") == 42);
  // No writes for a smaller catch or a tie once the record is saved.
  nvsFailWritesAfter(1);
  saveBobberRecord();
  assert(nvsWriteBudget() == 1);
  nvsResumeWrites();
  puts("Bobber persistence: reload, invalid data, failed writes/retry, upstream isolation passed");
  puts("Bobber: 1000 catches, records/ties, redraws, cancel and touch guards passed");
}
