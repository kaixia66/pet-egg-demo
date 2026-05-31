# P8 Platform Self-Test Snapshot

## Scope

P8 adds `apps/watch/pet_selftest/` as a no-hardware integration snapshot for the P1-P7 PetEgg Jieli
port work. It aggregates existing self-tests, records pass/fail/skip counts and exposes a read-only
capability snapshot. It does not enable real LCD flush, Pet2D runtime, VM/Flash/syscfg writes, BLE
GATT, NFC reader scans or MVP-A default path changes.

## P1-P7 Capability Matrix

| Area | Current P8 snapshot | Real hardware enabled |
|---|---:|---:|
| Shared interface ABI | 1 | n/a |
| Jieli platform HAL callback table | 1 | 0 |
| Display profile | 1 | 0 |
| Display owner boundary | 1 | 0 |
| Input mapping POC | 1 | 0 |
| Render owner / Pet2D boundary | 1 | 0 |
| Resource manifest adapter | 1 | 0 |
| Save transaction adapter | 1 | 0 |
| Protocol packet helper | 1 | 0 |
| BLE loopback test | 1 | 0 |
| NFC fake test | 1 | 0 |
| Debug injection test | 1 | 0 |

Real LCD flush, real key queue, real Flash storage, real BLE, real NFC and Pet2D runtime remain `0` in
`pet_platform_capability_snapshot_t`.

## Aggregator Design

`pet_selftest_run_case()` dispatches a single `pet_selftest_case_t`. `pet_selftest_run_all()` iterates
all cases and fills:

- `total`
- `passed`
- `failed`
- `skipped`
- `failed_mask`
- `skipped_mask`

`PET_RESULT_UNSUPPORTED` is counted as skipped. This lets default product builds compile the
aggregator while leaving `PET_PLATFORM_JIELI_TEST` / `PET_DEBUG` BLE, NFC and debug injection paths
disabled. Test builds can define `PET_PLATFORM_JIELI_TEST=1` to run the full stub suite.

## Self-Test Coverage

| Case | Entry point |
|---|---|
| shared_interface | local ABI sanity check |
| platform_hal | local `pet_platform_jieli_get()` callback-table check |
| display_profile | `pet_platform_jieli_display_self_test()` |
| display_owner | `pet_display_jieli_owner_self_test()` |
| input_mapping | `pet_platform_jieli_input_self_test()` |
| render_owner_boundary | `pet2d_boundary_self_test()` |
| resource_manifest | `pet_resource_jieli_self_test()` |
| pet2d_resource_probe | `pet2d_boundary_resource_probe_self_test()` |
| save_transaction | `pet_save_jieli_self_test()` |
| protocol_packet | `pet_protocol_jieli_self_test()` |
| ble_loopback | `pet_ble_jieli_self_test()` |
| nfc_fake | `pet_nfc_jieli_self_test()` |
| debug_injection | `pet_debug_jieli_self_test()` |

No case calls real hardware. No case writes Flash, starts BLE/NFC RF, writes the LCD or allocates a
full framebuffer.

## Debug UI Entry

No MVP-A debug page entry was added in P8. The self-test aggregator is kept as a compile/self-test API
only because adding UI invocation would touch the interactive MVP-A debug path and is unnecessary for
the pre-hardware integration baseline.

## Follow-Up Recommendations

1. P9 Display Flush Owner + LCD No-op/Stub-to-Real POC.
2. P10 Pet2D Minimal Real Flush POC.
3. P11 Storage Real VM/syscfg POC.
4. P12 BLE/NFC Real Hardware POC.
