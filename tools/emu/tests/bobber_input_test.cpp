// Include the generated firmware in this translation unit to inspect Bobber's
// private state while exercising the real setup/loop and touch-driver adapter.
#include "../sketch.cpp"
#include <cassert>

uint32_t g_seed = 7;
static uint32_t testNow = 0;
uint32_t millis() { return testNow; }
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
  auto tick = [&](unsigned ms) {
    testNow += ms;
    loop();
  };
  auto fight = [&]() {
    if (bobberState != BOBBER_FIGHT) return;
    bool held = false;
    for (int step = 0; bobberState == BOBBER_FIGHT && step < 400; ++step) {
      const bool reel = !bobberHardPull() && bobberFight.tension < (held ? 68000 : 35000);
      if (reel != held) sample(reel, 233, 400);
      held = reel;
      tick(100);
    }
    if (held) sample(false, 233, 400);
    assert(bobberState == BOBBER_CAUGHT);
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
  tick(3000);
  assert(bobberState == BOBBER_BITE);
  click(233, 400);
  fight();
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

  // Complete a new save using only firmware touch and clock inputs. Reading
  // state lets the harness react to the same cues and available actions as a player.
  const char *savePath = "fresh-bobber.nvs";
  nvs().clear();
  setup();
  g_seed = 92;
  auto mainTap = [&]() { click(233, 400); };
  auto openSupplies = [&]() {
    assert(bobberState == BOBBER_READY);
    click(302, 326);
    click(233, 234);
    click(233, 303);
    assert(bobberState == BOBBER_SUPPLIES);
  };
  auto openLegend = [&]() {
    assert(bobberState == BOBBER_READY);
    click(161, 326);
    for (int page = 0; page < 4; ++page) click(349, 398);
    assert(bobberState == BOBBER_JOURNAL && bobberJournalPage == 4);
  };
  auto restart = [&]() {
    const auto snapshot = nvs();
    nvsSave(savePath);
    nvs().clear();
    nvsLoad(savePath);
    assert(nvs() == snapshot);
    setup();
    assert(bobberState == BOBBER_TITLE);
    mainTap();
  };
  auto sellSurplus = [&]() {
    click(302, 326);
    click(233, 166);
    assert(bobberState == BOBBER_COOLER);
    int kept = 0;
    for (int page = 0; page < bobberEconomy.count; ++page) {
      const auto fish = bobberEconomy.fish[page];
      const auto &ingredients = bobberIngredients[bobberRegion];
      if ((fish.species == ingredients[0] || fish.species == ingredients[1]) && kept < 2) {
        if (!fish.reserved) click(161, 302);
        ++kept;
      } else if (fish.reserved) click(161, 302);
      click(349, 398);
    }
    mainTap();
    click(233, 234);
    const uint32_t quote = bobberSaleQuote(-1);
    assert(quote);
    const uint32_t wallet = bobberEconomy.coins;
    click(233, 234);
    mainTap();
    assert(bobberState == BOBBER_SHOP && bobberEconomy.coins == wallet + quote);
    assert(bobberEconomy.count == kept);
    mainTap();
    mainTap();
  };
  auto collect = [&](int recipeCount) {
    int attempts = 0;
    while (bobberDiscovered(bobberRegion) < 8 || bobberRecipeCount() < recipeCount) {
      assert(++attempts < 150);
      if (bobberEconomy.count == BOBBER_COOLER_CAPACITY) sellSurplus();
      mainTap();
      assert(bobberState == BOBBER_WAITING);
      const auto stock = bobberEconomy.bait;
      const bool search = stock && bobberDiscovered(bobberRegion) < 8;
      tick(2999);
      assert(bobberState == BOBBER_WAITING);
      tick(1);
      assert(bobberState == BOBBER_BITE);
      mainTap();
      assert(bobberEconomy.bait == stock - search);
      fight();
      assert(bobberState == BOBBER_CAUGHT);
      if (bobberEconomy.count == BOBBER_COOLER_CAPACITY) {
        mainTap();
        assert(bobberState == BOBBER_COOLER);
        mainTap();
        mainTap();
        assert(bobberState == BOBBER_READY);
      } else {
        mainTap(); // recast
        mainTap(); // cancel without using bait
      }
    }
    return attempts;
  };
  auto prepare = [&](bool failFirst = false) {
    openLegend();
    click(233, 326);
    assert(bobberState == BOBBER_RECIPE);
    const auto count = bobberEconomy.count;
    const auto eligible = bobberRecipeCount();
    if (failFirst) {
      const auto before = nvs();
      nvsFailWritesAfter(0);
      click(233, 312);
      assert(!bobberHasLegendBait() && bobberEconomy.count == count && nvs() == before);
      nvsResumeWrites();
    }
    click(233, 312);
    assert(bobberHasLegendBait() && bobberEconomy.count == count - 2 && bobberRecipeCount() == eligible - 2);
    click(233, 312);
    assert(bobberEconomy.count == count - 2); // one bait per biome in stock
    mainTap();
    mainTap();
  };
  auto hookLegend = [&](bool failFirst = false) {
    openLegend();
    click(233, 326);
    tick(3000);
    assert(bobberState == BOBBER_BITE);
    if (failFirst) {
      const auto before = nvs();
      nvsFailWritesAfter(0);
      mainTap();
      assert(bobberState == BOBBER_BITE && bobberHasLegendBait() && nvs() == before);
      nvsResumeWrites();
    }
    mainTap();
    assert(bobberState == BOBBER_FIGHT && !bobberHasLegendBait() && !bobberFight.reeling);
  };

  mainTap();
  openSupplies();
  click(233, 156); // no coins
  assert(!bobberEconomy.bait);
  click(233, 352); // the recipe can be read but is locked
  click(233, 312);
  assert(bobberState == BOBBER_RECIPE && !bobberHasLegendBait());
  mainTap(); mainTap(); mainTap(); mainTap();
  for (int region = 0; region < BOBBER_REGION_COUNT; ++region) {
    if (region) {
      click(302, 326);
      click(233, 303);
      click(233, 170 + 68 * region);
    }
    assert(bobberRegion == region && !bobberDiscovered(bobberRegion));
    openLegend();
    click(233, 326);
    assert(bobberState == BOBBER_JOURNAL);
    mainTap();
    int casts = collect(region == BOBBER_BAY ? 4 : 2);
    prepare(region == BOBBER_BAY);
    if (region == BOBBER_BAY) {
      const auto savedFish = bobberEconomy.count;
      restart();
      assert(bobberHasLegendBait() && bobberEconomy.count == savedFish && bobberDiscovered(bobberRegion) == 8);
      openLegend();
      click(233, 326);
      mainTap();
      assert(bobberState == BOBBER_READY && bobberHasLegendBait()); // early cancellation is free
      hookLegend(true);
      sample(true, 233, 400);
      tick(100);
      assert(bobberFight.reeling && bobberFight.progress > 0);
      sample(true, 20, 20);
      const auto beforeDrag = bobberFight.progress;
      tick(100);
      sample(true, 233, 400);
      tick(100);
      assert(!bobberFight.reeling && bobberFight.progress == beforeDrag);
      sample(false, 233, 400);
      sample(true, 233, 400);
      for (int step = 0; step < 100 && bobberState == BOBBER_FIGHT; ++step) tick(100);
      assert(bobberState == BOBBER_LOST && !bobberBests[BOBBER_OLD_MAN] &&
             !bobberRegionUnlocked(BOBBER_LAKE));
      sample(false, 233, 400);
      assert(bobberState == BOBBER_LOST); // finishing a held reel can't dismiss the loss
      mainTap();
      casts += collect(2);
      prepare();
    }
    hookLegend();
    const auto catches = bobberSessionCatches;
    const auto count = bobberEconomy.count;
    assert(bobberFight.pattern == 1 + region);
    fight();
    assert(bobberSpecies == bobberLegends[region] && bobberSessionCatches == catches + 1 &&
           bobberEconomy.count == count && bobberSavedBests[bobberSpecies] == bobberCatchOz);
    mainTap();
    if (region < BOBBER_GROTTO) {
      assert(bobberState == BOBBER_READY);
      sellSurplus();
      openSupplies();
      if (region == BOBBER_BAY) {
        const auto money = bobberEconomy.coins;
        nvsFailWritesAfter(0);
        click(233, 224);
        assert(!bobberEconomy.tackle && bobberEconomy.coins == money);
        nvsResumeWrites();
        click(233, 224);
        click(233, 224); // an equipped upgrade cannot charge twice
        click(233, 290);
        assert(bobberEconomy.tackle == (BOBBER_STRONG_LINE | BOBBER_FAST_REEL) &&
               bobberEconomy.coins == money - BOBBER_LINE_PRICE - BOBBER_REEL_PRICE);
      }
      for (int i = 0; i < 3; ++i) click(233, 156);
      assert(bobberEconomy.bait == 3);
      const auto wallet = bobberEconomy.coins;
      restart();
      assert(bobberEconomy.bait == 3 && bobberEconomy.coins == wallet &&
             bobberEconomy.tackle == (BOBBER_STRONG_LINE | BOBBER_FAST_REEL));
    } else {
      assert(bobberState == BOBBER_ENDING);
      mainTap();
    }
    printf("Fresh save: %s completed after %d regular catches in this biome\n", bobberRegionNames[region], casts);
  }
  restart();
  assert(bobberRegion == BOBBER_GROTTO);
  for (auto best : bobberBests) assert(best);
  openLegend();
  click(302, 326);
  assert(bobberState == BOBBER_ENDING);
  mainTap();
  assert(bobberState == BOBBER_READY);
  std::remove(savePath);
  puts("Bobber firmware playthrough: purchases, recipe and hook save failures, drag and snap, all legends, disk reload and ending passed");
}
