#pragma once

struct BobberFightPattern {
  uint32_t progressNeeded;
  uint16_t cycleMs;
  uint16_t pullMs;
  uint8_t pullTension;
  const char *cue;
};

static const BobberFightPattern bobberFightPatterns[] = {
  {33000, 2300, 720, 47, "Sudden run"},
  {49000, 2600, 1100, 61, "Heavy run"},
  {61000, 3700, 1700, 50, "Deep dive"},
  {71000, 2400, 1050, 68, "Iron-Maw surges"},
};

struct BobberFight {
  uint32_t at;
  uint32_t elapsed;
  uint32_t progress;
  int32_t tension;
  uint16_t strainMs;
  uint8_t pattern;
  bool reeling;
};
static BobberFight bobberFight = {};
static const int32_t BOBBER_DANGER = 85000;

static bool bobberHardPull() {
  const auto &pattern = bobberFightPatterns[bobberFight.pattern];
  const uint32_t phase = bobberFight.elapsed % pattern.cycleMs;
  if (bobberFight.pattern == 3 && phase >= 1700 && phase < 2060) return true;
  return phase < pattern.pullMs;
}

static void bobberStartFight(uint32_t now, uint8_t pattern) {
  bobberFight = {now, 0, 0, 16000, 0, pattern, false};
}
