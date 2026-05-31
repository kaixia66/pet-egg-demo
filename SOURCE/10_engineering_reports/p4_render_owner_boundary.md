# P4 Render Owner Boundary POC

## Owner State Machine

`pet_display_jieli.c` keeps the display owner as an in-memory state:
- initial owner is `PET_DISPLAY_OWNER_NONE`;
- acquiring `PET_DISPLAY_OWNER_NONE` returns `PET_RESULT_INVALID_ARGUMENT`;
- acquiring `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` from NONE succeeds;
- acquiring the same owner again succeeds as a reentrant-like stub policy without reference counting;
- acquiring `PET_DISPLAY_OWNER_PET2D` while LVGL owns display returns `PET_RESULT_BUSY`;
- releasing as a non-owner returns `PET_RESULT_BUSY`;
- releasing the current owner returns to NONE.

This is a boundary POC only. It is not yet the final LCD driver lock.

## LVGL Shell Owner Hook

`mvp_a_lvgl_shell_create()` now acquires `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI` before the first scene
render. `mvp_a_lvgl_shell_render_scene()` verifies LVGL ownership before cleaning and rebuilding the
LVGL scene tree. If another owner holds the display, rendering is skipped and a
`[MVP_A][LVGL_OWNER]` log is emitted.

`mvp_a_lvgl_shell_release_display_owner()` is available for a future controlled handoff before entering
Pet2D. P4 does not automatically enter Pet2D and does not modify the low-level LVGL flush callback.

## Pet2D Boundary Placeholder

`apps/watch/pet2d_boundary/` defines:
- `pet2d_boundary_enter_placeholder()`: attempts to acquire `PET_DISPLAY_OWNER_PET2D`;
- `pet2d_boundary_exit_placeholder()`: releases `PET_DISPLAY_OWNER_PET2D`;
- `pet2d_boundary_self_test()`: verifies LVGL blocks PET2D, LVGL release allows PET2D, PET2D exit
  succeeds, and LVGL can be acquired again.

The placeholder does not allocate a framebuffer, load resources, draw RGB565, call LCD flush, or import
simulator Pet2D code.

## Self-Test Expectations

`pet_display_jieli_owner_self_test()` is expected to return `PET_RESULT_OK`.
`pet2d_boundary_self_test()` is expected to return `PET_RESULT_OK`.

Both tests restore the original display owner when possible. They are compile/log validation helpers,
not a hardware display test.

## Boundaries

P4 does not enable real LCD flush, Pet2D runtime, HOME/Observe rendering, real key queue changes,
VM/Flash writes, BLE, NFC, audio hardware, or battery hardware integration.

## Follow-Up

Next display phases should connect the owner contract to the real LVGL flush/LCD driver boundary, then
validate RGB565 order, dirty-rect alignment, TE/wait behavior, frame timing and display-owner handoff on
the development board.
