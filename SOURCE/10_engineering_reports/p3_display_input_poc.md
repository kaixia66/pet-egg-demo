# P3 Display Profile + 4-Key Input Mapping POC

## Display Profile Audit

- Active board: `CONFIG_BOARD_701N_LVGL_DEMO` in `apps/watch/board/br28/board_config.h`.
- Board LCD config: `board_701n_lvgl_demo_cfg.h` enables `TCFG_LCD_SPI_SH8601A_ENABLE`,
  `TCFG_SPI_LCD_ENABLE` and `LVGL_TEST_ENABLE`.
- LVGL resolution source: `cpu/br28/ui_driver/lvgl/lv_port_disp.c` defines
  `MY_DISP_HOR_RES` and `MY_DISP_VER_RES` as 454.
- Current flush path: LVGL `disp_flush` calls `lcd_data_copy`/`lcd_draw_area`; P3 does not call or
  modify this path.

P3 platform profile returns 454x454, `PET_SCREEN_SHAPE_CIRCLE`, RGB565 rect flush mode, RGB order,
rotation 0, and safe area x/y/w/h = 34/34/386/386. These values are named constants in
`pet_platform_jieli_internal.h` and remain pending board-test confirmation.

## Input Source Audit

- Active key hardware path: IO keys are enabled; AD, IR, touch key and RDEC are disabled for the
  active board config.
- Board raw IO key values: `iokey_list` assigns raw values 0, 1, 2 and 3.
- Existing key table path: `iokey_table.c` maps those raw values to `KEY_UI_HOME`, `KEY_UI_PLUS`,
  `KEY_UI_MINUS` and `KEY_UI_SHORTCUT`.
- Current MVP-A path: `app_common.c` calls `mvp_a_ui_handle_system_key()`, then
  `mvp_a_ui_key_event()`, then `mvp_a_app_key_event()`.
- LVGL indev path: `lv_port_indev_init()` returns early and prints that the system key bridge is used.

P3 does not modify `mvp_a_ui_handle_system_key()`, `mvp_a_app_key_event()`, the board key tables,
`app_common.c`, or the LVGL input driver.

## Mapping Table

| Raw code | Source | Pet key | Placeholder |
|---|---|---|---|
| 0 | board IO key value 0 / `KEY_UI_HOME` path | `PET_KEY_OK` | physical label pending |
| 1 | board IO key value 1 / `KEY_UI_PLUS` path | `PET_KEY_LEFT_UP` | physical label pending |
| 2 | board IO key value 2 / `KEY_UI_MINUS` path | `PET_KEY_RIGHT_DOWN` | physical label pending |
| 3 | board IO key value 3 / `KEY_UI_SHORTCUT` path | `PET_KEY_CANCEL` | physical label pending |

| Raw event | Source | Pet event | Placeholder |
|---|---|---|---|
| 0 | SDK `KEY_EVENT_CLICK` | `PET_KEY_EVENT_CLICK` | no |
| 1 | SDK `KEY_EVENT_LONG` | `PET_KEY_EVENT_LONG_PRESS` | hold_ms POC value |
| 2 | SDK `KEY_EVENT_HOLD` | `PET_KEY_EVENT_REPEAT` | repeat cadence pending |
| 3 | SDK `KEY_EVENT_UP` | `PET_KEY_EVENT_UP` | no |
| 0x80 | P3 POC-only raw value | `PET_KEY_EVENT_DOWN` | yes |

## Self-Test Expectations

`pet_platform_jieli_display_self_test()` validates profile dimensions, RGB565 mode constants, safe
area bounds, and display-owner acquire/release conflict behavior without calling flush.
Expected result for P3 compile/log validation is `PET_RESULT_OK`.

`pet_platform_jieli_input_self_test()` pushes four private raw events into the POC queue, polls them
back through `pet_platform_t`, checks key/type/timestamp/raw fields, checks long/repeat metadata, and
checks unknown raw code behavior. Empty queue returns `PET_RESULT_AGAIN`.
Expected result for P3 compile/log validation is `PET_RESULT_OK`.

## Boundaries

P3 does not enable real LCD flush, Pet2D, raw Jieli key queue consumption, VM/Flash writes, BLE, NFC,
audio hardware, or battery hardware integration.
