# P22 MVP-A Pet2D Scene Skeleton

Baseline: `5135e813dff45f6a2f0fa7c334d651c7fca0c0fa feat(petegg): add internal save syscfg ab poc`.

P22 adds a reusable MVP-A Pet2D scene skeleton on top of the P18/P19 handoff and dirty-rect work. It is
a structure POC for later MVP-A HOME/Observe development, not the product HOME/Observe scene, not a full
Pet2D runtime enablement, and not a formal resource route.

## Goals And Non-Goals

Implemented goals:
- Debug-only manual entry into a reusable scene skeleton.
- LVGL -> PET2D -> LVGL display-owner handoff.
- Bounded scene context with placeholder pet position, pose, dirty rect, timeout and stats.
- LEFT_UP / RIGHT_DOWN movement, OK pose toggle, CANCEL exit and 4-second timeout exit.
- Small dirty-rect / scratch-buffer rendering with no full framebuffer.

Explicit non-goals:
- HOME/Observe gameplay, pet growth state machine, formal resources, background map scrolling or IMB
  acceleration.
- External Flash / virfat / raw NOR resource package work.
- PNG/JPG/GIF/JSON runtime decoding.
- Pet-state persistence or production use of P21 syscfg item IDs 206/207.
- NFC, audio/speaker or real BLE two-board integration.

## New Module

`apps/watch/pet2d_scene/pet2d_mvp_a_scene_skeleton.*` owns the skeleton:

- `pet2d_mvp_a_scene_skeleton_enter()`
- `pet2d_mvp_a_scene_skeleton_tick(now_ms)`
- `pet2d_mvp_a_scene_skeleton_handle_key(event)`
- `pet2d_mvp_a_scene_skeleton_exit()`
- `pet2d_mvp_a_scene_skeleton_is_active()`
- `pet2d_mvp_a_scene_skeleton_get_state()`
- `pet2d_mvp_a_scene_skeleton_get_stats()`
- `pet2d_mvp_a_scene_skeleton_self_test()`

The scene uses a 96x64 bounded stage patch and a 32x32 placeholder pet. Initial entry redraws the whole
96x64 patch; later movement and pose updates redraw only the old/new pet dirty union. With the committed
real-flush gate disabled, the same state machine runs and counts the flush as skipped rather than writing
the panel.

## Debug Entry

`apps/watch/mvp_a/core/mvp_a_debug.c/.h` adds the manual Debug-page action `P22 Scene`.

Safety properties:
- It is only reachable from the Debug page.
- It is not run at boot.
- It is not used by the normal HOME/LVGL path.
- It does not write save data.
- With `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`, it does not write the LCD.

## LVGL Handoff

Entry flow:

1. Debug action calls `pet2d_mvp_a_scene_skeleton_enter()`.
2. The MVP-A LVGL shell releases `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI`.
3. The skeleton acquires `PET_DISPLAY_OWNER_PET2D`.
4. The skeleton initializes context, draws a bounded patch, and enters `RUNNING`.

Exit flow:

1. CANCEL, timeout or error calls the skeleton exit helper.
2. PET2D releases display ownership.
3. The LVGL shell is asked to refresh.
4. The existing Debug page can reacquire owner and redraw.

`apps/watch/mvp_a/core/mvp_a_app.c` routes ticks and keys to the skeleton only while
`pet2d_mvp_a_scene_skeleton_is_active()` is true.

## Input Behavior

- `LEFT_UP`: move placeholder pet left by 8 pixels within the 96x64 stage.
- `RIGHT_DOWN`: move placeholder pet right by 8 pixels within the 96x64 stage.
- `OK`: toggle placeholder pose through idle, happy and blink.
- `CANCEL`: exit the skeleton and request LVGL refresh.
- Timeout: auto-exit after 4 seconds.

## Stats

`pet2d_mvp_a_scene_stats_t` records:

- enter/exit/error/tick/key/action counts;
- frame and render counts;
- flush success/fail/skipped counts;
- logic/render/flush/frame total and max timings;
- last enter/exit/duration and max duration;
- last dirty rect, pet position, pose, state, result and exit reason.

The board log line is intentionally concise:

```text
[PET2D_MVP_A_SCENE] frame=... pose=... pet=... dirty=... ret=... owner=...
[PET2D_MVP_A_SCENE] exit reason=... release_ret=... duration=... frames=... keys=... owner=...
```

## Self-Test And Capability Snapshot

Self-test adds `PET_SELFTEST_MVP_A_SCENE_SKELETON`. The test verifies:

- enter -> running;
- RIGHT_DOWN movement;
- OK pose toggle;
- CANCEL exit;
- second enter and timeout exit;
- stats counters and exit reason.

Capability snapshot additions:

- `has_mvp_a_scene_skeleton = 1`
- `mvp_a_scene_skeleton_debug_entry = 1`
- `mvp_a_scene_skeleton_real_board_verified = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`

`mvp_a_scene_skeleton_real_board_verified` remains 0 in the side-effect-free capability snapshot because
the snapshot does not know whether this board has run the latest manual real-flush smoke test.
`p22_manual_real_board_verified = 1` is recorded only in this engineering report from the COM3 manual
test evidence.
`pet2d_runtime_enabled` remains 0 because the skeleton is a POC shell, not the complete runtime.

## Verification

Syntax-only checks passed:

- `apps/watch/pet2d_scene/pet2d_mvp_a_scene_skeleton_compile_check.c`
- `apps/watch/pet2d_scene/pet2d_scene_compile_check.c`
- `apps/watch/pet2d_scene/pet2d_perf_poc_compile_check.c`
- `apps/watch/pet_selftest/pet_selftest_compile_check.c`
- `apps/watch/pet_save_jieli/pet_save_jieli_syscfg_compile_check.c`

The skeleton implementation itself also parsed with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0` and with a
temporary syntax-only `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` define.

A default `.vscode\winmk.bat all` build also generated `cpu/br28/tools/sdk.elf`, so the P22 source and
link path are valid. The remaining log hits were the known resource/download post-processing environment
messages such as stack-size warnings, `The system cannot execute the specified program`, missing files
and `open file JL failed`; they were not P22 compile or link failures.

## Real Board Smoke Test

Executed on COM3 at 1000000 baud with a temporary local
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` build. A local `packres.exe` tool replacement was used only to
work around the host-side security block during the download; it must be restored before commit.

Observed serial summary:

```text
[PET2D_MVP_A_SCENE] enter start owner=2
[MVP_A][LVGL_OWNER] release ret=0 owner=0
[PET2D_MVP_A_SCENE] frame=1 pose=0 pet=211,211 dirty=96x64 ret=0 owner=3
[PET2D_MVP_A_SCENE] enter ok owner=3 timeout=4000 patch=96x64
[PET2D_MVP_A_SCENE] key=0 pose=0 pet=203,211 owner=3
[PET2D_MVP_A_SCENE] frame=11 pose=0 pet=203,211 dirty=40x32 ret=0 owner=3
[PET2D_MVP_A_SCENE] key=1 pose=0 pet=219,211 owner=3
[PET2D_MVP_A_SCENE] frame=33 pose=0 pet=219,211 dirty=40x32 ret=0 owner=3
[PET2D_MVP_A_SCENE] exit reason=2 release_ret=0 duration=4000 frames=35 keys=6 owner=0
[MVP_A][LVGL_OWNER] acquired owner=2
```

Follow-up COM3 pass with the user pressing UP, DOWN, OK and CANCEL during the active scene:

```text
[00:01:23.586][PET2D_MVP_A_SCENE] enter start owner=2
[00:01:23.587][MVP_A][LVGL_OWNER] release ret=0 owner=0
[00:01:23.611][PET2D_MVP_A_SCENE] enter ok owner=3 timeout=4000 patch=96x64
[00:01:24.306][PET2D_MVP_A_SCENE] key=0 pose=0 pet=203,211 owner=3
[00:01:24.323][PET2D_MVP_A_SCENE] frame=68 pose=0 pet=203,211 dirty=40x32 ret=0 owner=3
[00:01:24.715][PET2D_MVP_A_SCENE] key=1 pose=0 pet=211,211 owner=3
[00:01:24.724][PET2D_MVP_A_SCENE] frame=70 pose=0 pet=211,211 dirty=40x32 ret=0 owner=3
[00:01:25.067][PET2D_MVP_A_SCENE] key=2 pose=1 pet=211,211 owner=3
[00:01:25.081][PET2D_MVP_A_SCENE] frame=72 pose=1 pet=211,211 dirty=32x32 ret=0 owner=3
[00:01:25.427][PET2D_MVP_A_SCENE] exit reason=1 release_ret=0 duration=1840 frames=73 keys=37 owner=0
[00:01:25.430][MVP_A][LVGL_OWNER] acquired owner=2
[00:01:25.951][MVP_A][LVGL] render scene=9 title=Debug prompt=Scene Skeleton action=P22 Scene
```

Result:
- `P22 Scene` entered and acquired `PET_DISPLAY_OWNER_PET2D`.
- The initial 96x64 patch rendered with `ret=0`.
- LEFT_UP / RIGHT_DOWN movement rendered expected dirty rects: 40x32 for movement, 32x32 for steady
  refresh.
- OK toggled the placeholder pose from `pose=0` to `pose=1` and rendered a 32x32 dirty rect.
- CANCEL exited immediately with `exit reason=1`, released PET2D ownership and LVGL reacquired owner.
- Timeout exit after 4 seconds was also observed in the first pass.
- During scene ownership, LVGL reacquire attempts returning `ret=10 owner=3` are expected owner
  protection, not a fault.
- No panic, assert, WDT, HardFault or exception was observed in the captured serial log.

## Safety Boundary

Committed state:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`
- P22 Scene Skeleton is Debug manual-only.
- HOME/Observe is not enabled.
- Full Pet2D runtime is not enabled.
- External Flash / virfat / raw NOR resource work remains paused.
- NFC, audio/speaker and real BLE two-board validation remain Future Scope.

## P21 Relationship

P22 does not write pet state and does not use P21 syscfg items 206/207 as production save slots. P21
remains the internal-save A/B POC; a later phase must define the real pet save schema, migration policy
and low-battery write veto before scene state persistence is enabled.

## Remaining Risks

- The placeholder stage is a bounded 96x64 patch, not a full HOME/Observe background restore strategy.
- The pose toggle is a generated-pixel placeholder, not a formal animation/resource table.
- Real board verification should be run with a temporary real-flush build before treating P22 as
  hardware-verified.
- Engineering Debug entry policy still needs a production decision.

## Next Steps

1. P23 MVP-A HOME/Observe skeleton or renderer-facing scene contract, still using internal fixtures.
2. P24 formal internal resource fixture / transparency policy without external Flash.
3. P25 power-fail and low-battery save validation if scene state persistence becomes necessary.
