#pragma once

#include "Arduino_GFX_Library.h"

// First Bobber milestone: an original, asset-free screen for the round display.
// Set to 0 to run the upstream game again while fishing is under development.
#define BOBBER_WELCOME 1

enum BobberState { BOBBER_READY, BOBBER_WAITING, BOBBER_BITE, BOBBER_CAUGHT };
static BobberState bobberState = BOBBER_READY;
static uint32_t bobberCastAt = 0;
static const int BOBBER_BUTTON_X = 143;
static const int BOBBER_BUTTON_Y = 376;
static const int BOBBER_BUTTON_W = 180;
static const int BOBBER_BUTTON_H = 48;

static bool bobberButtonContains(int16_t x, int16_t y) {
  return x >= BOBBER_BUTTON_X && x < BOBBER_BUTTON_X + BOBBER_BUTTON_W &&
         y >= BOBBER_BUTTON_Y && y < BOBBER_BUTTON_Y + BOBBER_BUTTON_H;
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
    // Original little flatfish: oval body, tail and two eyes on its upper side.
    for (int y = -37; y <= 37; ++y)
      for (int x = -66; x <= 66; ++x)
        if (x * x * 37 * 37 + y * y * 66 * 66 <= 66 * 66 * 37 * 37)
          canvas->fillRect(233 + x, 235 + y, 1, 1, 0xAD67);
    for (int y = -23; y <= 23; ++y)
      canvas->drawLine(289, 235, 320, 235 + y, orange);
    canvas->fillCircle(199, 220, 5, cream);
    canvas->fillCircle(211, 208, 5, cream);
    canvas->fillCircle(198, 220, 2, navy);
    canvas->fillCircle(210, 208, 2, navy);
    canvas->fillCircle(234, 225, 4, orange);
    canvas->fillCircle(254, 246, 4, orange);
    canvas->fillCircle(218, 251, 3, orange);
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
    canvas->setTextSize(1);
    canvas->setCursor(170, 347);
    canvas->print("A little Chincoteague.");
  } else {
    canvas->setCursor(131, 316);
    canvas->print("A little fishing.");
    canvas->setCursor(125, 340);
    canvas->print("A lot of relaxing.");
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
      else if (bobberState == BOBBER_BITE) bobberState = BOBBER_CAUGHT;
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
