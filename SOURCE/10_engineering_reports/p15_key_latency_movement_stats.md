# P15 Key Latency + Movement Repeated Flush Stats POC

## Scope

P15 extends the P14 minimal movement POC with stats and coarse latency accounting. It stays inside the
existing Pet2D-boundary POC layer and does not enable full Pet2D runtime, HOME/Observe, formal resources,
external Flash, NFC, audio or real BLE.

The committed source must keep:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Movement Stats

`pet2d_movement_poc_stats_t` now records:
- key event count and movement step count;
- render attempt/success/fail counters;
- existing movement probe attempt/success/fail counters;
- key, logic, render and flush timestamp checkpoints;
- key-to-logic, key-to-render and key-to-flush-done durations;
- key-to-flush min/max/average totals;
- last old sprite position, new sprite position and dirty rectangle;
- last key, event and result.

The timestamp source is the platform `millis` callback. If an injected key event has no timestamp, the
movement helper substitutes the current millisecond value. This is a coarse application-level metric, not
an interrupt-level key latency measurement.

## Dirty Rect Strategy

P15 keeps the P14 movement surface at 32x32. For each movement step:
- old rect is the previous sprite rectangle;
- new rect is the updated sprite rectangle;
- dirty rect is the bounding union of old and new;
- x movement is clamped to the existing display-safe movement range.

P15 records the dirty union but still does not implement full background restoration. The old pixels may
remain visible during manual board probes until LVGL redraws the Debug page.

## Repeated Movement Probe

New repeated-step APIs:
- `pet2d_movement_poc_run_repeated_steps(direction_key, repeat_count, delay_ms)`
- `pet2d_boundary_movement_repeated_probe(repeat_count, delay_ms)`

The repeated probe is finite and capped at 60 steps. It bounces direction at the movement bounds to avoid
out-of-range coordinates. With `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` off, the boundary probe returns
`PET_RESULT_UNSUPPORTED` and does not touch the LCD.

## Self-Test And Gate Behavior

Self-test coverage includes:
- stats reset/get behavior;
- single-step old/new dirty union;
- key and movement counters;
- repeat-count invalid argument handling;
- macro-off repeated probe skip behavior;
- capability snapshot bits for movement stats and key latency probe gate.

`pet_selftest_run_all()` must not trigger real LCD writes. The real movement flush path remains a manual
board-test path only.

## Manual Board-Test Plan

For local hardware validation only:
1. Temporarily set `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` to 1.
2. Temporarily add MVP-A Debug actions for P15 10-step, 30-step and optional 60-step movement probes.
3. Release LVGL owner before invoking the probe.
4. Trigger the repeated probes from the Debug page.
5. Record serial logs for repeat count, return value, success/fail counts, dirty rect and latency stats.
6. Confirm no panic, assert, WDT, HardFault or reset.
7. Confirm LVGL owner can be reacquired and the Debug page can redraw.
8. Restore the macro to 0 and remove temporary Debug UI changes before commit.

## Board-Test Results

A temporary local board build enabled `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` and added non-committed
MVP-A Debug actions for `P15S10`, `P15S30` and `P15S60`. The actions were removed after the test and the
committed source returns the macro to 0.

Captured COM3 log results:
- `P15S10`: `repeat=10 ret=0 steps=10 render=10 ok=10 fail=0 dirty=355,211 48x32
  old=355,211 new=371,211 key_flush last=30 min=10 max=30 avg=21 total=210`
- `P15S30`: `repeat=30 ret=0 steps=30 render=30 ok=30 fail=0 dirty=372,211 48x32
  old=372,211 new=388,211 key_flush last=20 min=10 max=30 avg=21 total=650`
- `P15S60`: `repeat=60 ret=0 steps=60 render=60 ok=60 fail=0 dirty=372,211 48x32
  old=372,211 new=388,211 key_flush last=30 min=20 max=30 avg=22 total=1340`

The user visually confirmed the movement pattern displayed during testing, but it still disappears after
the LVGL Debug page reacquires the display owner and redraws. The log shows LVGL owner release before the
probe and reacquire/render after the probe. No `panic`, `assert`, `WDT`, `HardFault` or `exception`
keyword was observed in the captured P15 test window.

## Safety Boundary

P15 does not:
- consume the real Jieli key queue;
- change `mvp_a_app_key_event`, MVP-A default input behavior or board key tables;
- enable full Pet2D runtime or HOME/Observe;
- load formal resources or external Flash;
- allocate a full framebuffer;
- replace LVGL flush;
- write VM/Flash/syscfg;
- connect real NFC, audio or BLE hardware.

## Rollback

To roll back any local board-test build state, restore `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` to 0,
remove any temporary MVP-A Debug actions, rebuild, and confirm `git diff` no longer includes
`apps/watch/mvp_a/core/mvp_a_debug.c`, `apps/watch/mvp_a/core/mvp_a_debug.h` or app-common key hooks.
Build logs, `sdk.elf` and `cpu/br28/tools/download/watch/*` outputs are local byproducts and must not be
committed.

## Future Work

Recommended next stages:
1. P16 real resource package / external Flash read POC.
2. P17 Pet2D scene mode / LVGL handoff POC.
3. P18 high-frequency movement and performance POC.
