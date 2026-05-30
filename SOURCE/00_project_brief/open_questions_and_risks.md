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
