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

## P6 A/B Save Transaction Adapter Status

Status: implemented as an isolated in-memory A/B transaction adapter and self-test; not a real VM/Flash
storage integration.

Current capability:
- `apps/watch/pet_save_jieli/` validates the P1/simulator 64-byte `pet_save_slot_header_t` layout:
  magic `0x50455453`, version `1`, schema range, payload type, payload length, counter, timestamp and
  payload CRC32.
- `load_latest` validates both slots and chooses the valid slot with the highest counter.
- `write_transaction` writes only the inactive slot, stages an intentionally invalid header before the
  payload, writes the final header only after payload bytes are present, verifies the target slot, and
  leaves the previous valid slot readable if the staged write fails.
- The caller-owned memory backend supports self-test fault modes for failure before write, after staged
  header, after payload, corrupt-after-write and low-battery write blocking.
- `pet_save_jieli_self_test()` covers empty slots, first and second writes, latest selection, CRC
  rollback, partial-write rollback, oversize payload, low-battery block, bad version and bad payload CRC.

P6 still does not:
- write real VM, Flash, syscfg, files, NOR or SFC;
- replace or alter the existing `mvp_a_save` default path;
- enable `pet_platform_jieli` storage callbacks;
- change MVP-A default page behavior or save data;
- enable Pet2D runtime, real LCD flush, BLE/NFC/audio hardware or production resources.

## P7 BLE/NFC Stub And Debug Injection Status

Status: implemented as a macro-isolated protocol/debug adapter; not a real BLE GATT or NFC reader
integration.

Current capability:
- `apps/watch/pet_protocol_jieli/` builds and validates P1/simulator `pet_packet_t` packets, including
  magic `0xE6`, version `1`, 10-byte header, 64-byte max payload, seq/ack preservation and CRC16
  CCITT-FALSE.
- The same helper validates the 24-byte NFC pair payload ABI and CRC16 range.
- `pet_ble_jieli.c` keeps default `send_packet` / `poll_packet` as `PET_RESULT_NOT_READY`, but
  `PET_PLATFORM_JIELI_TEST` or `PET_DEBUG` builds can enable a fixed 4-packet loopback queue.
- `pet_nfc_jieli.c` keeps default scan/poll callbacks as `PET_RESULT_NOT_READY`, but test/debug builds
  can inject fake `pet_nfc_card_t` and NFC pair payloads into small in-memory queues.
- `pet_debug_jieli.c` provides test/debug-only fake millis, fake now_sec, fake battery, BLE packet
  injection, NFC card injection, NFC pair payload injection and clear-all helpers.
- P7 self-tests cover packet build/validate, bad magic/version/len/CRC, BLE loopback, queue full,
  fake NFC card/pair polling and debug fake state.

P7 still does not:
- start BLE advertising, scanning, connecting or GATT services;
- start real NFC reader scans or RF transactions;
- write VM/Flash/syscfg or replace MVP-A save;
- change MVP-A default page behavior, key behavior or storage behavior;
- enable Pet2D runtime, real LCD flush, real resources or production pairing security.

## P8 Platform Self-Test Aggregator Status

Status: implemented as a unified self-test runner and capability snapshot; not a real hardware
integration.

Current capability:
- `apps/watch/pet_selftest/` exposes a P1-P7 self-test case enum, summary counters, failed/skipped
  masks and stable case names.
- The aggregator calls the shared-interface sanity check, platform callback-table check, display
  profile self-test, display owner self-test, input mapping self-test, Pet2D boundary self-test,
  resource manifest self-test, Pet2D resource probe, save transaction self-test, protocol packet
  self-test, BLE loopback self-test, NFC fake self-test and debug injection self-test.
- `pet_selftest_run_all()` records `PET_RESULT_UNSUPPORTED` as skipped so ordinary builds can compile
  test-only BLE/NFC/debug entries without treating disabled test hooks as failures.
- `pet_selftest_get_capability_snapshot()` marks P1-P7 test/stub capabilities as present while keeping
  real LCD flush, real key queue, real Flash storage, real BLE, real NFC and Pet2D runtime flags at 0.

P8 still does not:
- change MVP-A default page, input or save behavior;
- call real LCD flush or modify the LVGL flush callback;
- enable Pet2D runtime or allocate a full framebuffer;
- write VM/Flash/syscfg or connect platform storage callbacks;
- start real BLE/NFC RF paths or parse runtime PNG/JPG/GIF/JSON.

## P9 Display Flush Owner POC Status

Status: implemented as an owner-guarded diagnostic wrapper; real LCD flush remains disabled.

Current capability:
- P9 audits the active LVGL path: `cpu/br28/ui_driver/lvgl/lv_port_disp.c` owns `disp_flush`, 454x454
  LVGL resolution, two 20-line draw buffers and the existing calls to `lcd_data_copy`/`lcd_draw_area`.
- `pet_display_jieli_flush()` now validates RGB565 rect parameters and records diagnostic stats for
  call count, rejected count, busy count, last rect, last pitch, last owner, mode and requested pixels.
- The PetEgg flush path enforces owner guard: no owner returns `PET_RESULT_NOT_READY`, LVGL owner
  returns `PET_RESULT_BUSY`, and only PET2D/DEBUG owner can reach the no-op diagnostic path.
- The legal diagnostic path still returns `PET_RESULT_NOT_READY` because `real_lcd_flush_enabled` is
  0 and no hardware flush is started.
- `pet_display_jieli_flush_self_test()` covers no-owner, LVGL-busy, PET2D no-op, invalid parameter,
  out-of-bounds, pitch checks, stats and non-blocking wait behavior.
- `apps/watch/pet_selftest/` now includes `PET_SELFTEST_DISPLAY_FLUSH_OWNER` and marks
  `has_display_flush_owner_guard = 1` while keeping `real_lcd_flush_enabled = 0`.

P9 still does not:
- replace or modify the LVGL `disp_flush` callback;
- call `lcd_draw_area`, `lcd_data_copy`, `lcd_wait`, `lcd_wait_te` or any real LCD driver API;
- enable Pet2D runtime, HOME/Observe rendering or a full framebuffer;
- change MVP-A default page/input/save behavior;
- write VM/Flash/syscfg, connect storage callbacks, BLE or NFC.

## P10 Tiny Real LCD Flush POC Status

Status: implemented as a manual-only, macro-gated tiny flush POC boundary. Committed source still keeps
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0 and `real_lcd_flush_enabled` at 0.

- P10 re-audits the real LVGL/LCD path and chooses `lcd_draw_area(..., wait=1)` plus `lcd_wait()` as the
  tiny POC candidate API because it matches the non-QSPI branch in the active LVGL flush callback.
- `pet_display_jieli_real_flush_poc_rect()` and `pet_display_jieli_tiny_flush_poc()` exist, but committed
  builds return `PET_RESULT_UNSUPPORTED` before touching LCD hardware.
- Board-test note: a temporary local build with the macro enabled and a non-committed Debug `Tiny` trigger
  successfully drew the center 8x8 block on hardware and logged `P10_TINY_FLUSH ret=0`; the committed
  source keeps the macro disabled and does not keep that Debug UI entry.
- If a later manual board build enables `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`, the call is still gated by
  display owner state and an internal manual-arm flag; normal `display_flush` self-tests do not automatically
  write the panel.
- `pet2d_boundary_tiny_visual_probe()` verifies the future Pet2D handoff boundary without entering Pet2D
  runtime or loading resources. With the macro off, it returns `UNSUPPORTED`.
- Self-test aggregation adds a tiny flush POC case, but run-all marks it skipped so default test execution
  cannot cause a real LCD write.

P10 still does not:
- replace the LVGL flush callback or change default MVP-A page behavior;
- keep a Debug UI entry for tiny flush in the committed default;
- enable Pet2D runtime, HOME/Observe rendering, dirty rect rendering or full framebuffer allocation;
- write VM/Flash/syscfg or connect BLE/NFC.

## P11 Pet2D Minimal Real Flush POC Status

Status: implemented as a minimal Pet2D-boundary visual surface and manual real-flush probe gate. Committed
source still keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, `real_lcd_flush_enabled` at 0 and
`pet2d_runtime_enabled` at 0.

- `apps/watch/pet2d_boundary/pet2d_minimal_visual.*` can fill a caller-owned 16x16 RGB565 surface with a
  white border, red/green/blue/black quadrants and yellow/magenta diagonals for visual direction and
  byte-order smoke testing.
- `pet2d_boundary_minimal_real_flush_probe()` is a manual-only boundary entry. With the committed macro
  off it returns `PET_RESULT_UNSUPPORTED`; with a local macro-enabled board build it acquires PET2D owner,
  draws the 16x16 pattern at screen center through the P10 rectangle flush API, and releases the owner.
- Board-test note: a temporary local build with the macro enabled and a non-committed Debug `P11` trigger
  successfully displayed the small center pattern on hardware and logged `[P11_MINIMAL_VISUAL] debug
  trigger ret=0`; the committed source keeps the macro disabled and does not keep that Debug UI entry.
- The self-test aggregator now has a minimal visual gate case and capability bit. run-all validates the
  pattern helper but skips the real panel write path.

P11 still does not:
- enable the complete Pet2D runtime, HOME/Observe, background scrolling, sprite animation or dirty-rect
  scheduling;
- load formal resources or parse runtime image/JSON formats;
- allocate a full framebuffer;
- replace LVGL flush or change MVP-A default page behavior;
- write VM/Flash/syscfg or connect BLE/NFC.

## P12 Repeated Tiny Flush + Dirty Rect Alignment POC Status

Status: implemented as a dirty-rect/repeated-probe gate in source; real repeated LCD writes remain
manual-only and the committed source keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0,
`real_lcd_flush_enabled` at 0 and `pet2d_runtime_enabled` at 0.

Current hardware scope adjustment:
- The current development board has no NFC and no speaker.
- Only one development board is available, so real BT/BLE two-board connection testing is out of scope.
- Real NFC, real audio/SFX and real BLE two-board link are Future Scope for this hardware cycle. P12 keeps
  NFC fake/stub tests, audio callback stubs and BLE packet ABI/loopback only.

Current capability:
- `apps/watch/pet2d_boundary/pet2d_dirty_rect_poc.*` can generate 16x16, 32x32 and 64x64 RGB565 test
  patterns and provide center, odd-coordinate, near-edge and out-of-bounds rect cases.
- `pet2d_boundary_repeated_flush_probe()` is a manual-only owner-guarded probe. With the committed macro
  off it returns `PET_RESULT_UNSUPPORTED` and does not touch the LCD.
- Repeated-probe stats record attempts, successes, failures, last rect, repeat count, fail index and
  elapsed-time placeholders for later board runs.
- The self-test aggregator adds a repeated flush gate case and dirty-rect capability bit while still
  skipping real panel writes in run-all.
- Board-test note: a temporary macro-enabled build with a non-committed Debug `P12` trigger passed
  16x16/32x32/64x64 center repeated flushes, a 32x32 odd-coordinate flush and out-of-bounds rejection.
  Logs showed 40 real flush attempts, 40 successes and no observed panic/assert/WDT/HardFault/reset.
  The user also reported that physical LEFT/UP reached Debug where the earlier instruction expected
  RIGHT/DOWN; key physical labeling and logical mapping remain to be confirmed before changing defaults.

P12 still does not:
- enable the complete Pet2D runtime, HOME/Observe, background scrolling or sprite animation;
- load formal resources or parse runtime image/JSON formats;
- allocate a full framebuffer;
- replace LVGL flush or change MVP-A default page behavior;
- write VM/Flash/syscfg or connect real NFC/audio/BLE hardware.

## P13 Resource Sprite To Minimal Pet2D Surface POC Status

Status: implemented as a P5 fixture-to-surface bridge in source; real resource-derived LCD writes remain
manual-only and the committed source keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0,
`real_lcd_flush_enabled` at 0 and `pet2d_runtime_enabled` at 0.

Current capability:
- `apps/watch/pet2d_boundary/pet2d_resource_sprite_poc.*` opens the P5 static test blob, validates RGB565
  SPRITE entries and exposes resource 2001 as a 4x4 sprite view and resource 1001 as an 8x8
  background-role fixture view.
- `pet2d_minimal_visual_blit_sprite()` copies raw RGB565 fixture pixels into caller-owned minimal
  surfaces with clipping. It does not use transparency, malloc, file IO, Flash IO or image decoding.
- `pet2d_boundary_resource_sprite_flush_probe()` is a manual-only resource-derived visual probe. With
  the committed macro off it returns `PET_RESULT_UNSUPPORTED` before touching the LCD.
- The self-test aggregator adds a resource-sprite surface case and capability bit while still skipping
  real panel writes in run-all.
- Board-test note: a temporary macro-enabled build with a non-committed Debug `P13` / `Resource` action
  successfully displayed the resource-derived 32x32 pattern on hardware and logged
  `[P13_RESOURCE_SPRITE] debug trigger ret=0`. The committed source keeps the macro disabled and does
  not keep that Debug UI entry.

P13 still does not:
- enable the complete Pet2D runtime, HOME/Observe, background scrolling or sprite animation;
- load formal production resources or read external Flash/NOR/SFC/`flash_file_info`;
- parse runtime PNG/JPG/GIF/JSON;
- allocate a full framebuffer;
- replace LVGL flush or change MVP-A default page behavior;
- write VM/Flash/syscfg or connect real NFC/audio/BLE hardware.

## P14 Minimal Sprite Movement + Raw Key Calibration POC Status

Status: implemented as a passive key-calibration helper and minimal movement gate in source; committed
source keeps the real LCD movement path disabled by default.

Current capability:
- `apps/watch/pet_platform_jieli/pet_key_calibration_jieli.*` records recent raw/sdk key observations
  into a 16-entry ring and maps them through the P3 `pet_input_jieli_map_raw_event()` helper.
- The calibration helper documents the confirmed SDK key path: `KEY_UI_HOME` -> raw 0 / OK,
  `KEY_UI_PLUS` -> raw 1 / LEFT_UP, `KEY_UI_MINUS` -> raw 2 / RIGHT_DOWN, and `KEY_UI_SHORTCUT` -> raw 3 /
  CANCEL. P14 board logs confirm this mapping, so no P3 mapping correction is made.
- `apps/watch/pet2d_boundary/pet2d_movement_poc.*` keeps a small 32x32 movement state, uses the P13
  resource sprite fixture on a P12 pattern surface, handles LEFT_UP/RIGHT_DOWN/OK/CANCEL, clamps to the
  display safe area and computes the dirty bounding rect.
- `pet2d_boundary_movement_probe_step()` is manual-only. With the committed macro off it returns
  `PET_RESULT_UNSUPPORTED` before touching the LCD.
- The self-test aggregator adds key-calibration and minimal-movement cases plus capability bits while
  keeping real panel writes skipped in run-all.
- Board-test note: a temporary macro-enabled build with non-committed Debug `P14L` / `P14R` / `P14OK` /
  `P14C` actions confirmed that OK-triggered movement steps return `ret=0`; the small resource/pattern
  icon appears briefly and moves according to the selected prompt, then the LVGL Debug page redraws over it.

P14 still does not:
- change `mvp_a_app_key_event`, `mvp_a_ui_handle_system_key` or the board key table in committed source;
- consume the real Jieli key queue;
- enable full Pet2D runtime, HOME/Observe, background scrolling or formal resources;
- allocate a full framebuffer, replace LVGL flush or change MVP-A default page behavior;
- write VM/Flash/syscfg or connect real NFC/audio/BLE hardware.

## P15 Key Latency + Movement Repeated Flush Stats POC Status

Status: implemented as a stats extension to the P14 movement gate in source; committed source keeps the
real LCD movement path disabled by default.

Current capability:
- `apps/watch/pet2d_boundary/pet2d_movement_poc.*` now records movement key events, movement steps,
  render attempts, render success/failure, coarse key/logic/render/flush timestamps, key-to-flush
  min/max/average values and the last old/new sprite positions.
- Dirty rect strategy is explicit: old sprite rect and new sprite rect are merged into a bounding
  rectangle, then kept within the existing display-safe movement bounds. P15 still does not implement a
  full scene erase/restore system.
- `pet2d_movement_poc_run_repeated_steps()` and `pet2d_boundary_movement_repeated_probe()` support a
  finite repeated movement probe capped at 60 steps. With the committed real-flush macro off, the manual
  probe returns `PET_RESULT_UNSUPPORTED` before touching the LCD.
- The self-test aggregator adds a key-latency/movement-stats case plus capability bits while keeping real
  panel writes skipped in run-all.
- Board-test note: a temporary macro-enabled build with non-committed Debug `P15S10` / `P15S30` /
  `P15S60` actions confirmed 10/30/60 repeated movement probes all returned `ret=0` with matching
  success counts and no observed panic/assert/WDT/HardFault/exception. The movement pattern is visible
  during the probe but still disappears after LVGL reacquires owner and redraws the Debug page.

P15 still does not:
- route real key events into Pet2D by default or modify the MVP-A key path;
- enable full Pet2D runtime, HOME/Observe, background scrolling, formal resources or external Flash;
- allocate a full framebuffer, replace LVGL flush or change MVP-A default page behavior;
- write VM/Flash/syscfg or connect real NFC/audio/BLE hardware.

## P16 Real Resource Package / External Flash Read POC Status

Status: implemented as a read-only SDK resource path audit and adapter skeleton. It does not generate or
download a production PetEgg resource package.

Current capability:
- The SDK audit identifies two safe read candidates: `res_fopen/res_fread/res_flen/res_fclose` for
  ordinary resource-file reads, and `ui_res_flash_info_get`/`flash_file_info` for UI/IMB flash mapping
  when a packaged UI resource already exists.
- `apps/watch/pet_resource_jieli/pet_resource_jieli_real.*` provides a read-only real package probe. The
  default path is `storage/virfat_flash/C/petegg/manifest.bin`; it reads only the manifest header and
  validates magic/version/table bounds before reporting availability.
- If the package is absent, the adapter returns `PET_RESULT_NOT_FOUND` and the self-test aggregator marks
  `PET_SELFTEST_RESOURCE_PACKAGE_PROBE` as skipped rather than failing the platform snapshot.
- No `manifest.bin`, `sprites.pak` or `anim_table.bin` package was found in the current committed source
  or generated `download/watch` output. A later generator/download task is still required before real
  PetEgg package bytes can be validated.
- A temporary board-only Debug action triggered the read-only package probe on COM3. It returned
  `PET_RESULT_NOT_FOUND` (`12`) for both probe info and package probe, did not write Flash, and did not
  show panic/assert/WDT/HardFault/exception during the watch window.
- The temporary Debug action was removed from source after the board probe. A restored final-safe package
  was generated, but the SDK reported the device offline during that last package/download step; no
  further download attempt was made by user instruction.

Committed safety state:
- `real_resource_package_available = 0` unless a real package is already present on a board.
- `external_flash_resource_enabled = 0`.
- `real_lcd_flush_enabled = 0`.
- `pet2d_runtime_enabled = 0`.

P16 still does not:
- write external Flash, VM, syscfg or files;
- change resource packaging/download defaults;
- parse PNG/JPG/GIF/JSON or decode SDK image resources into PetEgg sprites;
- enable full Pet2D runtime, HOME/Observe, background scrolling or a full framebuffer;
- replace LVGL flush or keep a Debug UI trigger;
- connect real NFC/audio/BLE hardware.

## P17S External Flash Resource Pause Status

P17 attempted a minimal PetEgg package generator and external Flash download integration after P16.
Board testing confirmed that external Flash itself can be identified, written and mounted, and that a
`storage/virfat_flash/C/petegg` file can be opened with the expected 272-byte size. However, the bytes
read from the board did not match the local plain `MRTP` package header.

- Local package header: `4d 52 54 50 01 00 03 00 93 f4 f7 ac 00 00 00 00`.
- Runtime header from the board: `00 01 22 64 c9 b2 44 89 32 65 cb b6 6d db b7 6f`.
- Both `res_fopen/res_fread` and ordinary `fopen/fread` produced the same transformed payload.
- A direct raw NOR read experiment previously caused a soft reset, so raw NOR read is not a current
  PetEgg route.

The current stable baseline therefore remains P16 read-only probing plus the earlier compile-time /
in-memory resource fixture path from P5/P13/P15. Near-term work continues in the 2M internal Flash
environment with compile-time small resources or other explicitly bounded internal-resource paths.

Current committed-state expectations remain:
- `real_resource_package_available = 0`.
- `external_flash_resource_enabled = 0`.
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`.
- `real_lcd_flush_enabled = 0`.
- `pet2d_runtime_enabled = 0`.

External Flash / virfat / raw NOR PetEgg resources, NFC, audio/speaker and real BLE two-board validation
are Future Scope until their platform contracts are confirmed.
