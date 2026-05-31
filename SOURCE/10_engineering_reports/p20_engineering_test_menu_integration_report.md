# P20 Engineering Test Menu / Integration Report

## Summary

P20 is an engineering integration snapshot on top of baseline
`5ddab34 feat(petegg): add high res motion performance poc`.

This phase does not add gameplay, does not implement HOME/Observe, does not enable the full Pet2D
runtime, and does not reopen external Flash / virfat / raw NOR resource work. It records the P1-P19
capability matrix, the current engineering entry model, the safe macro state and the remaining V0.2 gaps
before P21/P22.

## Current Engineering Entry Model

The current lightweight engineering status model has two parts:

- `apps/watch/pet_selftest/` provides compile/self-test integration through `pet_selftest_run_all`,
  `pet_selftest_run_case` and `pet_selftest_get_capability_snapshot`.
- The MVP-A Debug page retains manual board entries for visual tests: `P18 Scene`, `P19 Perf32`,
  `P19 Perf64` and `P19 Perf96`.

P20 does not add a new on-device production engineering menu. That avoids adding another default UI path
while still documenting the existing test hooks. All retained Debug entries are manual-only and are safe
when `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` is 0.

## P1-P19 Capability Matrix

| Phase | Capability | Current verification | Status |
| --- | --- | --- | --- |
| P1 | Shared Interface Pack | compile-time ABI checks | C99 shared headers present; no hardware dependency |
| P2 | Jieli HAL skeleton | compile/self-test | callback table and stubs present |
| P3 | Display/Input POC | compile/self-test | profile and private input mapping POC present |
| P4 | Render owner boundary | self-test | LVGL/PET2D owner state machine present |
| P5 | Resource manifest adapter | self-test | in-memory manifest parser and fixture present |
| P6 | Save transaction memory backend | self-test | A/B save transaction model verified in memory only |
| P7 | Protocol/debug adapter | self-test | packet ABI, BLE loopback and fake NFC debug paths present |
| P8 | Platform selftest snapshot | self-test | capability snapshot aggregates P1-P7 and later phases |
| P9 | Display flush owner POC | self-test | owner-guarded no-op/real gate present |
| P10 | Tiny real LCD flush POC | manual board test | macro-gated tiny flush path verified |
| P11 | Pet2D minimal flush POC | manual board test | small Pet2D-boundary surface flush verified |
| P12 | Repeated flush dirty rect POC | manual board test | finite repeated dirty-rect flush verified |
| P13 | Resource sprite minimal surface POC | manual board test | P5 fixture sprite blit to small surface verified |
| P14 | Minimal movement + key calibration | manual board test | key mapping and sprite movement POC verified |
| P15 | Movement latency stats | manual board test | 10/30/60 movement runs passed; avg key-to-flush around 21-22 ms |
| P16 | Real resource read probe | manual board test | read-only probe returned `PET_RESULT_NOT_FOUND`; no crash |
| P17S | External Flash pause decision | engineering report | transformed payload and raw NOR reset recorded; route paused |
| P18 | Pet2D scene handoff POC | manual board test | LVGL -> PET2D -> LVGL ownership handoff verified |
| P19 | High-res dirty rect performance POC | manual board test | 32x32/64x64/96x96 60-frame probes verified |

## Real-board Verified

- Tiny gated LCD flush through the display owner guard.
- Pet2D minimal surface flush and repeated dirty-rect flush.
- Resource-derived sprite fixture blit from the in-memory P5/P13 path.
- Raw-key mapping calibration for the current 4-key board.
- Minimal movement and key-to-flush latency stats.
- P16 read-only resource probe safely reported missing `petegg/manifest.bin`.
- P18 scene handoff held PET2D ownership for the bounded test scene, then restored LVGL.
- P19 performance probes completed:

| Mode | Frames | Success | Fail | FPS x100 | Avg flush | Max flush | Avg frame | Max frame | Dirty rect |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 32x32 | 60 | 60 | 0 | 4545 | 21 ms | 30 ms | 22 ms | 30 ms | 40x32 |
| 64x64 | 60 | 60 | 0 | 4511 | 21 ms | 30 ms | 22 ms | 30 ms | 72x64 |
| 96x96 | 60 | 60 | 0 | 4477 | 20 ms | 30 ms | 22 ms | 30 ms | 104x96 |

128x128 remains unsupported for moving dirty-union testing because moving a 128x128 rect would exceed the
current 128x128 maximum scratch-buffer limit.

## Compile-time / Self-test Verified

- Shared ABI field/layout checks.
- Platform HAL skeleton callback presence.
- Display profile and owner state machine checks.
- Resource manifest parser, CRC and lookup checks against the small fixture.
- Save transaction memory backend and fault-path checks.
- Protocol packet ABI, fake NFC and BLE loopback helper checks.
- Display flush parameter/owner gate checks.
- Pet2D scene and performance POC gate/stat checks.
- Self-test capability snapshot bits, including `has_pet2d_scene_handoff` and `has_pet2d_perf_poc`.

## Stub / Fake Only

- Real NFC remains unavailable on the current development board; only fake/stub/self-test paths are kept.
- Real audio/speaker playback remains unavailable on the current development board; callback stubs only.
- Real BLE two-board link cannot be validated with one board; packet ABI and loopback helpers only.
- Real VM/Flash/syscfg PetEgg save ownership is not enabled by the P6 memory backend.

## Future Scope

- HOME/Observe Pet2D scene implementation.
- Full Pet2D runtime enablement and production scene graph.
- IMB/hardware acceleration and formal renderer profiling.
- Formal asset pipeline, transparency/RLE/compression policy and production resource metadata.
- External Flash / virfat / raw NOR resource route, pending vendor/toolchain clarification.
- Internal save/syscfg integration if a later phase needs persistent PetEgg state.
- Production engineering test menu and final integration report.
- Real NFC, audio/speaker and BLE two-board validation on suitable hardware.

## Safe State

Committed-source expectations remain:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`.
- `real_lcd_flush_enabled = 0`.
- `pet2d_runtime_enabled = 0`.
- P18/P19 Debug entries are manual-only.
- No boot-time Pet2D scene, no default HOME/Observe path and no automatic real LCD flush path.
- No external Flash / virfat / raw NOR resource package route.
- No VM/Flash/syscfg writes from PetEgg POC paths.
- No real NFC, audio or BLE two-board path.

## V0.2 Technical Gaps

- HOME/Observe is not implemented.
- Pet2D still has no full background restore, transparency, layered composition or animation state model.
- Performance data covers bounded dirty-rect test patches only; it is not a final HOME/Observe FPS target.
- IMB/hardware acceleration is not connected.
- Formal resource generation and external Flash delivery are paused.
- Real persistence through internal save/syscfg is still not wired to PetEgg.
- The on-device engineering menu is still the existing Debug page plus self-test APIs, not a polished
  product diagnostic screen.

## P21/P22 Recommendation

Recommended next steps:

1. P21 internal save/syscfg POC if persistent PetEgg state is still required before scene work expands.
2. P22 MVP-A Pet2D scene skeleton, using compiled/internal fixtures and the proven P18/P19 handoff path.
3. A later production engineering menu pass can consolidate Debug entries, self-test summaries and board
   diagnostics once the next runtime capability is chosen.
