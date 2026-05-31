# P25 Simulator Bring-up / Shared Core Consistency Harness

Current baseline:
`4565e7d2b9fbed148d421b290df49fc5ed36e30b feat(petegg): add mvp a placeholder renderer contract`

P25 adds a minimal host-side consistency harness for the P22-P24 scene/action/render contract. It is not
a full PC simulator, not SDL rendering, not HOME/Observe, not a formal resource path and not shared-core
completion.

## Goals And Non-Goals

Goals:

- Compile the scene/action/render contract from a host-oriented tools directory.
- Replay a fixed input sequence and produce stable text logs.
- Exercise the P24 renderer contract without Jieli private headers.
- Add small screen/key/save/packet fixtures as ABI sentinels for future simulator bring-up.
- Keep Jieli runtime safety gates unchanged.

Non-goals:

- No Windows/Mac simulator UI.
- No SDL window or real drawing.
- No HOME/Observe gameplay.
- No full Pet2D runtime.
- No formal resources, resource packages, external Flash, virfat or raw NOR.
- No real NFC, audio or BLE.
- No formal pet-state save writes and no P21 item 206/207 production use.

## Host Harness Files

The harness lives in `tools/sim_consistency/`:

- `README.md`: scope, build note and golden-output location.
- `sim_consistency_replay.h/.c`: replay state model, renderer checks and ABI fixtures.
- `sim_consistency_golden.h/.c`: embedded expected replay text.
- `sim_consistency_main.c`: optional host executable entry; compares replay output against golden text.
- `sim_consistency_build_check.c`: compile-check entry point.
- `build_sim_consistency.bat`: tries `clang`, `gcc`, `cl` or `%CC%`.
- `golden/p25_scene_replay_expected.txt`: tiny committed text golden.

The harness includes only shared PetEgg headers and the P24 renderer contract. It does not call
`pet_platform_jieli`, display owner APIs, syscfg, BLE, NFC, audio, files or resource package code.

## Scene Replay Design

The main replay uses a fixed 96x64 stage and 32x32 placeholder pet, starting at `pet=32,16`.

Sequence:

1. ENTER
2. TICK 0
3. LEFT_UP click
4. TICK +33ms
5. TICK +160ms
6. RIGHT_DOWN click
7. TICK +33ms
8. OK click
9. TICK +33ms
10. TICK +300ms
11. CANCEL click

Timeout replay:

1. ENTER
2. TICK to 4000ms
3. EXPECT timeout exit

Each logged line records state, pose, pet position, dirty rect, command count, first command, skipped
flush count and exit reason.

## Renderer Consistency Checks

The replay and fixture checks cover:

- enter: dirty rect `96x64`, command count 2, first command `STAGE_PATCH`;
- left/right move: pet x changes by 8 and dirty union is `40x32`;
- OK pose: pose becomes `HAPPY` and dirty rect is `32x32`;
- idle no-change: command count 0 and skipped flush increases;
- cancel: exit reason `CANCEL`;
- timeout: exit reason `TIMEOUT`;
- P24 renderer self-test: rect area, union, clip, clamp, stage patch, movement, pose-only and idle skip.

## Screen / Key / Save / Packet Fixtures

Screen profile fixture:

- `width = 454`
- `height = 454`
- RGB565 rect flush mode
- rotation 0
- safe area 0..453

Key replay fixture:

- LEFT_UP / RIGHT_DOWN / OK / CANCEL use the shared `PetProductKey` values.
- replay emits `PET_KEY_EVENT_CLICK` only.

Save slot fixture:

- Uses `pet_save_slot_header_t`.
- Checks `PET_SAVE_MAGIC`, version, payload length, counter and CRC field.
- Uses only a local stack struct; no syscfg write and no P21 item 206/207.

Packet fixture:

- Uses `pet_packet_t`.
- Checks packet magic, version, PING type, seq/ack/len and CRC field.
- Does not start BLE or use any transport.

## Golden Output

The committed golden file is:

`tools/sim_consistency/golden/p25_scene_replay_expected.txt`

Expected summary:

```text
step=enter state=IDLE pose=IDLE pet=32,16 dirty=96x64 cmd_count=2 cmd0=STAGE_PATCH skipped=0 exit=NONE
step=tick0 state=IDLE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=1 exit=NONE
step=left state=MOVE_LEFT pose=STEP pet=24,16 dirty=40x32 cmd_count=2 cmd0=CLEAR_DIRTY skipped=1 exit=NONE
step=left_done state=IDLE pose=IDLE pet=24,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE
step=right state=MOVE_RIGHT pose=STEP pet=32,16 dirty=40x32 cmd_count=2 cmd0=CLEAR_DIRTY skipped=1 exit=NONE
step=ok state=ACTION pose=HAPPY pet=32,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE
step=ok_done state=IDLE pose=IDLE pet=32,16 dirty=32x32 cmd_count=1 cmd0=PET_PLACEHOLDER skipped=1 exit=NONE
step=cancel state=DONE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=2 exit=CANCEL
step=timeout_enter state=IDLE pose=IDLE pet=32,16 dirty=96x64 cmd_count=2 cmd0=STAGE_PATCH skipped=0 exit=NONE
step=timeout state=DONE pose=IDLE pet=32,16 dirty=0x0 cmd_count=0 cmd0=NONE skipped=1 exit=TIMEOUT
fixtures=PASS screen=1 key=1 save=1 packet=1 renderer=1
```

## Host Executable Verification

A normal host compiler (`clang`, `gcc`, `cl` or `cc`) was not available in `PATH` on the current machine.
Therefore P25 verifies the host harness with syntax-only compile checks and records host executable
verification as pending. The `build_sim_consistency.bat` script is included for the next machine that has
a host compiler installed.

## Jieli Compile Result

P25 compile checks executed:

- `sim_consistency_replay.c` syntax-only check: passed.
- `sim_consistency_build_check.c` syntax-only check: passed.
- `sim_consistency_golden.c` syntax-only check: passed.
- `pet2d_mvp_a_renderer_contract_compile_check.c`: passed.
- `pet2d_mvp_a_scene_action_loop_compile_check.c`: passed.
- `pet2d_mvp_a_scene_skeleton_compile_check.c`: passed.
- `pet2d_scene_compile_check.c`: passed.
- `pet2d_perf_poc_compile_check.c`: passed.
- `pet_selftest_compile_check.c`: passed.
- `pet_save_jieli_syscfg_compile_check.c`: passed.

Default full Jieli build was run once and generated `cpu/br28/tools/sdk.elf`, so source compile/link
passed. The command then exited non-zero in the existing resource/download post-processing stage with
tool execution/open-file messages such as `open file JL failed`; this is treated as SDK post-processing
environment behavior, not a P25 harness source or link failure. Build outputs were cleaned after the
check.

## Capability Snapshot

P25 adds snapshot bits:

- `has_sim_consistency_harness = 1`
- `sim_consistency_scene_replay = 1`
- `sim_consistency_renderer_contract = 1`
- `sim_consistency_screen_profile_fixture = 1`
- `sim_consistency_key_replay_fixture = 1`
- `sim_consistency_save_slot_fixture = 1`
- `sim_consistency_packet_fixture = 1`
- `full_pc_simulator_enabled = 0`
- `sdl_simulator_enabled = 0`

These bits mean a consistency harness exists. They do not mean a complete simulator exists.

## Safety State

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `home_observe_enabled = 0`
- `full_pet2d_runtime_enabled = 0`
- `pet2d_runtime_enabled = 0`
- `full_pc_simulator_enabled = 0`
- `sdl_simulator_enabled = 0`
- No HOME/Observe route.
- No full Pet2D runtime.
- No external Flash / virfat / raw NOR route.
- No NFC, audio or real BLE path.
- No formal pet-state save writes.
- No P21 item 206/207 production use.

## Why P25 Is Not A Full Simulator

P25 has no graphics window, no SDL loop, no simulator asset pipeline, no simulated storage broker, no
input device integration and no runtime UI. It is a deterministic consistency harness that can later be
called by a real PC simulator.

## Why P25 Is Not Shared Core Completion

P25 proves a small contract slice can be replayed outside the Jieli runtime. It does not prove final
shared-core parity, production renderer semantics, resource loading, animation metadata, persistence
schema, protocol transport or HOME/Observe behavior.

## Remaining Risks

- Host executable still needs verification on a machine with a host C compiler.
- Golden output is text-only and manually curated.
- Replay model mirrors the current Jieli placeholder contract and may need refactoring when the PC
  simulator becomes the source of truth.
- Formal resources and external Flash delivery remain paused.

## P26 Recommendation

Recommended next step: P26 should either run this harness inside the real PC simulator tree and compare
against simulator shared-core output, or start a bounded HOME/Observe placeholder scene that consumes the
P24/P25 contract while keeping the full runtime disabled.
