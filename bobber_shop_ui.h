#pragma once

#include "bobber_shopkeeper_art.h"
#include "bobber_fish_tales.h"

static constexpr uint16_t BOBBER_CREAM = 0xF7B9;
static constexpr uint16_t BOBBER_INK = 0x08E4;
static constexpr uint16_t BOBBER_BRASS = 0xCD69;
static constexpr uint16_t BOBBER_MUTED_TEXT = 0xB5B5;

static void bobberPanel(Arduino_Canvas *canvas, int x, int y, int width, int height,
                        uint16_t edge = 0x5270, uint16_t face = 0x1948) {
  canvas->fillRoundRect(x, y, width, height, 12, edge);
  canvas->fillRoundRect(x + 1, y + 1, width - 2, height - 2, 11, face);
}

static void bobberShopText(Arduino_Canvas *canvas, const char *text, int y, int size = 1) {
  canvas->setTextSize(size);
  canvas->setCursor(233 - strlen(text) * size * 3, y);
  canvas->print(text);
}
static void bobberHeading(Arduino_Canvas *canvas, const char *text, int y) {
  canvas->setTextColor(BOBBER_CREAM);
  bobberShopText(canvas, text, y, 3);
  canvas->drawFastHLine(188, y + 31, 90, BOBBER_BRASS);
  canvas->fillRect(230, y + 29, 6, 5, BOBBER_BRASS);
}

static void bobberShopButton(Arduino_Canvas *canvas, int x, int y, int width,
                             const char *label, bool enabled = true, int height = 44) {
  bobberPanel(canvas, x, y, width, height, enabled ? 0x6573 : 0x39AB,
              enabled ? 0x234D : 0x1927);
  canvas->setTextColor(enabled ? BOBBER_CREAM : BOBBER_MUTED_TEXT);
  canvas->setTextSize(2);
  canvas->setCursor(x + (width - (int)strlen(label) * 12) / 2, y + (height - 16) / 2);
  canvas->print(label);
  canvas->setTextColor(BOBBER_CREAM);
}

static void bobberPrimaryButton(Arduino_Canvas *canvas, const char *label,
                                int x = 143, int width = 180, int size = 0) {
  canvas->fillRoundRect(x, 376, width, 48, 16, 0x7D62);
  canvas->fillRoundRect(x + 1, 376, width - 2, 45, 15, 0xBFE0);
  if (!size) size = strlen(label) > 7 ? 2 : 3;
  canvas->setTextColor(BOBBER_INK);
  canvas->setTextSize(size);
  canvas->setCursor(x + (width - (int)strlen(label) * 6 * size) / 2, size == 3 ? 387 : 391);
  canvas->print(label);
}

static void drawBobberShopCounters(Arduino_Canvas *canvas) {
  if (bobberEconomyUnreadable) {
    bobberPanel(canvas, 119, 97, 228, 33);
    bobberShopText(canvas, "SAVE UNAVAILABLE", 105, 2);
    return;
  }
  bobberPanel(canvas, 78, 97, 178, 33);
  bobberPanel(canvas, 264, 97, 124, 33);
  // Raised edge and slot on the coin. A fin and tail make the cooler icon readable.
  canvas->fillCircle(99, 113, 10, 0x8AA4);
  canvas->fillCircle(98, 112, 8, 0xE650);
  canvas->drawFastVLine(98, 108, 9, 0xAF06);
  canvas->fillTriangle(286, 113, 294, 107, 294, 119, 0x7D96);
  canvas->fillRoundRect(272, 108, 18, 11, 5, 0xABD7);
  canvas->fillRect(275, 111, 2, 2, BOBBER_INK);
  canvas->fillTriangle(278, 109, 281, 104, 286, 110, 0x7D96);
  char value[16];
  canvas->setTextColor(BOBBER_CREAM);
  snprintf(value, sizeof(value), "%lu", (unsigned long)bobberEconomy.coins);
  const bool longBalance = strlen(value) > 7;
  canvas->setTextSize(longBalance ? 2 : 3);
  canvas->setCursor(114, longBalance ? 105 : 101);
  canvas->print(value);
  snprintf(value, sizeof(value), "%u/%u", bobberEconomy.count, BOBBER_COOLER_CAPACITY);
  canvas->setTextSize(3);
  canvas->setCursor(298, 101);
  canvas->print(value);
}

enum BobberDockIcon { BOBBER_ICON_COOLER, BOBBER_ICON_SHOP, BOBBER_ICON_TRAVEL };

static void drawBobberDockIcon(Arduino_Canvas *canvas, int x, int y, BobberDockIcon icon) {
  constexpr uint16_t brass = BOBBER_BRASS, bright = BOBBER_CREAM;
  if (icon == BOBBER_ICON_COOLER) {
    canvas->fillRect(x + 4, y + 8, 22, 16, 0x6494);
    canvas->fillRect(x + 2, y + 5, 26, 5, bright);
    canvas->fillRect(x + 13, y + 8, 4, 6, brass);
    canvas->fillRect(x + 6, y + 21, 18, 2, 0x3B6F);
  } else if (icon == BOBBER_ICON_SHOP) {
    canvas->fillRect(x + 2, y + 10, 26, 14, 0x8B67);
    canvas->fillRect(x + 3, y + 7, 24, 5, brass);
    canvas->fillRect(x + 11, y + 3, 8, 4, brass);
    canvas->fillRect(x + 14, y + 11, 4, 5, bright);
  } else {
    canvas->fillTriangle(x + 14, y + 2, x + 14, y + 17, x + 4, y + 17, bright);
    canvas->fillTriangle(x + 16, y + 5, x + 24, y + 17, x + 16, y + 17, 0x9DB0);
    canvas->fillTriangle(x + 2, y + 20, x + 28, y + 20, x + 23, y + 25, brass);
    canvas->fillRect(x + 7, y + 23, 16, 2, brass);
  }
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
    if (y >= 268 && y < 312) return 40;
    if (y >= 323 && y < 367) return 46;
  }
  if (bobberState == BOBBER_SUPPLIES && x >= 95 && x < 371) {
    for (int row = 0; row < 3; ++row)
      if (y >= 134 + row * 68 && y < 178 + row * 68 && bobberCanBuySupply(bobberSupplyRows[row])) return 41 + row;
    if (y >= 330 && y < 374) return 44;
  }
  if (bobberState == BOBBER_FISH_TALE && x >= 123 && x < 343 &&
      y >= 304 && y < 348) return 46;
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
  else if (target == 31) {
    bobberState = bobberShopWelcomed ? BOBBER_SHOP : BOBBER_SHOP_INTRO;
    bobberShopWelcomed = true;
    bobberShopIntroAt = millis();
    bobberShopIntroFrame = 0;
  }
  else if (target == 1 && bobberState == BOBBER_SHOP_INTRO) bobberState = BOBBER_SHOP;
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
  else if (target == 46) {
    pickBobberFishTale();
    bobberState = BOBBER_FISH_TALE;
  }
  else if (target == 1 && bobberState == BOBBER_FISH_TALE) bobberState = BOBBER_SHOP;
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
static void renderBobberShopIntro(Arduino_Canvas *canvas) {
  bobberHeading(canvas, "BAIT & TACKLE", 58);
  constexpr int scale = 2;
  constexpr int x = (466 - scale * BOBBER_SHOPKEEPER_WIDTH) / 2;
  constexpr int y = 298 - scale * BOBBER_SHOPKEEPER_HEIGHT;
  auto drawSpan = [=](const BobberPortraitSpan &span) {
    canvas->fillRect(x + scale * span.x, y + scale * span.y, scale * span.width, scale, span.color);
  };
  for (const auto &span : bobberShopkeeperHead) drawSpan(span);
  const auto &pose = bobberShopkeeperPoses[bobberShopIntroFrame];
  for (size_t i = 0; i < pose.count; ++i) drawSpan(pose.spans[i]);
  bobberPanel(canvas, 83, 321, 300, 37, BOBBER_BRASS);
  canvas->setTextColor(BOBBER_CREAM);
  bobberShopText(canvas, "Fish stories are free.", 332, 2);
  bobberPrimaryButton(canvas, "SHOP");
  canvas->setTextSize(1);
  canvas->flush();
}

static bool renderBobberShop(Arduino_Canvas *canvas) {
  if (bobberState == BOBBER_SHOP_INTRO) {
    renderBobberShopIntro(canvas);
    return true;
  }
  if (bobberState == BOBBER_FISH_TALE) {
    const auto &tale = bobberFishTales[bobberFishTale];
    bobberHeading(canvas, tale.fact ? "FISH FACT" : "FISH TALE", 58);
    bobberPanel(canvas, 65, 127, 336, 153, BOBBER_BRASS);
    for (int i = 0; i < 4 && tale.lines[i]; ++i)
      bobberShopText(canvas, tale.lines[i], 153 + 26 * i, 2);
    bobberShopButton(canvas, 123, 304, 220, "ANOTHER");
    bobberPrimaryButton(canvas, "BACK");
    canvas->setTextSize(1);
    canvas->flush();
    return true;
  }
  if (bobberState != BOBBER_DOCK && bobberState != BOBBER_COOLER &&
      bobberState != BOBBER_SHOP && bobberState != BOBBER_SELL_FISH && bobberState != BOBBER_CONFIRM &&
      bobberState != BOBBER_SUPPLIES && bobberState != BOBBER_RECIPE) return false;
  const bool browsing = bobberState == BOBBER_COOLER || bobberState == BOBBER_SELL_FISH;
  bobberHeading(canvas, bobberState == BOBBER_DOCK ? "AT THE DOCK" :
                         bobberState == BOBBER_SHOP ? "BAIT & TACKLE" :
                         bobberState == BOBBER_SUPPLIES ? "SUPPLIES" :
                         bobberState == BOBBER_RECIPE ? "LEGEND BAIT" :
                         bobberState == BOBBER_CONFIRM ? "ARE YOU SURE?" :
                         bobberState == BOBBER_COOLER ? "COOLER" : "SELL FISH", 58);
  char line[72];
  drawBobberShopCounters(canvas);
  const char *notice = bobberEconomyUnreadable ? "Save preserved. Shop changes blocked." :
                       bobberEconomyDirty ? "Catch not saved. Return to retry." : bobberEconomyNotice;
  if (bobberState == BOBBER_DOCK) {
    bobberShopButton(canvas, 95, 145, 276, "COOLER");
    bobberShopButton(canvas, 95, 213, 276, "SHOP");
    bobberShopButton(canvas, 95, 281, 276, "TRAVEL");
    drawBobberDockIcon(canvas, 112, 154, BOBBER_ICON_COOLER);
    drawBobberDockIcon(canvas, 112, 222, BOBBER_ICON_SHOP);
    drawBobberDockIcon(canvas, 112, 290, BOBBER_ICON_TRAVEL);
  } else if (bobberState == BOBBER_SHOP) {
    bobberShopButton(canvas, 95, 145, 276, "SELL FISH", bobberEconomy.count != 0);
    bobberShopButton(canvas, 95, 213, 276, "SELL ALL", bobberSaleQuote(-1) != 0);
    bobberShopButton(canvas, 95, 268, 276, "SUPPLIES");
    bobberShopButton(canvas, 95, 323, 276, "FISH TALE");
  } else if (bobberState == BOBBER_SUPPLIES) {
    snprintf(line, sizeof(line), "SEARCH BAIT   %d", BOBBER_BAIT_PRICE);
    bobberShopButton(canvas, 95, 134, 276, line, bobberCanBuySupply(0));
    canvas->setTextColor(0xF7B9);
    snprintf(line, sizeof(line), "%u / %d | Helps find missing fish", bobberEconomy.bait, BOBBER_BAIT_LIMIT);
    bobberShopText(canvas, notice ? notice : line, 184);
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
    bobberPanel(canvas, 78, 133, 310, 143);
    bobberShopText(canvas, bobberRegionNames[bobberRegion], 139);
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
    bobberPanel(canvas, 77, 133, 312, 139);
    if (bobberEconomyUnreadable) {
      bobberShopText(canvas, "Saved cooler unavailable.", 201, 2);
      bobberShopText(canvas, "Your saved data has been preserved.", 240);
    } else if (!bobberEconomy.count) {
      bobberShopText(canvas, "Your cooler is empty.", 201, 2);
      bobberShopText(canvas, "Regular catches are kept here.", 240);
    } else {
      const auto &fish = bobberEconomy.fish[bobberCoolerPage];
      snprintf(line, sizeof(line), "%d / %u", bobberCoolerPage + 1, bobberEconomy.count);
      bobberShopText(canvas, line, 139);
      drawBobberTrophy(canvas, (BobberSpecies)fish.species, 174, true);
      bobberShopText(canvas, bobberNames[fish.species], 205, 2);
      snprintf(line, sizeof(line), "%u lb %u oz | %lu coins", fish.ounces / 16, fish.ounces % 16,
               (unsigned long)bobberFishValue(fish));
      bobberShopText(canvas, line, 236);
      bobberShopText(canvas, fish.reserved ? "Reserved for later. Safe from selling." : "Available to sell or release.", 255);
      bobberShopButton(canvas, 95, 280, 134, fish.reserved ? "UNRESERVE" : "RESERVE");
      bobberShopButton(canvas, 237, 280, 134, bobberState == BOBBER_COOLER ? "RELEASE" : "SELL", !fish.reserved);
    }
  } else if (bobberState == BOBBER_CONFIRM) {
    bobberPanel(canvas, 69, 148, 328, 150, BOBBER_BRASS);
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
  if (notice && bobberState != BOBBER_SUPPLIES)
    bobberShopText(canvas, notice, bobberState == BOBBER_SHOP ? 134 : 350);
  bobberPrimaryButton(canvas, bobberState == BOBBER_CONFIRM ? "CONFIRM" : "BACK");
  if (browsing) {
    bobberShopButton(canvas, 95, 376, 44, "<", bobberCoolerPage > 0);
    bobberShopButton(canvas, 327, 376, 44, ">", bobberCoolerPage + 1 < bobberEconomy.count);
  }
  canvas->setTextSize(1);
  canvas->flush();
  return true;
}
