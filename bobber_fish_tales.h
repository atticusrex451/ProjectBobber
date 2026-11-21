#pragma once

struct BobberFishTale {
  bool fact;
  const char *lines[4];
};

static constexpr BobberFishTale bobberFishTales[] = {
  {false, {"I caught a fish so big", "the boat took a picture", "of me for proof."}},
  {true, {"A young summer flounder", "moves its right eye", "across its head to", "the left side."}},
  {false, {"My best fishing spot?", "It's right between", "None of Your Business", "and Nice Try."}},
  {true, {"Lake sturgeon have", "four whiskers that", "help them find food."}},
  {false, {"The fish got away,", "but I kept the receipt.", "It says: one worm."}},
  {true, {"A pumpkinseed sunfish", "has an orange-red spot", "on its black ear flap."}},
  {false, {"I went fishing for", "a little peace and quiet.", "Caught both. Released", "them before dinner."}},
  {true, {"A lake sturgeon's armor", "is made of bony plates", "called scutes."}},
  {false, {"That fish was THIS big.", "Good thing your screen", "is round, or I'd need", "another corner."}},
};

static int bobberFishTale = -1;

static void pickBobberFishTale() {
  constexpr int count = sizeof(bobberFishTales) / sizeof(bobberFishTales[0]);
  if (bobberFishTale < 0) {
    bobberFishTale = random(count);
  } else {
    const int next = random(count - 1);
    bobberFishTale = next >= bobberFishTale ? next + 1 : next;
  }
}
