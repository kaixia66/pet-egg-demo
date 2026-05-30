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
