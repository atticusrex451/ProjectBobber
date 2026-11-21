#pragma once

static void bobberShopText(Arduino_Canvas *canvas, const char *text, int y, int size = 1) {
  canvas->setTextSize(size);
  canvas->setCursor(233 - strlen(text) * size * 3, y);
  canvas->print(text);
}
static void bobberShopButton(Arduino_Canvas *canvas, int x, int y, int width,
                              const char *label, bool enabled = true) {
  canvas->fillRoundRect(x, y, width, 44, 10, enabled ? 0x2B8E : 0x1946);
  canvas->setTextColor(enabled ? 0xF7B9 : 0x94B2);
  canvas->setTextSize(2);
  canvas->setCursor(x + (width - strlen(label) * 12) / 2, y + 14);
  canvas->print(label);
}
static const int bobberSupplyRows[] = {0, BOBBER_STRONG_LINE, BOBBER_FAST_REEL};

static int bobberShopTouchTarget(int16_t x, int16_t y) {
  if (bobberState == BOBBER_DOCK && x >= 95 && x < 371) {
    for (int row = 0; row < 3; ++row)
      if (y >= 145 + row * 68 && y < 189 + row * 68) return 30 + row;
  }
  if (bobberState == BOBBER_SHOP && x >= 95 && x < 371) {
    if (y >= 145 && y < 189 && bobberEconomy.count) return 33;
    if (y >= 213 && y < 257 && bobberSaleQuote(-1)) return 34;
    if (y >= 281 && y < 325) return 40;
  }
  if (bobberState == BOBBER_SUPPLIES && x >= 95 && x < 371) {
    for (int row = 0; row < 3; ++row)
      if (y >= 134 + row * 68 && y < 178 + row * 68 && bobberCanBuySupply(bobberSupplyRows[row])) return 41 + row;
    if (y >= 330 && y < 374) return 44;
  }
  if (bobberState == BOBBER_RECIPE && x >= 123 && x < 343 &&
      y >= 290 && y < 334 && bobberCanPrepareLegendBait()) return 45;
  if (bobberState == BOBBER_COOLER || bobberState == BOBBER_SELL_FISH) {
    if (bobberEconomy.count && y >= 280 && y < 324) {
      if (x >= 95 && x < 229) return 35;
      if (x >= 237 && x < 371 && !bobberEconomy.fish[bobberCoolerPage].reserved) return 36;
    }
    if (y >= 376 && y < 420) {
      if (x >= 95 && x < 139 && bobberCoolerPage > 0) return 38;
      if (x >= 327 && x < 371 && bobberCoolerPage + 1 < bobberEconomy.count) return 39;
    }
  }
  if (bobberState == BOBBER_CONFIRM && x >= 123 && x < 343 && y >= 304 && y < 348) return 37;
  return 0;
}
static bool handleBobberShopTouch(int target) {
  if (target == 30) { bobberCoolerPage = 0; bobberState = BOBBER_COOLER; }
  else if (target == 31) bobberState = BOBBER_SHOP;
  else if (target == 32) bobberState = BOBBER_TRAVEL;
  else if (target == 33) { bobberCoolerPage = 0; bobberState = BOBBER_SELL_FISH; }
  else if (target == 34 || target == 36) {
    bobberShopReturn = bobberState;
    bobberShopAction = target == 34 ? BOBBER_SELL_ALL :
                       bobberState == BOBBER_COOLER ? BOBBER_RELEASE_ONE : BOBBER_SELL_ONE;
    bobberState = BOBBER_CONFIRM;
  }
  else if (target == 35) bobberReserveFish(bobberCoolerPage);
  else if (target == 37) bobberState = bobberShopReturn;
  else if (target == 40) bobberState = BOBBER_SUPPLIES;
  else if (target >= 41 && target <= 43) {
    bobberBuySupply(bobberSupplyRows[target - 41]);
  }
  else if (target == 44) {
    bobberShopReturn = BOBBER_SUPPLIES;
    bobberState = BOBBER_RECIPE;
  }
  else if (target == 45) bobberPrepareLegendBait();
  else if (target == 1 && bobberState == BOBBER_RECIPE) bobberState = bobberShopReturn;
  else if (target == 1 && bobberState == BOBBER_SUPPLIES) bobberState = BOBBER_SHOP;
  else if (target == 38) --bobberCoolerPage;
  else if (target == 39) ++bobberCoolerPage;
  else if (target == 1 && bobberState == BOBBER_CONFIRM) {
    const bool applied = bobberShopAction == BOBBER_RELEASE_ONE ? bobberReleaseFish(bobberCoolerPage) :
                        bobberSellFish(bobberShopAction == BOBBER_SELL_ALL ? -1 : bobberCoolerPage);
    if (applied) {
      if (bobberCoolerPage >= bobberEconomy.count)
        bobberCoolerPage = bobberEconomy.count ? bobberEconomy.count - 1 : 0;
      bobberState = bobberShopReturn;
    }
  }
  else if (target == 1 && (bobberState == BOBBER_COOLER || bobberState == BOBBER_SHOP)) bobberState = BOBBER_DOCK;
  else if (target == 1 && bobberState == BOBBER_SELL_FISH) bobberState = BOBBER_SHOP;
  else if (target == 1 && bobberState == BOBBER_DOCK) bobberState = BOBBER_READY;
  else return false;
  return true;
}
static bool renderBobberShop(Arduino_Canvas *canvas) {
  if (bobberState != BOBBER_DOCK && bobberState != BOBBER_COOLER &&
      bobberState != BOBBER_SHOP && bobberState != BOBBER_SELL_FISH && bobberState != BOBBER_CONFIRM &&
      bobberState != BOBBER_SUPPLIES && bobberState != BOBBER_RECIPE) return false;
  const bool browsing = bobberState == BOBBER_COOLER || bobberState == BOBBER_SELL_FISH;
  bobberShopText(canvas, bobberState == BOBBER_DOCK ? "AT THE DOCK" :
                         bobberState == BOBBER_SHOP ? "BAIT & TACKLE" :
                         bobberState == BOBBER_SUPPLIES ? "SUPPLIES" :
                         bobberState == BOBBER_RECIPE ? "LEGEND BAIT" :
                         bobberState == BOBBER_CONFIRM ? "ARE YOU SURE?" :
                         bobberState == BOBBER_COOLER ? "COOLER" : "SELL FISH", 64, 3);
  char line[72];
  if (bobberEconomyUnreadable) snprintf(line, sizeof(line), "Cooler save unreadable");
  else snprintf(line, sizeof(line), "%lu coins | Cooler %u / 20", (unsigned long)bobberEconomy.coins, bobberEconomy.count);
  bobberShopText(canvas, line, 106);
  if (bobberState == BOBBER_DOCK) {
    bobberShopButton(canvas, 95, 145, 276, "COOLER");
    bobberShopButton(canvas, 95, 213, 276, "SHOP");
    bobberShopButton(canvas, 95, 281, 276, "TRAVEL");
  } else if (bobberState == BOBBER_SHOP) {
    bobberShopButton(canvas, 95, 145, 276, "SELL FISH", bobberEconomy.count != 0);
    bobberShopButton(canvas, 95, 213, 276, "SELL ALL", bobberSaleQuote(-1) != 0);
    bobberShopButton(canvas, 95, 281, 276, "SUPPLIES");
  } else if (bobberState == BOBBER_SUPPLIES) {
    snprintf(line, sizeof(line), "SEARCH BAIT   %d", BOBBER_BAIT_PRICE);
    bobberShopButton(canvas, 95, 134, 276, line, bobberCanBuySupply(0));
    canvas->setTextColor(0xF7B9);
    snprintf(line, sizeof(line), "%u / %d | Helps find missing fish", bobberEconomy.bait, BOBBER_BAIT_LIMIT);
    bobberShopText(canvas, line, 184);
    snprintf(line, sizeof(line), "STRONG LINE   %d", BOBBER_LINE_PRICE);
    bobberShopButton(canvas, 95, 202, 276, bobberEconomy.tackle & BOBBER_STRONG_LINE ? "LINE EQUIPPED" : line,
                     bobberCanBuySupply(BOBBER_STRONG_LINE));
    canvas->setTextColor(0xF7B9);
    bobberShopText(canvas, "Gives you longer to ease tension", 252);
    snprintf(line, sizeof(line), "FAST REEL     %d", BOBBER_REEL_PRICE);
    bobberShopButton(canvas, 95, 270, 276, bobberEconomy.tackle & BOBBER_FAST_REEL ? "REEL EQUIPPED" : line,
                     bobberCanBuySupply(BOBBER_FAST_REEL));
    canvas->setTextColor(0xF7B9);
    bobberShopText(canvas, "Gains more ground while reeling", 316);
    bobberShopButton(canvas, 143, 330, 180, "LEGEND BAIT");
  } else if (bobberState == BOBBER_RECIPE) {
    bobberShopText(canvas, bobberRegionNames[bobberRegion], 128);
    bobberShopText(canvas, "Use any two of these fish", 156, 2);
    bobberShopText(canvas, bobberNames[bobberIngredients[bobberRegion][0]], 186, 2);
    bobberShopText(canvas, bobberNames[bobberIngredients[bobberRegion][1]], 209, 2);
    snprintf(line, sizeof(line), "%d eligible in cooler | Reserved used first", bobberRecipeCount());
    bobberShopText(canvas, line, 237);
    bobberShopText(canvas, !bobberLegendAvailable(bobberRegion) ? "Discover all eight local fish first." :
                           bobberHasLegendBait() ? "Bait ready. Seek the legend in your journal." :
                           "Reserve ingredients to keep them when selling.", 260);
    bobberShopButton(canvas, 123, 290, 220, "PREPARE", bobberCanPrepareLegendBait());
  } else if (browsing) {
    if (bobberEconomyUnreadable) {
      bobberShopText(canvas, "Saved cooler unavailable.", 201, 2);
      bobberShopText(canvas, "Your saved data has been preserved.", 240);
    } else if (!bobberEconomy.count) {
      bobberShopText(canvas, "Your cooler is empty.", 201, 2);
      bobberShopText(canvas, "Regular catches are kept here.", 240);
    } else {
      const auto &fish = bobberEconomy.fish[bobberCoolerPage];
      snprintf(line, sizeof(line), "%d / %u", bobberCoolerPage + 1, bobberEconomy.count);
      bobberShopText(canvas, line, 124);
      drawBobberTrophy(canvas, (BobberSpecies)fish.species, 165, true);
      bobberShopText(canvas, bobberNames[fish.species], 205, 2);
      snprintf(line, sizeof(line), "%u lb %u oz | %lu coins", fish.ounces / 16, fish.ounces % 16,
               (unsigned long)bobberFishValue(fish));
      bobberShopText(canvas, line, 236);
      bobberShopText(canvas, fish.reserved ? "Reserved for later. Safe from selling." : "Available to sell or release.", 255);
      bobberShopButton(canvas, 95, 280, 134, fish.reserved ? "UNRESERVE" : "RESERVE");
      bobberShopButton(canvas, 237, 280, 134, bobberState == BOBBER_COOLER ? "RELEASE" : "SELL", !fish.reserved);
    }
  } else if (bobberState == BOBBER_CONFIRM) {
    bobberShopText(canvas, bobberShopAction == BOBBER_RELEASE_ONE ? "Release this fish?" :
                           bobberShopAction == BOBBER_SELL_ALL ? "Sell unreserved fish?" : "Sell this fish?", 162, 2);
    if (bobberShopAction != BOBBER_SELL_ALL && bobberCoolerPage < bobberEconomy.count) {
      const auto &fish = bobberEconomy.fish[bobberCoolerPage];
      bobberShopText(canvas, bobberNames[fish.species], 203, 2);
      snprintf(line, sizeof(line), "%u lb %u oz", fish.ounces / 16, fish.ounces % 16);
      bobberShopText(canvas, line, 229);
    } else bobberShopText(canvas, "Reserved fish will be kept.", 215);
    const uint32_t quote = bobberShopAction == BOBBER_RELEASE_ONE ? 0 :
                          bobberSaleQuote(bobberShopAction == BOBBER_SELL_ALL ? -1 : bobberCoolerPage);
    snprintf(line, sizeof(line), "You receive: %lu coins", (unsigned long)quote);
    bobberShopText(canvas, line, 253, 2);
    bobberShopText(canvas, "Discoveries and bests stay forever.", 281);
    bobberShopButton(canvas, 123, 304, 220, "CANCEL");
  }
  canvas->setTextColor(0xF7B9);
  const char *notice = bobberEconomyUnreadable ? "Save preserved. Shop changes blocked." :
                       bobberEconomyDirty ? "Catch not saved. Return to retry." : bobberEconomyNotice;
  if (notice) bobberShopText(canvas, notice, bobberState == BOBBER_SUPPLIES ? 120 : 350);
  canvas->fillRoundRect(143, 376, 180, 48, 16, 0xBFE0);
  canvas->setTextColor(0x08E4);
  bobberShopText(canvas, bobberState == BOBBER_CONFIRM ? "CONFIRM" : "BACK", 389, 3);
  if (browsing) {
    bobberShopButton(canvas, 95, 376, 44, "<", bobberCoolerPage > 0);
    bobberShopButton(canvas, 327, 376, 44, ">", bobberCoolerPage + 1 < bobberEconomy.count);
  }
  canvas->setTextSize(1);
  canvas->flush();
  return true;
}
