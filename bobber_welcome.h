#pragma once

#include "Arduino_GFX_Library.h"
#include <Preferences.h>

// First Bobber milestone: an original, asset-free screen for the round display.
// Set to 0 to run the upstream game again while fishing is under development.
#define BOBBER_WELCOME 1

enum BobberState { BOBBER_READY, BOBBER_WAITING, BOBBER_BITE, BOBBER_CAUGHT };
static BobberState bobberState = BOBBER_READY;
static uint32_t bobberCastAt = 0;
// Whole ounces avoid rounding when displaying pounds and ounces.
// The catch is session-only; the personal best is loaded from Bobber storage.
static uint16_t bobberCatchOz = 0;
static uint16_t bobberBestOz = 0;
static bool bobberNewBest = false;
static uint16_t bobberSavedBestOz = 0;

static void loadBobberRecord() {
  bobberBestOz = bobberSavedBestOz = 0;
  bobberCatchOz = 0;
  bobberNewBest = false;
  Preferences prefs;
  if (!prefs.begin("bobber", true)) return;
  const uint16_t saved = prefs.getUShort("bobberBestOz", 0);
  prefs.end();
  // Ignore invalid/obsolete data without changing any other game's save.
  if (saved >= 8 && saved <= 128) bobberBestOz = bobberSavedBestOz = saved;
}

static void saveBobberRecord() {
  if (bobberBestOz <= bobberSavedBestOz) return;
  Preferences prefs;
  if (!prefs.begin("bobber", false)) return;
  if (prefs.putUShort("bobberBestOz", bobberBestOz) == sizeof(uint16_t))
    bobberSavedBestOz = bobberBestOz;
  prefs.end();
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

static void renderBobberWelcome(Arduino_Canvas *canvas) {
  const uint16_t navy = 0x08E4;
  const uint16_t cream = 0xF7B9;
  const uint16_t lime = 0xBFE0;
  const uint16_t orange = 0xFB00;
  const uint16_t water = 0x2B8E;

  canvas->fillScreen(navy);
  canvas->setTextColor(cream);
  canvas->setTextSize(2);
  canvas->setCursor(191, 74);
  canvas->print("PROJECT");
  canvas->setTextSize(5);
  canvas->setCursor(143, 103);
  canvas->print("BOBBER");

  // Water ripples and a two-tone float, kept inside the circular bezel.
  canvas->drawFastHLine(119, 283, 228, water);
  canvas->drawFastHLine(143, 294, 180, water);
  if (bobberState == BOBBER_CAUGHT) {
    drawBobberFlounder(canvas);
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
    canvas->setCursor(113, 321);
    canvas->print("You caught a flounder!");
    char weight[40];
    snprintf(weight, sizeof(weight), "%u lb %u oz", bobberCatchOz / 16,
             bobberCatchOz % 16);
    canvas->setTextSize(2);
    canvas->setCursor(233 - strlen(weight) * 6, 343);
    canvas->print(weight);
    snprintf(weight, sizeof(weight), "%s: %u lb %u oz",
             bobberNewBest ? "New personal best" : "Personal best",
             bobberBestOz / 16, bobberBestOz % 16);
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(weight) * 3, 365);
    canvas->print(weight);
    if (bobberBestOz > bobberSavedBestOz) {
      canvas->setCursor(155, 301);
      canvas->print("Record not saved; try again");
    }
  } else {
    canvas->setCursor(131, 316);
    canvas->print("A little fishing.");
    canvas->setCursor(125, 340);
    canvas->print("A lot of relaxing.");
    char record[40];
    if (bobberBestOz) {
      snprintf(record, sizeof(record), "Personal best: %u lb %u oz",
               bobberBestOz / 16, bobberBestOz % 16);
    } else {
      snprintf(record, sizeof(record), "Your first catch awaits.");
    }
    canvas->setTextSize(1);
    canvas->setCursor(233 - strlen(record) * 3, 365);
    canvas->print(record);
  }
  canvas->fillRoundRect(BOBBER_BUTTON_X, BOBBER_BUTTON_Y,
                        BOBBER_BUTTON_W, BOBBER_BUTTON_H, 16, lime);
  canvas->setTextColor(navy);
  canvas->setTextSize(3);
  canvas->setCursor(197, 389);
  canvas->print(bobberState == BOBBER_WAITING ? "BACK" :
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
      if (bobberState == BOBBER_WAITING) bobberState = BOBBER_READY;
      else if (bobberState == BOBBER_BITE) {
        // Roll only on a successful hook, never on redraw or a cancelled cast.
        bobberCatchOz = (uint16_t)random(8, 129);
        bobberNewBest = bobberCatchOz > bobberBestOz;
        if (bobberNewBest) bobberBestOz = bobberCatchOz;
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
