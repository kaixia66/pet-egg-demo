# P18 Pet2D Scene Mode / LVGL Handoff POC

## Scope

P18 adds a controlled Pet2D test-scene handoff between the existing MVP-A LVGL shell and the minimal
Pet2D-boundary movement/resource fixture. It is a scene-mode POC only: no HOME/Observe, no full Pet2D
runtime, no external Flash, no formal resource package and no image decoding.

## Scene Mode Design

New module: `apps/watch/pet2d_scene/`.

The scene state machine is:
- `IDLE`
- `ENTERING`
- `RUNNING`
- `EXITING`
- `DONE`
- `ERROR`

Public APIs:
- `pet2d_scene_enter_test()`
- `pet2d_scene_tick(now_ms)`
- `pet2d_scene_handle_key(event)`
- `pet2d_scene_exit()`
- `pet2d_scene_is_active()`
- `pet2d_scene_get_state()`
- `pet2d_scene_get_stats()`
- `pet2d_scene_self_test()`

The scene uses the existing P13/P15 movement helper and compiled resource fixture. It does not allocate a
full framebuffer and does not depend on files, virfat, raw NOR or external Flash.

## LVGL Handoff Flow

Enter flow:
1. Debug action calls `pet2d_scene_enter_test()`.
2. Scene calls `mvp_a_lvgl_shell_release_display_owner()`.
3. Scene acquires `PET_DISPLAY_OWNER_PET2D`.
4. Scene initializes `pet2d_movement_poc`.
5. Scene renders once through the existing gated real-flush path.

Running flow:
- `mvp_a_lvgl_shell_tick()` calls `mvp_a_app_tick()` before LVGL repaint checks.
- `mvp_a_app_tick()` ticks the active Pet2D scene.
- The scene renders at a bounded 250 ms interval and exits automatically after 4 seconds.
- LVGL repaint requests are still owner-guarded, so LVGL cannot overwrite the panel while PET2D owns it.

Exit flow:
1. CANCEL, timeout or error calls the internal scene exit path.
2. Scene releases `PET_DISPLAY_OWNER_PET2D`.
3. Scene requests LVGL refresh.
4. The LVGL shell can reacquire `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` and redraw the Debug page.

## Input Handling

When `pet2d_scene_is_active()` is true, `mvp_a_app_key_event()` maps MVP-A keys into PetKey events:
- `MVP_A_KEY_UP` -> `PET_KEY_LEFT_UP`
- `MVP_A_KEY_DOWN` -> `PET_KEY_RIGHT_DOWN`
- `MVP_A_KEY_CONFIRM` -> `PET_KEY_OK`
- `MVP_A_KEY_BACK` -> `PET_KEY_CANCEL`

Scene behavior:
- LEFT_UP moves the sprite left through the P15 movement helper.
- RIGHT_DOWN moves the sprite right through the P15 movement helper.
- OK toggles the P13/P15 pattern.
- CANCEL exits immediately.

## Stats

`pet2d_scene_stats_t` records:
- enter / exit / error count
- tick / key / render count
- flush success / failure count
- last enter / exit / duration / max duration
- last key-to-flush coarse latency
- last exit reason
- last state

`PET_RESULT_UNSUPPORTED` from the real-flush gate is treated as a skipped render in committed macro-off
builds, not as a scene failure.

## Self-Test / Gate Behavior

Self-test case: `PET_SELFTEST_PET2D_SCENE_HANDOFF`.

The self-test validates:
- enter transition to `RUNNING`
- key handling
- CANCEL exit
- timeout exit
- stats updates
- owner release after scene exit

Capability snapshot adds `has_pet2d_scene_handoff = 1`, while keeping:
- `real_lcd_flush_enabled = 0`
- `external_flash_resource_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Debug Entry

The MVP-A Debug page now includes a retained manual-only action named `P18 Scene`.

Safety boundary:
- It is not entered automatically.
- It is only reachable from the Debug page.
- It does not write save data.
- With `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`, the scene exercises owner/state flow and logs skipped
  renders without touching the LCD.

## Board-Test Status

Executed on COM3 at 1000000 baud with temporary `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` board builds.
The committed/default macro was restored to 0 after testing.

Observed results:
- Debug action `P18 Scene` entered the scene manually from the Debug page.
- Enter path released LVGL owner, acquired `PET_DISPLAY_OWNER_PET2D`, rendered the fixture and kept LVGL
  repaint attempts owner-blocked during the scene window.
- The first movement build showed sprite extension/trailing when moving. This was fixed by rendering the
  old/new dirty union and restoring the union background before drawing the current sprite.
- A second trailing issue came from passing a 64-pixel pitch for a 48-pixel dirty rectangle to the real
  LCD path; the board expects a tightly packed buffer for `lcd_draw_area`, so the movement render now
  passes `surface.pitch_pixels = dirty_w`.
- A temporary full-safe-area background clear took too long and caused a `PET_RESULT_BUSY`/`ret=10`
  overlap with the scene tick. That experiment was discarded; the final POC uses a bounded 64x64 local
  scene patch and keeps movement inside that patch.
- Repeated OK presses after scene exit correctly re-enter the manual Debug action and draw the sprite
  again. This is expected behavior, not LVGL restore failure.
- Serial logs showed repeated `render ret=0 owner=3`, timeout exit with `exit reason=2 release_ret=0
  duration=4000 owner=0`, LVGL owner reacquire and Debug page render.
- No panic/assert/WDT/HardFault/exception was observed in the final board-test logs.
- Source was restored to `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0` after board testing. The final
  safe-source build generated `sdk.elf` but the SDK post-processing reported the device offline and only
  packaged files, so the physical board may still contain the temporary real-flush test image.

## Submission Safety Boundary

Committed/default source must keep:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- no HOME/Observe
- no full Pet2D runtime
- no external Flash / virfat / raw NOR work
- no formal resource package
- no image decoding
- no full framebuffer
- no LVGL flush callback replacement
- no VM/Flash/syscfg writes
- no NFC/audio/real BLE integration

## Remaining Risks

- The POC scene is intentionally only a 64x64 local test patch, not a full HOME/Observe scene.
- The scene still relies on the minimal P13/P15 fixture and restores only its local patch background, not
  a complete scene background or arbitrary LVGL pixels.
- Tick/render cadence is conservative; high-resolution motion and performance remain future work.
- Debug entry policy should be finalized in a later engineering test menu phase.
- External Flash resource work remains paused by P17S.

## Suggested Next Steps

1. P19 high-res motion/performance POC.
2. P20 engineering test menu / integration report.
3. P21 internal save/syscfg POC, if still needed.
