# P14 Minimal Sprite Movement + Raw Key Calibration POC

## Scope

P14 adds a passive raw-key calibration helper and a minimal Pet2D-boundary movement POC. It does not
change the MVP-A default input path, consume the real key queue, enable full Pet2D runtime or keep any
Debug UI entry in committed source.

Committed defaults remain:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`

## Key Calibration Design

New helper: `apps/watch/pet_platform_jieli/pet_key_calibration_jieli.*`.

It records the most recent 16 observations:
- raw code;
- raw event;
- SDK key value;
- mapped `PetKey`;
- mapped `PetKeyEvent`;
- timestamp, hold and repeat fields;
- mapping result.

The helper does not hook or consume the real Jieli key queue. Board-test logging can call
`pet_key_calibration_jieli_record_sdk_key()` from a temporary local hook, then remove that hook before
commit.

## Current Audited Mapping

The active board IO table maps:

| Board raw | SDK key value | SDK symbol | Current PetKey | Physical label status |
|---|---:|---|---|---|
| 0 | 64 | `KEY_UI_HOME` | `PET_KEY_OK` | pending board-label confirmation |
| 1 | 68 | `KEY_UI_PLUS` | `PET_KEY_LEFT_UP` | pending board-label confirmation |
| 2 | 67 | `KEY_UI_MINUS` | `PET_KEY_RIGHT_DOWN` | pending board-label confirmation |
| 3 | 65 | `KEY_UI_SHORTCUT` | `PET_KEY_CANCEL` | pending board-label confirmation |

P14 does not change the permanent mapping yet. The P12/P13 observation that LEFT/UP entered Debug where
RIGHT/DOWN was expected is recorded as a calibration issue until raw-key logs and physical labels are
captured together.

## Minimal Movement Design

New helper: `apps/watch/pet2d_boundary/pet2d_movement_poc.*`.

Behavior:
- 32x32 caller-owned RGB565 surface;
- P12 32x32 pattern background;
- P13 resource sprite fixture blit;
- `PET_KEY_LEFT_UP` moves the sprite rectangle left by 16 px;
- `PET_KEY_RIGHT_DOWN` moves it right by 16 px;
- `PET_KEY_OK` toggles the sprite/resource overlay;
- `PET_KEY_CANCEL` marks exit requested;
- coordinates are clamped to the P3 display safe area;
- dirty rect is the bounding box of old and new sprite rectangles.

The movement render path uses `pet_display_jieli_real_flush_poc_rect()` only when the real-flush macro is
locally enabled. With committed defaults it returns `PET_RESULT_UNSUPPORTED` before writing the panel.

## Self-Test / Gate Behavior

Self-tests cover:
- calibration ring clear/record/recent-read behavior;
- SDK key value to raw-code conversion;
- P3 raw mapping result capture;
- movement state update for LEFT_UP/RIGHT_DOWN/OK/CANCEL;
- safe-area clamping and dirty-rect bounds;
- macro-off render gate.

The self-test aggregator adds:
- `PET_SELFTEST_KEY_CALIBRATION`
- `PET_SELFTEST_MINIMAL_MOVEMENT_POC`
- `has_key_calibration`
- `has_minimal_sprite_movement_probe_gate`

`pet_selftest_run_all()` does not perform real LCD movement writes.

## Manual Board-Test Plan

For a temporary local board test:
1. Temporarily enable `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC=1`.
2. Temporarily add a key logging hook that records SDK key values and prints `[P14_KEY_CAL]`.
3. Temporarily add a Debug action or movement mode that calls `pet2d_boundary_movement_probe_step()`.
4. Ask the tester to press physical LEFT/UP, RIGHT/DOWN, OK and CANCEL in order.
5. Record raw/sdk/PetKey logs and visible movement behavior.
6. Confirm no panic, assert, WDT, HardFault, exception or unexpected reset.
7. Restore the macro to 0 and remove temporary Debug/key hooks before commit.

## Current Board Result

Temporary board-test build:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` was enabled locally.
- A temporary key-log hook printed `[P14_KEY_CAL]`.
- Temporary Debug actions `P14L` / `P14R` / `P14OK` / `P14C` called
  `pet2d_boundary_movement_probe_step()`.
- All temporary hooks were removed for committed source.

Observed raw key logs:

| Observed role | SDK key value | Board raw | SDK/MVP-A label | PetKey |
|---|---:|---:|---|---|
| LEFT/UP physical/navigation key | 68 | 1 | `UP` / `KEY_UI_PLUS` | `PET_KEY_LEFT_UP` |
| RIGHT/DOWN physical/navigation key | 67 | 2 | `DOWN` / `KEY_UI_MINUS` | `PET_KEY_RIGHT_DOWN` |
| OK / confirm | 64 | 0 | `CONFIRM` / `KEY_UI_HOME` | `PET_KEY_OK` |
| CANCEL / back | 65 | 3 | `BACK` / `KEY_UI_SHORTCUT` | `PET_KEY_CANCEL` |

The P3 PetKey mapping is therefore kept unchanged. The earlier confusion was in the manual Debug
navigation instruction, not in the raw-code table: LEFT/UP and RIGHT/DOWN can reach different Debug
items depending on the current selected item and wrap direction.

Movement board-test observations:
- The default movement image is not persistent on the LVGL Debug page.
- Pressing OK on a temporary Debug action makes the 32x32 resource/pattern surface visible briefly.
- Repeated OK presses show the small icon moving according to the selected screen prompt/direction.
- Logs include `P14_MOVEMENT ... ret=0` for movement steps.
- No `panic`, `assert`, `WDT`, `HardFault`, `exception` or unexpected reset was observed.
- LVGL owner can be reacquired and the Debug page redraws after the movement probe, which currently
  explains the brief visible duration.

## Safety Boundary

Committed source state:
- no default Debug UI trigger;
- no automatic real flush;
- no LVGL flush callback replacement;
- no full Pet2D runtime;
- no HOME/Observe rendering;
- no formal resource loading;
- no external Flash/NOR/SFC/`flash_file_info`;
- no VM/Flash/syscfg writes;
- no real NFC/audio/BLE hardware path.

## Follow-Up

Recommended next stages:
1. P15 key latency + movement repeated flush stats.
2. P16 real resource package / external Flash read POC.
3. P17 HOME/Observe minimal scene skeleton.
