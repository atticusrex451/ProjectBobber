#pragma once

static const int BOBBER_COOLER_CAPACITY = 20;
struct BobberCoolerFish {
  uint8_t species;
  uint16_t ounces;
  bool reserved;
};
struct BobberEconomy {
  uint32_t coins;
  uint8_t count;
  BobberCoolerFish fish[BOBBER_COOLER_CAPACITY];
};
static BobberEconomy bobberEconomy = {};
static bool bobberEconomyDirty = false;
static bool bobberEconomyUnreadable = false;
static const char *bobberEconomyNotice = nullptr;

// Starter sell prices per pound. Round up to a whole coin; every catch has value.
// Legend trophies are never inventory and cannot be sold.
static const uint8_t bobberCoinsPerPound[] = {
  8, 4, 4, 10, 12, 12, 10, 16, 4, 4, 10, 8, 10, 12, 12, 16,
  8, 8, 12, 10, 16, 20, 24, 20, 0, 0, 0
};
static_assert(sizeof(bobberCoinsPerPound) == BOBBER_SPECIES_COUNT, "Missing sell prices");

static uint32_t bobberFishValue(const BobberCoolerFish &fish) {
  if (fish.species >= BOBBER_OLD_MAN) return 0;
  return ((uint32_t)fish.ounces * bobberCoinsPerPound[fish.species] + 15) / 16;
}

// Fixed byte encoding avoids struct padding/compiler differences. Inventory and
// wallet share ONE NVS blob so a sale cannot save coins without removing fish.
static const int BOBBER_ECONOMY_BYTES = 96;
static uint32_t bobberRead32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void bobberWrite32(uint8_t *p, uint32_t n) {
  for (int i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (i * 8));
}
static uint32_t bobberEconomyChecksum(const uint8_t *bytes) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < BOBBER_ECONOMY_BYTES - 4; ++i) hash = (hash ^ bytes[i]) * 16777619u;
  return hash;
}
static void bobberEncodeEconomy(const BobberEconomy &economy, uint8_t *bytes) {
  memset(bytes, 0, BOBBER_ECONOMY_BYTES);
  bytes[0] = 'B'; bytes[1] = 'C'; bytes[2] = 'L'; bytes[3] = 1;
  bobberWrite32(bytes + 4, economy.coins);
  bytes[8] = economy.count;
  for (int i = 0; i < economy.count; ++i) {
    uint8_t *p = bytes + 12 + i * 4;
    p[0] = economy.fish[i].species;
    p[1] = (uint8_t)economy.fish[i].ounces;
    p[2] = (uint8_t)(economy.fish[i].ounces >> 8);
    p[3] = economy.fish[i].reserved ? 1 : 0;
  }
  bobberWrite32(bytes + 92, bobberEconomyChecksum(bytes));
}
static bool bobberDecodeEconomy(const uint8_t *bytes, BobberEconomy &economy) {
  if (bytes[0] != 'B' || bytes[1] != 'C' || bytes[2] != 'L' || bytes[3] != 1 ||
      bytes[8] > BOBBER_COOLER_CAPACITY || bytes[9] || bytes[10] || bytes[11] ||
      bobberRead32(bytes + 92) != bobberEconomyChecksum(bytes)) return false;
  BobberEconomy decoded = {};
  decoded.coins = bobberRead32(bytes + 4);
  decoded.count = bytes[8];
  for (int i = 0; i < BOBBER_COOLER_CAPACITY; ++i) {
    const uint8_t *p = bytes + 12 + i * 4;
    if (i >= decoded.count) {
      if (p[0] || p[1] || p[2] || p[3]) return false;
      continue;
    }
    const uint16_t ounces = (uint16_t)p[1] | (uint16_t)p[2] << 8;
    if (p[0] >= BOBBER_OLD_MAN || p[3] > 1 ||
        ounces < bobberMinOz[p[0]] || ounces > bobberMaxOz[p[0]]) return false;
    decoded.fish[i] = {p[0], ounces, p[3] != 0};
  }
  economy = decoded;
  return true;
}
static void loadBobberEconomy() {
  bobberEconomy = {};
  bobberEconomyDirty = false;
  bobberEconomyUnreadable = false;
  bobberEconomyNotice = nullptr;
  Preferences prefs;
  // A fresh ESP32 has no Bobber namespace yet: read-only begin would fail and
  // incorrectly block the first cast. Opening writable creates only that empty
  // namespace; existing keys are still read/validated without replacement.
  if (!prefs.begin("bobber", false)) { bobberEconomyUnreadable = true; return; }
  if (prefs.isKey("bobberCooler")) {
    uint8_t bytes[BOBBER_ECONOMY_BYTES];
    if (prefs.getBytesLength("bobberCooler") != sizeof(bytes) ||
        prefs.getBytes("bobberCooler", bytes, sizeof(bytes)) != sizeof(bytes) ||
        !bobberDecodeEconomy(bytes, bobberEconomy)) bobberEconomyUnreadable = true;
  }
  prefs.end();
}
static bool bobberWriteEconomy(const BobberEconomy &economy) {
  if (bobberEconomyUnreadable) return false; // preserve unknown/corrupt data
  uint8_t bytes[BOBBER_ECONOMY_BYTES];
  bobberEncodeEconomy(economy, bytes);
  Preferences prefs;
  if (!prefs.begin("bobber", false)) return false;
  const bool saved = prefs.putBytes("bobberCooler", bytes, sizeof(bytes)) == sizeof(bytes);
  prefs.end();
  return saved;
}
static bool retryBobberEconomy() {
  if (bobberEconomyUnreadable) { bobberEconomyNotice = "Cooler save unreadable"; return false; }
  if (!bobberEconomyDirty) return true;
  if (!bobberWriteEconomy(bobberEconomy)) {
    bobberEconomyNotice = "Save failed. Please retry.";
    return false;
  }
  bobberEconomyDirty = false;
  bobberEconomyNotice = nullptr;
  return true;
}
static bool bobberCommitEconomy(const BobberEconomy &next) {
  if (!retryBobberEconomy()) return false;
  if (!bobberWriteEconomy(next)) {
    bobberEconomyNotice = "Save failed. Nothing changed.";
    return false;
  }
  bobberEconomy = next;
  bobberEconomyNotice = nullptr;
  return true;
}
static bool bobberStoreCatch(BobberSpecies species, uint16_t ounces) {
  if (species >= BOBBER_OLD_MAN) return true; // permanent journal trophy only
  if (bobberEconomyUnreadable || bobberEconomy.count >= BOBBER_COOLER_CAPACITY) return false;
  bobberEconomy.fish[bobberEconomy.count++] = {(uint8_t)species, ounces, false};
  bobberEconomyDirty = true;
  return retryBobberEconomy(); // retain this catch in memory if storage fails
}
static uint32_t bobberSaleQuote(int index) {
  uint32_t quote = 0;
  for (int i = 0; i < bobberEconomy.count; ++i)
    if ((index < 0 || index == i) && !bobberEconomy.fish[i].reserved)
      quote += bobberFishValue(bobberEconomy.fish[i]);
  return quote;
}
static bool bobberSellFish(int index) {
  const uint32_t quote = bobberSaleQuote(index);
  if (!quote) return false;
  if (quote > UINT32_MAX - bobberEconomy.coins) {
    bobberEconomyNotice = "Wallet full. Fish kept.";
    return false;
  }
  BobberEconomy next = bobberEconomy;
  next.coins += quote;
  next.count = 0;
  for (int i = 0; i < bobberEconomy.count; ++i)
    if (!((index < 0 || index == i) && !bobberEconomy.fish[i].reserved))
      next.fish[next.count++] = bobberEconomy.fish[i];
  return bobberCommitEconomy(next);
}
static bool bobberReserveFish(int index) {
  if (index < 0 || index >= bobberEconomy.count) return false;
  BobberEconomy next = bobberEconomy;
  next.fish[index].reserved = !next.fish[index].reserved;
  return bobberCommitEconomy(next);
}
static bool bobberReleaseFish(int index) {
  if (index < 0 || index >= bobberEconomy.count || bobberEconomy.fish[index].reserved) return false;
  BobberEconomy next = bobberEconomy;
  for (int i = index; i + 1 < next.count; ++i) next.fish[i] = next.fish[i + 1];
  --next.count;
  return bobberCommitEconomy(next);
}
