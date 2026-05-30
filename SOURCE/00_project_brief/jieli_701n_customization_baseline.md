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
- write VM/Flash, or connect BLE/NFC/audio hardware.

## P4 Render Owner Boundary POC Status

Status: implemented as a compile/log validation boundary; not a real Pet2D or LCD integration.

Current capability:
- `pet_display_jieli_get_owner()` exposes the in-memory display owner state for controlled boundary
  checks.
- `pet_display_jieli_owner_self_test()` verifies NONE rejection, LVGL acquire, same-owner re-acquire,
  PET2D conflict, non-owner release rejection and release back to NONE.
- `mvp_a_lvgl_shell_create()` acquires `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` before rendering.
- `mvp_a_lvgl_shell_render_scene()` verifies LVGL ownership before touching the LVGL scene tree and
  skips rendering if another owner holds the display.
- `mvp_a_lvgl_shell_release_display_owner()` provides a future handoff hook before entering Pet2D.
- `apps/watch/pet2d_boundary/` contains a placeholder enter/exit/self-test that only exercises
  owner handoff. It does not allocate a framebuffer, call LCD flush, load resources, or run Pet2D.

P4 still does not:
- change the LVGL low-level flush callback;
- write RGB565 pixels to the real LCD;
- enable Pet2D runtime or the HOME/Observe scene;
- change MVP-A default page behavior or key handling;
- write VM/Flash, or connect BLE/NFC/audio hardware.

## P5 Resource Manifest Adapter Status

Status: implemented as a read-only in-memory manifest parser and fixture backend; not a real external
Flash resource integration.

Current capability:
- `apps/watch/pet_resource_jieli/` opens a caller-provided memory blob and parses the P1/simulator
  resource manifest ABI.
- The parser validates manifest magic, version, 16-byte header size, 28-byte entry size, entry count,
  table bounds, data bounds, table CRC32 and entry data CRC32.
- Lookup is available by resource ID and by type/index. `read_entry` returns a pointer/size into the
  opened read-only blob and does not copy or decode resource data.
- The P5 test blob contains three small fixture entries: ID 1001 as an 8x8 RGB565 background-role
  placeholder, ID 2001 as a 4x4 RGB565 sprite placeholder, and ID 3001 as an animation-table
  placeholder.
- `pet2d_boundary_resource_probe_self_test()` lightly references the resource self-test without loading
  sprites, entering Pet2D runtime, allocating a framebuffer, or writing the display.

P5 still does not:
- read external Flash, NOR, SFC or `flash_file_info`;
- write VM/Flash;
- parse PNG/JPG/GIF/JSON at runtime;
- import final production art;
- enable Pet2D runtime or real LCD flush;
- change MVP-A default page behavior, or connect BLE/NFC/audio hardware.
