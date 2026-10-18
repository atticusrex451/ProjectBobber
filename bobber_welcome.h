#pragma once

#include "Arduino_GFX_Library.h"
#include <Preferences.h>

// First Bobber milestone: an original, asset-free screen for the round display.
// Set to 0 to run the upstream game again while fishing is under development.
#define BOBBER_WELCOME 1

enum BobberState { BOBBER_TITLE, BOBBER_READY, BOBBER_WAITING, BOBBER_BITE, BOBBER_CAUGHT };
static BobberState bobberState = BOBBER_TITLE;
static uint32_t bobberCastAt = 0;
static uint32_t bobberAnimationAt = 0;
static uint8_t bobberAnimationFrame = 0;
// Whole ounces avoid rounding when displaying pounds and ounces.
// The catch is session-only; the personal best is loaded from Bobber storage.
static uint16_t bobberCatchOz = 0;
static uint32_t bobberSessionCatches = 0;
enum BobberSpecies { BOBBER_FLOUNDER, BOBBER_CROAKER, BOBBER_SPECIES_COUNT };
static BobberSpecies bobberSpecies = BOBBER_FLOUNDER;
static const char *const bobberNames[] = {"Summer Flounder", "Atlantic Croaker"};
// Keep the original flounder key so existing personal bests survive this update.
static const char *const bobberRecordKeys[] = {"bobberBestOz", "croakerBestOz"};
static const uint16_t bobberMinOz[] = {8, 4};
static const uint16_t bobberMaxOz[] = {128, 48};
static uint16_t bobberBests[BOBBER_SPECIES_COUNT] = {};
static bool bobberNewBest = false;
static uint16_t bobberSavedBests[BOBBER_SPECIES_COUNT] = {};

static void loadBobberRecord() {
  bobberState = BOBBER_TITLE;
  bobberAnimationAt = 0;
  bobberAnimationFrame = 0;
  bobberSpecies = BOBBER_FLOUNDER;
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
    if (bobberSpecies == BOBBER_CROAKER) drawBobberCroaker(canvas);
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
    canvas->setCursor(125, 321);
    canvas->print("Ready when you are.");
    char record[64];
    if (bobberBests[bobberSpecies]) {
      snprintf(record, sizeof(record), "%s best: %u lb %u oz",
               bobberSpecies == BOBBER_FLOUNDER ? "Flounder" : "Croaker", bobberBests[bobberSpecies] / 16, bobberBests[bobberSpecies] % 16);
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
  if (pressed) {
    if (!wasDown) {
      armed = bobberButtonContains(x, y);
      pressedState = bobberState;
    }
    else if (!bobberButtonContains(x, y)) armed = false;
  } else if (wasDown) {
    if (armed && pressedState == bobberState) {
      if (bobberState == BOBBER_TITLE) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_WAITING) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_BITE) {
        // Roll only on a successful hook, never on redraw or a cancelled cast.
        if (bobberSessionCatches < UINT32_MAX) ++bobberSessionCatches;
        // Equal chances for this first two-species prototype; balance later.
        bobberSpecies = (BobberSpecies)random(BOBBER_SPECIES_COUNT);
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
