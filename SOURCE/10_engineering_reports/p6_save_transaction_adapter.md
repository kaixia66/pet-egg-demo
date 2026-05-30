# P6 Save Transaction Adapter

## MVP-A Save Audit

- `apps/watch/mvp_a/core/mvp_a_save.c` owns the current MVP-A save object.
- `mvp_a_save_data_t` uses magic `0x4D565041`, version `1`, `data_len` and an FNV-style checksum over
  the structure excluding the checksum field.
- `apps/watch/mvp_a/services/mvp_a_platform.c` persists MVP-A through
  `syscfg_read(MVP_A_SAVE_VM_ID, ...)` and `syscfg_write(MVP_A_SAVE_VM_ID, ...)`, where
  `MVP_A_SAVE_VM_ID` is `40`.
- Domain modules call `mvp_a_save_store()` and may restore an in-memory backup if that store fails.
- P6 does not alter this path and does not migrate existing MVP-A save data.

Future Jieli storage candidates found during audit include `syscfg_read`, `syscfg_write`,
`apps/watch/user_api/vm_api.c`, `vm_read`/`vm_write` references, NOR/SFC modules and NAND VM support.
Those are only candidates; P6 does not connect them.

## 64-Byte Slot Header ABI

P6 follows `apps/watch/pet_shared/include/pet_save_format.h`, aligned with simulator
`shared_portable/include/pet_save_format.h`:
- `PET_SAVE_MAGIC` = `0x50455453`;
- `PET_SAVE_VERSION` = `1`;
- `PET_SAVE_SCHEMA_VERSION` = `2`, with min-readable schema `1`;
- payload type `PET_SAVE_PAYLOAD_DEVICE`;
- serialized slot header size `64` bytes;
- `payload_len` at offset 12, `counter` at 16, `timestamp_sec` at 24 and payload `crc32` at 32.

The simulator ABI CRC covers payload bytes only. There is no header CRC field in the frozen P1 header.

## A/B Selection

`pet_save_jieli_choose_latest_slot()` validates both slots independently:
- both valid: choose the larger counter;
- only A valid: choose A;
- only B valid: choose B;
- both empty/corrupt: return `PET_RESULT_NOT_FOUND`.

Corrupt latest slots do not block rollback because the older valid slot is still selected.

## Transaction And Rollback

`pet_save_jieli_write_transaction()` writes only the inactive slot:
1. validate current A/B slots and pick the latest counter;
2. choose the inactive slot as the target;
3. build a new header with `counter = latest + 1` or `1` for an empty save set;
4. write a staged header with an intentionally invalid CRC;
5. write payload bytes;
6. write the final header with the correct payload CRC;
7. verify the target slot.

The old slot is never erased before the new slot verifies. If write fails after the staged header or
after the payload, the target slot remains invalid and `load_latest` still returns the previous slot.

## Memory Backend

`apps/watch/pet_save_jieli/pet_save_jieli_memory_backend.*` owns a caller-provided in-memory test
backend:
- no malloc;
- caller-provided slot A and slot B byte arrays;
- clear/init helpers;
- fault modes for `fail_before_write`, `fail_after_header`, `fail_after_payload` and
  `corrupt_after_write`;
- low-battery write block flag.

The backend is a P6 fixture only and is not a production storage route.

## Self-Test

`pet_save_jieli_self_test()` covers:
- invalid backend open;
- empty A/B slots returning `PET_RESULT_NOT_FOUND`;
- first write with counter 1;
- load latest payload/counter;
- second write with counter 2 to the inactive slot;
- highest-counter selection when both slots are valid;
- latest-slot payload CRC corruption rolling back to the older slot;
- older-slot corruption while latest remains readable;
- fail-after-header and fail-after-payload rollback;
- fail-before-write behavior;
- oversize payload rejection;
- low-battery write blocking;
- bad version rejection;
- corrupted payload CRC rejection;
- corrupt-after-write verify failure.

The self-test does not write real VM/Flash/syscfg and does not touch MVP-A save.

## Platform Storage Boundary

`apps/watch/pet_platform_jieli/pet_storage_jieli.c` remains unchanged: reads return
`PET_RESULT_NOT_READY` and writes return `PET_RESULT_UNSUPPORTED`. P6 keeps the save adapter separate
because VM item IDs, Flash region ownership, erase policy, page alignment, low-battery policy and
power-fail behavior are still unconfirmed.

## Follow-Up

Before a real storage phase, confirm VM/Flash ownership, slot capacity, sector erase behavior, program
granularity, brownout behavior, write blocking under low battery, wear leveling expectations and
whether MVP-A's existing save format migrates into the shared save slot payload or remains independent.
