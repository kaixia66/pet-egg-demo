# P36 Jieli Import of Simulator-developed Scene

## Baseline

- Dev board baseline: `d97bb882364493aa01f420da175910a07088fd53`
  `merge: integrate petegg dev board p1-p25 platform baseline`
- Simulator P35 source commit: `0cf4fdc62690da5660e2389a73cbd914ccbfe64f`
- Simulator P34 scene source commit: `cb5f9a61639292e6c459dc384fa20ecf2f8c3049`
- Branch: `codex/p36-jieli-import-simulator-scene`

## Goal

P36 imports the simulator-developed bounded HOME/Observe placeholder scene contract into the Jieli board
project. It creates a board-side adapter and Debug manual entry for the imported scene while preserving
the existing LVGL -> PET2D -> LVGL owner handoff, dirty-rect rendering path and real-flush gate.

P36 is not complete HOME/Observe, not product gameplay, not a simulator binary port and not the full
Pet2D runtime.

## Imported Manifest Summary

The imported constants come from the simulator P35 export manifest and P34 placeholder scene:

- Scene id: `MVP_A_HOME_OBSERVE_PLACEHOLDER`
- Stage / viewport: `160x96`
- Placeholder pet: `32x32`
- Initial pet position: `64,32`
- Move step: `8px`
- Move action duration: `160ms`
- OK action duration: `300ms`
- Timeout: `4000ms`
- Dirty expectations:
  - enter: `160x96`
  - LEFT_UP / RIGHT_DOWN: typical `40x32`
  - OK pose/action: `32x32`
  - CANCEL / timeout: skip final flush

The imported golden is treated as a contract reference. Host-side CRC lines from the simulator golden are
not treated as Jieli LCD framebuffer CRCs.

## Board-side Adapter

New adapter:

- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_imported.h`
- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_imported.c`
- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_imported_compile_check.c`

The adapter owns:

- imported state names and constants;
- pure model transitions for enter, move, OK action, cancel and timeout;
- render-plan generation through the P24 renderer contract;
- bounded real-flush rendering only when `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` is explicitly enabled;
- runtime stats for enter, key, tick, render, flush success/fail, skipped flush, exit reason and duration.

The side-effect-free self-test uses the pure model only. It does not acquire display ownership and does
not write the LCD.

## Debug Manual Entry

`mvp_a_debug` adds a manual `P36 Import` action. It is an engineering entry only.

On entry:

1. LVGL shell releases display ownership.
2. PET2D acquires `PET_DISPLAY_OWNER_PET2D`.
3. The imported model initializes at pet position `64,32`.
4. A bounded 160x96 stage patch render plan is generated.

During scene activity:

- LEFT_UP maps to `PET_KEY_LEFT_UP` and moves left by 8px.
- RIGHT_DOWN maps to `PET_KEY_RIGHT_DOWN` and moves right by 8px.
- OK maps to `PET_KEY_OK` and toggles the placeholder action/pose.
- CANCEL maps to `PET_KEY_CANCEL` and exits.
- Tick handles action completion and the 4000ms timeout.

On exit:

1. PET2D releases display ownership.
2. LVGL refresh is requested.
3. The Debug page can reacquire and redraw.

If LVGL reacquire is refused while PET2D owns the display, that is expected owner protection.

## Render / Dirty Summary

The adapter reuses the P24 renderer contract:

- enter plan: stage patch plus pet draw, dirty `160x96`;
- move plan: dirty clear/stage restore plus pet draw, dirty old/new union, typically `40x32`;
- OK action: pet draw only, dirty `32x32`;
- idle no-change: zero commands and skipped flush accounting;
- cancel/timeout: no final dirty flush; scene layer handles owner release.

The real-flush path uses a bounded static scratch buffer sized for the 160x96 stage only. It does not
allocate a 454x454 full framebuffer or replace the LVGL flush callback.

## Self-test / Capability Snapshot

Added self-test case:

- `PET_SELFTEST_MVP_A_HOME_OBSERVE_IMPORTED`

It verifies:

- imported stage `160x96`;
- placeholder pet `32x32`;
- initial position `64,32`;
- LEFT/RIGHT 8px movement;
- move dirty rect `40x32`;
- OK dirty rect `32x32`;
- CANCEL exit;
- TIMEOUT exit;
- no display owner modification.

Capability snapshot additions:

- `has_mvp_a_home_observe_imported_scene = 1`
- `mvp_a_home_observe_imported_selftest = 1`
- `mvp_a_home_observe_imported_debug_entry = 1`
- `simulator_scene_import_contract = 1`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

The imported scene bit does not mean HOME/Observe is complete.

## Compile / Build Results

Syntax-only checks executed:

- `pet2d_mvp_a_home_observe_imported_compile_check.c`: passed.
- `pet2d_mvp_a_home_observe_imported.c` with default real-flush gate off: passed.
- `pet2d_mvp_a_home_observe_imported.c` with temporary `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`: passed.
- `pet2d_mvp_a_renderer_contract_compile_check.c`: passed.
- `pet2d_mvp_a_scene_action_loop_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton_compile_check.c`: passed.
- `pet2d_scene_compile_check.c`: passed.
- `pet2d_perf_poc_compile_check.c`: passed.
- `pet_selftest_compile_check.c`: passed.
- `pet_save_jieli_syscfg_compile_check.c`: passed.
- `tools/sim_consistency` syntax-only checks: passed where applicable with the Jieli clang front-end.

Full Jieli build:

- `.vscode\winmk.bat all` generated `cpu/br28/tools/sdk.elf`, so source compile/link passed.
- The post-link resource/download stage still reported the known environment/tooling messages such as
  `open file JL failed`, `The system cannot execute the specified program` and resource copy output.
  These are treated as existing SDK post-processing/download environment issues, not P36 source/link
  failures.
- Build artifacts were cleaned before commit.

## Real-board Verification

No new P36 real-board smoke run was required before this commit. P22 already verified the LVGL/PET2D
owner handoff, basic input, OK, CANCEL, timeout and LVGL recovery path on hardware. P36 adds an imported
bounded scene contract and side-effect-free verification on top of that path.

Recommended later smoke test:

1. Temporarily enable `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`.
2. Enter Debug page.
3. Run `P36 Import`.
4. Verify the 160x96 patch appears.
5. Press LEFT_UP, RIGHT_DOWN, OK and CANCEL.
6. Re-enter and wait for timeout.
7. Confirm LVGL Debug page recovers and no panic/assert/WDT/HardFault occurs.
8. Restore `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`.

## Safety Boundary

Committed source keeps:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

P36 does not:

- enable complete HOME/Observe;
- enable full Pet2D runtime;
- import SDL, C++ simulator runtime, SimBroker, SaveManager or simulator binaries;
- restore external Flash / virfat / raw NOR;
- modify `download/watch`, `fat_comm`, external resource package or raw NOR paths;
- write formal pet-state saves;
- use P21 syscfg item 206/207 as production pet slots;
- connect real NFC, audio or BLE.

## Why P36 Is Not Full HOME/Observe

P36 imports a bounded placeholder contract, not product gameplay. It lacks formal pet resources, real
animation tables, production renderer policy, background restore, map/camera scrolling, persistence,
final interaction design, resource packaging and simulator/Jieli parity proof for the complete shared
core.

## Remaining Gaps

- Real-board P36 smoke test remains optional/pending for a later temporary real-flush build.
- Full HOME/Observe still needs production scene composition, background restore, multi-object dirty
  merging and final UX timing.
- Formal resources, transparency/RLE/compression policy and IMB acceleration remain unimplemented.
- External Flash / virfat / raw NOR resource work remains paused.
- P21 save POC remains a test namespace and is not a production pet schema.
- Real NFC, audio and BLE two-board work remain Future Scope.

## P37 Recommendation

Proceed to a narrow P37 that either:

1. performs board smoke validation and log capture for the imported P36 scene with a temporary real-flush
   build; or
2. starts shared-core reconciliation between the simulator portable P34/P35 source and the Jieli P36
   imported adapter without enabling product HOME/Observe.
