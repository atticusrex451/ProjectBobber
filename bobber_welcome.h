#pragma once

#include "Arduino_GFX_Library.h"
#include <Preferences.h>
#include "bobber_species.h"

// First Bobber milestone: an original, asset-free screen for the round display.
// Set to 0 to run the upstream game again while fishing is under development.
#define BOBBER_WELCOME 1

enum BobberState { BOBBER_TITLE, BOBBER_JOURNAL, BOBBER_TRAVEL, BOBBER_READY, BOBBER_WAITING, BOBBER_BITE, BOBBER_CAUGHT };
static BobberState bobberState = BOBBER_TITLE;
static uint32_t bobberCastAt = 0;
static uint32_t bobberAnimationAt = 0;
static uint8_t bobberAnimationFrame = 0;
// Whole ounces avoid rounding when displaying pounds and ounces.
// The catch is session-only; the personal best is loaded from Bobber storage.
static uint16_t bobberCatchOz = 0;
static uint32_t bobberSessionCatches = 0;
static BobberSpecies bobberSpecies = BOBBER_FLOUNDER;
static BobberRegion bobberRegion = BOBBER_BAY;
static BobberRegion bobberSavedRegion = BOBBER_BAY;
static bool bobberLegendCast = false;
static int bobberJournalPage = 0;
static uint16_t bobberBests[BOBBER_SPECIES_COUNT] = {};
static bool bobberNewBest = false;
static uint16_t bobberSavedBests[BOBBER_SPECIES_COUNT] = {};

static int bobberDiscovered(BobberRegion region) {
  int count = 0;
  for (int i = 0; i < BOBBER_REGULAR_COUNT; ++i)
    if (bobberBests[bobberRosters[region][i]]) ++count;
  return count;
}

static bool bobberLegendAvailable(BobberRegion region) {
  return bobberDiscovered(region) == BOBBER_REGULAR_COUNT;
}

static bool bobberRegionUnlocked(BobberRegion region) {
  if (region == BOBBER_BAY) return true;
  if (region == BOBBER_LAKE) return bobberBests[BOBBER_OLD_MAN] != 0;
  return bobberBests[BOBBER_OLD_MAN] != 0 && bobberBests[BOBBER_LEVIATHAN] != 0;
}

static void loadBobberRecord() {
  bobberState = BOBBER_TITLE;
  bobberAnimationAt = 0;
  bobberAnimationFrame = 0;
  bobberRegion = bobberSavedRegion = BOBBER_BAY;
  bobberLegendCast = false;
  bobberSpecies = BOBBER_FLOUNDER;
  bobberJournalPage = 0;
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i)
    bobberBests[i] = bobberSavedBests[i] = 0;
  bobberCatchOz = 0;
  bobberSessionCatches = 0;
  bobberNewBest = false;
  Preferences prefs;
  if (!prefs.begin("bobber", true)) return;
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) {
    const uint16_t saved = prefs.getUShort(bobberRecordKeys[i], 0);
    if (saved >= bobberMinOz[i] && saved <= bobberMaxOz[i])
      bobberBests[i] = bobberSavedBests[i] = saved;
  }
  const uint8_t savedRegion = prefs.getUChar("bobberRegion", BOBBER_BAY);
  if (savedRegion < BOBBER_REGION_COUNT && bobberRegionUnlocked((BobberRegion)savedRegion))
    bobberRegion = bobberSavedRegion = (BobberRegion)savedRegion;
  bobberSpecies = bobberRosters[bobberRegion][0];
  prefs.end();
}

static void saveBobberRecord() {
  // Retry pending records for all species, without rewriting saved records.
  Preferences prefs;
  bool opened = false;
  for (int i = 0; i < BOBBER_SPECIES_COUNT; ++i) {
    if (bobberBests[i] <= bobberSavedBests[i]) continue;
    if (!opened) {
      if (!prefs.begin("bobber", false)) return;
      opened = true;
    }
    if (prefs.putUShort(bobberRecordKeys[i], bobberBests[i]) == sizeof(uint16_t))
      bobberSavedBests[i] = bobberBests[i];
  }
  if (bobberRegion != bobberSavedRegion) {
    if (!opened) {
      if (!prefs.begin("bobber", false)) return;
      opened = true;
    }
    if (prefs.putUChar("bobberRegion", (uint8_t)bobberRegion) == sizeof(uint8_t))
      bobberSavedRegion = bobberRegion;
  }
  if (opened) prefs.end();
}
static const int BOBBER_BUTTON_X = 143;
static const int BOBBER_BUTTON_Y = 376;
static const int BOBBER_BUTTON_W = 180;
static const int BOBBER_BUTTON_H = 48;

static bool bobberButtonContains(int16_t x, int16_t y) {
  return x >= BOBBER_BUTTON_X && x < BOBBER_BUTTON_X + BOBBER_BUTTON_W &&
         y >= BOBBER_BUTTON_Y && y < BOBBER_BUTTON_Y + BOBBER_BUTTON_H;
}

// Journal is available while ready, so browsing cannot interrupt a cast.
static int bobberTouchTarget(int16_t x, int16_t y) {
  if (bobberButtonContains(x, y)) return 1;
  if (bobberState == BOBBER_JOURNAL && y >= 376 && y < 420) {
    if (x >= 95 && x < 139 && bobberJournalPage > 0) return 3;
    if (x >= 327 && x < 371 && bobberJournalPage + 1 < BOBBER_JOURNAL_PAGES) return 4;
  }
  if (bobberState == BOBBER_READY && y >= 304 && y < 348) {
    if (x >= 95 && x < 229) return 2;
    if (x >= 237 && x < 371) return 5;
  }
  if (bobberState == BOBBER_TRAVEL && x >= 87 && x < 379) {
    for (int i = 0; i < BOBBER_REGION_COUNT; ++i)
      if (y >= 145 + i * 68 && y < 201 + i * 68 &&
          bobberRegionUnlocked((BobberRegion)i)) return 10 + i;
  }
  if (bobberState == BOBBER_JOURNAL && bobberJournalPage == 4 &&
      bobberLegendAvailable(bobberRegion) && x >= 123 && x < 343 &&
      y >= 304 && y < 348) return 20;
  return 0;
}

// Small original flatfish illustration, drawn with the same primitives as the UI.
static void drawBobberFlounder(Arduino_Canvas *canvas) {
  const uint16_t outline = 0x39E5;
  const uint16_t sand = 0xC58E;
  const uint16_t fin = 0x8BE9;
  const uint16_t light = 0xE6B3;

  // Fan-shaped tail sits behind the tapered body.
  for (int y = -23; y <= 23; ++y)
    canvas->drawLine(285, 230, 327, 230 + y, fin);
  canvas->drawLine(285, 230, 327, 207, outline);
  canvas->drawLine(285, 230, 327, 253, outline);
  canvas->drawLine(327, 207, 327, 253, outline);
  for (int y = -18; y <= 18; y += 9)
    canvas->drawLine(294, 230, 323, 230 + y, sand);

  // Broad dorsal and anal fins frame a flat, leaf-shaped body.
  for (int x = -72; x <= 72; ++x) {
    int halfHeight = 47 * (72 * 72 - x * x) / (72 * 72);
    int centerY = 230 + x / 12;
    canvas->drawFastVLine(225 + x, centerY - halfHeight,
                          2 * halfHeight + 1, outline);
    if (halfHeight > 3)
      canvas->drawFastVLine(225 + x, centerY - halfHeight + 2,
                            2 * halfHeight - 3, fin);
    if (x % 6 == 0 && halfHeight > 10) {
      canvas->drawLine(225 + x, centerY - halfHeight + 3,
                       225 + x - 4, centerY - halfHeight + 13, light);
      canvas->drawLine(225 + x, centerY + halfHeight - 3,
                       225 + x - 4, centerY + halfHeight - 13, light);
    }
  }
  for (int x = -70; x <= 70; ++x) {
    int halfHeight = 34 * (70 * 70 - x * x) / (70 * 70);
    int centerY = 230 + x / 12;
    canvas->drawFastVLine(225 + x, centerY - halfHeight,
                          2 * halfHeight + 1, sand);
  }

  // Fixed mottling: the fish does not change its markings on each redraw.
  const int16_t spots[][3] = {
    {211, 211, 4}, {236, 213, 5}, {258, 227, 4}, {232, 242, 5},
    {206, 243, 3}, {275, 239, 3}, {195, 225, 3}, {249, 250, 2}
  };
  for (const auto &spot : spots) {
    canvas->fillCircle(spot[0], spot[1], spot[2] + 1, light);
    canvas->fillCircle(spot[0], spot[1], spot[2], fin);
    canvas->fillCircle(spot[0], spot[1], 1, outline);
  }
  // Both eyes are visible on the upper side, with a small mouth and side fin.
  canvas->fillCircle(179, 218, 6, outline);
  canvas->fillCircle(190, 208, 6, outline);
  canvas->fillCircle(179, 218, 4, light);
  canvas->fillCircle(190, 208, 4, light);
  canvas->fillCircle(178, 218, 2, outline);
  canvas->fillCircle(189, 208, 2, outline);
  canvas->drawLine(157, 225, 169, 229, outline);
  canvas->drawLine(193, 232, 213, 237, outline);
  canvas->drawLine(193, 232, 207, 244, fin);
  canvas->drawLine(207, 244, 213, 237, outline);
}

// Original silver-bronze croaker profile, distinct from the flatfish.
static void drawBobberCroaker(Arduino_Canvas *canvas) {
  const uint16_t dark = 0x5288, silver = 0xCE57, bronze = 0xAC6E;
  canvas->fillTriangle(282, 231, 329, 203, 329, 259, bronze);
  canvas->fillTriangle(200, 206, 228, 178, 267, 211, bronze);
  canvas->fillTriangle(223, 250, 255, 274, 274, 247, bronze);
  for (int x = -76; x <= 76; ++x) {
    int h = 32 * (76 * 76 - x * x) / (76 * 76);
    canvas->drawFastVLine(224 + x, 232 - h, h * 2 + 1, dark);
    if (h > 2) canvas->drawFastVLine(224 + x, 234 - h, h * 2 - 3, silver);
  }
  for (int x = 204; x < 280; x += 12)
    canvas->drawLine(x, 216, x + 8, 240, bronze);
  canvas->drawLine(185, 214, 194, 232, dark);
  canvas->drawLine(194, 232, 185, 249, dark);
  canvas->fillTriangle(203, 235, 225, 240, 210, 251, bronze);
  canvas->fillCircle(171, 224, 5, dark);
  canvas->fillCircle(170, 223, 2, 0xF7B9);
  canvas->drawLine(148, 234, 161, 237, dark);
}

// Original Spot profile: yellow fins, silver body and its dark shoulder spot.
static void drawBobberSpot(Arduino_Canvas *canvas) {
  const uint16_t silver = 0xD6B8, gold = 0xCDA8, dark = 0x4208;
  canvas->fillTriangle(278, 232, 320, 209, 320, 255, gold);
  canvas->fillTriangle(208, 213, 229, 187, 267, 216, gold);
  canvas->fillTriangle(218, 250, 244, 271, 266, 247, gold);
  for (int x = -65; x <= 65; ++x) {
    const int h = 30 * (65 * 65 - x * x) / (65 * 65);
    canvas->drawFastVLine(224 + x, 232 - h, h * 2 + 1, dark);
    if (h > 2) canvas->drawFastVLine(224 + x, 234 - h, h * 2 - 3, silver);
  }
  for (int x = 229; x <= 267; x += 10)
    canvas->drawLine(x, 216, x + 8, 244, gold);
  canvas->drawLine(191, 215, 198, 232, dark);
  canvas->drawLine(198, 232, 191, 249, dark);
  canvas->fillCircle(210, 220, 7, dark);
  canvas->fillCircle(177, 224, 4, dark);
  canvas->fillCircle(176, 223, 1, silver);
}

// Original bay profiles, shared by catch reveals and miniature trophies.
// Coordinates scale together so undiscovered silhouettes retain each fish's shape.
static void drawBobberBayFish(Arduino_Canvas *canvas, BobberSpecies species,
                               int centerY, int divisor, bool caught) {
  const uint16_t shadow = 0x4A69;
  const uint16_t dark = caught ? 0x2945 : shadow;
  uint16_t body = 0xCE98, fin = 0x9C6D;
  int height = 29;
  if (species == BOBBER_WEAKFISH) { body = 0xB5B6; fin = 0xDDA9; }
  if (species == BOBBER_STRIPED_BASS) { body = 0xC638; fin = 0x736D; height = 33; }
  if (species == BOBBER_SPECKLED_TROUT) { body = 0x9DB2; fin = 0xB56C; }
  if (species == BOBBER_BLACK_DRUM) { body = 0x9491; fin = 0x630B; height = 39; }
  if (species == BOBBER_COBIA) { body = 0x8C6B; fin = 0x5288; height = 22; }
  if (!caught) body = fin = shadow;
  auto px = [&](int x) { return 233 + x / divisor; };
  auto py = [&](int y) { return centerY + y / divisor; };
  auto line = [&](int x1, int y1, int x2, int y2, uint16_t color) {
    canvas->drawLine(px(x1), py(y1), px(x2), py(y2), color);
  };
  auto triangle = [&](int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) {
    canvas->fillTriangle(px(x1), py(y1), px(x2), py(y2), px(x3), py(y3), color);
  };
  // Forked tail and two dorsal sections; cobia has a low, swept profile.
  triangle(65, 0, 102, -28, 94, 0, fin);
  triangle(65, 0, 94, 0, 102, 28, fin);
  triangle(-32, -height + 5, -16, -height - 20, 9, -height + 3, fin);
  triangle(6, -height + 3, 28, -height - 10, 62, -9, fin);
  triangle(0, height - 4, 29, height + 15, 53, 10, fin);
  for (int x = -82; x <= 76; ++x) {
    const int h = height * (80 * 80 - x * x) / (80 * 80);
    if (h < 0) continue;
    line(x, -h, x, h, dark);
    if (h > 2) line(x, -h + 2, x, h - 2, body);
  }
  if (!caught) return;
  if (species == BOBBER_STRIPED_BASS) {
    for (int y = -18; y <= 18; y += 6) line(-42, y, 57, y, dark);
  } else if (species == BOBBER_BLACK_DRUM) {
    // Broad juvenile-style bars and a small chin barbel.
    for (int x = -35; x <= 45; x += 20)
      for (int w = 0; w < 6; ++w) line(x + w, -27, x + w, 27, fin);
    line(-68, 17, -63, 29, dark);
  } else if (species == BOBBER_COBIA) {
    for (int y = -6; y <= 4; ++y) line(-60, y, 67, y, dark);
    line(-48, 9, 54, 9, 0xE6F8);
  } else {
    // Weakfish: small oblique flecks; seatrout: bold spots extending onto fins.
    for (int x = -28; x <= 48; x += 15) {
      for (int y = -12; y <= 12; y += 12) {
        if (species == BOBBER_WEAKFISH) line(x, y, x + 3, y - 3, fin);
        else canvas->fillCircle(px(x), py(y), divisor == 1 ? 3 : 1, dark);
      }
    }
    if (species == BOBBER_SPECKLED_TROUT) {
      canvas->fillCircle(px(88), py(-15), divisor == 1 ? 3 : 1, dark);
      canvas->fillCircle(px(29), py(-32), divisor == 1 ? 3 : 1, dark);
    }
  }
  line(-49, -18, -40, 0, dark);
  line(-40, 0, -49, 18, dark);
  triangle(-30, 4, -7, 11, -24, 23, fin);
  canvas->fillCircle(px(-63), py(-9), divisor == 1 ? 5 : 2, dark);
  canvas->fillCircle(px(-64), py(-10), 1, 0xF7B9);
  line(-79, 5, -65, 8, dark);
}

#include "bobber_fish_art.h"

// Small trophy portraits: identical outlines stay recognizable before discovery.
static void drawBobberTrophy(Arduino_Canvas *canvas, BobberSpecies species,
                              int centerY, bool caught) {
  if (species >= BOBBER_YELLOW_PERCH) {
    drawBobberOtherFish(canvas, species, centerY, 2, caught);
    return;
  }
  if (species >= BOBBER_WEAKFISH) {
    drawBobberBayFish(canvas, species, centerY, 2, caught);
    return;
  }
  const uint16_t silhouette = 0x4A69;
  const uint16_t edge = caught ? 0x39E5 : silhouette;
  const uint16_t body = caught ? (species == BOBBER_FLOUNDER ? 0xC58E : 0xCE57)
                               : silhouette;
  const uint16_t fin = caught ? (species == BOBBER_FLOUNDER ? 0x8BE9 : 0xAC6E)
                              : silhouette;
  canvas->fillTriangle(268, centerY, 292, centerY - 16, 292, centerY + 16, fin);
  if (species != BOBBER_FLOUNDER) {
    canvas->fillTriangle(216, centerY - 10, 232, centerY - 28, 254, centerY - 9, fin);
    canvas->fillTriangle(225, centerY + 10, 245, centerY + 25, 258, centerY + 9, fin);
  }
  const int height = species == BOBBER_FLOUNDER ? 23 : 17;
  for (int x = -48; x <= 48; ++x) {
    const int h = height * (48 * 48 - x * x) / (48 * 48);
    canvas->drawFastVLine(224 + x, centerY - h, h * 2 + 1, edge);
    if (h > 2) canvas->drawFastVLine(224 + x, centerY - h + 2, h * 2 - 3, body);
  }
  if (!caught) return; // No eye, gill or markings until this species is caught.
  canvas->fillCircle(190, centerY - 5, 3, edge);
  if (species == BOBBER_FLOUNDER) {
    canvas->fillCircle(197, centerY - 12, 3, edge);
    const int spots[][2] = {{214, -9}, {232, 7}, {248, -5}, {216, 11}};
    for (const auto &spot : spots)
      canvas->fillCircle(spot[0], centerY + spot[1], 3, fin);
  } else {
    canvas->drawLine(201, centerY - 10, 207, centerY, edge);
    canvas->drawLine(207, centerY, 201, centerY + 10, edge);
    for (int x = 218; x <= 250; x += 8)
      canvas->drawLine(x, centerY - 9, x + 4, centerY + 7, fin);
    if (species == BOBBER_SPOT) canvas->fillCircle(214, centerY - 6, 5, edge);
  }
}

// Original pixel scene, animated in small steps without blocking touch.
static void drawBobberScene(Arduino_Canvas *canvas) {
  const uint16_t sky = bobberRegion == BOBBER_GROTTO ? 0x318B :
                       bobberRegion == BOBBER_LAKE ? 0xADDA : 0x8D79;
  const uint16_t cloud = bobberRegion == BOBBER_GROTTO ? 0x9E37 : 0xDF5B;
  const uint16_t water = bobberRegion == BOBBER_GROTTO ? 0x19EB :
                         bobberRegion == BOBBER_LAKE ? 0x3BB3 : 0x3B50;
  const uint16_t marsh = 0x5B88, reeds = 0x8C6A, dark = 0x1968;
  const uint16_t wood = 0xA3E9, rim = 0xDE31, skin = 0xE5D1;
  canvas->fillRect(73, 145, 320, 150, sky);
  if (bobberRegion == BOBBER_BAY) {
  // Squared cloud clusters, a distant marsh bank and calm bay water.
  canvas->fillRect(109, 162, 42, 6, cloud);
  canvas->fillRect(121, 156, 24, 6, cloud);
  canvas->fillRect(295, 171, 55, 6, cloud);
  canvas->fillRect(307, 165, 25, 6, cloud);
  canvas->fillRect(73, 202, 320, 93, water);
  canvas->fillRect(73, 196, 93, 12, marsh);
  canvas->fillRect(73, 190, 44, 6, marsh);
  canvas->fillRect(326, 193, 67, 15, marsh);
  for (int x = 79; x < 156; x += 12) {
    canvas->fillRect(x, 190 - x % 3 * 3, 3, 12, reeds);
  }
  for (int x = 337; x < 390; x += 12)
    canvas->fillRect(x, 187, 3, 12, reeds);
  } else if (bobberRegion == BOBBER_LAKE) {
    // Cold peaks, snow caps, mist and evergreen banks.
    canvas->fillTriangle(73, 208, 139, 151, 214, 208, 0x6C53);
    canvas->fillTriangle(176, 208, 269, 146, 365, 208, 0x8497);
    canvas->fillTriangle(123, 165, 139, 151, 155, 165, cloud);
    canvas->fillTriangle(248, 160, 269, 146, 291, 160, cloud);
    canvas->fillRect(73, 202, 320, 93, water);
    canvas->fillRect(81, 191, 93, 3, cloud);
    canvas->fillRect(286, 185, 98, 3, cloud);
    for (int x = 77; x < 143; x += 16) {
      canvas->fillRect(x + 6, 184, 4, 25, wood);
      canvas->fillTriangle(x, 200, x + 8, 169 + x % 7, x + 16, 200, 0x2308);
    }
    for (int x = 337; x < 379; x += 16)
      canvas->fillTriangle(x, 210, x + 8, 176, x + 16, 210, 0x2308);
  } else {
    // Thurl's salt-ivy arch, mixed tidal pools and a hint of old treasure.
    canvas->fillRect(73, 202, 320, 93, water);
    canvas->fillRect(73, 145, 320, 12, 0x2128);
    canvas->fillRect(73, 157, 34, 79, 0x420C);
    canvas->fillRect(359, 157, 34, 79, 0x420C);
    for (int x = 105; x < 359; x += 24) {
      canvas->fillRect(x, 157, 20, 8 + (x % 5) * 3, 0x420C);
      canvas->fillRect(x + 3, 157, 3, 18 + (x % 4) * 5, 0x43AD);
    }
    canvas->fillRect(78, 227, 45, 5, 0xAC6B);
    canvas->fillRect(336, 256, 48, 5, 0x3D33);
    canvas->fillRoundRect(344, 211, 31, 20, 3, wood);
    canvas->drawRect(344, 219, 31, 12, 0xDDA9);
    canvas->fillRect(357, 218, 5, 7, 0xFEC0);
    canvas->fillCircle(335, 233, 3, 0xFEC0);
    canvas->fillCircle(378, 237, 2, 0xFEC0);
  }
  const int ripples[][3] = {
    {94, 225, 31}, {281, 218, 34}, {340, 243, 28},
    {104, 272, 41}, {259, 280, 38}, {192, 290, 24}
  };
  const int drift[] = {0, 1, 2, 3, 4, 3, 2, 1};
  const int bob[] = {0, 1, 2, 1, 0, -1, -2, -1};
  const int phase = bobberAnimationFrame % 8;
  for (const auto &r : ripples)
    canvas->fillRect(r[0] + drift[phase], r[1], r[2], 2, sky);
  // Keep the bite marker steady and visible; the float rests lower on a bite.
  const int floatY = bobberState == BOBBER_BITE ? 3 : bob[phase];
  // Boat shadow, wooden hull, and pale gunwale.
  canvas->fillRect(153, 264, 102, 4, dark);
  canvas->fillRect(145, 240, 122, 8, wood);
  canvas->fillRect(153, 248, 106, 8, wood);
  canvas->fillRect(161, 256, 90, 4, dark);
  canvas->fillRect(141, 237, 130, 4, rim);
  // Seated fisherman facing right, wearing a brimmed hat.
  canvas->fillRect(190, 211, 18, 22, dark);
  canvas->fillRect(199, 228, 24, 7, dark);
  canvas->fillRect(216, 231, 7, 6, dark);
  canvas->fillRect(192, 196, 16, 15, skin);
  canvas->fillRect(187, 192, 28, 5, rim);
  canvas->fillRect(192, 185, 17, 8, wood);
  canvas->fillRect(205, 201, 5, 5, skin);
  canvas->fillRect(204, 215, 18, 5, skin);
  canvas->fillRect(219, 211, 6, 9, skin);
  // Rod and line lead the eye toward the little two-tone float.
  canvas->drawLine(222, 214, 268, 178, dark);
  canvas->drawLine(222, 215, 268, 179, dark);
  canvas->drawLine(268, 178, 306, 244 + floatY, cloud);
  canvas->fillRect(303, 239 + floatY, 7, 6, 0xBFE0);
  canvas->fillRect(303, 245 + floatY, 7, 5, 0xFB00);
  canvas->fillRect(297, 252 + floatY, 19, 2, sky);
  if (bobberState == BOBBER_BITE) {
    canvas->fillRect(305, 220, 3, 10, cloud);
    canvas->drawLine(294, 232, 289, 226, cloud);
    canvas->drawLine(317, 232, 322, 226, cloud);
  }
}

static void renderBobberWelcome(Arduino_Canvas *canvas) {
  const uint16_t navy = 0x08E4;
  const uint16_t cream = 0xF7B9;
  const uint16_t lime = 0xBFE0;
  const uint16_t orange = 0xFB00;
  const uint16_t water = 0x2B8E;

  canvas->fillScreen(navy);
  canvas->setTextColor(cream);
  if (bobberState == BOBBER_TRAVEL) {
    canvas->setTextSize(3);
    canvas->setCursor(152, 72);
    canvas->print("SET SAIL");
    canvas->setTextSize(1);
    canvas->setCursor(158, 110);
    canvas->print("A new shore. A new story.");
    for (int i = 0; i < BOBBER_REGION_COUNT; ++i) {
      const bool unlocked = bobberRegionUnlocked((BobberRegion)i);
      const int y = 145 + i * 68;
      canvas->fillRoundRect(87, y, 292, 56, 10,
                            i == bobberRegion ? 0x336D : unlocked ? water : 0x1946);
      canvas->setTextColor(unlocked ? cream : 0x94B2);
      canvas->setTextSize(2);
      canvas->setCursor(233 - strlen(bobberRegionNames[i]) * 6, y + 8);
      canvas->print(bobberRegionNames[i]);
      canvas->setTextSize(1);
      char detail[48];
      if (unlocked) snprintf(detail, sizeof(detail), "%s | %d / 8 discovered",
                              i == bobberRegion ? "Here" : "Visit", bobberDiscovered((BobberRegion)i));
      else snprintf(detail, sizeof(detail), "%s", i == BOBBER_LAKE ?
                    "Land the Old Man to unlock" : "Land both main-area legends");
      canvas->setCursor(233 - strlen(detail) * 3, y + 34);
      canvas->print(detail);
    }
    canvas->fillRoundRect(143, 376, 180, 48, 16, lime);
    canvas->setTextColor(navy);
    canvas->setTextSize(3);
    canvas->setCursor(197, 389);
    canvas->print("BACK");
    canvas->setTextSize(1);
    canvas->flush();
    return;
  }
  if (bobberState == BOBBER_JOURNAL) {
    canvas->setTextSize(3);
    canvas->setCursor(125, 60);
    canvas->print("TROPHY WALL");
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(bobberRegionNames[bobberRegion]) * 3, 96);
    canvas->print(bobberRegionNames[bobberRegion]);
    char pageLabel[24];
    snprintf(pageLabel, sizeof(pageLabel), "%d / %d", bobberJournalPage + 1,
             BOBBER_JOURNAL_PAGES);
    canvas->setCursor(233 - strlen(pageLabel) * 3, 107);
    canvas->print(pageLabel);
    if (bobberJournalPage == 4) {
      const BobberSpecies legend = bobberLegends[bobberRegion];
      drawBobberOtherFish(canvas, legend, 192, 1, bobberBests[legend] != 0);
      canvas->setTextSize(2);
      canvas->setCursor(233 - strlen(bobberNames[legend]) * 6, 251);
      canvas->print(bobberNames[legend]);
      char detail[64];
      if (bobberBests[legend]) snprintf(detail, sizeof(detail), "Best: %u lb %u oz",
                                      bobberBests[legend] / 16, bobberBests[legend] % 16);
      else snprintf(detail, sizeof(detail), "%d / 8 local fish discovered", bobberDiscovered(bobberRegion));
      canvas->setTextSize(1);
      canvas->setCursor(233 - strlen(detail) * 3, 279);
      canvas->print(detail);
      if (bobberLegendAvailable(bobberRegion)) {
        canvas->fillRoundRect(123, 304, 220, 44, 12, water);
        canvas->setTextSize(2);
        canvas->setCursor(167, 318);
        canvas->print("SEEK LEGEND");
      } else {
        canvas->setCursor(146, 317);
        canvas->print("Discover all eight to seek");
        canvas->setCursor(173, 333);
        canvas->print("the local legend.");
      }
      if (bobberBests[legend] > bobberSavedBests[legend]) {
        canvas->setTextSize(1);
        canvas->setCursor(185, 354);
        canvas->print("Record not saved");
      }
    }
    for (int slot = 0; bobberJournalPage < 4 && slot < 2; ++slot) {
      const int rosterIndex = bobberJournalPage * 2 + slot;
      if (rosterIndex >= BOBBER_REGULAR_COUNT) break;
      const int i = bobberRosters[bobberRegion][rosterIndex];
      const int y = 178 + slot * 128;
      drawBobberTrophy(canvas, (BobberSpecies)i, y - 34, bobberBests[i] != 0);
      canvas->setTextSize(2);
      canvas->setCursor(233 - strlen(bobberNames[i]) * 6, y);
      canvas->print(bobberNames[i]);
      char record[64];
      if (bobberBests[i])
        snprintf(record, sizeof(record), "Best: %u lb %u oz", bobberBests[i] / 16,
                 bobberBests[i] % 16);
      else snprintf(record, sizeof(record), "Not caught yet");
      canvas->setTextSize(2);
      canvas->setCursor(233 - strlen(record) * 6, y + 24);
      canvas->print(record);
      if (bobberBests[i] > bobberSavedBests[i]) {
        canvas->setTextSize(1);
        canvas->setCursor(188, y + 44);
        canvas->print("Record not saved");
      }
    }
    canvas->fillRoundRect(BOBBER_BUTTON_X, BOBBER_BUTTON_Y,
                          BOBBER_BUTTON_W, BOBBER_BUTTON_H, 16, lime);
    canvas->setTextColor(navy);
    canvas->setTextSize(3);
    canvas->setCursor(197, 389);
    canvas->print("BACK");
    for (int direction = 0; direction < 2; ++direction) {
      const bool enabled = direction == 0 ? bobberJournalPage > 0 :
                           bobberJournalPage + 1 < BOBBER_JOURNAL_PAGES;
      const int x = direction == 0 ? 95 : 327;
      canvas->fillRoundRect(x, 376, 44, 44, 10, enabled ? water : 0x1946);
      canvas->setTextColor(enabled ? cream : 0x4A69);
      canvas->setTextSize(3);
      canvas->setCursor(x + 13, 388);
      canvas->print(direction == 0 ? "<" : ">");
    }
    canvas->setTextSize(1);
    canvas->flush();
    return;
  }
  if (bobberState == BOBBER_TITLE) {
    canvas->setTextSize(2);
    canvas->setCursor(191, 74);
    canvas->print("PROJECT");
    canvas->setTextSize(5);
    canvas->setCursor(143, 103);
    canvas->print("BOBBER");
  } else {
    canvas->setTextSize(2);
    canvas->setCursor(233 - strlen(bobberRegionTitles[bobberRegion]) * 6, 82);
    canvas->print(bobberRegionTitles[bobberRegion]);
    char catches[40];
    snprintf(catches, sizeof(catches), "Fish caught: %lu",
             (unsigned long)bobberSessionCatches);
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(catches) * 3, 112);
    canvas->print(catches);
  }

  if (bobberRegion != bobberSavedRegion) {
    canvas->setTextSize(1);
    canvas->setCursor(155, 128);
    canvas->print("Travel not saved; retry");
  }
  // Water ripples and a two-tone float, kept inside the circular bezel.
  canvas->drawFastHLine(119, 283, 228, water);
  canvas->drawFastHLine(143, 294, 180, water);
  if (bobberState == BOBBER_CAUGHT) {
    if (bobberSpecies >= BOBBER_YELLOW_PERCH) drawBobberOtherFish(canvas, bobberSpecies, 232, 1, true);
    else if (bobberSpecies >= BOBBER_WEAKFISH) drawBobberBayFish(canvas, bobberSpecies, 232, 1, true);
    else if (bobberSpecies == BOBBER_SPOT) drawBobberSpot(canvas);
    else if (bobberSpecies == BOBBER_CROAKER) drawBobberCroaker(canvas);
    else drawBobberFlounder(canvas);
  } else if (bobberState != BOBBER_TITLE) {
    drawBobberScene(canvas);
  } else {
  canvas->fillRoundRect(225, 169, 16, 31, 7, lime);
  canvas->fillCircle(233, 238, 52, lime);
  for (int y = 0; y <= 52; ++y) {
    for (int x = -52; x <= 52; ++x) {
      if (x * x + y * y <= 52 * 52)
        canvas->fillRect(233 + x, 238 + y, 1, 1, orange);
    }
  }
  canvas->fillRect(182, 235, 103, 5, cream);
  canvas->fillCircle(215, 211, 7, cream);
  if (bobberState == BOBBER_BITE) {
    canvas->drawLine(160, 209, 145, 195, cream);
    canvas->drawLine(306, 209, 321, 195, cream);
    canvas->drawLine(233, 158, 233, 145, cream);
  }
  }

  canvas->setTextColor(cream);
  canvas->setTextSize(2);
  if (bobberState == BOBBER_WAITING) {
    canvas->setCursor(107, 321);
    canvas->print(bobberLegendCast ? "Seeking the legend..." : "Waiting for a bite...");
    canvas->setTextSize(1);
    canvas->setCursor(188, 347);
    canvas->print("Enjoy the quiet.");
  } else if (bobberState == BOBBER_BITE) {
    canvas->setCursor(125, 321);
    canvas->print(bobberLegendCast ? "Legend! Tap to hook!" : "Bite! Tap to hook!");
    canvas->setTextSize(1);
    canvas->setCursor(176, 347);
    canvas->print("Take your time, Dad.");
  } else if (bobberState == BOBBER_CAUGHT) {
    canvas->setCursor(233 - strlen(bobberNames[bobberSpecies]) * 6, 321);
    canvas->print(bobberNames[bobberSpecies]);
    char weight[40];
    snprintf(weight, sizeof(weight), "%u lb %u oz", bobberCatchOz / 16,
             bobberCatchOz % 16);
    canvas->setTextSize(2);
    canvas->setCursor(233 - strlen(weight) * 6, 343);
    canvas->print(weight);
    snprintf(weight, sizeof(weight), "%s: %u lb %u oz",
             bobberNewBest ? "New personal best" : "Personal best",
             bobberBests[bobberSpecies] / 16, bobberBests[bobberSpecies] % 16);
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(weight) * 3, 365);
    canvas->print(weight);
    if (bobberBests[bobberSpecies] > bobberSavedBests[bobberSpecies]) {
      canvas->setCursor(155, 301);
      canvas->print("Record not saved; try again");
    } else {
      const char *notice = nullptr;
      if (bobberSpecies == BOBBER_OLD_MAN) notice = "Blue Mountain Lake unlocked!";
      else if (bobberSpecies == BOBBER_LEVIATHAN) notice = "Thurl's Grotto unlocked!";
      else if (bobberSpecies == BOBBER_IRON_MAW) notice = "Thurl's legend is yours.";
      else if (bobberLegendAvailable(bobberRegion) && !bobberBests[bobberLegends[bobberRegion]])
        notice = "All eight found! Seek the legend.";
      if (notice) {
        canvas->setCursor(233 - strlen(notice) * 3, 301);
        canvas->print(notice);
      }
    }
  } else if (bobberState == BOBBER_TITLE) {
    canvas->setCursor(131, 316);
    canvas->print("A little fishing.");
    canvas->setCursor(125, 340);
    canvas->print("A lot of relaxing.");
  } else {
    canvas->fillRoundRect(95, 304, 134, 44, 12, water);
    canvas->fillRoundRect(237, 304, 134, 44, 12, water);
    canvas->setTextSize(2);
    canvas->setCursor(120, 318);
    canvas->print("JOURNAL");
    canvas->setCursor(268, 318);
    canvas->print("TRAVEL");
    char record[64];
    if (bobberBests[bobberSpecies]) {
      snprintf(record, sizeof(record), "%s best: %u lb %u oz",
               bobberShortNames[bobberSpecies], bobberBests[bobberSpecies] / 16, bobberBests[bobberSpecies] % 16);
    } else {
      snprintf(record, sizeof(record), "Your first catch awaits.");
    }
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(record) * 3, 355);
    canvas->print(record);
  }
  canvas->fillRoundRect(BOBBER_BUTTON_X, BOBBER_BUTTON_Y,
                        BOBBER_BUTTON_W, BOBBER_BUTTON_H, 16, lime);
  canvas->setTextColor(navy);
  canvas->setTextSize(3);
  canvas->setCursor(197, 389);
  canvas->print(bobberState == BOBBER_TITLE ? "PLAY" :
                bobberState == BOBBER_WAITING ? "BACK" :
                bobberState == BOBBER_BITE ? "HOOK" : "CAST");
  canvas->setTextSize(1);
  canvas->flush();
}

static void updateBobberFishing(Arduino_Canvas *canvas, uint32_t now) {
  // A fixed, short wait makes this first interaction easy to try repeatedly.
  // The bite stays available: this relaxed prototype has no reaction deadline.
  if (bobberState == BOBBER_WAITING && now - bobberCastAt >= 3000) {
    bobberState = BOBBER_BITE;
    renderBobberWelcome(canvas);
    bobberAnimationAt = now;
    return;
  }
  if ((bobberState == BOBBER_READY || bobberState == BOBBER_WAITING ||
       bobberState == BOBBER_BITE) && now - bobberAnimationAt >= 150) {
    bobberAnimationAt = now;
    bobberAnimationFrame = (bobberAnimationFrame + 1) % 8;
    // Redraw only the scene rectangle, leaving labels and touch targets intact.
    drawBobberScene(canvas);
    canvas->flush();
  }
}

// Activate on release; dragging outside the button cancels the tap.
static void updateBobberTouch(Arduino_Canvas *canvas, bool pressed,
                              int16_t x, int16_t y) {
  static bool wasDown = false;
  static bool armed = false;
  static BobberState pressedState = BOBBER_READY;
  static int pressedTarget = 0;
  if (pressed) {
    if (!wasDown) {
      pressedTarget = bobberTouchTarget(x, y);
      armed = pressedTarget != 0;
      pressedState = bobberState;
    }
    else if (bobberTouchTarget(x, y) != pressedTarget) armed = false;
  } else if (wasDown) {
    if (armed && pressedState == bobberState && bobberTouchTarget(x, y) == pressedTarget) {
      if (pressedTarget == 3) --bobberJournalPage;
      else if (pressedTarget == 4) ++bobberJournalPage;
      else if (pressedTarget == 5) bobberState = BOBBER_TRAVEL;
      else if (pressedTarget >= 10 && pressedTarget < 10 + BOBBER_REGION_COUNT) {
        const BobberRegion destination = (BobberRegion)(pressedTarget - 10);
        if (!bobberRegionUnlocked(destination)) { armed = false; wasDown = pressed; return; }
        bobberRegion = destination;
        bobberSpecies = bobberRosters[bobberRegion][0];
        bobberCatchOz = 0;
        bobberNewBest = false;
        bobberLegendCast = false;
        bobberJournalPage = 0;
        bobberAnimationFrame = 0;
        bobberAnimationAt = millis();
        saveBobberRecord();
        bobberState = BOBBER_READY;
      }
      else if (pressedTarget == 20) {
        if (!bobberLegendAvailable(bobberRegion)) { armed = false; wasDown = pressed; return; }
        bobberLegendCast = true;
        bobberCastAt = millis();
        bobberState = BOBBER_WAITING;
      }
      else if (pressedTarget == 2) {
        bobberJournalPage = 0;
        bobberState = BOBBER_JOURNAL;
      }
      else if (bobberState == BOBBER_JOURNAL || bobberState == BOBBER_TRAVEL) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_TITLE) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_WAITING) { bobberState = BOBBER_READY; bobberLegendCast = false; }
      else if (bobberState == BOBBER_BITE) {
        // Roll only on a successful hook, never on redraw or a cancelled cast.
        if (bobberSessionCatches < UINT32_MAX) ++bobberSessionCatches;
        // Legends are explicitly sought after discovering all local species.
        // Ordinary casts use only the current area's eight regular fish.
        bobberSpecies = bobberLegendCast ? bobberLegends[bobberRegion] :
                        bobberRosters[bobberRegion][random(BOBBER_REGULAR_COUNT)];
        bobberCatchOz = (uint16_t)random(bobberMinOz[bobberSpecies],
                                       bobberMaxOz[bobberSpecies] + 1);
        bobberNewBest = bobberCatchOz > bobberBests[bobberSpecies];
        if (bobberNewBest) bobberBests[bobberSpecies] = bobberCatchOz;
        // If storage failed, retry the outstanding record on the next catch.
        saveBobberRecord();
        bobberState = BOBBER_CAUGHT;
      }
      else {
        bobberLegendCast = false;
        bobberState = BOBBER_WAITING;
        bobberCastAt = millis();
      }
      renderBobberWelcome(canvas);
    }
    armed = false;
  }
  wasDown = pressed;
}
