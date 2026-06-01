# P38 Simulator / Jieli Imported Scene Reconciliation

## Baseline

- P37 baseline: `5adfccc test(petegg): verify imported simulator scene on board`
- Branch: `codex/p38-sim-jieli-imported-scene-reconciliation`
- Simulator P35 source commit: `0cf4fdc62690da5660e2389a73cbd914ccbfe64f`
  `test(sim): add MVP-A scene export contract checks`
- Simulator P34 source commit: `cb5f9a61639292e6c459dc384fa20ecf2f8c3049`
  `test(sim): add bounded HOME observe placeholder scene`
- Jieli P36 source commit: `982d7a87dc6402a1abf3d223791fdebbdef2789e`
  `feat(petegg): import simulator HOME observe placeholder scene`
- Jieli P37 source commit: `5adfccc`
  `test(petegg): verify imported simulator scene on board`

## Goal

P38 reconciles the simulator P34/P35 bounded HOME/Observe placeholder scene contract with the Jieli
P36/P37 imported scene. The goal is to show that the imported scene has not drifted semantically before
P39 starts the first simulator-developed mini app scene.

P38 does not add gameplay, does not expand the scene, does not run new real-board smoke, does not enable
complete HOME/Observe and does not enable the full Pet2D runtime.

## Added Fixture

New files:

- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_reconcile.h`
- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_reconcile.c`
- `apps/watch/pet2d_scene/pet2d_mvp_a_home_observe_reconcile_compile_check.c`

The fixture embeds the P35 manifest constants that are relevant to the Jieli imported scene:

- stage / viewport `160x96`
- pet `32x32`
- initial pet position `64,32`
- move step `8px`
- move duration `160ms`
- action duration `300ms`
- timeout `4000ms`
- dirty expectations: enter `160x96`, move `40x32`, OK/action `32x32`

It also records the matrix sizes:

- exact match: 20 fields
- semantic match: 5 fields
- explicit non-match: 7 fields

## Reconciliation Matrix

### Exact Match

| Field | Simulator P34/P35 | Jieli P36/P37 | Result |
| --- | --- | --- | --- |
| scene id / scene name | `MVP_A_HOME_OBSERVE_PLACEHOLDER` | imported placeholder scene id | exact |
| scene status | `test_only` | imported placeholder / Debug-only | exact |
| screen/profile semantics | `round454` | 701N round 454 profile context | exact |
| stage / viewport | `160x96` | `160x96` | exact |
| pet rect | `32x32` | `32x32` | exact |
| initial pet position | `64,32` | `64,32` | exact |
| move step | `8px` | `8px` | exact |
| move duration | `160ms` | `160ms` | exact |
| action duration | `300ms` | `300ms` | exact |
| timeout | `4000ms` | `4000ms` | exact |
| enter dirty | `160x96` | `160x96` | exact |
| move dirty | `40x32` | `40x32` | exact |
| OK/action dirty | `32x32` | `32x32` | exact |
| CANCEL exit semantic | `DONE / CANCEL` | `DONE / CANCEL` | exact |
| TIMEOUT exit semantic | `DONE / TIMEOUT` | `DONE / TIMEOUT` | exact |
| `home_observe_enabled` | false | 0 | exact |
| `full_pet2d_runtime_enabled` | false | 0 | exact |
| `pet2d_runtime_enabled` | false | 0 | exact |
| formal pet save writes | false | none | exact |
| external Flash / virfat / raw NOR | false | paused / unused | exact |

### Semantic Match

| Field | Relationship |
| --- | --- |
| simulator pose enum vs Jieli pose enum | Values map to IDLE/HAPPY/BLINK/STEP semantics; names are local to each side. |
| simulator exit reason name vs Jieli numeric exit reason | Jieli preserves NONE=0, CANCEL=1, TIMEOUT=2, END=3, ERROR=4. |
| simulator render command semantics vs Jieli dirty flush path | Stage patch, clear/restore, pet placeholder and skip semantics are mapped to bounded dirty flush plans. |
| simulator offscreen CRC vs Jieli visual smoke evidence | CRC is host-only evidence; Jieli uses serial logs and visual smoke instead. |
| simulator replay transcript vs Jieli Debug manual sequence | ENTER/LEFT/RIGHT/OK/CANCEL/TIMEOUT maps to Debug `P36 Import` manual operation and P37 logs. |

### Explicit Non-match

| Field | Non-match Declaration |
| --- | --- |
| host CRC | Not a Jieli LCD CRC. |
| simulator offscreen pixels | Not proof of real LCD pixels. |
| simulator timing | Not real hardware flush timing. |
| production resource CRC | Not covered. |
| SDL visible renderer parity | Not covered. |
| real NFC / Audio / BLE | Not covered. |
| full HOME/Observe | Not covered or enabled. |

## P37 Smoke Relationship

P37 provides board-side manual evidence for the imported scene path:

- Debug `P36 Import` entry: PASS
- initial 160x96 patch: PASS
- LEFT_UP / RIGHT_DOWN 8px movement with 40x32 dirty rect: PASS
- OK placeholder action with 32x32 dirty rect: PASS
- CANCEL exit with `release_ret=0` and LVGL reacquire: PASS
- TIMEOUT exit with `release_ret=0` and LVGL reacquire: PASS
- fault strings: no `panic`, `assert`, `WDT`, `HardFault` or `exception` observed

P38 records this as `mvp_a_home_observe_imported_real_board_smoke_verified = 1`, but that bit is a
static record of the P37 manual smoke result. The snapshot and self-test do not execute real LCD writes.

## Self-test / Capability Snapshot

Added self-test case:

- `PET_SELFTEST_MVP_A_HOME_OBSERVE_RECONCILE`

The self-test is side-effect free and validates:

- imported constants match simulator manifest values;
- enter/move/OK/idle dirty rules match the simulator golden semantics;
- CANCEL/TIMEOUT numeric exit semantics match the simulator contract;
- the imported scene's existing self-test still passes;
- host CRC, SDL parity and production resource parity remain explicitly unclaimed;
- HOME/Observe and full runtime flags remain 0.

Capability snapshot additions:

- `has_mvp_a_home_observe_reconciliation = 1`
- `mvp_a_home_observe_reconciliation_selftest = 1`
- `mvp_a_home_observe_imported_real_board_smoke_verified = 1`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Compile / Build Results

Syntax-only checks executed:

- `pet2d_mvp_a_home_observe_reconcile_compile_check.c`: passed.
- `pet2d_mvp_a_home_observe_imported_compile_check.c`: passed.
- `pet2d_mvp_a_renderer_contract_compile_check.c`: passed.
- `pet2d_mvp_a_scene_action_loop_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton_compile_check.c`: passed.
- `pet2d_scene_compile_check.c`: passed.
- `pet2d_perf_poc_compile_check.c`: passed.
- `pet_selftest_compile_check.c`: passed.
- `pet_save_jieli_syscfg_compile_check.c`: passed.
- `tools/sim_consistency` syntax-only check: passed.

Full Jieli build:

- `.vscode\winmk.bat all` generated `cpu/br28/tools/sdk.elf`, so source compile/link passed.
- The post-link resource/download stage still reported the known SDK environment/tool messages such as
  `The system cannot execute the specified program` and `open file JL failed`. These are treated as
  existing post-processing/resource-tool issues, not P38 source/link failures.
- Build artifacts were cleaned before commit.

## Safety Boundary

Committed source keeps:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

P38 does not:

- enable complete HOME/Observe;
- enable full Pet2D runtime;
- add a boot/default HOME path;
- add production pet resources, formal animation tables, transparency/RLE/compression policy or IMB
  acceleration;
- claim Jieli LCD CRC parity or SDL-visible parity;
- restore external Flash / virfat / raw NOR resource loading;
- modify `download/watch`, `fat_comm`, external resource package or raw NOR paths;
- write formal pet-state saves;
- use P21 syscfg item 206/207 as production pet slots;
- connect real NFC, audio or BLE.

## Remaining Gaps

- Full HOME/Observe product scene remains unimplemented.
- The shared portable source is not yet the single compiled implementation across simulator and Jieli.
- Renderer timing under real hardware busy periods still needs a policy beyond skipped bounded flushes.
- Production art/resource parity, formal resource CRC, transparency/RLE/compression and IMB acceleration
  remain future work.
- Automated hardware regression does not exist; P37 is manual smoke evidence.
- External Flash / virfat / raw NOR remains paused.

## P39 Recommendation

Proceed to P39 as the first simulator-developed mini app scene, using the P38 reconciliation matrix as
the handoff checklist. P39 should still avoid full HOME/Observe, formal production resources, external
Flash, formal pet-state persistence, NFC, audio and real BLE unless a later phase explicitly reopens
those scopes.
