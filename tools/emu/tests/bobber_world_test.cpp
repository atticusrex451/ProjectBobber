// End-to-end biome progression through the real Bobber touch/state handlers.
#include "bobber_welcome.h"
#include <cassert>
#include <cstdio>

uint32_t g_seed = 19;
uint32_t millis() { return 100; }

int main() {
  Arduino_Canvas canvas(466, 466, nullptr);
  auto tap = [&](int x, int y) {
    updateBobberTouch(&canvas, true, x, y);
    updateBobberTouch(&canvas, false, x, y);
  };
  auto mainTap = [&]() { tap(233, 400); };
  auto openLegend = [&]() {
    assert(bobberState == BOBBER_READY);
    tap(161, 326);
    assert(bobberState == BOBBER_JOURNAL && bobberJournalPage == 0);
    for (int page = 1; page < BOBBER_JOURNAL_PAGES; ++page) {
      tap(349, 398);
      assert(bobberJournalPage == page);
    }
    tap(349, 398);
    assert(bobberJournalPage == 4);
  };
  auto returnReady = [&]() {
    assert(bobberState == BOBBER_CAUGHT);
    if (bobberEconomy.count == BOBBER_COOLER_CAPACITY) assert(bobberSellFish(-1));
    mainTap(); // ordinary recast
    assert(!bobberLegendCast && bobberState == BOBBER_WAITING);
    mainTap(); // cancel
    assert(bobberState == BOBBER_READY);
  };
  auto chooseRegion = [&](BobberRegion region) {
    assert(bobberState == BOBBER_READY);
    tap(302, 326);
    assert(bobberState == BOBBER_DOCK);
    tap(233, 303);
    assert(bobberState == BOBBER_TRAVEL);
    const auto seed = g_seed;
    const auto count = bobberSessionCatches;
    tap(233, 170 + region * 68);
    assert(bobberRegion == region && bobberState == BOBBER_READY);
    assert(bobberJournalPage == 0 && g_seed == seed && bobberSessionCatches == count);
    assert(bobberSpecies == bobberRosters[region][0]);
    updateBobberTouch(&canvas, false, 233, 400);
    assert(bobberState == BOBBER_READY);
  };

  assert(BOBBER_SPECIES_COUNT == 27);
  bool listed[BOBBER_SPECIES_COUNT] = {};
  for (int r = 0; r < BOBBER_REGION_COUNT; ++r) {
    for (auto species : bobberRosters[r]) {
      assert(!listed[species]);
      listed[species] = true;
    }
    assert(!listed[bobberLegends[r]]);
    listed[bobberLegends[r]] = true;
  }
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) {
    assert(listed[i] && bobberMinOz[i] > 0 && bobberMinOz[i] <= bobberMaxOz[i]);
    assert(strlen(bobberRecordKeys[i]) <= 15 && strlen(bobberNames[i]) <= 24);
    for (int j = 0; j < i; ++j) assert(strcmp(bobberRecordKeys[i], bobberRecordKeys[j]) != 0);
  }
  // An older bay-only save needs no migration or reset.
  nvs().clear();
  Preferences prefs;
  prefs.putUShort("bobberBestOz", 100);
  prefs.putUShort("croakerBestOz", 40);
  prefs.putUShort("spotBestOz", 12);
  loadBobberRecord();
  assert(bobberRegion == BOBBER_BAY && bobberBests[BOBBER_FLOUNDER] == 100);
  assert(bobberBests[BOBBER_CROAKER] == 40 && bobberBests[BOBBER_SPOT] == 12);
  for (int i = BOBBER_YELLOW_PERCH; i < BOBBER_SPECIES_COUNT; ++i) assert(!bobberBests[i]);

  nvs().clear();
  loadBobberRecord();
  mainTap();
  assert(bobberState == BOBBER_READY);
  tap(302, 326);
  assert(bobberState == BOBBER_DOCK);
  tap(233, 303);
  assert(bobberState == BOBBER_TRAVEL);
  for (int r = 1; r < BOBBER_REGION_COUNT; ++r) {
    tap(233, 170 + r * 68);
    assert(bobberState == BOBBER_TRAVEL && bobberRegion == BOBBER_BAY);
  }
  // Releasing outside a target without a move event cancels the tap, too.
  updateBobberTouch(&canvas, true, 233, 170);
  updateBobberTouch(&canvas, false, 10, 10);
  assert(bobberState == BOBBER_TRAVEL);
  mainTap();

  uint32_t totalCatches = 0;
  for (int r = 0; r < BOBBER_REGION_COUNT; ++r) {
    const auto region = (BobberRegion)r;
    if (r) chooseRegion(region);
    assert(bobberRegion == region && bobberDiscovered(region) == 0);
    assert(!bobberLegendAvailable(region));
    // Each area's animation stays within the scene and leaves gameplay alone.
    bobberAnimationAt = 0;
    bobberAnimationFrame = 0;
    renderBobberWelcome(&canvas);
    const auto still = canvas.fb;
    const auto animationSeed = g_seed;
    updateBobberFishing(&canvas, 150);
    assert(canvas.fb != still && g_seed == animationSeed && bobberState == BOBBER_READY);
    for (int y = 0; y < 466; ++y) for (int x = 0; x < 466; ++x)
      if (x < 73 || x >= 393 || y < 145 || y >= 295)
        assert(canvas.fb[y * 466 + x] == still[y * 466 + x]);
    openLegend();
    tap(233, 326);
    assert(bobberState == BOBBER_JOURNAL); // no legend before all eight
    mainTap();

    bool checkedSeven = false;
    unsigned catches[BOBBER_SPECIES_COUNT] = {};
    for (int sample = 0; sample < 500; ++sample) {
      if (!checkedSeven && bobberDiscovered(region) == 7) {
        openLegend();
        tap(233, 326);
        assert(bobberState == BOBBER_JOURNAL && !bobberLegendAvailable(region));
        mainTap();
        checkedSeven = true;
      }
      const auto before = std::vector<uint16_t>(bobberBests, bobberBests + BOBBER_SPECIES_COUNT);
      mainTap();
      assert(bobberState == BOBBER_WAITING && !bobberLegendCast);
      // Neither travel nor journal can interrupt a cast.
      tap(302, 326); tap(161, 326);
      assert(bobberState == BOBBER_WAITING);
      updateBobberFishing(&canvas, bobberCastAt + 3000);
      mainTap();
      assert(bobberState == BOBBER_CAUGHT && bobberSessionCatches == ++totalCatches);
      bool local = false;
      for (auto species : bobberRosters[r]) local |= bobberSpecies == species;
      assert(local && bobberCatchOz >= bobberMinOz[bobberSpecies] && bobberCatchOz <= bobberMaxOz[bobberSpecies]);
      ++catches[bobberSpecies];
      for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) {
        if (i != bobberSpecies) assert(bobberBests[i] == before[i]);
        else assert(bobberBests[i] == (bobberCatchOz > before[i] ? bobberCatchOz : before[i]));
      }
      returnReady();
    }
    assert(checkedSeven && bobberDiscovered(region) == 8 && bobberLegendAvailable(region));
    for (auto species : bobberRosters[r]) assert(catches[species] > 0);
    assert(!bobberBests[bobberLegends[r]]); // ordinary casts never roll a legend
    if (r < 2) assert(!bobberRegionUnlocked((BobberRegion)(r + 1)));

    // Legend casts can be cancelled without consuming a catch or rolling RNG.
    openLegend();
    const auto seed = g_seed;
    const auto frame = canvas.fb;
    updateBobberFishing(&canvas, 90000);
    assert(canvas.fb == frame && g_seed == seed);
    updateBobberTouch(&canvas, true, 233, 326);
    updateBobberTouch(&canvas, false, 10, 10);
    assert(bobberState == BOBBER_JOURNAL);
    tap(233, 326);
    assert(bobberLegendCast && bobberState == BOBBER_WAITING);
    mainTap();
    assert(bobberState == BOBBER_READY && !bobberLegendCast && g_seed == seed);
    openLegend();
    tap(233, 326);
    updateBobberTouch(&canvas, true, 233, 400);
    updateBobberFishing(&canvas, bobberCastAt + 3000);
    updateBobberTouch(&canvas, false, 233, 400);
    assert(bobberState == BOBBER_BITE && g_seed == seed);
    updateBobberFishing(&canvas, bobberCastAt + 60000);
    assert(bobberState == BOBBER_BITE);
    mainTap();
    assert(bobberState == BOBBER_CAUGHT && bobberSpecies == bobberLegends[r]);
    assert(bobberSessionCatches == ++totalCatches);
    assert(bobberCatchOz >= bobberMinOz[bobberSpecies] && bobberCatchOz <= bobberMaxOz[bobberSpecies]);
    assert(bobberSavedBests[bobberSpecies] == bobberCatchOz);
    if (r < 2) assert(bobberRegionUnlocked((BobberRegion)(r + 1)));
    returnReady();
  }

  const auto records = std::vector<uint16_t>(bobberBests, bobberBests + BOBBER_SPECIES_COUNT);
  loadBobberRecord();
  assert(bobberState == BOBBER_TITLE && bobberRegion == BOBBER_GROTTO && !bobberSessionCatches);
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) assert(records[i] && bobberBests[i] == records[i]);
  mainTap();
  // Failed travel saves preserve the live selection and retry later.
  nvsFailWritesAfter(0);
  chooseRegion(BOBBER_BAY);
  assert(bobberRegion == BOBBER_BAY && bobberSavedRegion == BOBBER_GROTTO);
  nvsResumeWrites();
  saveBobberRecord();
  loadBobberRecord();
  assert(bobberRegion == BOBBER_BAY && bobberSavedRegion == BOBBER_BAY);
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) assert(bobberBests[i] == records[i]);
  prefs.putUChar("bobberRegion", 255);
  loadBobberRecord();
  assert(bobberRegion == BOBBER_BAY);
  const auto completeSave = nvs();
  nvs().erase("oldManBestOz");
  prefs.putUChar("bobberRegion", BOBBER_GROTTO);
  loadBobberRecord();
  assert(bobberRegion == BOBBER_BAY && !bobberRegionUnlocked(BOBBER_GROTTO));
  nvs() = completeSave;
  loadBobberRecord();

  // All 19 new profiles render distinctly and never roll RNG or modify records.
  const auto portraitSeed = g_seed;
  std::vector<std::vector<uint16_t>> portraits;
  for (int i = BOBBER_YELLOW_PERCH; i < BOBBER_SPECIES_COUNT; ++i) {
    canvas.fillScreen(0);
    drawBobberTrophy(&canvas, (BobberSpecies)i, 144, false);
    const auto silhouette = canvas.fb;
    canvas.fillScreen(0);
    drawBobberTrophy(&canvas, (BobberSpecies)i, 144, true);
    assert(canvas.fb != silhouette);
    for (const auto &previous : portraits) assert(canvas.fb != previous);
    portraits.push_back(canvas.fb);
  }
  assert(g_seed == portraitSeed);
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) assert(bobberBests[i] == records[i]);
  puts("Bobber world: 1500 local catches, three gated legends, travel, all 27 records/reloads and portraits passed");
}
