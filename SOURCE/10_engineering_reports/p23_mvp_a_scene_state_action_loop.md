# P23 MVP-A Scene State / Placeholder Action Loop

Baseline: `84e14c814229ec28ae1eda99c46025477e218df2 feat(petegg): add mvp a pet2d scene skeleton`.

## Goal

P23 extends the P22 MVP-A Pet2D scene skeleton with a clearer scene state and placeholder action-loop
contract. It is a reusable structure for later HOME/Observe work, but it is not HOME/Observe, not a
formal pet gameplay loop and not full Pet2D runtime enablement.

## Non-Goals

- No HOME/Observe default route or product gameplay.
- No formal resources, animation table, transparency/RLE/compression policy or image decoding.
- No external Flash / virfat / raw NOR resource route.
- No full framebuffer, no LVGL flush callback replacement and no full Pet2D runtime.
- No production pet-state save and no use of P21 syscfg item 206/207 as production slots.
- No real NFC, audio/speaker or real BLE two-board integration.

## Scene State Model

`pet2d_mvp_a_scene_state_t` now describes the renderer-facing action state:

| State | Meaning |
| --- | --- |
| `NONE` | Reset/no scene model initialized. |
| `ENTER` | Owner handoff and context initialization in progress. |
| `IDLE` | Scene is active and ready for input. |
| `MOVE_LEFT` | A short left-step action is active. |
| `MOVE_RIGHT` | A short right-step action is active. |
| `ACTION` | Placeholder pose action is active. |
| `EXITING` | Manual/timeout/error exit is releasing owner. |
| `DONE` | Scene exited cleanly. |
| `ERROR` | Scene hit an error path. |

`pet2d_mvp_a_scene_model_t` exposes state, pose, current and previous pet positions, frame index, action
timing, enter/timeout timestamps and exit reason.

## Action And Pose Model

Placeholder poses:

| Pose | Use |
| --- | --- |
| `IDLE` | Default pet block. |
| `HAPPY` | OK-triggered placeholder action. |
| `BLINK` | OK-triggered placeholder action variant. |
| `STEP` | LEFT/RIGHT movement action pose. |

Move actions use `PET2D_MVP_A_SCENE_MOVE_ACTION_MS = 160`. OK pose actions use
`PET2D_MVP_A_SCENE_POSE_ACTION_MS = 300`. When the action duration expires, tick advances the scene
back to `IDLE`.

## Input Transition Table

| Input | Transition | Position/Pose | Dirty rect |
| --- | --- | --- | --- |
| Enter | `ENTER -> IDLE` | Center pet in 96x64 stage | Initial 96x64 patch |
| LEFT_UP | `IDLE/ACTION -> MOVE_LEFT` | `pet_x -= 8`, clamped | old/new union, typically 40x32 |
| RIGHT_DOWN | `IDLE/ACTION -> MOVE_RIGHT` | `pet_x += 8`, clamped | old/new union, typically 40x32 |
| OK | `IDLE/MOVE -> ACTION` | cycle happy/blink/idle placeholder pose | pet rect, 32x32 |
| CANCEL | active state -> `EXITING -> DONE` | no pet movement | release owner |
| timeout | active state -> `EXITING -> DONE` | no pet movement | release owner |

## Tick Advance

`pet2d_mvp_a_scene_skeleton_tick()` checks timeout first, then checks whether the current MOVE/ACTION
duration has elapsed. If the action is complete, it resets pose to `IDLE`, increments
`action_done_count`, emits a concise action log and returns the state to `IDLE`.

The regular render interval remains `250 ms`, and real LCD writes remain gated by
`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC`.

## Renderer-Facing Contract

`pet2d_mvp_a_scene_draw_cmd_t` is the P23 renderer-facing contract:

- `x`, `y`, `w`, `h`: current placeholder pet rect.
- `pose`: current placeholder pose.
- `pattern_id`: currently mirrors pose for generated pattern selection.
- `flags`: marks action-specific rendering hints.

This is intentionally small and does not require formal resources or a full scene graph.

## Dirty Rect And Buffer Strategy

The dirty rect stays the old/new union of the 32x32 pet rect unless the initial full stage patch is
pending. The maximum stage patch remains 96x64, so no full framebuffer is allocated. With the real flush
macro off, render/flush stats still update but the LCD write is skipped.

## Self-Test

P23 adds `PET_SELFTEST_MVP_A_SCENE_ACTION_LOOP`. The self-test is side-effect free and validates:

- enter -> `IDLE`;
- LEFT_UP enters `MOVE_LEFT`, moves left 8 pixels and produces a 40x32 dirty rect;
- tick action completion returns to `IDLE`;
- RIGHT_DOWN enters `MOVE_RIGHT` and moves right;
- OK enters `ACTION`, changes pose and updates draw command fields;
- CANCEL sets cancel exit reason and exits;
- timeout sets timeout exit reason and exits;
- owner is restored after the test.

Capability snapshot additions:

- `has_mvp_a_scene_action_loop = 1`
- `mvp_a_scene_action_loop_selftest = 1`
- `mvp_a_scene_action_loop_debug_entry = 1`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Debug Entry

P23 reuses the P22 Debug-page `P22 Scene` entry. The entry remains manual-only, does not run at boot,
does not enter HOME/Observe by default, and is safe with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`.

New action logs use the `[PET2D_MVP_A_ACTION]` prefix and report state, pose, input, old/new position,
action completion and exit reason.

## Verification

Compile checks executed:

- `pet2d_mvp_a_scene_action_loop_compile_check.c` syntax-only: pass.
- `pet2d_mvp_a_scene_skeleton_compile_check.c` syntax-only: pass.
- `pet2d_mvp_a_scene_skeleton.c` syntax-only with default real-flush macro off: pass.
- `pet2d_mvp_a_scene_skeleton.c` syntax-only with temporary `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`:
  pass.
- `pet2d_scene_compile_check.c` syntax-only: pass.
- `pet2d_perf_poc_compile_check.c` syntax-only: pass.
- `pet_selftest_compile_check.c` syntax-only: pass.
- `pet_save_jieli_syscfg_compile_check.c` syntax-only: pass.

Default `.vscode\winmk.bat all` generated `cpu/br28/tools/sdk.elf`, so the P23 source and link path are
valid. The command returned non-zero during the known SDK resource/download post-processing stage with
messages such as stack-size warnings, `The system cannot execute the specified program`, missing files
and `open file JL failed`; those are recorded as existing post-processing environment behavior rather
than P23 compile/link failures.

No P23 real-board run has been performed yet. P22 already verified the owner handoff and basic manual
entry path on hardware; P23 can be board-tested with a temporary real-flush build if needed.

## Safety Boundary

Committed state must remain:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`

P23 does not reopen external Flash / virfat / raw NOR, does not modify download/watch or fat_comm, does
not connect NFC/audio/real BLE, and does not write production pet-state saves.

## P21/P22 Relationship

P21 proved a small syscfg A/B save POC. P23 does not write pet state and does not use item 206/207 as
production slots. P22 proved the LVGL/Pet2D skeleton handoff; P23 adds the reusable state/action-loop
contract on top of that skeleton.

## Remaining Risks

- Placeholder poses are not final pet animation states.
- Draw commands are not yet a full renderer API.
- Dirty rect restoration is still bounded to the 96x64 test patch.
- No IMB acceleration or formal resource binding is present.
- No power-fail or persistence behavior is added.

## Next Step

Suggested P24: MVP-A placeholder renderer contract refinement or a small engineering test menu that
groups P18/P19/P21/P22/P23 manual entries and status snapshots.
