# P21 Internal Save / syscfg A-B POC

## Summary

P21 is implemented on baseline `8654e77 docs(petegg): add engineering integration report`.

The goal is a low-risk internal-save proof for the current 2M internal environment. It is not the full
pet growth save system. The POC keeps the P1/P6 save slot ABI, adds a syscfg-backed A/B adapter and wires
`PET_STORAGE_AREA_SAVE` to that backend.

## Backend

The selected backend is the Jieli SDK `syscfg_read` / `syscfg_write` persistent configuration API. The
backend code isolates these symbols inside `apps/watch/pet_save_jieli/pet_save_jieli_syscfg_backend.c`.
For `PET_PLATFORM_JIELI_TEST`, the same adapter uses a small fake-syscfg in-memory backend so syntax and
self-test behavior can be checked without linking SDK syscfg internals.

P21 reserves two PetEgg test namespace IDs:

- slot A: `206`
- slot B: `207`

The current audit of `apps/watch/include/user_cfg_id.h` shows known watch/FMNA VM IDs through `205`, so
206/207 are used as dedicated POC slots. This must be rechecked if the SDK assigns new IDs.

## A/B Slot Format

P21 reuses the P1/P6 `pet_save_slot_header_t`:

- magic: `PET_SAVE_MAGIC`
- version: `PET_SAVE_VERSION`
- schema version: `PET_SAVE_SCHEMA_VERSION`
- payload type: `PET_SAVE_PAYLOAD_DEVICE`
- payload length
- counter
- timestamp
- payload CRC32
- reserved bytes

Each syscfg slot is 160 bytes. The 64-byte header leaves a 96-byte maximum P21 payload.

## Write Strategy

The adapter reads both syscfg slots into caller-private static buffers, opens the existing P6 memory
backend over those buffers, then calls `pet_save_jieli_write_transaction()`.

After the P6 transaction chooses and stages the next A/B slot in RAM, P21 writes only that selected slot
back through `syscfg_write`. It then rereads both slots through `syscfg_read`, loads the latest valid slot
and verifies that counter, length and payload match the requested write.

This depends on the SDK `syscfg_write` item update being the safe persistent-write wrapper. P21 does not
perform raw Flash erase or direct NOR writes.

## CRC / Counter / Fallback

Selection and validation reuse P6 rules:

- empty all-zero/all-0xff slots are treated as not found;
- invalid magic/version/schema/payload type is rejected;
- payload length must fit the slot;
- CRC32 must match the payload;
- when both slots are valid, the highest counter wins;
- when the newest slot is corrupt, the previous valid slot is selected.

## Self-test Case

P21 adds `PET_SELFTEST_SAVE_AB_INTERNAL`.

The self-test sequence:

1. writes payload 1 to the alternate slot;
2. reads it back and records the selected counter;
3. writes payload 2 with a higher counter;
4. corrupts the latest PetEgg test slot;
5. reads latest again and verifies fallback to payload 1;
6. writes payload 3 to recover the namespace and leave a valid latest slot.

The test namespace is non-destructive to existing watch settings because only item IDs 206/207 are used.

For board verification, P21 also adds a Debug-page manual entry named `P21 Save`. The entry is not run
at boot and is not part of HOME/Observe. Pressing OK on that entry runs only
`PET_SELFTEST_SAVE_AB_INTERNAL` and prints a serial summary with explicit PASS/FAIL fields:

- `result=PASS/FAIL`: overall self-test result.
- `backend=syscfg` / `backend_id=2`: real SDK syscfg backend, not fake-syscfg.
- `items=206,207`: PetEgg test namespace item IDs.
- `slot_a` / `slot_b`: slot read status after the recovery write.
- `selected_slot`, `counter`, `payload_len`, `crc`: latest selected valid save slot.
- `write_cases`, `readback_cases`, `fallback_cases`: pass/fail case counts, not result codes.
- `manual_selftest_real_write_verified`: real syscfg write evidence from the manual Debug action.
- `snapshot_side_effect_free=1`: capability snapshots do not write syscfg.

## Platform Storage Hook

`pet_storage_jieli_read()` and `pet_storage_jieli_write_atomic()` now support only:

- area: `PET_STORAGE_AREA_SAVE`
- offset: `0`

Other storage areas remain `PET_RESULT_UNSUPPORTED`, and non-zero offsets return
`PET_RESULT_INVALID_ARGUMENT`.

## Capability Snapshot

P21 adds capability fields:

- `has_internal_save_syscfg_backend = 1`
- `internal_save_ab_supported = 1`
- `internal_save_crc_supported = 1`
- `internal_save_rollback_supported = 1`
- `internal_save_low_battery_guard_supported = 0`
- `internal_save_low_battery_guard_planned = 1`
- `internal_save_real_write_verified = 0`

`internal_save_real_write_verified` remains 0 in the snapshot because snapshot reads must be side-effect
free. The separate manual Debug self-test may verify a real syscfg write, but that result is reported as
`manual_selftest_real_write_verified` and documented from the board log instead of changing the
side-effect-free snapshot field.

Low-battery write veto is planned, not supported in P21. The P6 memory backend can simulate a
low-battery block, but the P21 syscfg write path does not yet call a real battery/power policy hook before
`syscfg_write`.

## Verification Status

Completed compile checks:

- `pet_save_jieli_compile_check.c` syntax-only
- `pet_save_jieli_syscfg_compile_check.c` syntax-only
- `pet_selftest_compile_check.c` syntax-only
- `pet_platform_jieli_compile_check.c` syntax-only
- direct syntax-only checks for `pet_save_jieli_syscfg_backend.c` and `pet_storage_jieli.c`

Real board syscfg write verification has been run through the Debug-page `P21 Save` manual action after
download. The first board run returned `ret=0` on the real syscfg backend, proving the self-test path can
write/read/corrupt/fallback/recover within item IDs 206/207. A follow-up log format fix changed ambiguous
`write=9/readback=9/fallback=9` fields into explicit pass/fail case counters before final reporting.

Final P21 board log after the summary fix:

```text
[MVP_A][P21] save_ab_internal begin
[MVP_A][P21] result=PASS backend=syscfg backend_id=2 non_destructive=1 items=206,207
[MVP_A][P21] slot_a=VALID slot_b=VALID selected_slot=B counter=4 payload_len=32 crc=0xd3ca3239
[MVP_A][P21] write_cases pass=3 fail=0
[MVP_A][P21] readback_cases pass=2 fail=0
[MVP_A][P21] fallback_cases pass=1 fail=0
[MVP_A][P21] real_syscfg_backend=1 manual_selftest_real_write_verified=1 snapshot_side_effect_free=1 low_battery_guard=planned
```

No `panic`, `assert`, `WDT`, `HardFault` or `exception` keywords were observed in the P21 watch window.

## Safety Boundaries

Committed-source expectations:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`
- no HOME/Observe;
- no full Pet2D runtime;
- no external Flash / virfat / raw NOR resource package;
- no file-system or SD-card save path;
- no raw Flash erase;
- no NFC, audio or real BLE path.

## Remaining Risks

- Real `syscfg_write` item-level atomicity under reset is not yet proven.
- Low-battery/power-fail guard is planned but not connected to a real battery veto for syscfg writes.
- Item IDs 206/207 must remain reserved for PetEgg or be moved to an officially assigned range.
- The final pet payload schema, migration/version policy and save size budget are still undefined.

## P22 Recommendation

Recommended next step: P22 MVP-A Pet2D scene skeleton, using compiled/internal fixtures and the existing
P18/P19 handoff path. If persistent pet state is required first, run the P21 syscfg self-test on hardware
and decide whether IDs 206/207 become the formal PetEgg save namespace.
