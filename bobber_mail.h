#pragma once

// The journal determines which letters exist. Only read receipts need storage.
enum BobberLetter {
  BOBBER_MAIL_FIRST_CATCH, BOBBER_MAIL_BAY_LEGEND, BOBBER_MAIL_LAKE_TRAVEL,
  BOBBER_MAIL_LAKE_LEGEND, BOBBER_MAIL_GROTTO_TRAVEL, BOBBER_MAIL_GROTTO_LEGEND,
  BOBBER_MAIL_COUNT
};

struct BobberLetterText {
  const char *subject;
  const char *lines[7];
};

static constexpr BobberLetterText bobberLetters[] = {
  {"First fish in the cooler", {"Nice first catch!", "The Journal keeps your", "record if you sell it.",
                               "Visit Dock > Shop", "to sell fish or buy", "bait and tackle."}},
  {"Something's stirring", {"You found all 8 Bay fish.", "Old Man of the Mud",
                             "is stirring down there.", "For the bait recipe:",
                             "Dock > Shop > Supplies", "then Legend Bait."}},
  {"The lake is open", {"Word sure travels fast.", "You can fish Blue",
                        "Mountain Lake now.", "Take Dock > Travel",
                        "to head up there."}},
  {"A ripple in the mist", {"You found all 8 lake fish.", "Summit Leviathan",
                            "is stirring down there.", "For the bait recipe:",
                            "Dock > Shop > Supplies", "then Legend Bait."}},
  {"Thurl's secret spot", {"That Leviathan was", "something, wasn't it?",
                           "Thurl's Grotto is open.", "Take Dock > Travel",
                           "to see his secret spot."}},
  {"A light under the water", {"You found all 8 cave fish.", "Sovereign Iron-Maw",
                                "is stirring down there.", "For the bait recipe:",
                                "Dock > Shop > Supplies", "then Legend Bait."}},
};
static_assert(sizeof(bobberLetters) / sizeof(bobberLetters[0]) == BOBBER_MAIL_COUNT,
              "Every mail milestone needs a letter");
static_assert(BOBBER_MAIL_COUNT <= 8, "Mail receipts must fit in one byte");

static uint8_t bobberMailRead = 0;
static int bobberOpenLetter = -1;
static bool bobberCatchMail = false;

static uint8_t bobberMailBit(int letter) { return 1u << letter; }

static uint8_t bobberAvailableMail() {
  uint8_t mail = 0;
  if (bobberDiscovered(BOBBER_BAY)) mail |= bobberMailBit(BOBBER_MAIL_FIRST_CATCH);
  if (bobberLegendAvailable(BOBBER_BAY)) mail |= bobberMailBit(BOBBER_MAIL_BAY_LEGEND);
  if (bobberRegionUnlocked(BOBBER_LAKE)) mail |= bobberMailBit(BOBBER_MAIL_LAKE_TRAVEL);
  if (bobberLegendAvailable(BOBBER_LAKE)) mail |= bobberMailBit(BOBBER_MAIL_LAKE_LEGEND);
  if (bobberRegionUnlocked(BOBBER_GROTTO)) mail |= bobberMailBit(BOBBER_MAIL_GROTTO_TRAVEL);
  if (bobberLegendAvailable(BOBBER_GROTTO)) mail |= bobberMailBit(BOBBER_MAIL_GROTTO_LEGEND);
  return mail;
}

static int bobberUnreadMail() {
  const uint8_t unread = bobberAvailableMail() & ~bobberMailRead;
  int count = 0;
  for (int i = 0; i < BOBBER_MAIL_COUNT; ++i)
    if (unread & bobberMailBit(i)) ++count;
  return count;
}

static int bobberAdjacentLetter(int from, int step, uint8_t mask) {
  for (int i = from + step; i >= 0 && i < BOBBER_MAIL_COUNT; i += step)
    if (mask & bobberMailBit(i)) return i;
  return -1;
}

static void bobberReadLetter(int letter) {
  bobberOpenLetter = letter;
  if (letter < 0 || (bobberMailRead & bobberMailBit(letter))) return;
  Preferences prefs;
  if (!prefs.begin("bobber", false)) return;
  const uint8_t read = bobberMailRead | bobberMailBit(letter);
  if (prefs.putUChar("bobberMailRead", read) == sizeof(uint8_t)) bobberMailRead = read;
  prefs.end();
}

static void bobberOpenInbox() {
  const uint8_t available = bobberAvailableMail();
  const int newestUnread = bobberAdjacentLetter(BOBBER_MAIL_COUNT, -1, available & ~bobberMailRead);
  bobberReadLetter(newestUnread >= 0 ? newestUnread :
                   bobberAdjacentLetter(BOBBER_MAIL_COUNT, -1, available));
}
