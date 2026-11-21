#pragma once

#include "bobber_bay_background.h"

// Half-resolution art writes directly to the landscape canvas. Repaint only
// this patch during idle animation so headings and controls keep their pixels.
static constexpr int BOBBER_BAY_PATCH_X = 53;
static constexpr int BOBBER_BAY_PATCH_Y = 101;
static constexpr int BOBBER_BAY_PATCH_RIGHT = 180;
static constexpr int BOBBER_BAY_PATCH_BOTTOM = 152;

static void restoreBobberBay(Arduino_Canvas *canvas, int left, int top, int right, int bottom) {
  constexpr int stride = 466;
  auto *pixels = canvas->getFramebuffer();
  for (int y = top; y < bottom; ++y) {
    auto *row = pixels + (2 * y) * stride;
    int x = 0;
    for (int run = bobberBayRows[y]; run < bobberBayRows[y + 1] && x < right; ++run) {
      const uint16_t packed = bobberBayRuns[run];
      const int end = x + (packed >> 8);
      const uint16_t color = bobberBayPalette[packed & 255];
      for (int pixel = x < left ? left : x; pixel < end && pixel < right; ++pixel) {
        const int px = 2 * pixel;
        row[px] = row[px + 1] = row[px + stride] = row[px + stride + 1] = color;
      }
      x = end;
    }
  }
}

static void drawBobberBayScene(Arduino_Canvas *canvas, bool full) {
  if (full) restoreBobberBay(canvas, 0, 0, 233, 233);
  else restoreBobberBay(canvas, BOBBER_BAY_PATCH_X, BOBBER_BAY_PATCH_Y,
                        BOBBER_BAY_PATCH_RIGHT, BOBBER_BAY_PATCH_BOTTOM);
  const int phase = bobberAnimationFrame % 8;
  static const int bob[] = {0, 0, 1, 1, 0, 0, -1, -1};
  const int drift = phase <= 4 ? phase : 8 - phase;
  const int hull = 127 + bob[phase];
  const bool lineOut = bobberState == BOBBER_WAITING || bobberState == BOBBER_BITE ||
                       bobberState == BOBBER_FIGHT;
  const bool splashing = bobberState == BOBBER_BITE ||
                         (bobberState == BOBBER_FIGHT && bobberHardPull());
  const int floatY = splashing ? 131 : 128 + bob[(phase + 2) % 8];
  constexpr uint16_t ink = 0x1988, umber = 0x51E5, copper = 0x9B88;
  constexpr uint16_t wood = 0xC4C9, rim = 0xE651, canvasCoat = 0x53C9;
  constexpr uint16_t skin = 0xCE0F, reflection = 0x2AEB, glint = 0x6510;
  auto span = [canvas](int x, int y, int w, int h, uint16_t color) {
    canvas->fillRect(2 * x, 2 * y, 2 * w, 2 * h, color);
  };
  auto line = [&span](int x, int y, int xEnd, int yEnd, uint16_t color) {
    const int dx = xEnd > x ? xEnd - x : x - xEnd;
    const int dy = yEnd > y ? y - yEnd : yEnd - y;
    const int sx = x < xEnd ? 1 : -1, sy = y < yEnd ? 1 : -1;
    int error = dx + dy;
    while (true) {
      span(x, y, 1, 1, color);
      if (x == xEnd && y == yEnd) break;
      const int twice = 2 * error;
      if (twice >= dy) { error += dy; x += sx; }
      if (twice <= dx) { error += dx; y += sy; }
    }
  };

  // Broken reflections carry the skiff's shape into the water.
  span(69, hull + 9, 47, 2, reflection);
  span(75 + drift, hull + 12, 32, 1, reflection);
  span(66 + drift, hull + 15, 20, 1, glint);
  span(104 - drift, hull + 17, 15, 1, glint);
  span(59 + drift, 149, 15, 1, reflection);
  span(157 - drift, 146, 19, 1, glint);

  // An occasional tern skims the channel above the float.
  if (bobberState != BOBBER_FIGHT && bobberAnimationFrame >= 48) {
    const int flight = bobberAnimationFrame - 48;
    const int x = 168 - flight * 5;
    const int y = 106 + flight / 4;
    const int wing = (flight % 4 < 2) ? -2 : 1;
    line(x - 3, y + wing, x, y, ink);
    line(x, y, x + 3, y + wing, ink);
    span(x, y, 2, 1, rim);
  }

  // A seated angler in a canvas jacket, braced against the side of the skiff.
  span(85, hull - 14, 8, 12, ink);
  span(86, hull - 13, 5, 8, canvasCoat);
  span(87, hull - 19, 6, 6, skin);
  span(91, hull - 17, 3, 2, skin);
  span(86, hull - 21, 7, 3, umber);
  span(84, hull - 19, 12, 1, rim);
  span(90, hull - 5, 10, 4, ink);
  line(89, hull - 11, 97, hull - 8, canvasCoat);
  span(95, hull - 9, 4, 2, skin);
  span(71, hull - 4, 7, 4, umber);
  span(72, hull - 5, 5, 2, wood);

  // Weathered lapstrake planks, narrow bow and a sunlit gunwale.
  span(66, hull, 53, 2, ink);
  span(68, hull + 2, 49, 2, umber);
  span(70, hull + 4, 45, 2, copper);
  span(74, hull + 6, 37, 2, ink);
  span(67, hull - 1, 50, 1, rim);
  span(74, hull + 2, 32, 1, wood);
  span(80, hull + 4, 22, 1, wood);
  span(115, hull - 2, 5, 2, wood);

  const int tipX = lineOut ? 132 : 112;
  const int tipY = lineOut ? hull - (splashing ? 15 : 23) : hull - 9;
  const int bendX = lineOut ? 118 : 105;
  const int bendY = lineOut ? hull - 22 : hull - 9;
  line(97, hull - 8, bendX, bendY, ink);
  line(bendX, bendY, tipX, tipY, umber);
  if (!lineOut) return;

  line(tipX, tipY + 1, 151, floatY - 1, 0x9DD1);
  span(147 - drift, floatY + 3, 9 + phase, 1, glint);
  span(149, floatY, 5, 2, ink);
  span(150, floatY - 3, 3, 3, 0xD7E4);
  span(150, floatY, 3, 2, 0xFC22);
  if (splashing) {
    span(145 - phase / 2, 134, 4, 1, rim);
    span(154 + drift, 134, 4, 1, rim);
    span(147 - drift, 127 - phase / 2, 1, 2, rim);
    span(155, 126 - phase / 2, 1, 2, rim);
  }
}
