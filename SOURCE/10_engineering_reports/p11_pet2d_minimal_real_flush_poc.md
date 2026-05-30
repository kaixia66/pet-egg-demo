# P11 Pet2D Minimal Real Flush POC

## Scope

P11 adds a minimal Pet2D-boundary visual surface and a manual real-flush probe gate. It does not enable
the full Pet2D runtime, HOME/Observe, resource loading, dirty-rect scheduling, full framebuffer allocation,
LVGL flush replacement, storage, BLE or NFC.

Committed source keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`, so the real LCD path remains disabled by
default.

## Minimal Visual Helper

`apps/watch/pet2d_boundary/pet2d_minimal_visual.*` defines `pet2d_minimal_surface_t` and
`pet2d_minimal_visual_fill_test_pattern()`.

The helper writes into caller-owned pixels only:

- width: 16
- height: 16
- pitch: caller-provided, must be at least width
- format: RGB565
- allocation: caller-owned stack/static buffer, no malloc and no full framebuffer

`pet2d_minimal_visual_self_test()` validates null handling, pitch checks and representative pattern pixels.

## Test Pattern

The 16x16 pattern is designed to be recognizable on the round screen:

- white border
- red top-left quadrant
- green top-right quadrant
- blue bottom-left quadrant
- black bottom-right quadrant
- yellow main diagonal
- magenta anti-diagonal

This is enough to spot obvious RGB565 byte-order or orientation mistakes, while remaining small enough for
a bounded manual LCD smoke test.

## Minimal Real Flush Probe Gate

`pet2d_boundary_minimal_real_flush_probe()` is the manual Pet2D-boundary probe:

- with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`, returns `PET_RESULT_UNSUPPORTED` and does not call the LCD;
- with a local macro-enabled board build, it requires a free display owner or existing PET2D owner;
- it acquires `PET_DISPLAY_OWNER_PET2D` only when owner is `NONE`;
- it generates the 16x16 RGB565 pattern;
- it calls `pet_display_jieli_real_flush_poc_rect()` at screen center;
- it releases PET2D owner if it acquired it;
- it does not loop, animate, load resources, or enter Pet2D runtime.

The probe intentionally does not auto-release LVGL owner. A manual board test must release LVGL owner first
or verify that the probe returns `PET_RESULT_BUSY`.

## Self-Test And Capability Snapshot

P11 adds `PET_SELFTEST_PET2D_MINIMAL_REAL_FLUSH_GATE` and
`has_pet2d_minimal_visual_probe_gate`.

`pet_selftest_run_all()` validates the minimal pattern helper and reports the real panel path as skipped.
This keeps automated self-tests from writing the LCD even in `PET_PLATFORM_JIELI_TEST` builds.

Committed capability flags remain:

- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Manual Board Test Plan

1. Keep the committed source with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0` and confirm compile checks pass.
2. For a temporary local board test only, set `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`.
3. Add a temporary, non-committed debug action that first releases LVGL owner, then calls
   `pet2d_boundary_minimal_real_flush_probe()`.
4. Build and download to the board.
5. Trigger the action once.
6. Expected first safety check: if LVGL still owns the display, the probe returns `PET_RESULT_BUSY` and
   should not write the panel.
7. Expected manual success: after LVGL owner is released, a 16x16 pattern appears at screen center.
8. Confirm there is no panic, assert, watchdog reset, hard fault, continuous refresh loop or permanent
   default-page corruption.
9. Restore the macro to 0 and remove the temporary debug action before any commit.

## Board Test Result

Manual P11 board test was run on COM3 at 1000000 baud with a temporary local build that set
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` and added a non-committed MVP-A Debug `P11` / `Visual` action.
The temporary action released LVGL owner before calling `pet2d_boundary_minimal_real_flush_probe()`.

Observed log sequence:

- `[MVP_A][LVGL_OWNER] release ret=0 owner=0`
- `[P11_MINIMAL_VISUAL] debug trigger ret=0`
- `[MVP_A][LVGL_OWNER] acquired owner=2`
- `[MVP_A][LVGL] render scene=9 title=Debug prompt=Visual action=P11`

The user visually confirmed the small center pattern appeared on the board. No `panic`, `assert`,
`WDT_RST`, `HardFault`, `exception` or `Reset` keyword was observed in the captured COM3 window.

The temporary debug action and macro-enabled build are board-test-only and must not be committed.

## Submission Boundary

P11 source may be committed only with:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`;
- no temporary Debug UI trigger;
- no build logs, `sdk.elf`, firmware packages or download outputs;
- no Pet2D runtime enablement;
- no LVGL flush callback changes.

## Follow-Up

Recommended next steps:

1. P12 repeated tiny flush / dirty rect alignment.
2. P13 resource sprite to minimal Pet2D surface.
3. P14 HOME/Observe minimal scene skeleton.
