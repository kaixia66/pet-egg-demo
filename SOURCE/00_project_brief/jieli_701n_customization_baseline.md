# Jieli 701N Customization Baseline

## P1 Shared Interface Pack Status

Status: complete for interface freeze preparation, with simulator header second-pass alignment.

P1 adds `apps/watch/pet_shared/include/` as the board-repository copy of the PetEgg Shared Interface
Pack. The directory is intentionally outside `apps/watch/mvp_a` so the existing MVP-A LVGL demo keeps
its current behavior while future Jieli port work can include the same contract headers as the PC
simulator shared portable layer.

Simulator reference path used after discovery:
- `D:/0-jieli_sdk/simulator/shared_portable/include`

Added interface headers:
- `pet_types.h`
- `pet_display_profile.h`
- `pet_key.h`
- `pet_protocol.h`
- `pet_save_format.h`
- `pet_resource_format.h`
- `pet_platform.h`

Build integration is limited to `apps/watch/pet_shared/pet_shared_compile_check.c`, which only includes
the headers and performs version/layout static asserts. No real LCD, key, storage, NFC, BLE, audio,
power, VM, Flash, LVGL, or simulator adapter implementation is connected in P1.

Interface freeze principles:
- C99 compatible headers only.
- No Jieli SDK private includes.
- No malloc, printf, file-system, Flash, or BLE private struct dependency.
- Packet/save/resource layouts must remain checkable with compile-time static asserts.
- Any future field change requires an explicit version bump and simulator/Jieli compatibility note.

P1 alignment notes:
- Packet, NFC pair payload, save slot header, result code, key enum and resource manifest layouts now
  mirror the simulator portable ABI where the simulator already exposes a frozen layout.
- Board-only display/platform extensions remain in the P1 headers because they define future Jieli port
  boundaries, but they are not connected to real hardware in this phase.

## P2 Jieli Platform HAL Skeleton Status

Status: skeleton implemented for compile checking; not a real hardware HAL.

P2 adds `apps/watch/pet_platform_jieli/` as the Jieli-specific adapter layer for the P1 shared ABI. The
directory now exposes `pet_platform_jieli_get()` and `pet_platform_jieli_init()`, fills a complete
`pet_platform_t` callback table, and compiles all callback stubs into the SDK.

Current capability:
- `millis` calls the SDK `timer_get_ms()` symbol.
- `now_sec` returns `0` until RTC ownership and epoch policy are confirmed.
- `get_device_identity` returns stable P2 stub identity values.
- `get_display_profile` returns a 454x454 circular board profile placeholder.
- display owner acquire/release is an in-memory state machine that rejects owner conflicts.
- power percent returns a fixed test value.
- storage, BLE, NFC, display flush and SFX playback remain `NOT_READY` or `UNSUPPORTED` stubs.

P2 does not:
- change current MVP-A LVGL behavior;
- change LVGL flush;
- enable Pet2D;
- read real keys;
- write VM/Flash/files;
- connect real BLE, NFC, LCD, audio or battery hardware paths.

## P3 Display Profile + 4-Key Input Mapping POC Status

Status: implemented as a controlled POC for compile/log validation; not a real hardware integration.

Display profile source:
- The active board selection is `CONFIG_BOARD_701N_LVGL_DEMO`.
- `board_701n_lvgl_demo_cfg.h` enables `TCFG_LCD_SPI_SH8601A_ENABLE` and `LVGL_TEST_ENABLE`.
- `cpu/br28/ui_driver/lvgl/lv_port_disp.c` fixes the LVGL display driver to 454x454 via
  `MY_DISP_HOR_RES` and `MY_DISP_VER_RES`.
- The P3 profile therefore returns a named 454x454 RGB565 circular profile with a 34 px inset
  386x386 safe area and rotation 0, pending board-test confirmation.
- `pet_platform_jieli_display_self_test()` validates profile bounds and display-owner acquire/release
  conflicts without calling real LCD flush.

Input mapping state:
- The active board has four IO keys with raw values 0, 1, 2 and 3.
- Existing key tables map those to `KEY_UI_HOME`, `KEY_UI_PLUS`, `KEY_UI_MINUS` and
  `KEY_UI_SHORTCUT`, which MVP-A already consumes through its current system-key path.
- P3 adds a private PetEgg raw-code/event mapping table and a small internal POC queue. The platform
  `poll_key_event` only reads that queue and returns `PET_RESULT_AGAIN` when empty.
- `pet_platform_jieli_input_self_test()` validates four mapped events, timestamps, hold/repeat/raw
  fields, unknown raw-code handling and empty-queue behavior.

P3 still does not:
- change current MVP-A LVGL behavior;
- change `mvp_a_app_key_event` or `mvp_a_ui_handle_system_key`;
- consume or take over the real Jieli key queue;
- enable real LCD flush or Pet2D;
- write VM/Flash/files;
- connect real BLE, NFC, audio or battery hardware paths.
