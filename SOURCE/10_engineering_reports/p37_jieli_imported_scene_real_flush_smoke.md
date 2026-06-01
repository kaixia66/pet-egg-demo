# P37 Jieli Imported Scene Real-flush Smoke

## Baseline

- Dev board baseline: `982d7a87dc6402a1abf3d223791fdebbdef2789e`
  `feat(petegg): import simulator HOME observe placeholder scene`
- Branch: `codex/p37-jieli-imported-scene-real-flush-smoke`
- Simulator source references remain:
  - P35 `0cf4fdc62690da5660e2389a73cbd914ccbfe64f`
  - P34 `cb5f9a61639292e6c459dc384fa20ecf2f8c3049`

## Goal

P37 performs the real-board smoke test that P36 left as the next step. It uses a temporary local
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` build to validate that the imported simulator-developed bounded
HOME/Observe placeholder scene can run through the existing LVGL -> PET2D -> LVGL handoff on the Jieli
board.

P37 is not complete HOME/Observe, not production gameplay and not full Pet2D runtime enablement.

## Runtime Hardening From Board Logs

The first P37 smoke build exposed two board-only recovery issues:

- the first bounded real-flush call can return `PET_RESULT_BUSY` while the display pipeline settles after
  LVGL releases ownership;
- CANCEL/TIMEOUT can move the imported model to `DONE` before the public exit helper is called, so an
  overly strict "inactive means return" guard can skip PET2D owner release.

The imported scene now:

- treats `PET_RESULT_BUSY` and `PET_RESULT_NOT_READY` from the bounded real-flush call as skipped flushes
  rather than fatal scene errors;
- suppresses idle no-change re-flushes so the 160x96 enter patch is not re-sent every tick;
- releases PET2D ownership if the owner is still `PET_DISPLAY_OWNER_PET2D`, even when the model has
  already reached `DONE` or `ERROR`.

These changes keep the scene bounded and do not add any production HOME/Observe path.

## Board Smoke Setup

- Board: Jieli 701N development board
- UART: COM3 at 1000000 baud
- Build mode for smoke: temporary `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`
- Debug entry: manual `P36 Import`
- Submission state: restored to `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`

Download evidence from the smoke build:

- internal flash online: `ef4015`, 2M
- external flash detected by downloader: `ef4018`, 16M
- firmware write blocks completed
- external flash data stage completed as part of the standard SDK download script

P37 does not rely on external Flash resources; the imported scene uses compiled placeholder patterns.

## Smoke Procedure

1. Download temporary real-flush firmware.
2. Boot the board and open COM3 at 1000000 baud.
3. Navigate to the Debug page.
4. Trigger `P36 Import`.
5. Press LEFT_UP, RIGHT_DOWN, OK and CANCEL.
6. Re-enter `P36 Import` and wait for timeout.
7. Confirm LVGL Debug page recovery and check for fault strings.

## Serial Evidence

Representative log lines:

```text
[PET2D_MVP_A_IMPORT] enter start owner=2 manifest=stage160x96 pet32 initial=64,32
[PET2D_MVP_A_IMPORT] frame=1 state=OBSERVE_IDLE pose=IDLE pet=64,32 dirty=160x96 cmd_count=0 ret=0 raw_ret=0 owner=3
[PET2D_MVP_A_IMPORT] enter ok owner=3 screen=147,179 timeout=4000 dirty=0x0
[PET2D_MVP_A_IMPORT] input key=0 state=OBSERVE_MOVE_LEFT pose=STEP pet=56,32 dirty=40x32 exit=NONE
[PET2D_MVP_A_IMPORT] input key=1 state=OBSERVE_MOVE_RIGHT pose=STEP pet=64,32 dirty=40x32 exit=NONE
[PET2D_MVP_A_IMPORT] input key=2 state=OBSERVE_ACTION pose=HAPPY pet=64,32 dirty=32x32 exit=NONE
[PET2D_MVP_A_IMPORT] input key=3 state=DONE pose=IDLE pet=64,32 dirty=0x0 exit=CANCEL
[PET2D_MVP_A_IMPORT] exit reason=CANCEL release_ret=0 duration=2940 frames=7 keys=4 owner=0
[MVP_A][LVGL_OWNER] acquired owner=2
[MVP_A][LVGL] render scene=9 title=Debug prompt=Imported HOME action=P36 Import
[PET2D_MVP_A_IMPORT] render skipped state=DONE exit=TIMEOUT skipped=394
[PET2D_MVP_A_IMPORT] exit reason=TIMEOUT release_ret=0 duration=4000 frames=7 keys=3 owner=0
[MVP_A][LVGL_OWNER] acquired owner=2
```

No `panic`, `assert`, `WDT`, `HardFault` or `exception` strings were found in the P37 smoke log.

## Smoke Result Matrix

| Check | Result |
| --- | --- |
| Debug manual `P36 Import` entry | PASS |
| LVGL release -> PET2D acquire | PASS |
| Initial 160x96 bounded patch | PASS |
| LEFT_UP 8px move / 40x32 dirty | PASS |
| RIGHT_DOWN 8px move / 40x32 dirty | PASS |
| OK placeholder action / 32x32 dirty | PASS |
| CANCEL exit | PASS |
| Timeout exit | PASS |
| PET2D owner release | PASS |
| LVGL Debug page reacquire/redraw | PASS |
| Fault strings | PASS, none observed |

## Compile / Build Results

Syntax-only checks executed after restoring the committed macro state:

- `pet2d_mvp_a_home_observe_imported_compile_check.c`: passed.
- `pet2d_mvp_a_renderer_contract_compile_check.c`: passed.
- `pet2d_mvp_a_scene_action_loop_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton_compile_check.c`: passed.
- `pet2d_scene_compile_check.c`: passed.
- `pet2d_perf_poc_compile_check.c`: passed.
- `pet_selftest_compile_check.c`: passed.
- `pet_save_jieli_syscfg_compile_check.c`: passed.
- `tools/sim_consistency` syntax-only checks: passed where applicable with the Jieli clang front-end.

Full Jieli build/download was executed for the temporary smoke firmware. It generated `sdk.elf`, wrote
the firmware to the board and completed the SDK external data stage. Build/download artifacts were
removed before commit.

## Safety Boundary

Committed source keeps:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

P37 does not:

- enable complete HOME/Observe;
- enable full Pet2D runtime;
- add a boot/default HOME path;
- add production pet resources, formal animation tables, transparency/RLE/compression policy or IMB
  acceleration;
- restore external Flash / virfat / raw NOR resource loading;
- modify `download/watch`, `fat_comm`, external resource package or raw NOR paths;
- write formal pet-state saves;
- use P21 syscfg item 206/207 as production pet slots;
- connect real NFC, audio or BLE.

## Remaining Gaps

- The scene is still a bounded 160x96 placeholder, not full-screen HOME/Observe.
- The current real-flush smoke is manual and log-based, not an automated hardware regression.
- Renderer scheduling still needs a later policy for busy display periods instead of simple skip
  accounting.
- Formal resources, animation tables, background restore, map/camera scrolling, IMB acceleration and
  final UX timing remain unimplemented.
- External Flash / virfat / raw NOR resource work remains paused.
- Real NFC, audio and BLE two-board work remain Future Scope.

## P38 Recommendation

Proceed to P38 with one of these narrow tracks:

1. shared-core reconciliation between the simulator portable P34/P35 source and the Jieli imported
   adapter; or
2. bounded full-screen-ish layout exploration that still avoids full HOME/Observe, external Flash,
   formal resources and full Pet2D runtime enablement.
