# Open Questions and Risks

## P1 Shared Interface Pack Alignment Risks

Status: simulator headers located and used for a second-pass alignment. The earlier risk that the
simulator headers were unavailable is resolved for P1.

Simulator reference path:
- `D:/0-jieli_sdk/simulator/shared_portable/include`

P1 board headers now align the key ABI values and layouts with simulator portable headers where the
simulator already has a frozen contract:
- `pet_result_t` follows simulator `PetResult` values for OK, invalid argument, buffer-too-small,
  unsupported, duplicate, full, bad CRC, bad version and storage error.
- `pet_key.h` follows simulator `PetProductKey`, `PetKeyEventType`, and `PetPhysicalKey` names and
  numeric values, while adding timestamp/hold/repeat/raw fields required by the board P1 task.
- `pet_protocol.h` follows simulator `pet_packet_t`: magic `0xE6`, version `1`, 10-byte serialized
  header, 64-byte max payload, and CRC16. NFC pair payload follows the simulator 24-byte payload.
- `pet_save_format.h` follows the simulator 64-byte save slot header layout, including magic
  `0x50455453`, schema version, payload length, 64-bit counter, timestamp and CRC32.
- `pet_resource_format.h` follows simulator resource manifest magic, header and 28-byte entry layout.

Remaining intentional differences:
- The board repository keeps the P1 requested file names (`pet_protocol.h`, `pet_key.h`,
  `pet_resource_format.h`) even where the simulator splits them as `pet_packet.h`, `pet_input.h`,
  `pet_resource_manifest.h`, and `pet_nfc_pair_payload.h`.
- `pet_display_profile_t` extends simulator `PetScreenProfile` with RGB565, rotation, flush mode,
  owner and safe-area details needed by the future Jieli display owner/flush boundary.
- `pet_platform_t` extends simulator `PetPlatformCallbacks` with the future board callback surface for
  display, input, storage, audio, BLE packet, NFC scan/poll and power. Real hardware binding remains
  disabled until P2+.
- The board P1 save header is aligned, but full simulator `pet_device_save_payload_t` is not copied in
  P1 because this phase freezes interface headers only and must not change current MVP-A save behavior.

Known risks:
- Before real BLE bridge work, confirm whether the board should add compatibility wrapper headers named
  exactly like the simulator (`pet_packet.h`, `pet_input.h`, etc.) or keep only the P1 requested names.
- Real LCD flush may need stricter RGB565 alignment, byte order or async completion rules than the
  simulator profile can express.
- Real VM/Flash ownership, item IDs, capacity and power-fail behavior are still unconfirmed.
- Resource container naming constants are frozen for P1, but actual Jieli resource-tool output and
  external-Flash addressing still need validation.

## P2 Jieli Platform HAL Skeleton Risks

Status: skeleton boundary exists; real hardware work is still pending.

Open items:
- Display profile values in `pet_display_jieli.c` are 454x454 placeholders. P3 must confirm true panel
  size, shape, safe area, RGB565 byte order, flush alignment, TE/wait behavior and owner policy.
- Raw Jieli key mapping is not connected. P3 must map board key events to `PetKeyEvent` without
  changing current MVP-A LVGL key behavior.
- VM/Flash save ownership, capacity, IDs, erase/write timing and atomic A/B behavior remain pending
  for P6. P2 must not write persistent storage.
- BLE packet bridge is not connected. P7 must choose the Jieli transport path and avoid private BLE
  structs leaking into shared headers.
- NFC card and pair scan paths are not connected. P7 must confirm reader driver ownership, UID format,
  timeout behavior and pair payload flow.
- Power/battery callbacks return fixed values or zero. A later phase must confirm whether to use
  `get_vbat_percent`, `vbat_check`, or another SDK power API.
- The existing winmk post-build/resource packaging failure remains independent of P2. P2 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P3 Display Profile + Input Mapping POC Risks

Status: P3 resolves the basic source audit and POC mapping design, but real board behavior still needs
hardware logs and measurements.

Open items:
- The 454x454 profile is sourced from the active LVGL port and SH8601A board config, but the true LCD
  visible mask, safe area, RGB565 byte order, TE timing and flush alignment still require board-test
  confirmation.
- The raw IO key codes 0, 1, 2 and 3 are sourced from the board `iokey_list`; final physical labeling
  for left/up, right/down, OK and cancel still needs serial button-log confirmation.
- SDK native key events expose click, long, hold/repeat and up. A true down event was not confirmed in
  the audited path, so P3 uses a placeholder raw DOWN code only inside the PetEgg POC mapper.
- Long/hold timing is currently inferred from SDK scan counts and POC constants. Real key latency,
  repeat cadence and hold duration must be measured on the development board.
- `PET_RESULT_AGAIN` is used as the shared ABI's current no-event return for an empty P3 input queue;
  add an explicit `NO_EVENT` result only through a future shared ABI version bump if needed.
- The existing winmk post-build/resource packaging failure remains independent of P3. P3 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P4 Render Owner Boundary Risks

Status: P4 adds an in-memory owner boundary only. Real LVGL flush ownership and Pet2D rendering remain
unverified on hardware.

Open items:
- The P4 owner state machine is not yet tied to the SDK LCD driver lock, TE wait, DMA state, or flush
  completion path. A later phase must decide where the real display lock lives.
- MVP-A LVGL now acquires `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` before shell rendering, but the existing
  low-level LVGL flush callback remains unchanged. Final integration must ensure the flush callback
  cannot write while PET2D owns the display.
- `apps/watch/pet2d_boundary/` only validates owner handoff. It does not prove RGB565 byte order,
  clipping, dirty-rect alignment, framebuffer ownership, resource loading, or Pet2D frame timing.
- Same-owner re-acquire is currently a reentrant-like stub policy without reference counting. Real
  driver integration may need a stricter lock or lifecycle contract.
- No debug UI entry was added in P4, so owner self-tests are compile/log validation hooks rather than a
  field-operable menu path.
- The existing winmk post-build/resource packaging failure remains independent of P4. P4 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P5 Resource Manifest Adapter Risks

Status: P5 validates the shared manifest ABI against a tiny in-memory fixture only. It does not prove
the production Jieli resource path or external-Flash packaging.

Open items:
- The formal resource package generator is still pending. A later phase must decide how manifest.bin,
  sprites.pak, anim_table.bin, font.bin and sfx.pak are generated from the Jieli UI tools / image_dll
  and how those binaries are versioned.
- External Flash offsets, sizes, erase regions, CRC table ownership and update/rollback behavior remain
  unconfirmed. P5 does not call NOR/SFC/`flash_file_info`.
- IMB / image_dll / `open_image_by_id` / `ui_res_flash_info_get` integration is still future work and
  must not be inferred from the in-memory fixture.
- Runtime byte order, compression format, palette handling, dirty-rect alignment and IMB-readable
  address requirements still need board validation.
- The P5 test blob marks resource 1001 as a background-role placeholder, but the frozen P1/simulator ABI
  has no dedicated BACKGROUND enum; it is encoded as RGB565 SPRITE to avoid changing shared ABI.
- The P5 test blob does not represent full production resource coverage, large assets, compression,
  font/SFX packs, multi-pet manifests or content-card activation.
- The existing winmk post-build/resource packaging failure remains independent of P5. P5 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.
