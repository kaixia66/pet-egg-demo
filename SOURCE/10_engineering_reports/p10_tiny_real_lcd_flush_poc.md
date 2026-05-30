# P10 Tiny Real LCD Flush POC

## LCD API Second Audit

- Active LVGL flush remains `cpu/br28/ui_driver/lvgl/lv_port_disp.c::disp_flush()`.
- The non-QSPI branch calls `lcd_draw_area(0, color_p, left, top, width, height, 1)` followed by
  `lcd_wait()`.
- The QSPI/ST77903 branch calls `lcd_wait_te()`, `lcd_data_copy(2, &rect, color_p, stride, left, top,
  width, height, 2)`, then `lcd_data_copy_wait()`.
- Public IMD declarations include `lcd_wait()`, `lcd_draw_area()`, `lcd_draw_rect()` and
  `lcd_draw_area_stride()` under `include_lib/system/ui_new/ui/cpu/br28/asm/imd.h`.
- The SH8601A panel configuration is 454x454 RGB565 with row/column address alignment of 2 in
  `cpu/br28/ui_driver/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.c`.

P10 selects `lcd_draw_area(..., wait=1)` plus `lcd_wait()` as the tiny POC candidate because it is the
same simple push path used by the active LVGL non-QSPI branch. Cache maintenance, PSRAM/non-cache address
requirements and exact byte order still need board validation before this can become a normal runtime path.

## Tiny Flush POC API

- `pet_display_jieli_real_flush_poc_rect(x, y, w, h, rgb565, pitch_pixels)`: manual rectangular entry.
- `pet_display_jieli_tiny_flush_poc()`: fixed 8x8 RGB565 color block at screen center
  `(PET_JIELI_DISPLAY_WIDTH / 2 - 4, PET_JIELI_DISPLAY_HEIGHT / 2 - 4)`.
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` defaults to 0, so committed builds return `PET_RESULT_UNSUPPORTED`
  and do not call LCD driver APIs.
- If a manual board build sets the macro to 1, the call still requires owner guard success and an internal
  manual-arm flag. Normal platform `display_flush` calls and run-all self-tests do not automatically write
  the panel.

## Diagnostics

P10 extends flush stats with `tiny_poc_enabled`, real attempt/success/fail counters, last real-flush result,
last driver status and last real-flush duration in milliseconds. The committed default keeps attempts at 0.

## Pet2D Boundary Probe

`pet2d_boundary_tiny_visual_probe()` is a manual visual boundary probe. It does not enter Pet2D runtime,
load resources or allocate a framebuffer. With the real-flush macro off it returns `PET_RESULT_UNSUPPORTED`.
When manually enabled, it acquires `PET_DISPLAY_OWNER_PET2D` only if no other owner is active.

## Board Test Result

Manual P10 board test was run on COM3 at 1000000 baud with a temporary local build that set
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` and added a non-committed MVP-A Debug `Tiny` action. The first
attempt returned `PET_RESULT_BUSY` while LVGL still owned the display. After the temporary trigger released
the LVGL owner and moved the 8x8 block to screen center, the probe succeeded:

- `mvp_a_lvgl_shell_release_display_owner()` logged `release ret=0 owner=0`.
- `pet2d_boundary_tiny_visual_probe()` logged `[P10_TINY_FLUSH] debug trigger ret=0`.
- LVGL reacquired owner afterward and rendered the Debug page again.
- The user visually confirmed the center color block appeared on the board.
- No `panic`, `assert`, `WDT_RST`, `Reset`, `exception` or `HardFault` keywords were observed in the
  captured COM3 log window.

The temporary debug menu action and macro-enabled build are board-test-only and must not be committed.

## Self-Test Behavior

The self-test aggregator adds `PET_SELFTEST_DISPLAY_TINY_FLUSH_POC`, but run-all always reports it as
skipped. This prevents automated test execution from triggering a real panel write even in
`PET_PLATFORM_JIELI_TEST` builds.

## Manual Board Test Steps

1. Confirm the branch builds with the committed macro value of 0.
2. Confirm the normal LVGL watch UI still displays before any manual POC.
3. For a temporary hardware test only, enable `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` via a local build
   define or local edit that must not be committed.
4. Manually call `pet2d_boundary_tiny_visual_probe()` or acquire an allowed owner and call
   `pet_display_jieli_tiny_flush_poc()`.
5. Observe whether an 8x8 color block appears near screen center.
6. Check logs/stats for attempt count, result, driver status and duration.
7. Confirm there is no crash, permanent flicker, continuous refresh loop, or default page corruption.
8. Return to LVGL UI and confirm normal rendering still works.

## Acceptance Criteria

- Default source does not trigger real LCD writes.
- Macro-enabled manual test writes only the tiny rectangle.
- No full framebuffer allocation or Pet2D runtime is involved.
- No VM/Flash/syscfg, BLE or NFC path is touched.
- The macro is restored to 0 before any commit.

## Rollback

Restore `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` to 0 or remove the temporary build define, then rebuild.
Use `git restore` on any local macro-test edits before committing later stages.

## Not Covered

P10 does not validate Pet2D runtime, HOME/Observe scenes, dirty rect scheduling, background scrolling,
sprite drawing, resource streaming or performance.
