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

## P6 Save Transaction Adapter Risks

Status: P6 validates the shared save slot ABI and rollback behavior only against caller-owned memory
slots. It does not prove real Jieli VM/Flash persistence.

Open items:
- Real syscfg/VM/Flash item IDs, ownership, region size, erase policy and update ownership remain
  unconfirmed. P6 intentionally does not write `syscfg_write`, VM, NOR, SFC or files.
- The P1/simulator save header has a payload CRC32 but no header CRC field. P6 uses a staged invalid
  CRC header for partial-write tests; a future real backend may need an erase/program/commit marker
  policy that matches the physical storage medium.
- Power-fail atomicity, brownout behavior, page alignment, sector erase timing and wear leveling still
  require board tests before connecting the adapter to real storage.
- Low-battery blocking is a test flag only. It must later be wired to the confirmed battery/power API.
- The existing MVP-A save path still uses `syscfg_read/write(MVP_A_SAVE_VM_ID)`. Migration timing and
  compatibility from MVP-A's magic/version/checksum save data into shared save slots remain undecided.
- `pet_platform_jieli` storage callbacks remain `NOT_READY` / `UNSUPPORTED`; later phases must decide
  whether shared save routes through `pet_platform_t` or a storage-specific backend.
- The existing winmk post-build/resource packaging failure remains independent of P6. P6 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P7 Protocol Debug Adapter Risks

Status: P7 validates packet/NFC-pair ABI behavior and test-only queues. It does not prove real BLE or
NFC hardware behavior.

Open items:
- Real BLE GATT service UUIDs, characteristic UUIDs, notify/write-with-response policy and security
  requirements remain undefined.
- BLE MTU, fragmentation, retransmit policy, connection state, reconnect behavior, timeout handling and
  packet scheduling still need a real transport phase.
- The P7 loopback queue validates `pet_packet_t` ABI and queue behavior only; it does not start
  advertising, scanning, connection, pairing or BLE controller paths.
- Real NFC reader ownership, scan timing, UID length, card type mapping, signature/counter validation
  and used-flag write policy remain unconfirmed.
- Fake NFC card/pair injection does not represent RF success rate, anti-collision behavior, card
  security, tag write endurance or field failure behavior.
- NFC pair payload security is still placeholder-level. Pairing token/session binding, replay
  protection, privacy and tamper checks need a later design.
- `PET_DEBUG` / `PET_PLATFORM_JIELI_TEST` injection must stay disabled in the default product path.
- The existing winmk post-build/resource packaging failure remains independent of P7. P7 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P8 Platform Self-Test Snapshot Risks

Status: P8 aggregates P1-P7 compile/self-test entry points and reports a capability snapshot. It is a
software integration baseline only, not a board-level hardware validation.

Open items:
- Real LCD flush ownership still needs binding to the SDK LCD driver lock, TE/wait behavior, DMA flush
  completion and byte-order/alignment checks.
- Real key queue integration still needs raw key serial logs, down/up/long/repeat semantics and latency
  measurements without breaking the MVP-A input path.
- Real storage still needs confirmed syscfg/VM/Flash region ownership, power-fail behavior, erase
  policy, wear leveling, low-battery blocking and migration timing from `mvp_a_save`.
- Real BLE/NFC RF paths still need GATT/NFC reader ownership, MTU/fragmentation, reconnect/timeout,
  UID/signature/used-flag policy, pairing security and field reliability tests.
- P8 skipped test accounting only means macro-gated test hooks are not compiled in a default product
  build; it is not evidence that hardware capabilities are absent or present on the board.
- Later work should split into real hardware tracks: LCD/Pet2D display, storage/power-fail and BLE/NFC
  RF validation. The existing winmk post-build/resource packaging failure remains independent of P8.

## P9 Display Flush Owner POC Risks

Status: P9 adds a no-op display flush owner guard and diagnostics. It does not prove real LCD flush or
Pet2D rendering on hardware.

Open items:
- The true safe real LCD API for PetEgg/Pet2D is still undecided. Candidate SDK calls include
  `lcd_draw_area`, `lcd_data_copy`, `lcd_wait`, `lcd_wait_te` and IMD/IMB callbacks, but P9 does not
  call them.
- TE, IMD busy, DMA completion, `lcd_data_copy_wait`, `lcd_wait` and async flush-done ownership still
  need board validation before enabling a real flush path.
- RGB565 byte order, cache flush requirements, PSRAM/no-cache address handling, line stride, row/column
  alignment and round-screen clipping must be confirmed on the panel.
- LVGL and future Pet2D still need a real mutual-exclusion strategy at the low-level LCD driver boundary;
  P9 only enforces the PetEgg platform callback owner state.
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` defaults to 0. Enabling it requires explicit manual hardware
  authorization and a tiny, bounded smoke test plan.
- The existing winmk post-build/resource packaging failure remains independent of P9. P9 must not
  modify toolchain paths, resource scripts or download packaging to mask that environment issue.

## P10 Tiny Real LCD Flush POC Risks

Status: P10 adds a manual-only tiny flush gate and visual probe. A temporary local board-test build
confirmed a center 8x8 flush can succeed with `ret=0`, but the committed default still does not perform
real LCD writes.

- The center tiny flush smoke test passed once on the current board. It still needs repeat testing across
  cold boot, wake/sleep, different LVGL pages and longer observation windows.
- `lcd_draw_area()` appears to accept `(index, buffer, left, top, width, height, wait)`, but RGB565 byte
  order, cache maintenance, PSRAM/non-cache address constraints and row/column alignment still need panel
  validation.
- The QSPI/ST77903 path uses `lcd_wait_te()`, `lcd_data_copy()` and `lcd_data_copy_wait()`; P10 chooses the
  simpler `lcd_draw_area()` candidate for manual POC and does not prove the QSPI copy path.
- LVGL recovery after a manual tiny flush must be board-tested. P10 does not yet define a final policy for
  restoring or invalidating LVGL content after an out-of-band tiny rectangle write.
- Entering later Pet2D runtime or broader dirty-rect rendering should depend on successful tiny flush board
  testing and an explicit owner/flush recovery plan.

## P11 Pet2D Minimal Real Flush POC Risks

Status: P11 adds a 16x16 RGB565 minimal visual helper and manual Pet2D-boundary flush probe gate. A
temporary macro-enabled board test displayed the center pattern with `ret=0`, while the committed default
still skips real LCD writes and does not enable Pet2D runtime.

- The minimal pattern is only a smoke-test surface. It does not prove HOME/Observe rendering, sprite
  animation, resource streaming, dirty-rect scheduling or frame pacing.
- RGB565 byte order can be visually inspected with the quadrant/diagonal pattern, but it still needs repeat
  confirmation before larger rendering phases.
- Dirty-rect alignment, even coordinate constraints, row stride and larger 32x32 or repeated flush behavior
  remain untested.
- LVGL recovery after out-of-band Pet2D-boundary writes still needs a final policy for invalidation,
  owner handoff and page restoration.
- The resource-to-surface path is not connected. P11 generates pixels procedurally and does not load
  sprites from the P5 manifest adapter or formal Jieli resource tools.
- Any temporary Debug UI trigger used for board testing must remain local and must not be committed.

## P12 Repeated Tiny Flush + Dirty Rect Alignment POC Risks

Status: P12 adds dirty-rect helper coverage and a finite repeated-flush probe gate. A temporary board
build passed repeated 16x16/32x32/64x64 center flushes plus odd-coordinate and out-of-bounds checks. The
committed default still skips real LCD writes, keeps full Pet2D runtime disabled and does not keep a Debug
UI trigger.

Current hardware scope adjustment:
- The available development board has no NFC and no speaker, and only one board is available. Real NFC,
  real audio/SFX and real BLE two-board link validation are therefore Future Scope.
- P12 keeps NFC fake/stub tests, audio callback stubs and BLE packet ABI/loopback coverage only; those do
  not prove the missing hardware paths.

Open items:
- Repeated 16x16/32x32/64x64 passed once on the current board, but still needs repeat testing across cold
  boot, longer observation windows, page transitions and LVGL recovery cases.
- Dirty-rect odd-coordinate and near-edge cases are represented in source, but physical panel alignment,
  clipping and cache behavior must be confirmed on hardware.
- Physical LEFT/UP and RIGHT/DOWN key labeling appears inconsistent with the earlier test instruction. Do
  not change default key behavior until raw key logs and physical labeling are confirmed together.
- Larger rectangles, repeated stress beyond the bounded P12 probe, frame pacing and performance remain
  future work.
- LVGL recovery after repeated out-of-band writes still needs a final policy for invalidation, owner
  handoff and page restoration.
- Resource sprite to RGB565 surface conversion remains disconnected; P12 patterns are procedural fixtures.
- The existing winmk post-build/resource packaging failure remains independent of P12. P12 must not modify
  toolchain paths, resource scripts or download packaging to mask that environment issue.

## P13 Resource Sprite To Minimal Surface POC Risks

Status: P13 connects the P5 in-memory test resource fixture to the minimal Pet2D-boundary surface and
manual flush gate. A temporary board build displayed the resource-derived pattern once with `ret=0`, but
it does not prove the production resource package, external Flash path or full Pet2D runtime.

Current hardware scope adjustment remains unchanged:
- The available development board has no NFC and no speaker, and only one board is available. Real NFC,
  real audio/SFX and real BLE two-board link validation remain Future Scope.

Open items:
- P13 validates only fixture resource IDs 1001 and 2001. It does not prove formal manifest generation,
  sprite metadata completeness, palette/transparent color rules, RLE/compression or multi-frame assets.
- The resource data is treated as raw RGB565. Production resources still need byte-order, alignment,
  compression and IMB-readable address validation.
- External Flash/NOR/SFC/`flash_file_info` reading remains disconnected; P13 uses only the compiled P5
  test blob.
- Resource-derived flush is still manual-only and macro-gated. Repeated sprite flush, movement, LVGL
  recovery and performance remain future work.
- The bridge from resource sprite views into a full Pet2D runtime, scene graph, dirty-rect scheduler and
  HOME/Observe rendering is not implemented.
- The existing winmk post-build/resource packaging failure remains independent of P13. P13 must not modify
  toolchain paths, resource scripts or download packaging to mask that environment issue.

## P14 Minimal Sprite Movement + Raw Key Calibration POC Risks

Status: P14 adds passive key-calibration and a minimal movement gate. It does not yet make the PetKey
mapping authoritative for physical labels and does not route real key events into Pet2D by default.

Current hardware scope adjustment remains unchanged:
- The available development board has no NFC and no speaker, and only one board is available. Real NFC,
  real audio/SFX and real BLE two-board link validation remain Future Scope.

Open items:
- P14 raw-key board logs confirm the current P3 mapping table: HOME/raw0 -> OK, PLUS/raw1 -> LEFT_UP,
  MINUS/raw2 -> RIGHT_DOWN, SHORTCUT/raw3 -> CANCEL. No P3 mapping correction is made in P14.
- The earlier LEFT_UP/RIGHT_DOWN confusion is now recorded as a Debug navigation/instruction ambiguity:
  LEFT/UP and RIGHT/DOWN can reach different Debug actions depending on current selected item and wrap
  direction, so future board-test instructions must name the expected prompt/action, not only a direction.
- Minimal movement currently moves horizontally for LEFT_UP/RIGHT_DOWN to avoid overcommitting to
  physical up/down semantics. True directional movement needs confirmed labels and product UX rules.
- The P14 movement probe is visible only briefly because LVGL reacquires owner and redraws the Debug page
  after the manual flush. A persistent interactive scene requires an explicit LVGL handoff / scene mode.
- The first movement probe does not erase old sprite pixels through a full scene system; dirty rect merge
  and old-rect clearing remain future work.
- Key latency, long/repeat cadence and debouncing need later measurement if movement becomes interactive.
- The bridge from P14 movement helpers to a complete Pet2D runtime, HOME/Observe scene and resource
  animation pipeline is not implemented.
- The existing winmk post-build/resource packaging failure remains independent of P14. P14 must not modify
  toolchain paths, resource scripts or download packaging to mask that environment issue.

## P15 Key Latency + Movement Repeated Flush Stats POC Risks

Status: P15 adds coarse movement timing and repeated-step stats around the existing manual movement gate.
A temporary board build passed 10/30/60 repeated movement probes, but P15 does not turn the movement POC
into a default interactive scene and does not enable full Pet2D runtime.

Current hardware scope adjustment remains unchanged:
- The available development board has no NFC and no speaker, and only one board is available. Real NFC,
  real audio/SFX and real BLE two-board link validation remain Future Scope.

Open items:
- P15 latency values are coarse application-level timestamps from the movement helper and platform
  `millis`; they are not ISR-level key latency, debounce latency or SDK key-queue latency.
- The LVGL Debug page still redraws over the manual movement probe after the owner is reacquired. The
  user observed the P15 pattern appear and then disappear; a persistent scene requires a formal
  LVGL/Pet2D handoff mode.
- Dirty rect tracking records old/new bounding rectangles, but old-pixel clearing, background restore and
  dirty-rect merging across multiple sprites remain future work.
- Repeated movement is bounded and manual-only. Higher-frequency movement, long/repeat key cadence,
  frame pacing and performance metrics need later dedicated tests.
- Full Pet2D runtime, HOME/Observe scene, resource animation pipeline and external resource package
  reading are still not implemented.
- The existing winmk post-build/resource packaging failure remains independent of P15. P15 must not modify
  toolchain paths, resource scripts or download packaging to mask that environment issue.
