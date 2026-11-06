#pragma once

// Stable species order and separate keys preserve all existing bay records.
enum BobberSpecies {
  BOBBER_FLOUNDER, BOBBER_CROAKER, BOBBER_SPOT, BOBBER_WEAKFISH,
  BOBBER_STRIPED_BASS, BOBBER_SPECKLED_TROUT, BOBBER_BLACK_DRUM, BOBBER_COBIA,
  BOBBER_YELLOW_PERCH, BOBBER_PUMPKINSEED, BOBBER_BROOK_TROUT, BOBBER_LARGEMOUTH_BASS,
  BOBBER_RAINBOW_TROUT, BOBBER_BROWN_TROUT, BOBBER_LAKE_TROUT, BOBBER_MUSKELLUNGE,
  BOBBER_DOUBLOON_DARTER, BOBBER_BARNACLE_GOBY, BOBBER_COPPER_FLOUNDER, BOBBER_RUM_DRUM,
  BOBBER_CUTLASS_SNAPPER, BOBBER_GILDED_RAVEN, BOBBER_CORAL_PROWLER, BOBBER_POWDER_EEL,
  BOBBER_OLD_MAN, BOBBER_LEVIATHAN, BOBBER_IRON_MAW,
  BOBBER_SPECIES_COUNT
};

enum BobberRegion { BOBBER_BAY, BOBBER_LAKE, BOBBER_GROTTO, BOBBER_REGION_COUNT };
static const int BOBBER_REGULAR_COUNT = 8;
static const int BOBBER_JOURNAL_PAGES = 5; // four regular pages, then the legend
static const char *const bobberRegionNames[] = {
  "Chincoteague Bay", "Blue Mountain Lake", "Captain Thurl's Grotto"
};
static const char *const bobberRegionTitles[] = {
  "CHINCOTEAGUE BAY", "BLUE MOUNTAIN LAKE", "CAPTAIN THURL'S GROTTO"
};
static const BobberSpecies bobberRosters[BOBBER_REGION_COUNT][BOBBER_REGULAR_COUNT] = {
  {BOBBER_FLOUNDER, BOBBER_CROAKER, BOBBER_SPOT, BOBBER_WEAKFISH, BOBBER_STRIPED_BASS, BOBBER_SPECKLED_TROUT, BOBBER_BLACK_DRUM, BOBBER_COBIA},
  {BOBBER_YELLOW_PERCH, BOBBER_PUMPKINSEED, BOBBER_BROOK_TROUT, BOBBER_LARGEMOUTH_BASS, BOBBER_RAINBOW_TROUT, BOBBER_BROWN_TROUT, BOBBER_LAKE_TROUT, BOBBER_MUSKELLUNGE},
  {BOBBER_DOUBLOON_DARTER, BOBBER_BARNACLE_GOBY, BOBBER_COPPER_FLOUNDER, BOBBER_RUM_DRUM, BOBBER_CUTLASS_SNAPPER, BOBBER_GILDED_RAVEN, BOBBER_CORAL_PROWLER, BOBBER_POWDER_EEL},
};
static const BobberSpecies bobberLegends[] = { BOBBER_OLD_MAN, BOBBER_LEVIATHAN, BOBBER_IRON_MAW };
static const char *const bobberNames[] = {
  "Summer Flounder", "Atlantic Croaker", "Spot", "Weakfish",
  "Striped Bass", "Speckled Trout", "Black Drum", "Cobia",
  "Yellow Perch", "Pumpkinseed Sunfish", "Brook Trout", "Largemouth Bass",
  "Rainbow Trout", "Brown Trout", "Lake Trout", "Muskellunge",
  "Doubloon Darter", "Barnacle-Back Goby", "Copper-Gill Flounder", "Rum-Barrel Drum",
  "Cutlass Snapper", "Gilded Sea Raven", "Jeweled Coral-Prowler", "Black-Powder Eel",
  "Old Man of the Mud", "Summit Leviathan", "Sovereign Iron-Maw",
};
static const char *const bobberShortNames[] = {
  "Flounder", "Croaker", "Spot", "Weakfish",
  "Striped Bass", "Speckled Trout", "Black Drum", "Cobia",
  "Perch", "Pumpkinseed", "Brook Trout", "Largemouth",
  "Rainbow", "Brown Trout", "Lake Trout", "Muskie",
  "Darter", "Goby", "Copper Flounder", "Rum Drum",
  "Cutlass", "Sea Raven", "Coral-Prowler", "Powder Eel",
  "Old Man", "Leviathan", "Iron-Maw",
};
static const char *const bobberRecordKeys[] = {
  "bobberBestOz", "croakerBestOz", "spotBestOz", "weakfishBestOz",
  "striperBestOz", "seatroutBestOz", "drumBestOz", "cobiaBestOz",
  "perchBestOz", "sunfishBestOz", "brookBestOz", "largemBestOz",
  "rainbowBestOz", "brownBestOz", "lakerBestOz", "muskieBestOz",
  "darterBestOz", "gobyBestOz", "copperBestOz", "rumBestOz",
  "cutlassBestOz", "ravenBestOz", "coralBestOz", "eelBestOz",
  "oldManBestOz", "leviathBestOz", "ironMawBestOz",
};
// Provisional gameplay ranges in whole ounces, not biological limits.
static const uint16_t bobberMinOz[] = {8, 4, 2, 8, 16, 8, 16, 32, 4, 2, 4, 8, 8, 8, 16, 32, 1, 2, 8, 16, 16, 16, 16, 16, 640, 1280, 960};
static const uint16_t bobberMaxOz[] = {128, 48, 16, 128, 480, 160, 800, 960, 48, 24, 96, 160, 192, 256, 640, 800, 8, 32, 160, 320, 480, 640, 800, 480, 1120, 2560, 1920};
static_assert(sizeof(bobberNames) / sizeof(bobberNames[0]) == BOBBER_SPECIES_COUNT, "Incomplete species data");
static_assert(sizeof(bobberShortNames) / sizeof(bobberShortNames[0]) == BOBBER_SPECIES_COUNT, "Incomplete species data");
static_assert(sizeof(bobberRecordKeys) / sizeof(bobberRecordKeys[0]) == BOBBER_SPECIES_COUNT, "Incomplete species data");
static_assert(sizeof(bobberMinOz) / sizeof(bobberMinOz[0]) == BOBBER_SPECIES_COUNT, "Incomplete species data");
static_assert(sizeof(bobberMaxOz) / sizeof(bobberMaxOz[0]) == BOBBER_SPECIES_COUNT, "Incomplete species data");
