#pragma once

// Simple original stand-ins for the later art pass. One renderer keeps catch
// portraits and undiscovered trophy silhouettes consistent at both sizes.
static void drawBobberOtherFish(Arduino_Canvas *canvas, BobberSpecies species,
                                int centerY, int divisor, bool caught) {
  const uint16_t silhouette = 0x4A69;
  uint16_t body = 0xA593, fin = 0x73AB, marking = 0x2945;
  int length = 78, height = 28;
  switch (species) {
    case BOBBER_YELLOW_PERCH: body = 0xDE4A; fin = 0xD3A6; length = 67; height = 31; break;
    case BOBBER_PUMPKINSEED: body = 0xDD6B; fin = 0xA46A; length = 57; height = 42; break;
    case BOBBER_BROOK_TROUT: body = 0x4BAC; fin = 0xDAA9; marking = 0xE6EF; break;
    case BOBBER_LARGEMOUTH_BASS: body = 0x7CAF; fin = 0x5B8A; height = 33; break;
    case BOBBER_RAINBOW_TROUT: body = 0xBDF7; fin = 0x7CD3; break;
    case BOBBER_BROWN_TROUT: body = 0xBD4C; fin = 0x9C09; break;
    case BOBBER_LAKE_TROUT: body = 0x6390; fin = 0x42AD; marking = 0xDF18; height = 30; break;
    case BOBBER_MUSKELLUNGE: body = 0x9CCE; fin = 0x6BC9; length = 98; height = 19; break;
    case BOBBER_DOUBLOON_DARTER: body = 0xF6C7; fin = 0xC4C4; length = 62; height = 22; break;
    case BOBBER_BARNACLE_GOBY: body = 0x7C90; fin = 0xB4A8; length = 59; height = 33; break;
    case BOBBER_COPPER_FLOUNDER: body = 0x6B28; fin = 0xB3CA; length = 72; height = 39; break;
    case BOBBER_RUM_DRUM: body = 0xA3C9; fin = 0x6287; length = 65; height = 40; break;
    case BOBBER_CUTLASS_SNAPPER: body = 0x8BD2; fin = 0xE73B; height = 32; break;
    case BOBBER_GILDED_RAVEN: body = 0x2127; fin = 0xE609; length = 67; height = 34; break;
    case BOBBER_CORAL_PROWLER: body = 0x72F1; fin = 0x3D73; height = 31; break;
    case BOBBER_POWDER_EEL: body = 0x18E5; fin = 0x528A; length = 99; height = 12; break;
    case BOBBER_OLD_MAN: body = 0xC3AA; fin = 0x8A87; length = 89; height = 38; break;
    case BOBBER_LEVIATHAN: body = 0x8CD3; fin = 0x528C; length = 99; height = 24; break;
    case BOBBER_IRON_MAW: body = 0x528B; fin = 0xE5E8; length = 88; height = 38; break;
    default: return;
  }
  if (!caught) body = fin = marking = silhouette;
  const uint16_t dark = caught ? 0x2105 : silhouette;
  auto px = [&](int x) { return 233 + x / divisor; };
  auto py = [&](int y) { return centerY + y / divisor; };
  auto line = [&](int x1, int y1, int x2, int y2, uint16_t c) {
    canvas->drawLine(px(x1), py(y1), px(x2), py(y2), c);
  };
  auto circle = [&](int x, int y, int r, uint16_t c) {
    canvas->fillCircle(px(x), py(y), r / divisor > 0 ? r / divisor : 1, c);
  };
  auto triangle = [&](int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) {
    canvas->fillTriangle(px(x1), py(y1), px(x2), py(y2), px(x3), py(y3), c);
  };
  const bool eel = species == BOBBER_POWDER_EEL;
  const bool flat = species == BOBBER_COPPER_FLOUNDER;
  const bool spiny = species == BOBBER_GILDED_RAVEN || species == BOBBER_IRON_MAW;
  const bool rearDorsal = species == BOBBER_MUSKELLUNGE;
  if (!eel) {
    triangle(length - 10, 0, length + 24, -25, length + 17, 0, fin);
    triangle(length - 10, 0, length + 17, 0, length + 24, 25, fin);
    if (spiny) {
      for (int x = -40; x <= 45; x += 17)
        triangle(x - 9, -height + 8, x, -height - 21, x + 9, -height + 8, fin);
      triangle(-20, height - 5, 0, height + 18, 28, height - 4, fin);
    } else if (species == BOBBER_CUTLASS_SNAPPER) {
      triangle(-34, -height + 5, 15, -height - 28, 0, -height + 5, fin);
      triangle(0, -height + 5, 15, -height - 28, 55, -10, fin);
    } else if (!flat) {
      const int dorsalX = rearDorsal ? 37 : -18;
      triangle(dorsalX - 19, -height + 5, dorsalX, -height - 17, dorsalX + 28, -height + 6, fin);
      triangle(0, height - 3, 25, height + 14, 43, 9, fin);
    }
  }
  // Broad flatfish fin fringe, or a continuous undulating eel fin.
  for (int x = -length; x <= length; ++x) {
    const int h = height * (length * length - x * x) / (length * length);
    const int bend = eel ? (x < -30 ? -9 : x > 30 ? 9 : x * 9 / 30) : 0;
    if (flat || eel) line(x, bend - h - 5, x, bend + h + 5, fin);
    line(x, bend - h, x, bend + h, dark);
    if (h > 2) line(x, bend - h + 2, x, bend + h - 2, body);
  }
  if (species == BOBBER_BARNACLE_GOBY) {
    triangle(-35, 10, -47, 39, -5, 22, fin);
    triangle(7, 14, 24, 35, 38, 9, fin);
  }
  if (species == BOBBER_LEVIATHAN) {
    // Extended sturgeon snout and ventral barbels stay in the silhouette.
    triangle(-length - 17, 6, -length + 22, -12, -length + 22, 14, body);
    for (int x = -79; x < -59; x += 6) line(x, 10, x + 2, 20, fin);
  }
  if (!caught) return;
  switch (species) {
    case BOBBER_YELLOW_PERCH:
    case BOBBER_MUSKELLUNGE:
      for (int x = -35; x <= 43; x += 16)
        for (int w = 0; w < 4; ++w) line(x + w, -height + 9, x + w + 3, height - 9, marking);
      break;
    case BOBBER_PUMPKINSEED:
      for (int y = -15; y <= 15; y += 10) line(-43, y, -18, y + 6, 0x4599);
      circle(-15, -9, 7, 0xEAA7); circle(-15, -9, 5, dark);
      for (int x = 2; x <= 32; x += 10) circle(x, 4, 3, 0xF66B);
      break;
    case BOBBER_LARGEMOUTH_BASS:
      for (int y = -4; y <= 5; ++y) line(-55, y, 57, y, 0x3247);
      line(-length, 4, -43, 10, dark); line(-43, 10, -45, -5, dark);
      break;
    case BOBBER_BROOK_TROUT:
    case BOBBER_RAINBOW_TROUT:
    case BOBBER_BROWN_TROUT:
    case BOBBER_LAKE_TROUT:
      if (species == BOBBER_RAINBOW_TROUT)
        for (int y = -3; y <= 5; ++y) line(-52, y, 56, y, 0xD3B2);
      for (int x = -30; x <= 48; x += 15) for (int y = -12; y <= 12; y += 12) {
        if (species == BOBBER_BROOK_TROUT) circle(x, y, 4, 0x6D7B);
        circle(x, y, 2, species == BOBBER_BROOK_TROUT || species == BOBBER_BROWN_TROUT ? 0xE2A9 : marking);
      }
      if (species == BOBBER_BROOK_TROUT) {
        line(4, height, 25, height + 14, 0xF7B9);
        for (int x = -20; x < 40; x += 13) line(x, -21, x + 7, -18, marking);
      }
      break;
    case BOBBER_DOUBLOON_DARTER:
      for (int x = -25; x <= 35; x += 20) {
        circle(x, 0, 7, 0xB424); circle(x, 0, 5, 0xFF2B);
        line(x, -3, x, 3, 0xB424);
      }
      break;
    case BOBBER_BARNACLE_GOBY:
      for (int x = -18; x <= 30; x += 15) {
        circle(x, -12 + x % 9, 7, 0xD6B6); circle(x, -12 + x % 9, 3, 0x6B2C);
      }
      break;
    case BOBBER_COPPER_FLOUNDER:
      for (int x = -25; x <= 40; x += 20) circle(x, x % 17, 5, 0x9BC9);
      circle(-46, -12, 5, dark); circle(-37, -23, 5, dark);
      line(-32, -8, -24, 13, 0xFBA8); line(-27, -10, -19, 12, 0xFBA8);
      break;
    case BOBBER_RUM_DRUM:
      for (int x = -36; x <= 36; x += 12) line(x, -25, x, 25, 0xC4CC);
      for (int x : {-29, 29}) for (int w = 0; w < 5; ++w) line(x + w, -30, x + w, 30, 0x528A);
      break;
    case BOBBER_CUTLASS_SNAPPER:
      line(-24, -height + 3, 15, -height - 28, 0xFFFF);
      line(15, -height - 28, 50, -12, 0xB63C);
      line(-length + 3, 7, -52, 10, dark);
      triangle(-68, 8, -65, 15, -62, 8, 0xF7B9);
      break;
    case BOBBER_GILDED_RAVEN:
      for (int x = -36; x <= 36; x += 18) line(x, -17, x + 9, 20, 0xE609);
      triangle(-20, 6, 0, 33, 23, 6, 0xBD05);
      break;
    case BOBBER_CORAL_PROWLER:
      for (int x = -24; x <= 42; x += 22) {
        const uint16_t jewel = x < 0 ? 0x4E9A : x < 30 ? 0xFB75 : 0xFEC9;
        triangle(x - 6, 0, x, -9, x + 6, 0, jewel);
        triangle(x - 6, 0, x, 9, x + 6, 0, jewel);
      }
      break;
    case BOBBER_POWDER_EEL:
      line(-80, -3, -35, -3, 0x630B); line(12, 10, 72, 16, 0x630B);
      circle(-78, -12, 5, 0xFC40); circle(-78, -12, 2, dark);
      line(-97, -5, -78, 0, 0xAC4C);
      return;
    case BOBBER_OLD_MAN:
      circle(58, -1, 7, dark); // red drum tail spot, with old scars
      for (int x = -24; x <= 10; x += 15) line(x, -15, x + 9, 12, 0xE5F1);
      break;
    case BOBBER_LEVIATHAN:
      for (int x = -35; x <= 65; x += 17)
        triangle(x - 5, -9, x, -19, x + 5, -9, 0xDEFA);
      break;
    case BOBBER_IRON_MAW:
      for (int x = -50; x <= 45; x += 20) circle(x, -14, 4, 0xE5E8);
      line(-85, 4, -43, 12, 0xBDF7);
      for (int x = -74; x < -45; x += 9) triangle(x, 8, x + 4, 17, x + 8, 9, 0xF7B9);
      break;
    default: break;
  }
  if (!flat) {
    line(-length + 30, -16, -length + 39, 0, dark);
    line(-length + 39, 0, -length + 30, 16, dark);
    circle(-length + 17, -9, 5, dark);
    circle(-length + 18, -10, 1, 0xF7B9);
  }
}
