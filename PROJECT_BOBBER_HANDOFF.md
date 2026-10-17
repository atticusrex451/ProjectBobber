# Project Bobber — development handoff

Updated October 17, 2026. Read this before starting a new development chat.

## Purpose and working style

A relaxed handheld touchscreen fishing game as a Christmas gift for Adam's
retired father, who fishes around Chincoteague and catches flounder each summer.
This is a collaborative learning project. Explain changes, work one small,
testable milestone at a time, and let Adam participate. Keep the tone friendly.
Do not rewrite the whole game or make major changes without checking first.
No daily care penalties. Use original fishing artwork and respect upstream licenses.

## Repository and hardware

- Repository: https://github.com/atticusrex451/ProjectBobber
- Upstream: https://github.com/Squithy/TamaPokeLAN
- Development base: `dev-fishing`. Preserve `main` as the upstream baseline.
- Intended hardware: Waveshare ESP32-S3 Touch AMOLED 1.75-inch Standard,
  466 x 466 circular touch display, rechargeable battery, possible audio/SD.
- Planned enclosure: rounded bobber, chartreuse top and orange bottom,
  printed on a Bambu Lab A1 mini. No CAD or hardware testing yet.

## Current implementation and verified progress

The upstream code remains in place. `TamaPoke.ino` includes `bobber_welcome.h`.
With `BOBBER_WELCOME` set to 1, setup initializes display/touch then returns
before pet initialization; loop runs only the Bobber prototype.
Setting it to 0 restores the upstream path for development.

Implemented: original welcome screen; CAST; three-second wait; persistent bite
prompt; HOOK; flounder illustration and catch message; repeat casting. BACK
cancels while waiting. Button actions occur on release; dragging off cancels.
A press spanning a state change cannot activate the newly displayed action.
No weights, catch journal, persistent fishing records, upgrades, or locations yet.

Adam built and ran the original emulator in Ubuntu/WSL2, selected Kanto and
Charmander, then built and manually verified each Bobber milestone, including
the full catch loop and revised fish artwork. These are user-observed tests;
the local agent could not execute WSL due to Access denied. Hardware is untested.
Checkpoint `b6ee295` contains the first complete fishing interaction.

## Build and run

Linux dependencies: C++17 compiler (g++), Python 3, SDL2 development files,
and Bash. On Ubuntu/Debian, provision `build-essential python3 libsdl2-dev`
using the environment's supported installation method.

From the repository root:

```bash
bash tools/emu/build.sh
tools/emu/tamapoke-emu --scale 1
```

Adam prefers **scale 1**. Do not prefix the compiled emulator command with bash.
The desktop checkout is at `C:\Users\Adam\Documents\Codex\ProjectBobber`;
in WSL it is `/mnt/c/Users/Adam/Documents/Codex/ProjectBobber`.
The Windows checkout initially converted build.sh to CRLF; it was normalized
to LF and repository-local core.autocrlf was disabled. Keep shell scripts LF.

The upstream `--shot` mode prepares Pokemon state and calls upstream rendering;
do not treat it as a validated Bobber screenshot/test. Cloud work should first
build the current branch, then inspect/adapt a small Bobber-specific headless
validation path if needed. A cloud build alone does not establish touch or
visible-window correctness. Existing upstream tests are not Bobber acceptance tests.

## Cloud transition and continuity

The October 16-17 development chat used local Windows files, with emulator
commands run by Adam in WSL. A separate earlier cloud onboarding chat reported
a successful upstream emulator build and partial tests, then was stopped.
Its environment draft/publication status has NOT been verified.

Use GitHub as the durable code and project-context record. Start cloud tasks
from the latest `dev-fishing`, or a feature branch based on it, and integrate
reviewed work back into `dev-fishing`. Do not force-push or target main.
End milestones with a commit/push or reviewed PR and update this handoff.
Do not assume separate tasks share live files or full conversation history.
Keep the existing local chat and checkout as reference; no deletion is needed.
Local saves, installed tools, and generated emulator binaries are not synced
by GitHub and should not be committed just to move development between devices.

## Next small gameplay milestone

Fish weights and a session personal-best record, followed by persistent saving
as a separate milestone. First complete and verify the cloud transition.
