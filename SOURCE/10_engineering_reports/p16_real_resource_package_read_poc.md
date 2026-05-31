# P16 Real Resource Package / External Flash Read POC

## Scope

P16 audits the Jieli SDK resource path and adds a read-only real package probe for the PetEgg resource
ABI. It does not write external Flash, change packaging defaults, enable full Pet2D runtime, parse image
formats, replace LVGL flush, connect NFC/audio/real BLE or keep a Debug UI trigger.

Committed source keeps:
- `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC = 0`
- `real_lcd_flush_enabled = 0`
- `pet2d_runtime_enabled = 0`
- `external_flash_resource_enabled = 0`

## SDK Resource Path Audit

The active resource root is defined as `RES_PATH = "storage/virfat_flash/C/"` in
`apps/watch/include/ui/res_config.h`. The UI path registers `.sty`, `.res`, `.str`, `.view` and `.json`
files such as `JL/JL.res`, `watch/watch.res` and `sidebar/sidebar.res`.

`include_lib/system/ui_new/res/resfile.h` declares the main read APIs:
- `res_fopen`
- `res_fread`
- `res_fseek`
- `res_flen`
- `res_fclose`
- `open_image_by_id`
- `read_image_data`

It also declares `struct flash_file_info`, which contains a Flash page table pointer, table size, offset
and last-table length. `ui_res_flash_info_get()` is implemented in
`cpu/br28/ui_driver/interface/ui_platform.c` and forwards to `__ui_res_flash_info_get()` with
`UI_FAT_PHY_FLASH` and `UI_FAT_PHY_BASE`. This is the UI/IMB candidate for locating already-packaged
resources in Flash.

`cpu/br28/ui_driver/imb_demo.c` shows the intended external-Flash/IMB patterns:
- `get_file_addr()` uses `ui_res_flash_info_get()` and `flash_file_info.tab[0] + offset` to derive a
  physical file address.
- `get_image_addr()` combines `ui_res_flash_info_get()` with `open_image_by_id()` and an image offset.
- `flash_tab_info_get()` can construct a table for a known contiguous Flash range, but it allocates and
  assumes known physical placement, so P16 does not use it in committed code.
- IMB tasks can load with `LOAD_FROM_FLASH_WITH_TAB` when `flash_file_info` is available.

`open_image_by_id()` is image-index oriented. It returns `struct image_file` metadata for SDK UI images
inside `.res` files and is useful for future IMB/image-tool integration, but it is not a raw PetEgg
package reader.

`cpu/br28/tools/download/watch/download*.bat` currently runs `packres.exe` over SDK UI files:
`JL.sty/JL.res/JL.str`, `sidebar.*`, `watch*.sty/res/str/view/json` and font resources, then packs
`new_res.bin` into `res.bin`. The current `download/watch` directory contains generated package pieces
such as `res.ori/`, `ui_upgrade/`, `sidebar`, `AITHUMB`, `ota.bin`, `p11_code.bin`, `config.dat`,
`tone.cfg` and helper scripts. No `manifest.bin`, `sprites.pak` or `anim_table.bin` exists there.

The requested `include_lib/driver/device/imd.h` path was not present in this checkout; IMD use appears
through `asm/imd.h` includes and UI driver references, not through that exact header path.

## Adapter Design

New files:
- `apps/watch/pet_resource_jieli/pet_resource_jieli_real.h`
- `apps/watch/pet_resource_jieli/pet_resource_jieli_real.c`

The adapter is read-only and does not allocate. It declares only the SDK `res_*` symbols it needs in the
`.c` file so the public PetEgg header stays independent of SDK private headers.

Public API:
- `pet_resource_jieli_real_probe_info()`
- `pet_resource_jieli_real_read()`
- `pet_resource_jieli_real_open_manifest()`
- `pet_resource_jieli_real_package_probe()`
- `pet_resource_jieli_real_self_test()`

The default probe path is:

```text
storage/virfat_flash/C/petegg/manifest.bin
```

This path is intentionally a future package location, not a hardcoded physical address. If it does not
exist, the adapter returns `PET_RESULT_NOT_FOUND`. Bounded read range failures map to
`PET_RESULT_INVALID_ARGUMENT` because the frozen shared result-code ABI currently has no dedicated
`OUT_OF_RANGE` value.

## Package Probe Result

The package probe opens `manifest.bin`, reads the 16-byte P1 manifest header, checks magic/version,
checks nonzero entry count and validates that the entry table fits inside the file length. It does not
read large unknown Flash regions and does not validate data entries unless a real package file exists.

Current repository/download output audit found:
- `manifest.bin`: not found
- `sprites.pak`: not found
- `anim_table.bin`: not found

Therefore P16 completes the SDK path audit and adapter skeleton, but real package validation requires a
later generator/download integration task.

## Self-Test Integration

`apps/watch/pet_selftest/` adds `PET_SELFTEST_RESOURCE_PACKAGE_PROBE`. `run_all` maps `NOT_FOUND` or
`NOT_READY` from the real package probe to skipped/`UNSUPPORTED`, so a missing package is not treated as
a platform failure.

Capability snapshot additions:
- `has_real_resource_read_probe = 1`
- `real_resource_package_available = 1` only if the header probe succeeds on a board
- `external_flash_resource_enabled = 0` in committed source

## Optional Visual Probe

No P16 visual probe was added. The current resource system did not expose a confirmed raw RGB565 PetEgg
package entry, and P16 does not treat SDK `.res` image IDs as raw RGB565 sprite bytes. P13 fixture-based
visual probes remain available but are not evidence of real package reads.

## Board-Test Status

A temporary local Debug action was used to trigger only the read-only P16 package probe, then removed
from source before returning to the final state. No temporary LCD flush macro was enabled.

The temporary board build compiled, generated `cpu/br28/tools/sdk.elf`, detected the online external
Flash as 16M and completed the SDK download script's external-Flash data step. The PowerShell process
returned exit code 1 because the SDK script emits native-command diagnostics during post-build, but the
log shows FW/UFW generation and download completion.

Manual trigger result on COM3 at 1000000 baud:
- `info_ret=12 source=0 base=0x0 size=0 crc=0x0`
- `package_probe ret=12`
- `12` is `PET_RESULT_NOT_FOUND`, matching the audited absence of
  `storage/virfat_flash/C/petegg/manifest.bin`.
- The trigger did not perform any visible screen action; it only logged the read-only probe result.
- The watch window did not match `panic`, `assert`, `WDT`, `HardFault` or `exception`. Boot reset-source
  lines were present before the manual trigger and are not evidence of a P16 probe crash.
- Source was restored after the manual test; `mvp_a_debug.c/.h` have no committed P16 Debug action diff.
  A final safe package was generated after restore, but the SDK reported `Device offline, only package
  the file`, so it was not downloaded. Per user instruction, no further download attempt was made.

Expected future read-only board test, once a package is generated:
1. Place or package `petegg/manifest.bin` through the confirmed resource tool path.
2. Trigger only `pet_resource_jieli_real_package_probe()`.
3. Log return code, file size, source type and first header bytes.
4. Confirm missing package returns `NOT_FOUND`, valid header returns `OK`, and no illegal address access,
   reset, panic, assert, WDT, HardFault or exception occurs.

## Compile And Build

Compile checks passed:
- `apps/watch/pet_resource_jieli/pet_resource_jieli_compile_check.c`
- `apps/watch/pet_selftest/pet_selftest_compile_check.c`
- `apps/watch/pet_platform_jieli/pet_platform_jieli_compile_check.c`

Full build generated `cpu/br28/tools/sdk.elf`. The SDK post-build emits existing linker stack-size
warnings and PowerShell native-command diagnostics; these were treated as environment/post-build noise
when `sdk.elf`, FW and UFW outputs were produced. The final safe build log also reported
`Device offline, only package the file`, so that final restored image was packaged but not downloaded.

## Safety Boundary

P16 does not:
- write external Flash, VM, syscfg or files;
- modify `download/watch/*.bat`, `packres.exe`, `image_dll` or resource-tool defaults;
- read raw SFC/NOR physical addresses without a confirmed package;
- parse PNG/JPG/GIF/JSON;
- enable full Pet2D runtime, HOME/Observe, background scrolling or full framebuffer allocation;
- replace LVGL flush or change MVP-A default page behavior;
- keep a Debug UI entry;
- connect NFC, audio or real BLE.

Current hardware scope remains unchanged: the board has no NFC, no speaker and only one unit is
available, so real NFC, audio and BLE two-board validation stay Future Scope.

## Follow-Up

1. P17 resource package generator / download integration for `manifest.bin`, `sprites.pak` and
   `anim_table.bin`.
2. P18 Pet2D scene mode / LVGL handoff POC after resource presence is real.
3. P19 high-res motion/performance POC with confirmed resource byte order, alignment and IMB policy.
