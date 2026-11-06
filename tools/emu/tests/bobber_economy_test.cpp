#include "bobber_welcome.h"
#include <cassert>
#include <cstdio>

uint32_t g_seed = 31;
uint32_t millis() { return 100; }

int main() {
  Arduino_Canvas canvas(466, 466, nullptr);
  auto tap = [&](int x, int y) {
    updateBobberTouch(&canvas, true, x, y);
    updateBobberTouch(&canvas, false, x, y);
  };
  auto mainTap = [&]() { tap(233, 400); };
  auto sameRecords = [&](const std::vector<uint16_t> &records) {
    for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) assert(bobberBests[i] == records[i]);
  };
  nvs().clear();
  loadBobberRecord();
  assert(!bobberEconomy.count && !bobberEconomy.coins && !bobberEconomyUnreadable);
  assert(bobberFishValue({BOBBER_SPOT, 2, false}) == 1);
  assert(bobberFishValue({BOBBER_FLOUNDER, 16, false}) == 8);
  for (int i = 0; i < BOBBER_OLD_MAN; ++i) {
    assert(bobberFishValue({(uint8_t)i, bobberMinOz[i], false}) > 0);
    assert(bobberFishValue({(uint8_t)i, bobberMaxOz[i], false}) >=
           bobberFishValue({(uint8_t)i, bobberMinOz[i], false}));
  }
  mainTap();
  for (int i = 0; i < 20; ++i) {
    mainTap();
    assert(bobberState == BOBBER_WAITING);
    updateBobberFishing(&canvas, bobberCastAt + 3000);
    mainTap();
    assert(bobberState == BOBBER_CAUGHT && bobberEconomy.count == i + 1);
    const auto &fish = bobberEconomy.fish[i];
    assert(fish.species == bobberSpecies && fish.ounces == bobberCatchOz && !fish.reserved);
    assert(!bobberEconomyDirty);
  }
  const auto fullCooler = bobberEconomy;
  loadBobberEconomy();
  assert(bobberEconomy.count == 20);
  for (int i = 0; i < 20; ++i) {
    assert(bobberEconomy.fish[i].species == fullCooler.fish[i].species);
    assert(bobberEconomy.fish[i].ounces == fullCooler.fish[i].ounces);
  }
  const auto records = std::vector<uint16_t>(bobberBests, bobberBests + BOBBER_SPECIES_COUNT);
  const auto seed = g_seed;
  const auto lastFish = bobberEconomy.fish[19];
  assert(!bobberStoreCatch(BOBBER_SPOT, 8) && bobberEconomy.count == 20);
  assert(bobberEconomy.fish[19].ounces == lastFish.ounces);
  mainTap();
  assert(bobberState == BOBBER_COOLER && g_seed == seed && bobberSessionCatches == 20);
  tap(161, 302); // reserve the first fish
  assert(bobberEconomy.fish[0].reserved);
  updateBobberTouch(&canvas, false, 161, 302);
  assert(bobberEconomy.fish[0].reserved); // duplicate release does not toggle again
  tap(302, 302); // protected fish cannot be released
  assert(bobberState == BOBBER_COOLER);
  tap(349, 398);
  assert(bobberCoolerPage == 1);
  tap(302, 302);
  assert(bobberState == BOBBER_CONFIRM && bobberShopAction == BOBBER_RELEASE_ONE);
  tap(233, 326); // cancel release
  assert(bobberState == BOBBER_COOLER && bobberEconomy.count == 20);
  tap(302, 302);
  mainTap();
  assert(bobberState == BOBBER_COOLER && bobberEconomy.count == 19 && !bobberEconomy.coins);
  sameRecords(records);
  mainTap();
  assert(bobberState == BOBBER_DOCK);
  tap(233, 234);
  assert(bobberState == BOBBER_SHOP);
  tap(233, 166);
  assert(bobberState == BOBBER_SELL_FISH && bobberCoolerPage == 0);
  tap(302, 302);
  assert(bobberState == BOBBER_SELL_FISH); // protected fish cannot be sold
  tap(349, 398);
  const uint32_t singleQuote = bobberSaleQuote(1);
  tap(302, 302);
  assert(bobberState == BOBBER_CONFIRM && bobberShopAction == BOBBER_SELL_ONE);
  updateBobberTouch(&canvas, true, 233, 400);
  updateBobberTouch(&canvas, false, 10, 10);
  assert(bobberState == BOBBER_CONFIRM && !bobberEconomy.coins);
  mainTap();
  assert(bobberState == BOBBER_SELL_FISH && bobberEconomy.count == 18 && bobberEconomy.coins == singleQuote);
  updateBobberTouch(&canvas, false, 233, 400);
  assert(bobberEconomy.coins == singleQuote);
  mainTap();
  const uint32_t allQuote = bobberSaleQuote(-1);
  tap(233, 234);
  assert(bobberState == BOBBER_CONFIRM && bobberShopAction == BOBBER_SELL_ALL);
  tap(233, 326);
  assert(bobberState == BOBBER_SHOP && bobberEconomy.count == 18);
  tap(233, 234);
  mainTap();
  assert(bobberState == BOBBER_SHOP && bobberEconomy.count == 1 && bobberEconomy.fish[0].reserved);
  assert(bobberEconomy.coins == singleQuote + allQuote && !bobberSaleQuote(-1));
  sameRecords(records);
  loadBobberRecord();
  assert(bobberEconomy.count == 1 && bobberEconomy.fish[0].reserved);
  assert(bobberEconomy.coins == singleQuote + allQuote);
  sameRecords(records);
  // Legends remain trophies, even with a full cooler, and never award sale coins.
  assert(bobberStoreCatch(BOBBER_OLD_MAN, 900) && bobberEconomy.count == 1);
  assert(bobberReserveFish(0));
  assert(bobberReleaseFish(0) && !bobberEconomy.count);

  // A failed sale writes neither wallet nor cooler. A later retry sells once.
  assert(bobberStoreCatch(BOBBER_FLOUNDER, 32));
  assert(bobberStoreCatch(BOBBER_SPOT, 8));
  const auto beforeSale = nvs();
  const auto beforeCoins = bobberEconomy.coins;
  const auto quote = bobberSaleQuote(-1);
  nvsFailWritesAfter(0);
  assert(!bobberSellFish(-1));
  assert(bobberEconomy.count == 2 && bobberEconomy.coins == beforeCoins && nvs() == beforeSale);
  loadBobberRecord();
  assert(bobberEconomy.count == 2 && bobberEconomy.coins == beforeCoins);
  nvsResumeWrites();
  assert(bobberSellFish(-1));
  loadBobberRecord();
  assert(!bobberEconomy.count && bobberEconomy.coins == beforeCoins + quote);
  assert(!bobberSellFish(-1) && bobberEconomy.coins == beforeCoins + quote);

  // Failed catch saves retain the catch live, stop casting, and offer a retry.
  const auto beforeCatch = nvs();
  nvsFailWritesAfter(0);
  assert(!bobberStoreCatch(BOBBER_SPOT, 8));
  assert(bobberEconomyDirty && bobberEconomy.count == 1 && nvs() == beforeCatch);
  mainTap(); // PLAY
  mainTap(); // failed save retry; must not cast or roll another catch
  assert(bobberState == BOBBER_READY && bobberEconomyDirty && bobberEconomy.count == 1);
  nvsResumeWrites();
  mainTap();
  assert(bobberState == BOBBER_READY && !bobberEconomyDirty);
  mainTap();
  assert(bobberState == BOBBER_WAITING);
  mainTap();
  loadBobberRecord();
  assert(bobberEconomy.count == 1 && bobberEconomy.fish[0].ounces == 8);
  sameRecords(records);

  // Semantic corruption is rejected even if the checksum is valid. Never
  // overwrite an unreadable/future save with an empty wallet or inventory.
  const auto healthySave = nvs();
  for (int variant = 0; variant < 9; ++variant) {
    nvs() = healthySave;
    auto &bytes = nvs()["bobberCooler"];
    if (variant == 0) bytes.resize(95);
    if (variant == 1) bytes.push_back(0);
    if (variant == 2) bytes[4] ^= 1;
    if (variant >= 3) {
      if (variant == 3) bytes[3] = 2;
      if (variant == 4) bytes[8] = 21;
      if (variant == 5) bytes[12] = BOBBER_OLD_MAN;
      if (variant == 6) bytes[13] = bytes[14] = 0;
      if (variant == 7) bytes[15] = 2;
      if (variant == 8) bytes[12] = 255;
      bobberWrite32(bytes.data() + 92, bobberEconomyChecksum(bytes.data()));
    }
    const auto damagedSave = nvs();
    loadBobberRecord();
    assert(bobberEconomyUnreadable);
    assert(!bobberStoreCatch(BOBBER_SPOT, 8) && !bobberSellFish(-1));
    assert(!bobberReserveFish(0) && !retryBobberEconomy());
    mainTap(); // PLAY still lets the user inspect the journal/dock
    mainTap();
    assert(bobberState == BOBBER_READY && nvs() == damagedSave);
    sameRecords(records);
  }
  nvs() = healthySave;
  loadBobberRecord();
  BobberEconomy fullWallet = bobberEconomy;
  fullWallet.coins = UINT32_MAX;
  assert(bobberCommitEconomy(fullWallet));
  assert(!bobberSellFish(-1) && bobberEconomy.count == 1 && bobberEconomy.coins == UINT32_MAX);
  // A pre-economy save starts empty without fabricating fish from old bests.
  nvs().erase("bobberCooler");
  loadBobberRecord();
  assert(!bobberEconomyUnreadable && !bobberEconomy.count && !bobberEconomy.coins);
  sameRecords(records);
  puts("Bobber economy: auto-keep/capacity, reserve/release, confirmed sales, wallet reloads, failed-write atomicity and corrupt-save preservation passed");
}
