#pragma once

#include "Arduino_GFX_Library.h"
#include <Preferences.h>

// First Bobber milestone: an original, asset-free screen for the round display.
// Set to 0 to run the upstream game again while fishing is under development.
#define BOBBER_WELCOME 1

enum BobberState { BOBBER_TITLE, BOBBER_JOURNAL, BOBBER_READY, BOBBER_WAITING, BOBBER_BITE, BOBBER_CAUGHT };
static BobberState bobberState = BOBBER_TITLE;
static uint32_t bobberCastAt = 0;
static uint32_t bobberAnimationAt = 0;
static uint8_t bobberAnimationFrame = 0;
// Whole ounces avoid rounding when displaying pounds and ounces.
// The catch is session-only; the personal best is loaded from Bobber storage.
static uint16_t bobberCatchOz = 0;
static uint32_t bobberSessionCatches = 0;
enum BobberSpecies { BOBBER_FLOUNDER, BOBBER_CROAKER, BOBBER_SPOT, BOBBER_SPECIES_COUNT };
static BobberSpecies bobberSpecies = BOBBER_FLOUNDER;
// Current region is Chincoteague; extend its roster when adding areas.
static const BobberSpecies bobberBayRoster[] = {BOBBER_FLOUNDER, BOBBER_CROAKER, BOBBER_SPOT};
static const int BOBBER_BAY_COUNT = sizeof(bobberBayRoster) / sizeof(bobberBayRoster[0]);
static const int BOBBER_JOURNAL_PAGES = (BOBBER_BAY_COUNT + 1) / 2;
static int bobberJournalPage = 0;
static const char *const bobberNames[] = {"Summer Flounder", "Atlantic Croaker", "Spot"};
// Keep the original flounder key so existing personal bests survive this update.
static const char *const bobberRecordKeys[] = {"bobberBestOz", "croakerBestOz", "spotBestOz"};
static const uint16_t bobberMinOz[] = {8, 4, 2};
static const uint16_t bobberMaxOz[] = {128, 48, 16};
static uint16_t bobberBests[BOBBER_SPECIES_COUNT] = {};
static bool bobberNewBest = false;
static uint16_t bobberSavedBests[BOBBER_SPECIES_COUNT] = {};

static void loadBobberRecord() {
  bobberState = BOBBER_TITLE;
  bobberAnimationAt = 0;
  bobberAnimationFrame = 0;
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
  prefs.end();
}

static void saveBobberRecord() {
  // Retry pending records for either species, without rewriting saved records.
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
  if (bobberState == BOBBER_READY && x >= 143 && x < 323 &&
      y >= 304 && y < 344) return 2;
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

// Small trophy portraits: identical outlines stay recognizable before discovery.
static void drawBobberTrophy(Arduino_Canvas *canvas, BobberSpecies species,
                              int centerY, bool caught) {
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
static void drawBobberBay(Arduino_Canvas *canvas) {
  const uint16_t sky = 0x8D79, cloud = 0xDF5B, water = 0x3B50;
  const uint16_t marsh = 0x5B88, reeds = 0x8C6A, dark = 0x1968;
  const uint16_t wood = 0xA3E9, rim = 0xDE31, skin = 0xE5D1;
  canvas->fillRect(73, 145, 320, 150, sky);
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
  if (bobberState == BOBBER_JOURNAL) {
    canvas->setTextSize(3);
    canvas->setCursor(125, 60);
    canvas->print("TROPHY WALL");
    canvas->setTextSize(1);
    canvas->setCursor(185, 96);
    canvas->print("Chincoteague Bay");
    char pageLabel[24];
    snprintf(pageLabel, sizeof(pageLabel), "%d / %d", bobberJournalPage + 1,
             BOBBER_JOURNAL_PAGES);
    canvas->setCursor(233 - strlen(pageLabel) * 3, 107);
    canvas->print(pageLabel);
    for (int slot = 0; slot < 2; ++slot) {
      const int rosterIndex = bobberJournalPage * 2 + slot;
      if (rosterIndex >= BOBBER_BAY_COUNT) break;
      const int i = bobberBayRoster[rosterIndex];
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
    canvas->setCursor(143, 82);
    canvas->print("CHINCOTEAGUE BAY");
    char catches[40];
    snprintf(catches, sizeof(catches), "Fish caught: %lu",
             (unsigned long)bobberSessionCatches);
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(catches) * 3, 112);
    canvas->print(catches);
  }

  // Water ripples and a two-tone float, kept inside the circular bezel.
  canvas->drawFastHLine(119, 283, 228, water);
  canvas->drawFastHLine(143, 294, 180, water);
  if (bobberState == BOBBER_CAUGHT) {
    if (bobberSpecies == BOBBER_SPOT) drawBobberSpot(canvas);
    else if (bobberSpecies == BOBBER_CROAKER) drawBobberCroaker(canvas);
    else drawBobberFlounder(canvas);
  } else if (bobberState != BOBBER_TITLE) {
    drawBobberBay(canvas);
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
    canvas->print("Waiting for a bite...");
    canvas->setTextSize(1);
    canvas->setCursor(188, 347);
    canvas->print("Enjoy the quiet.");
  } else if (bobberState == BOBBER_BITE) {
    canvas->setCursor(125, 321);
    canvas->print("Bite! Tap to hook!");
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
    }
  } else if (bobberState == BOBBER_TITLE) {
    canvas->setCursor(131, 316);
    canvas->print("A little fishing.");
    canvas->setCursor(125, 340);
    canvas->print("A lot of relaxing.");
  } else {
    canvas->fillRoundRect(143, 304, 180, 40, 12, water);
    canvas->setTextSize(2);
    canvas->setCursor(191, 316);
    canvas->print("JOURNAL");
    char record[64];
    if (bobberBests[bobberSpecies]) {
      snprintf(record, sizeof(record), "%s best: %u lb %u oz",
               bobberSpecies == BOBBER_FLOUNDER ? "Flounder" :
               bobberSpecies == BOBBER_CROAKER ? "Croaker" : "Spot", bobberBests[bobberSpecies] / 16, bobberBests[bobberSpecies] % 16);
    } else {
      snprintf(record, sizeof(record), "Your first catch awaits.");
    }
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(record) * 3, 350);
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
    drawBobberBay(canvas);
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
    if (armed && pressedState == bobberState) {
      if (pressedTarget == 3) --bobberJournalPage;
      else if (pressedTarget == 4) ++bobberJournalPage;
      else if (pressedTarget == 2) {
        bobberJournalPage = 0;
        bobberState = BOBBER_JOURNAL;
      }
      else if (bobberState == BOBBER_JOURNAL) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_TITLE) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_WAITING) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_BITE) {
        // Roll only on a successful hook, never on redraw or a cancelled cast.
        if (bobberSessionCatches < UINT32_MAX) ++bobberSessionCatches;
        // Equal chances for this first three-species prototype; balance later.
        bobberSpecies = bobberBayRoster[random(BOBBER_BAY_COUNT)];
        bobberCatchOz = (uint16_t)random(bobberMinOz[bobberSpecies],
                                       bobberMaxOz[bobberSpecies] + 1);
        bobberNewBest = bobberCatchOz > bobberBests[bobberSpecies];
        if (bobberNewBest) bobberBests[bobberSpecies] = bobberCatchOz;
        // If storage failed, retry the outstanding record on the next catch.
        saveBobberRecord();
        bobberState = BOBBER_CAUGHT;
      }
      else {
        bobberState = BOBBER_WAITING;
        bobberCastAt = millis();
      }
      renderBobberWelcome(canvas);
    }
    armed = false;
  }
  wasDown = pressed;
}
