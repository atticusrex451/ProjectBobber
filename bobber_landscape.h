#pragma once

#include "bobber_bay_background.h"
#include "bobber_lake_background.h"
#include "bobber_grotto_background.h"
#include "bobber_shop_background.h"

// Half-resolution art writes directly to the landscape canvas. Repaint only
// this patch during idle animation so headings and controls keep their pixels.
static constexpr int BOBBER_LANDSCAPE_PATCH_X = 53;
static constexpr int BOBBER_LANDSCAPE_PATCH_Y = 101;
static constexpr int BOBBER_LANDSCAPE_PATCH_RIGHT = 183;
static constexpr int BOBBER_LANDSCAPE_PATCH_BOTTOM = 152;

static constexpr int BOBBER_LAKE_ANIMATION_FRAMES = 160;
static constexpr int BOBBER_FLOCK_START = 36;
static constexpr int BOBBER_FLOCK_END = 78;
static constexpr int BOBBER_FLOCK_SKY_TOP = 14;
static constexpr int BOBBER_FLOCK_SKY_BOTTOM = 40;

struct BobberLandscape {
  const uint16_t *rows;
  const uint16_t *runs;
  const uint16_t *palette;
};

static constexpr BobberLandscape bobberLandscapes[] = {
  {bobberBayRows, bobberBayRuns, bobberBayPalette},
  {bobberLakeRows, bobberLakeRuns, bobberLakePalette},
  {bobberGrottoRows, bobberGrottoRuns, bobberGrottoPalette},
};

static void blitBobberArtwork(Arduino_Canvas *canvas, const BobberLandscape &art,
                              int left, int top, int right, int bottom, bool dim) {
  constexpr int stride = 466;
  auto *pixels = canvas->getFramebuffer();
  for (int y = top; y < bottom; ++y) {
    auto *row = pixels + (2 * y) * stride;
    int x = 0;
    for (int run = art.rows[y]; run < art.rows[y + 1] && x < right; ++run) {
      const uint16_t packed = art.runs[run];
      const int end = x + (packed >> 8);
      const uint16_t original = art.palette[packed & 255];
      const uint16_t color = dim ? (original >> 1) & 0x7BEF : original;
      for (int pixel = x < left ? left : x; pixel < end && pixel < right; ++pixel) {
        const int px = 2 * pixel;
        row[px] = row[px + 1] = row[px + stride] = row[px + stride + 1] = color;
      }
      x = end;
    }
  }
}

static void blitBobberLandscape(Arduino_Canvas *canvas, BobberRegion region,
                                int left, int top, int right, int bottom, bool dim) {
  blitBobberArtwork(canvas, bobberLandscapes[region], left, top, right, bottom, dim);
}

static void drawBobberShopBackdrop(Arduino_Canvas *canvas) {
  static constexpr BobberLandscape shop = {bobberShopRows, bobberShopRuns, bobberShopPalette};
  blitBobberArtwork(canvas, shop, 0, 0, 233, 233, false);
}

static void restoreBobberLandscape(Arduino_Canvas *canvas, int left, int top, int right, int bottom) {
  blitBobberLandscape(canvas, bobberRegion, left, top, right, bottom, false);
}

static void drawBobberTravelThumbnail(Arduino_Canvas *canvas, BobberRegion region, int x, int y) {
  const auto &art = bobberLandscapes[region];
  for (int row = 0; row < 36; ++row) {
    const int sourceY = 20 + row * 5;
    int run = art.rows[sourceY];
    int end = art.runs[run] >> 8;
    for (int column = 0; column < 44; ++column) {
      const int sourceX = 8 + column * 5;
      while (sourceX >= end) end += art.runs[++run] >> 8;
      canvas->drawFastHLine(x + column, y + row, 1, art.palette[art.runs[run] & 255]);
    }
  }
}

static void drawBobberScene(Arduino_Canvas *canvas, bool full = false) {
  if (full) restoreBobberLandscape(canvas, 0, 0, 233, 233);
  else restoreBobberLandscape(canvas, BOBBER_LANDSCAPE_PATCH_X, BOBBER_LANDSCAPE_PATCH_Y,
                        BOBBER_LANDSCAPE_PATCH_RIGHT, BOBBER_LANDSCAPE_PATCH_BOTTOM);
  const bool lake = bobberRegion == BOBBER_LAKE;
  const bool grotto = bobberRegion == BOBBER_GROTTO;
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
  const uint16_t wood = grotto ? 0x93E9 : lake ? 0xAC69 : 0xC4C9;
  const uint16_t rim = grotto ? 0xD611 : lake ? 0xD632 : 0xE651;
  const uint16_t canvasCoat = grotto ? 0x6C12 : lake ? 0x9A87 : 0x53C9;
  constexpr uint16_t skin = 0xCE0F;
  const uint16_t reflection = grotto ? 0x1AEC : lake ? 0x2BB0 : 0x2AEB;
  const uint16_t glint = grotto ? 0x44CE : lake ? 0x5CF3 : 0x6510;
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

  if (lake && bobberState != BOBBER_FIGHT) {
    const int frame = bobberAnimationFrame;
    // Restore one extra frame after the flock exits. This band ends above the title.
    if (!full && frame >= BOBBER_FLOCK_START && frame <= BOBBER_FLOCK_END)
      restoreBobberLandscape(canvas, 0, BOBBER_FLOCK_SKY_TOP, 233, BOBBER_FLOCK_SKY_BOTTOM);
    if (frame >= BOBBER_FLOCK_START && frame < BOBBER_FLOCK_END) {
      const int travel = frame - BOBBER_FLOCK_START;
      static const int8_t formation[][3] = {
        {0, 0, 4}, {-9, -4, 3}, {-11, 5, 4}, {-19, -7, 3},
        {-22, 9, 3}, {-29, -8, 2}, {-34, 6, 3},
      };
      constexpr uint16_t feathers = 0x10C5;
      for (int i = 0; i < 7; ++i) {
        const auto &bird = formation[i];
        const int x = -8 + 7 * travel + bird[0];
        const int y = 28 + bird[1] - travel / 12;
        const int wing = ((travel + i) % 4 < 2) ? -2 : 1;
        line(x - bird[2], y + wing, x, y, feathers);
        line(x, y, x + bird[2] - 1, y + wing, feathers);
        span(x, y, 2, 1, feathers);
      }
    }
  }

  // Broken reflections carry the skiff's shape into the water.
  span(69, hull + 9, 47, 2, reflection);
  span(75 + drift, hull + 12, 32, 1, reflection);
  span(66 + drift, hull + 15, 20, 1, glint);
  span(104 - drift, hull + 17, 15, 1, glint);
  span(59 + drift, 149, 15, 1, reflection);
  span(157 - drift, 146, 19, 1, glint);

  // An occasional tern skims the Bay channel above the float.
  if (bobberRegion == BOBBER_BAY && bobberState != BOBBER_FIGHT && bobberAnimationFrame >= 48) {
    const int flight = bobberAnimationFrame - 48;
    const int x = 168 - flight * 5;
    const int y = 106 + flight / 4;
    const int wing = (flight % 4 < 2) ? -2 : 1;
    line(x - 3, y + wing, x, y, ink);
    line(x, y, x + 3, y + wing, ink);
    span(x, y, 2, 1, rim);
  }

  // A loon sometimes cuts a quiet wake across the far side of the lake.
  if (lake && bobberState != BOBBER_FIGHT && bobberAnimationFrame >= 92 &&
      bobberAnimationFrame < 108) {
    const int x = 145 + (bobberAnimationFrame - 92);
    span(x - 9, 120, 9, 1, glint);
    span(x, 118, 8, 2, ink);
    span(x + 5, 115, 3, 4, ink);
    span(x + 8, 116, 2, 1, ink);
    span(x + 5, 117, 2, 1, rim);
  }

  if (grotto && bobberState != BOBBER_FIGHT && bobberAnimationFrame >= 40) {
    // A drop falls through the light, then briefly ripples on the pool.
    const int drip = bobberAnimationFrame - 40;
    if (drip < 9) span(171, 103 + drip * 3, 1, 2, 0x9DF1);
    else if (drip < 17) {
      const int spread = 2 + drip - 9;
      span(171 - spread, 132, 3, 1, glint);
      span(170 + spread, 132, 3, 1, glint);
      span(169, 133, 5, 1, reflection);
    }
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
