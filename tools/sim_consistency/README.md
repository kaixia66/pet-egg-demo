# P25 Sim Consistency Harness

This directory contains a minimal host-side consistency harness for the PetEgg MVP-A scene/action/render
contract. It is a bring-up scaffold, not a complete PC simulator.

## Scope

The harness verifies that the P22-P24 contract can be replayed without Jieli SDK private headers:

- scene state/action replay for ENTER, LEFT_UP, RIGHT_DOWN, OK, CANCEL and timeout;
- P24 renderer contract checks for stage patch, dirty union, pose-only dirty and idle skip;
- fixed screen profile fixture;
- key replay fixture using the shared `PetProductKey` enum;
- save slot header fixture using the shared save ABI;
- packet fixture using the shared packet ABI.

It does not open an SDL window, draw pixels, play audio, start BLE/NFC, read resources, write syscfg or
enable HOME/Observe.

## Build

From this directory, run:

```bat
build_sim_consistency.bat
```

The script looks for `clang`, `gcc`, `cl`, or a compiler named in `CC`. The current project machine did
not expose a normal host compiler in `PATH`, so P25 is verified with syntax-only compile checks here.

## Expected Output

The golden text is checked by `sim_consistency_main.c` and also committed as:

```text
golden/p25_scene_replay_expected.txt
```

The output is intentionally plain text so future simulator work can compare it from any test runner.
