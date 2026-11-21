#pragma once

// Exercise the same hold/release gestures as the player, advancing only time.
static void finishBobberFight(Arduino_Canvas *canvas) {
  if (bobberState != BOBBER_FIGHT) return;
  bool reeling = false;
  for (int i = 0; bobberState == BOBBER_FIGHT && i < 400; ++i) {
    const bool reel = !bobberHardPull() && bobberFight.tension < (reeling ? 68000 : 35000);
    if (reel != reeling) updateBobberTouch(canvas, reel, 233, 400);
    reeling = reel;
    updateBobberFishing(canvas, bobberFight.at + 100);
  }
  if (reeling) updateBobberTouch(canvas, false, 233, 400);
}
