# Project Bobber — game design

Working design recorded October 17, 2026. This describes the intended game;
most systems below are not implemented. See [the handoff](PROJECT_BOBBER_HANDOFF.md)
for the actual prototype, build instructions, and validation limits.

## Purpose and tone

A Christmas gift for Adam's retired father, inspired by his fishing around
Chincoteague and catching flounder each summer. A relaxed fishing adventure:
quiet scenery, collecting, occasional stubborn fish, and local legends worth
preparing for. No daily care penalties, streak pressure, or punishment for absence.

**Thurl is Dad's best friend and fishing buddy.** Captain Thurl's Grotto is a
personal tribute to him; retain that connection when developing its character
and dialogue. Pirate lore is fictional, not a claim about the real Thurl.

Original pixel art, coastal/natural colors, and a chill mood. Cast n' Chill is a
reference for pixel art, palette and atmosphere; create our own artwork and UI.
Target: 466 × 466 circular touchscreen on Waveshare ESP32-S3 Touch AMOLED 1.75-inch.
Keep touch controls large, text legible, and scenery visible. Current prototype
layout is cramped; Adam accepts it temporarily, with layout polish deferred.

## Intended play loop and screens

Cast → catch → collect or sell → prepare for a local legend → unlock another spot.

- Splash screen: our bobber slowly bouncing in a loop, with a Play button.
- Fishing screen: fisherman in a boat, animated environment, large CAST button.
  Gentle ripples, float movement, clouds, wildlife and casting/landing animation
  are intended additions, not current features.
- Journal: species discoveries, missing silhouettes, current quests, catch totals,
  personal bests, and special specimens. Biggest fish is the initial record;
  a most-valuable-catch record is an open option.
- Bait shop: area World Quest, optional catch requests, fish sales, bait and gear
  upgrades, and preparation of special boss bait.
- Cooler: fish currently available to sell or use as ingredients.
- Area navigation: revisit unlocked destinations. Exact map/control layout remains open.

Move information into appropriate screens rather than filling the fishing scene
with counters. Screen layouts and navigation are still to be designed.

## Fishing and fights

Most catches are straightforward: cast, peaceful wait, clear bite, hook, landing
animation and reveal. Preserve the patient bite prompt; no missed bite because
Dad looked away. Occasionally a fish fights, introduced by a short banner such
as **“We got a fight on our hands!”**

Working fight design: hold REEL to gain ground while tension rises; release to
let tension settle, especially during a hard pull. Rod bend, splashes, an obvious
tension indicator and line creaking communicate danger. Sustained excessive
tension can snap the line; no unexplained random snapping. Better line provides
more room for error. Exact timing, fight frequency and tuning are undecided.

Species suggests typical behavior, but temperament varies independently of
weight and special appearance: a small ordinary fish can be a stubborn fighter.
Legends get distinctive fights. Initial proposed patterns: heavy runs for the
Old Man, deep dives/endurance for the Leviathan, a combination of learned cues
for Iron-Maw. Represent descriptive hazards through the shared tension mechanic
first; separate mechanics for chains, fraying, etc. are not commitments.

A snap loses that fish and the bait used, preserving gear, coins and prior
catches. Basic bait should let the player cast again without getting stuck.
No gear destruction or escalating punishment loop.

## Collection, value and progression

Two main areas and one secret area: Chincoteague Bay, Blue Mountain Lake, then
Captain Thurl's Grotto. Working roster: eight species plus one boss in each,
24 regular species and three bosses total. Finish Chincoteague before building
all destinations; this is a scope target, not a promise to implement everything
in one milestone.

Each main area's goal is to discover its eight base species, prepare special
bait, and catch its legend. Proposed order: Old Man unlocks Blue Mountain Lake;
both main legends lead to the grotto. Exact unlock narrative is open. Two pieces
of an old chart were suggested as a possible way to reveal the grotto.

Adam proposed using all an area's base fish to make boss bait. Working alternative:
discovering all eight unlocks the recipe, but consume a few common catches so a
rare fish accidentally sold does not block progress. Ingredient quantities,
reservation controls and recipes still need agreement. Mark/protect quest
ingredients when selling.

Grounded collection direction replaces universal Gold/Diamond/Rainbow tiers:

- **Prime:** an especially fine specimen, strong natural markings, increased value.
- **Trophy:** awarded by exceptional weight for that species, not an independent
  appearance roll. Thresholds remain open.
- **Rare natural coloration:** researched, species-appropriate unusual forms;
  avoid treating “Mutation” as a generic tier in the grounded areas.
- **Named local legends:** distinct designs and encounters.

Specimen odds, overlap of traits, price multipliers, weight ranges and economy
are undecided. Earlier sample Gold/Diamond/Rainbow probabilities and multipliers
are superseded, not balance specifications. Fictional grotto creatures can have
shiny, strange designs as part of their species identity.

Proposed pricing basis: species value × weight adjustment × specimen modifier.
Gear should have understandable benefits (heavier fish, greater tension tolerance,
cooler space); bait can favor species. Exact upgrades and prices remain open.

Keep rare appearances optional for ordinary story progression. A main ending on
catching Iron-Maw, followed by a collection-completion reward, is a proposal.
Adam also suggested a further secret fish or level for completing all variants;
final reward and completion requirements are not decided. Do not silently add
a fourth area to scope.

## Shopkeeper quests

The shopkeeper gives the area's persistent **World Quest**, centered on its
legend, plus one or a few optional **Daily Catches** for extra coins.

Working request rules: up to three random offers from accessible species/gear;
no streaks or absence penalties; accepted requests remain until completed;
turn-in pays ordinary fish value plus a bonus. Examples include several croaker,
a flounder above a target weight, or a special specimen. Avoid routinely demanding
extremely rare fish. Earlier “Gold fish” examples should use the grounded traits.

Actual calendar rotation requires a reliable handheld date source. Session-based
refresh is a fallback proposal. Offer rotation, acceptance limits, rewards and
clock handling remain open; World Quest progress must persist independently.

## Chincoteague Bay and nearby inlet waters

Warm coastal light, marshes, docks and familiar saltwater fishing. These are
user-proposed gameplay difficulty ratings, not claims of precise real-world
catch probability. Verify ecology and journal facts before final writing.

| Species | Difficulty / 5 | Character and bait reference |
| --- | --- | --- |
| Atlantic Croaker | 1 | Common shallow-water bottom fish; bloodworms/shrimp; croaking sound. |
| Spot | 1 | Small schooling fish around flats/channels; eager bait-stealers. |
| Summer Flounder (Fluke) | 2 | Dad's signature catch; bottom ambusher along channel edges; minnows/squid. |
| Weakfish (Gray Trout) | 2.5 | Fragile mouth; gentle handling and controlled tension. |
| Striped Bass (Rockfish) | 3 | Strong runs around marsh structure, bridges and sod banks. |
| Speckled Trout (Spotted Seatrout) | 3.5 | Wary fish of shallow grass beds; careful presentation. |
| Black Drum | 4 | Heavy bottom-dweller; powerful pulls; clam/crab bait. |
| Cobia | 4.5 | Large seasonal coastal/inlet visitor; strong runs and heavy tackle. |

**Boss: “The Old Man of the Mud” — giant red drum, difficulty 5/5.**
User's size reference: 40–60+ lb, 45+ inches. Deep inlet channels, outer shoals
and broad flats inform the scene. Suggested distinct look: old scars and
copper-red coloring. Final boss weight range and art are undecided.

## Blue Mountain Lake

Cold clear water, mountain scenery, forest and mist; a contrast to the bay.
Treat this as our fictional destination unless Adam specifies a real lake.
Do not claim every listed species is native to one real lake; research native
versus stocked populations for final journal text.

| Species | Difficulty / 5 | Character and bait reference |
| --- | --- | --- |
| Yellow Perch | 1 | Schooling panfish near weeds/docks; worms and small jigs. |
| Pumpkinseed Sunfish | 1.5 | Colorful, eager shallow-water fish around rocky coves. |
| Brook Trout | 2 | Cold-water fish; worm-like markings and white-edged fins. |
| Largemouth Bass | 2.5 | Ambusher around logs/lily pads; sudden strikes. |
| Rainbow Trout | 3 | Fast runs and aerial leaps. |
| Brown Trout | 3.5 | Wary fish around deep rocky edges; patient presentation. |
| Lake Trout (Lakers) | 4 | Deep cold-water heavyweight; heavy jigs/deep-water gear. |
| Muskellunge (Muskie) | 4.5 | Large toothy predator; big lures and powerful fights. |

**Boss: “The Summit Leviathan” — giant lake sturgeon, difficulty 5/5.**
Weight range, artwork, bait recipe and detailed encounter are open.

## Captain Thurl's Grotto

User's fictional lore: beneath a forgotten, crumbling sea arch, hidden by a
curtain of hanging salt-ivy, lies a subterranean tidal cave fed by cool back-bay
springs and warm ocean surges. Captain Thurl, a rogue pirate, hid treasure here.
Centuries of gold and copper coins and decaying rum barrels in mineral-rich
water have produced strange, shiny, treasure-obsessed life. This is fantasy
worldbuilding, not a biological explanation for the grounded areas.

| Species | Difficulty / 5 | Appearance and behavior |
| --- | --- | --- |
| Doubloon Darter | 1 | Coin-sized, metallic gold; flashing schools; bites shiny bait. |
| Barnacle-Back Goby | 1.5 | Barnacle-encrusted, copper patina; grips rocks and treasure chests. |
| Copper-Gill Flounder | 2.5 | Dark mud camouflage; copper-red gills flash as it darts away. |
| Rum-Barrel Drum | 3 | Stout body, oak-brown stripes; hollow booming swim-bladder sound. |
| Cutlass Snapper | 3.5 | Curved metallic dorsal fin; sudden lateral cuts. |
| Gilded Sea Raven | 4 | Spiky, gold and midnight-black fins; heavy strikes from crevices. |
| Jeweled Coral-Prowler | 4.5 | Gem-like scales; agile runs around chains and ironwork. |
| Black-Powder Eel | 4.8 | Jet black, orange eyes; short explosive bursts of speed. |

**Boss: “Old Thurl's Curse” — the Sovereign Iron-Maw, difficulty 5/5.**
Boss anatomy, weight, bait recipe and final encounter remain open.

## Current milestone and next decisions

Implemented prototype: separate static title screen with PLAY and a fishing
screen headed CHINCOTEAGUE BAY, one flounder, cast/wait/hook/repeat, weights,
persistent personal best shown on ready/catch screens, and a session catch
counter on fishing screens. Ready/waiting/bite now have an original
pixel scene with a fisherman, boat, marshes, sky, water and small float. The
title retains its large bobber; catch reveal retains its flounder. Gentle float
bobbing and shifting water ripples animate the fishing scene. Bite markers stay
steady with a lowered float; title and catch reveal remain static.
Adam confirmed the counter works and the best survives restarting the laptop
emulator at scale 1. The current screen layout is temporary.

This document adds no gameplay implementation. Pause feature additions while
agreeing on design. Next choose one small milestone together: screen/navigation
structure, first animated fishing scene, or a minimal Chincoteague species system.
Do not implement all these systems at once. Revisit recipe rules, fight tuning,
economy, daily rotation and endgame rewards when their milestones arrive.
