# P9 Display Flush Owner POC

## LCD/LVGL Flush Path Audit

- Active board config remains `CONFIG_BOARD_701N_LVGL_DEMO`.
- `apps/watch/board/br28/board_701n_lvgl_demo/board_701n_lvgl_demo_cfg.h` enables
  `TCFG_LCD_SPI_SH8601A_ENABLE`, `TCFG_SPI_LCD_ENABLE` and `LVGL_TEST_ENABLE`.
- `cpu/br28/ui_driver/lvgl/lv_port_disp.c` defines `MY_DISP_HOR_RES = 454`,
  `MY_DISP_VER_RES = 454` and `MY_DISP_VLOCK_H = 20`.
- The LVGL port registers `disp_drv.flush_cb = disp_flush` and keeps two 454x20 draw buffers.
- `disp_flush()` clips the LVGL area, flushes cache and then either calls
  `lcd_wait_te()` + `lcd_data_copy()` + `lcd_data_copy_wait()` or calls
  `lcd_draw_area()` + `lcd_wait()`. P9 does not call or modify this path.
- `cpu/br28/ui_driver/lcd_drive/lcd_spi/lcd_spi_sh8601a_454x454.c` provides the SH8601A 454x454
  `imd_param` and registers the LCD device. It sets row/column address alignment to 2.
- `include_lib/system/ui_new/ui/cpu/br28/asm/imd.h` exposes IMD busy/lock state and LCD APIs such as
  `lcd_wait`, `lcd_draw_area`, width/height getters and related draw helpers.
- IMB candidates exist through `asm/imb.h`, `imb_set_output_cb()` and demo uses of `lcd_draw_area`, but
  IMB/Pet2D real rendering is not enabled in P9.

## Owner Guard Design

`pet_display_jieli_flush()` now validates:

- non-null rect and RGB565 buffer;
- non-zero width/height;
- stride byte alignment to RGB565;
- pitch pixels greater than or equal to rect width;
- rect bounds inside the 454x454 P3 display profile.

Owner policy:

- `PET_DISPLAY_OWNER_NONE` returns `PET_RESULT_NOT_READY`;
- `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` returns `PET_RESULT_BUSY`;
- `PET_DISPLAY_OWNER_PET2D` and `PET_DISPLAY_OWNER_DEBUG` may reach the diagnostic no-op path;
- other owners are rejected as busy for now.

The accepted diagnostic path records stats and returns `PET_RESULT_NOT_READY` because the default
real LCD flush flag is still disabled.

## Diagnostic Stats

`pet_display_jieli_flush_stats_t` records:

- `flush_call_count`
- `rejected_count`
- `busy_count`
- `total_requested_pixels`
- `last_x`, `last_y`, `last_w`, `last_h`
- `last_pitch_pixels`
- `last_mode`
- `last_owner`
- `real_flush_enabled`
- `busy`

APIs:

- `pet_display_jieli_get_flush_stats()`
- `pet_display_jieli_reset_flush_stats()`

## Wait / Busy Semantics

P9 does not start real async LCD work, so `pet_display_jieli_wait()` returns `PET_RESULT_OK` and does
not block. The internal `flush_busy` flag remains false and is surfaced in stats for a future real
flush phase.

## Tiny Real Flush POC Macro

`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` defaults to 0 in `pet_platform_jieli_internal.h`.
`pet_display_jieli_tiny_flush_poc()` returns `PET_RESULT_UNSUPPORTED` while the macro is 0. No real LCD
API is called in P9.

## Self-Test Result

`pet_display_jieli_flush_self_test()` covers:

- stats reset;
- no-owner flush returns `PET_RESULT_NOT_READY`;
- LVGL owner blocks Pet2D-style flush with `PET_RESULT_BUSY`;
- PET2D owner reaches no-op diagnostic path;
- null buffer, out-of-bounds rect and pitch smaller than width are rejected;
- stats record calls, rejects, busy count, last rect and total requested pixels;
- wait returns without blocking.

`PET_SELFTEST_DISPLAY_FLUSH_OWNER` is added to the P8 aggregator and
`has_display_flush_owner_guard` is set to 1. `real_lcd_flush_enabled` remains 0.

## Remaining Boundary

P9 does not replace the LVGL flush callback, does not call `lcd_draw_area`, `lcd_data_copy`, `lcd_wait`,
`lcd_wait_te` or IMB output callbacks, and does not enable Pet2D runtime.

## Follow-Up

Before enabling any real flush path, confirm LCD API ownership, owner lock placement, TE/wait behavior,
DMA/cache requirements, RGB565 byte order, row/column alignment and a bounded board smoke-test plan.
