# P19 High-res Motion / Performance POC

## Scope

P19 adds a bounded high-res dirty-rect performance probe on top of the P18 LVGL/Pet2D handoff. It does
not implement HOME/Observe, does not reopen external Flash, and does not enable the full Pet2D runtime.

## POC Mode Design

- `PET2D_PERF_MODE_RECT_32`: 32x32 generated RGB565 moving patch, 60-frame Debug entry.
- `PET2D_PERF_MODE_RECT_64`: 64x64 generated RGB565 moving patch, 60-frame Debug entry.
- `PET2D_PERF_MODE_RECT_96`: 96x96 generated RGB565 moving patch, 60-frame Debug entry.
- `PET2D_PERF_MODE_RECT_128`: API-visible optional mode, currently unsupported for motion because a
  moving old/new dirty union would exceed the 128x128 maximum scratch-buffer limit.

Each supported frame moves horizontally by 8 pixels with boundary bounce, computes the old/new dirty
union, renders a generated background plus sprite pattern into a static scratch buffer, then flushes that
dirty rect through the existing owner-guarded real-flush POC path.

## Stats

`pet2d_perf_stats_t` records:
- run count;
- frame attempt/success/fail counts;
- logic/render/flush/frame total, min, max and average milliseconds;
- approximate FPS as `fps * 100`;
- last rect size, dirty rect size, requested frame count, mode and result.

The probe samples logic time from motion-step start to end, render time from dirty-surface pattern fill
start to end, flush time around the gated LCD flush call, and frame time from frame start through optional
delay. The current board-test serial summary prints one compact line per mode and includes FPS,
flush/frame timing and dirty rect size; logic/render totals remain available through
`pet2d_perf_poc_get_stats()` but are not printed per frame to avoid log spam and because the millisecond
timer commonly rounds the tiny logic/render slices to 0 ms.

## Integration

- `apps/watch/pet2d_scene/pet2d_perf_poc.*` owns the performance POC.
- Debug actions `P19 Perf32`, `P19 Perf64` and `P19 Perf96` are manual-only engineering entries.
- `PET_SELFTEST_PET2D_PERF_POC` validates argument bounds, dirty-union math and default gate behavior
  without writing the LCD.
- Capability snapshot adds `has_pet2d_perf_poc = 1` while keeping `pet2d_runtime_enabled = 0`.

## Board Test Plan

For real performance numbers, use a temporary local build with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`,
enter the Debug page, then run:

1. `P19 Perf32`
2. `P19 Perf64`
3. `P19 Perf96`

Record the summary line for each mode:
- `ret`
- `frame_success_count` / `frame_fail_count`
- `approx_fps_x100`
- `flush_avg_ms` / `flush_max_ms`
- `frame_avg_ms` / `frame_max_ms`
- `last_dirty_w` / `last_dirty_h`
- LVGL owner recovery and Debug page redraw

## Board Test Results

Executed on COM3 at 1000000 baud with a temporary `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` build.

| Mode | Frames | Success | Fail | FPS x100 | Avg flush ms | Max flush ms | Avg frame ms | Max frame ms | Dirty rect | Result |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | ---: |
| 32x32 | 60 | 60 | 0 | 4545 | 21 | 30 | 22 | 30 | 40x32 | 0 |
| 64x64 | 60 | 60 | 0 | 4511 | 21 | 30 | 22 | 30 | 72x64 | 0 |
| 96x96 | 60 | 60 | 0 | 4477 | 20 | 30 | 22 | 30 | 104x96 | 0 |

The log also showed LVGL owner release before each run and LVGL owner reacquire / Debug page redraw after
each run. No panic/assert/WDT/HardFault/exception line was observed in the captured serial window. The
user-provided video showed the manual Debug run visually; `ffprobe` was not available in this local
environment, so the video was used as operator-side confirmation while the numeric result comes from the
serial log.

## Compile And Build

- `pet2d_scene_compile_check.c` syntax-only: passed.
- `pet2d_perf_poc_compile_check.c` syntax-only: passed.
- `pet_selftest_compile_check.c` syntax-only: passed.
- `pet2d_perf_poc.c` syntax-only with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`: passed.
- Default full build generated `cpu/br28/tools/sdk.elf`.
- The build command exited non-zero in the existing post-processing/download-resource phase with
  `res.ori already exists`, `ui_upgrade already exists`, `The system cannot execute the specified
  program`, `The system cannot find the file specified`, and `open file JL failed`; no P19 compile or
  link error was observed.

## Safety Boundary

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` must be 0 in committed source.
- `real_lcd_flush_enabled = 0`.
- `pet2d_runtime_enabled = 0`.
- No HOME/Observe, no full runtime, no background map scrolling.
- No external Flash / virfat / raw NOR package route.
- No PNG/JPG/GIF/JSON parsing.
- No full framebuffer or LVGL flush callback replacement.
- No VM/Flash/syscfg writes.
- NFC, audio/speaker and real BLE two-board validation remain Future Scope.

## Gap To Final HOME/Observe

P19 measures only generated local dirty-rect motion. It does not cover scene composition, formal
resources, transparency, IMB acceleration, large background restore, frame pacing policy, or production
input routing.

## Next Recommendations

1. P20 engineering test menu / integration report.
2. P21 internal save/syscfg POC if still required.
3. P22 MVP-A Pet2D scene skeleton.
