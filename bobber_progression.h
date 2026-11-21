#pragma once

static const BobberSpecies bobberIngredients[BOBBER_REGION_COUNT][2] = {
  {BOBBER_CROAKER, BOBBER_SPOT},
  {BOBBER_YELLOW_PERCH, BOBBER_PUMPKINSEED},
  {BOBBER_DOUBLOON_DARTER, BOBBER_BARNACLE_GOBY},
};
static const int BOBBER_RECIPE_FISH = 2;

static bool bobberHasLegendBait() {
  return bobberEconomy.legendBait & (1 << bobberRegion);
}

static bool bobberIsIngredient(const BobberCoolerFish &fish) {
  return fish.species == bobberIngredients[bobberRegion][0] ||
         fish.species == bobberIngredients[bobberRegion][1];
}

static int bobberRecipeCount() {
  int count = 0;
  for (int i = 0; i < bobberEconomy.count; ++i)
    if (bobberIsIngredient(bobberEconomy.fish[i])) ++count;
  return count;
}

static bool bobberCanPrepareLegendBait() {
  return !bobberEconomyUnreadable && bobberLegendAvailable(bobberRegion) &&
         !bobberHasLegendBait() && bobberRecipeCount() >= BOBBER_RECIPE_FISH;
}

static bool bobberPrepareLegendBait() {
  if (!bobberCanPrepareLegendBait()) return false;
  BobberEconomy next = bobberEconomy;
  next.legendBait |= 1 << bobberRegion;
  // Prefer reserved ingredients. Preserve the order of all fish left in the cooler.
  int needed = BOBBER_RECIPE_FISH;
  bool consume[BOBBER_COOLER_CAPACITY] = {};
  for (int reserved = 1; reserved >= 0; --reserved) {
    for (int i = 0; i < bobberEconomy.count && needed; ++i) {
      const auto &fish = bobberEconomy.fish[i];
      if (fish.reserved == reserved && bobberIsIngredient(fish)) {
        consume[i] = true;
        --needed;
      }
    }
  }
  next.count = 0;
  for (int i = 0; i < bobberEconomy.count; ++i)
    if (!consume[i]) next.fish[next.count++] = bobberEconomy.fish[i];
  return bobberCommitEconomy(next);
}

static uint32_t bobberSupplyPrice(int tackle) {
  if (tackle == BOBBER_STRONG_LINE) return BOBBER_LINE_PRICE;
  if (tackle == BOBBER_FAST_REEL) return BOBBER_REEL_PRICE;
  return BOBBER_BAIT_PRICE;
}

static bool bobberCanBuySupply(int tackle) {
  const bool available = tackle ? !(bobberEconomy.tackle & tackle) : bobberEconomy.bait < BOBBER_BAIT_LIMIT;
  return !bobberEconomyUnreadable && available && bobberEconomy.coins >= bobberSupplyPrice(tackle);
}

static bool bobberBuySupply(int tackle) {
  if (!bobberCanBuySupply(tackle)) return false;
  BobberEconomy next = bobberEconomy;
  next.coins -= bobberSupplyPrice(tackle);
  if (tackle) next.tackle |= tackle;
  else ++next.bait;
  return bobberCommitEconomy(next);
}

static BobberSpecies bobberRollFish(bool useSearchBait) {
  const auto &roster = bobberRosters[bobberRegion];
  if (useSearchBait) {
    BobberSpecies missing[BOBBER_REGULAR_COUNT];
    int count = 0;
    for (int i = 0; i < BOBBER_REGULAR_COUNT; ++i)
      if (!bobberBests[roster[i]]) missing[count++] = roster[i];
    if (count && random(4) != 0) return missing[random(count)];
  }
  return roster[random(BOBBER_REGULAR_COUNT)];
}
