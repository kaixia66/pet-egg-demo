# P24 MVP-A Placeholder Renderer Contract Refinement

Current baseline:
`8a41938a276a9a7b52b3d91e54287664daf389e8 feat(petegg): add mvp a scene action loop`

P24 refines the P23 placeholder draw command into a minimal renderer-facing contract. It is not
HOME/Observe, not the formal renderer, not a formal resource path and not the full Pet2D runtime.

## Goals And Non-Goals

P24 adds a small, testable contract between the MVP-A scene model and a future renderer:

- scene model -> render plan -> dirty rect -> optional gated flush;
- stage patch / pet rect / previous pet rect / dirty rect responsibilities;
- placeholder render commands for stage restore and pet draw;
- rect helpers for valid/union/clip/clamp/area/equality;
- idle no-change skip behavior.

P24 does not add production art, animation tables, transparency, RLE, compression, IMB acceleration,
external Flash, virfat, raw NOR, NFC, audio, real BLE, HOME/Observe, formal pet-state saves or P21 item
206/207 production save usage.

## Renderer Contract Data

New contract types live in `apps/watch/pet2d_scene/pet2d_mvp_a_renderer_contract.h`:

- `pet2d_mvp_a_render_cmd_type_t`
  - `NONE`
  - `STAGE_PATCH`
  - `PET_PLACEHOLDER`
  - `CLEAR_DIRTY`
- `pet2d_mvp_a_render_pattern_t`
  - `STAGE`
  - `IDLE`
  - `HAPPY`
  - `BLINK`
  - `STEP`
- `pet2d_mvp_a_rect_t`
  - `x / y / w / h`
- `pet2d_mvp_a_render_cmd_t`
  - command type, destination rect, placeholder pattern, pose, alpha mode and flags.
- `pet2d_mvp_a_render_plan_t`
  - stage rect, previous pet rect, current pet rect, dirty rect, up to four commands and plan flags.
- `pet2d_mvp_a_render_stats_t`
  - frame index, command count, dirty size, render/flush/frame timing and fail/skip counters.

`pet2d_mvp_a_scene_draw_cmd_t` is now an alias of `pet2d_mvp_a_render_cmd_t` for compatibility with the
P23 action-loop getter.

## Render Plan Rules

- Initial enter builds a `STAGE_PATCH` plan. Dirty rect is the 96x64 stage patch and command count is at
  least one.
- Movement builds an old/new pet union. A typical 8-pixel horizontal step from a 32x32 placeholder pet
  produces a 40x32 dirty rect.
- Pose-only changes keep the pet position and dirty the 32x32 pet rect.
- Idle no-change ticks produce a zero-command plan, mark skipped flush and do not generate a meaningless
  dirty rect.
- CANCEL and timeout remain scene-layer exits. The renderer contract does not release display ownership.

## Scene Integration

`pet2d_mvp_a_scene_skeleton.c` now builds a render plan before filling/flushing a dirty patch. With
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`, the scene records skipped/unsupported flush behavior and does
not write the LCD. With a temporary real-flush build, the existing P22/P23 Debug-page `P22 Scene` entry
can still exercise the path.

The Debug entry is not renamed and no new production menu is added. It remains manual-only, does not run
at boot and does not enter HOME/Observe.

## Self-Test

P24 adds `PET_SELFTEST_MVP_A_RENDERER_CONTRACT`. It is side-effect free and validates:

- 32x32 pet rect area;
- old/new union = 40x32 for an 8-pixel horizontal step;
- clipping to the 96x64 stage;
- clamp to stage bounds;
- initial stage patch plan = 96x64;
- movement plan with stage restore and pet draw;
- pose-only dirty = 32x32;
- idle no-change plan with skipped flush and no dirty rect.

Capability snapshot additions:

- `has_mvp_a_renderer_contract = 1`
- `mvp_a_renderer_contract_selftest = 1`
- `mvp_a_renderer_contract_debug_entry = 0`
- `mvp_a_renderer_contract_used_by_scene_debug_entry = 1`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Compile And Build Result

Compile checks executed:

- `pet2d_mvp_a_renderer_contract_compile_check.c`: passed.
- `pet2d_mvp_a_scene_action_loop_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton_compile_check.c`: passed.
- `pet2d_scene_compile_check.c`: passed.
- `pet2d_perf_poc_compile_check.c`: passed.
- `pet_selftest_compile_check.c`: passed.
- `pet_save_jieli_syscfg_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton.c` syntax-only with default real-flush gate off: passed.
- `pet2d_mvp_a_scene_skeleton.c` syntax-only with temporary
  `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`: passed.

Default `.vscode\winmk.bat all` generated `cpu/br28/tools/sdk.elf`, so the P24 source and link path are
valid. The post-link resource/download stage still reported existing SDK environment issues such as
stack-limit warnings, `The system cannot execute the specified program`, `The system cannot find the
file specified` and `open file JL failed`; these are treated as post-processing/tooling behavior rather
than P24 contract compile failures.

## Real-Board Verification

No new P24 board run was required. P24 is a contract/self-test refinement; P22 already verified the
real-board LVGL -> Pet2D -> LVGL handoff, basic input path, OK pose toggle, CANCEL exit, timeout exit
and LVGL recovery. P24 can be board-tested later with a temporary real-flush build if renderer-plan logs
need to be correlated with screen output.

## Safety State

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`
- No HOME/Observe route.
- No full Pet2D runtime enablement.
- No external Flash / virfat / raw NOR route.
- No NFC, audio or real BLE path.
- No formal pet-state save writes.
- No use of P21 syscfg item 206/207 as production slots.

## P21/P22/P23 Relationship

P21 proved a small syscfg A/B save POC, but P24 does not write pet state. P22 proved the real-board
scene handoff and Debug manual entry. P23 added state/action loop semantics. P24 narrows the renderer
contract that a later HOME/Observe renderer or PC simulator bring-up can consume.

P24 still does not mean the shared core is complete: production resources, animation metadata,
transparent compositing, background restore, simulator alignment, IMB acceleration and product scene
contracts remain open.

## Remaining Risks

- The command set is intentionally tiny and placeholder-only.
- Full background restore and multi-object dirty merging are not implemented.
- P24 self-test does not prove real-board timing; it validates the contract shape.
- PC simulator/shared-core parity is still pending.
- Formal resource and external Flash delivery remain paused.

## P25 Recommendation

Suggested next step: P25 should either align this renderer contract with the PC simulator/shared core, or
start a bounded HOME/Observe placeholder scene that consumes the P24 plan without enabling the full
runtime.
