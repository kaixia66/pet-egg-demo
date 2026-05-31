# P12 Repeated Tiny Flush + Dirty Rect Alignment POC

## Scope

P12 extends the P10/P11 manual LCD smoke-test path with repeated tiny flush and dirty-rect alignment
coverage. The committed source still keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, so no default
build writes the LCD through this path.

Current hardware limits are now part of the acceptance baseline:
- The current development board has no NFC.
- The current development board has no speaker.
- Only one board is available, so real BT/BLE two-board connection testing is not possible.
- Real NFC, real audio/SFX and real BLE two-board link are Future Scope. P12 keeps only fake/stub/self-test
  coverage for those paths.

## Dirty Rect Helper

New helper: `apps/watch/pet2d_boundary/pet2d_dirty_rect_poc.*`.

It provides:
- 16x16 pattern: white border, four color quadrants and diagonals.
- 32x32 pattern: white border and multicolor 4-pixel checker tiles.
- 64x64 pattern: white border, horizontal/vertical stripes and a center cross.
- Rect cases:
  - center aligned;
  - odd coordinate case at 101/103;
  - near-edge but inside bounds;
  - out-of-bounds case for rejection checks.

The helper uses caller-owned buffers only. It does not malloc, does not allocate a full framebuffer,
does not load resources and does not enter Pet2D runtime.

## Repeated Probe

New boundary APIs:
- `pet2d_boundary_repeated_flush_probe()`
- `pet2d_boundary_repeated_flush_default_probe()`
- `pet2d_boundary_reset_repeated_flush_stats()`
- `pet2d_boundary_get_repeated_flush_stats()`
- `pet2d_boundary_repeated_flush_gate_self_test()`

The repeated probe is finite and bounded by `PET2D_DIRTY_RECT_REPEAT_MAX` (60). With the real-flush macro
disabled it returns `PET_RESULT_UNSUPPORTED` before touching LCD hardware. With a local macro-enabled
board build, it still requires display-owner guard and stops on the first failed flush.

Stats captured:
- attempt/success/fail counters;
- last pattern size;
- requested repeat count;
- successful flush count;
- first failing index;
- last rect;
- last result;
- total and max single flush duration fields for future board runs.

## Self-Test Behavior

Self-test coverage validates:
- pattern generation for 16/32/64 surfaces;
- rect bounds for center, odd coordinate and near-edge cases;
- out-of-bounds rejection;
- repeated-probe stats reset/get behavior;
- macro-off gate behavior, where real LCD writes are skipped and recorded as unsupported.

The platform self-test aggregator adds `PET_SELFTEST_REPEATED_FLUSH_GATE` and the capability bit
`has_dirty_rect_poc_gate`. `pet_selftest_run_all()` still does not perform real LCD writes.

## Manual Board-Test Plan

For a local-only board test:
1. Temporarily enable `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`.
2. Temporarily add a Debug action that releases LVGL owner and calls the repeated probe.
3. Test 16x16 center repeated 10 times.
4. Test 32x32 center repeated 10 times.
5. Test 64x64 center repeated 10 times.
6. Optionally test 32x32 odd x/y repeated 10 times.
7. Optionally test near-edge inside rect.
8. Verify the out-of-bounds case returns an error and does not write the LCD.
9. Confirm no panic, assert, WDT, HardFault, reset, infinite loop or continuous repaint.
10. Restore the macro to 0 and remove any temporary Debug action before commit.

## Current Board Result

A temporary local board build enabled `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` and added a non-committed
MVP-A Debug `P12` action. The test passed on COM3 at 1000000 baud.

Observed log summary:
- `pattern=0 rect=219,219 16x16 repeat=10 ret=0`
- `pattern=1 rect=211,211 32x32 repeat=10 ret=0`
- `pattern=2 rect=195,195 64x64 repeat=10 ret=0`
- `odd rect=101,103 32x32 repeat=10 ret=0`
- `oob rect=438,438 32x32 repeat=10 ret=1 expected_invalid=1`
- `stats attempts=4 success=4 fail=0 last_size=32 repeat=10 ok=10 fail_idx=65535 result=0 total_ms=220 max_ms=30`
- `display attempts=40 success=40 fail=0 last_ret=0 duration=20 driver=0`
- `debug trigger ret=0 owner=0`

The user confirmed the repeated dirty-rect patterns were visible. No panic, assert, WDT, HardFault or
reset keyword was observed during the watch window. LVGL owner was reacquired after the probe and the
Debug page re-rendered.

Board-operation note: the user reached Debug by pressing the physical LEFT/UP key where the earlier
instruction expected RIGHT/DOWN to have the same navigation effect. Treat physical key labeling and
logical up/down mapping as still requiring confirmation before changing default key behavior.

## Safety Boundary

Committed source state:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`
- no default Debug UI trigger
- no LVGL flush callback replacement
- no HOME/Observe rendering
- no formal resource loading
- no full framebuffer allocation
- no VM/Flash/syscfg writes
- no real NFC/audio/BLE hardware path

## Follow-Up

Recommended next stages:
1. P13 resource sprite to minimal Pet2D surface.
2. P14 minimal sprite movement + key input.
3. P15 high-res motion/performance POC.

Before increasing rectangle size or frame count, run the P12 board plan and record stability, LVGL
recovery behavior, visible alignment and timing stats.
