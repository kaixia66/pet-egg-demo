# P7 Protocol Debug Adapter

## BLE/NFC SDK Path Audit

- Active MVP-A BLE/NFC-facing service stubs still live in
  `apps/watch/mvp_a/services/mvp_a_platform.c` and return `MVP_A_RESULT_NOT_READY`.
- Jieli BLE candidates are spread through `include_lib/system/user_cfg.h`, `apps/common/config`,
  `apps/common/third_party_profile/jieli`, SmartBox/RCSP paths and BT stack libraries. P7 does not use
  these paths.
- NFC candidates include `apps/common/device/nfc/nfc_fm11nt081d.c` and board/config references. P7
  does not start reader scans or RF transactions.
- Existing P2 files `pet_ble_jieli.c`, `pet_nfc_jieli.c` and `pet_debug_jieli.c` were stubs. P7 keeps
  the real hardware path disabled and adds only macro-isolated test/debug queues.

## Packet ABI Alignment

`apps/watch/pet_protocol_jieli/` follows P1 `pet_protocol.h` and simulator `pet_packet.h` /
`pet_nfc_pair_payload.h`:
- packet magic `0xE6`;
- packet version `1`;
- 10-byte serialized packet header;
- max payload `64` bytes;
- CRC16 CCITT-FALSE over header + payload, excluding the stored CRC;
- seq/ack/flags are preserved by build and validate;
- NFC pair payload is 24 bytes, with CRC over the first 22 bytes.

Bad packet magic/version/len/CRC map to `pet_packet_status_t` values. NFC pair bad version and CRC map
to shared `PET_RESULT_BAD_VERSION` and `PET_RESULT_BAD_CRC`.

## BLE Loopback Queue

`pet_ble_jieli.c` owns a fixed 4-packet loopback queue:
- default build: `send_packet` and `poll_packet` return `PET_RESULT_NOT_READY`;
- `PET_PLATFORM_JIELI_TEST` or `PET_DEBUG`: loopback can be enabled explicitly;
- send validates magic/version/len/CRC before enqueue;
- full queue returns `PET_RESULT_BUSY`;
- empty poll returns `PET_RESULT_AGAIN`;
- no malloc, no BLE private structs and no real controller calls.

## NFC Fake Card And Pair

`pet_nfc_jieli.c` owns small fake queues for test/debug builds:
- default build: scan/poll callbacks return `PET_RESULT_NOT_READY`;
- fake mode can inject `pet_nfc_card_t` with UID, type and `card_id`;
- fake mode can inject a validated 24-byte `pet_nfc_pair_payload_t`, which is converted to the current
  platform `pet_nfc_pair_t` fields for polling;
- empty fake queues return `PET_RESULT_AGAIN`;
- no real reader scan, no RF transaction and no card used-flag write.

## Debug Injection

`pet_debug_jieli.c` exposes debug/test-only helpers:
- fake millis and fake now_sec;
- fake battery percent/voltage;
- enable BLE loopback;
- inject BLE packet;
- inject NFC card;
- inject NFC pair payload;
- clear all debug state.

The default product path does not enable these helpers and does not change MVP-A behavior.

## Self-Test

P7 self-tests cover:
- packet build/validate success;
- bad magic, bad version, oversize len and corrupted CRC;
- seq/ack preservation;
- NFC pair payload build/validate and CRC/version failures;
- disabled BLE send returning `NOT_READY`;
- enabled BLE loopback send/poll, empty poll and queue-full behavior;
- invalid BLE packet rejection;
- disabled NFC poll returning `NOT_READY`;
- fake NFC card injection and one-shot poll;
- fake NFC pair payload injection and one-shot poll;
- fake battery/time set/read and clear-all debug state.

The self-tests are compile/log validation hooks and do not touch real BLE, NFC, storage, display or
MVP-A save paths.

## Follow-Up

Future BLE/NFC phases should define the GATT service/characteristics, MTU/fragmentation, reconnect and
timeout behavior, pairing security, NFC UID/signature/used-flag policy, RF scan timing and field-test
log strategy before enabling any real hardware path.
