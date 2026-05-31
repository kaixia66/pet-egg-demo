# P13 Resource Sprite To Minimal Surface POC

## Scope

P13 connects the P5 in-memory resource fixture to the Pet2D boundary's caller-owned RGB565 surfaces. It
does not enable full Pet2D runtime, HOME/Observe, formal art resources, external Flash, image decoding,
full framebuffer allocation, LVGL flush replacement, storage writes, NFC, audio or real BLE.

Committed source keeps `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=0`, so the resource-derived visual probe is
manual-only and cannot write the LCD by default.

## Resource Sprite View Helper

New helper: `apps/watch/pet2d_boundary/pet2d_resource_sprite_poc.*`.

It opens the P5 `pet_resource_jieli_test_blob`, validates the manifest through the P5 parser, and exposes
small raw RGB565 views for fixture entries:

- resource ID 1001: 8x8 RGB565 background-role fixture, encoded as SPRITE in the P1 ABI.
- resource ID 2001: 4x4 RGB565 sprite fixture.

Validation covers resource existence, type `SPRITE`, format `RGB565`, expected dimensions, data size and
the P5 entry CRC path. The helper uses no malloc, no file IO, no Flash IO and no PNG/JPG/GIF/JSON decoder.

## Blit To Minimal Surface

`pet2d_minimal_visual_blit_sprite()` copies raw fixture pixels into a caller-owned
`pet2d_minimal_surface_t`.

Behavior:
- supports normal destination coordinates;
- clips negative or partially out-of-bounds destinations;
- does not implement transparency in P13;
- does not allocate memory;
- does not enter full Pet2D runtime.

Self-test covers 4x4 sprite blit into a 32x32 surface, 8x8 fixture blit, clipping and invalid arguments.

## Resource-Derived Visual Probe

`pet2d_boundary_resource_sprite_flush_probe()` is the manual board-test entry:

- with the committed macro off, returns `PET_RESULT_UNSUPPORTED` before touching the LCD;
- with a local macro-enabled build, requires display-owner guard;
- creates a 32x32 surface;
- fills a P12 32x32 pattern background;
- blits fixture 1001 at a small offset;
- blits fixture 2001 at surface center;
- calls the P10 real-flush rectangle API at screen center;
- releases PET2D owner if it acquired it.

Stats track attempts, successes, failures, last resource ID, surface size, sprite size and last result.

## Self-Test And Capability Snapshot

P13 adds `PET_SELFTEST_RESOURCE_SPRITE_SURFACE` and
`has_resource_sprite_surface_probe_gate`.

`pet_selftest_run_all()` validates resource lookup and surface blit behavior, then reports the real panel
gate as skipped. It does not automatically write the LCD.

Committed capability flags remain:

- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Manual Board-Test Plan

For a local-only board test:

1. Temporarily enable `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`.
2. Temporarily add a Debug action that releases LVGL owner and calls
   `pet2d_boundary_resource_sprite_flush_probe()`.
3. Build and download to the board.
4. Trigger the action once.
5. Expected safety check: if LVGL still owns the display, the probe returns `PET_RESULT_BUSY` and should
   not write the panel.
6. Expected manual success: after LVGL owner is released, a resource-derived 32x32 pattern appears at
   screen center.
7. Confirm no panic, assert, WDT, HardFault, reset, infinite loop or continuous repaint.
8. Restore the macro to 0 and remove any temporary Debug action before commit.

## Current Board Result

A temporary local board build was prepared with `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1` and a
non-committed MVP-A Debug `P13` / `Resource` action. The build downloaded successfully to the board and
the download log reported main Flash plus external Flash writes complete.

After COM3 reappeared, the action was triggered on hardware. The user visually confirmed the
resource-derived 32x32 pattern appeared on the screen.

Observed serial window:

- `[MVP_A][LVGL_OWNER] release ret=0 owner=0`
- `[P13_RESOURCE_SPRITE] debug trigger ret=0`
- `[MVP_A] ui key result=0 scene=DEBUG(9)`
- `[MVP_A][LVGL_OWNER] acquired owner=2`
- `[MVP_A][LVGL] render scene=9 title=Debug prompt=Resource action=P13`

No `panic`, `assert`, `WDT`, `HardFault` or `exception` keyword was observed around the trigger window.
`Reset` matches in the session were from boot/config history rather than a post-trigger fault.

The temporary debug action and macro-enabled source changes were restored after download and must not be
committed.

## Safety Boundary

Committed source state:

- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`
- no default Debug UI trigger
- no LVGL flush callback replacement
- no HOME/Observe rendering
- no formal resource loading
- no external Flash/NOR/SFC/`flash_file_info`
- no full framebuffer allocation
- no VM/Flash/syscfg writes
- no real NFC/audio/BLE hardware path

## Follow-Up

Recommended next stages:

1. P14 minimal sprite movement + key input.
2. P15 high-res motion/performance POC.
3. P16 real resource package / external Flash read POC.
