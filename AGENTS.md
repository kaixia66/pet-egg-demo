# AGENTS.md

## PetEgg P1 Shared Interface Pack Rule

`apps/watch/pet_shared/include` owns the frozen cross-platform PetEgg interface headers used to align
future Jieli ports with the PC simulator shared portable layer. These headers must remain C99
compatible and must not include Jieli SDK private headers, LVGL, SDL, simulator adapters, malloc,
printf, file-system APIs, Flash APIs, BLE private structs, or real hardware driver APIs.

P1 has been aligned against the simulator reference headers in
`D:/0-jieli_sdk/simulator/shared_portable/include`. Keep that path as the comparison source before
changing packet, key, save, resource, result-code, or display-profile ABI fields.

The interface pack is a contract boundary, not a hardware implementation. New fields may be added only
with version macros and explicit layout/compatibility notes. Existing field names, sizes, packet/save
layout, and resource manifest semantics should be treated as frozen unless a later phase explicitly
bumps the shared interface version and documents the simulator/Jieli migration impact.

Build byproducts must not be committed for this pack. In particular, keep `build_logs/`,
`cpu/br28/tools/download/watch/*`, `cpu/br28/tools/sdk.elf`, firmware/package outputs, object
directories, `*.depend`, `*.layout`, and build-mutated tool executables out of commits unless a task
explicitly proves they are source changes.

When reading repository text files, avoid PowerShell `Get-Content` for files that may contain Chinese
or mixed encodings. Prefer `rg`, `git show`, `cmd /c type`, Git Bash, or another tool that can preserve
or explicitly select encoding. If output appears garbled, switch reading tools before editing and do
not rewrite the whole file just to fix display encoding.

## PetEgg P2 Jieli Platform HAL Skeleton Rule

`apps/watch/pet_platform_jieli/` owns the Jieli-specific platform adapter layer for the shared PetEgg
ABI. `apps/watch/pet_shared/include` remains platform-independent and must not include Jieli SDK
private APIs; Jieli-specific code belongs only in `pet_platform_jieli`.

P2 is skeleton/stub only. It may expose a complete `pet_platform_t` callback table and minimal compile
checks, but it must not enable real Display, Input, Storage, BLE, NFC, Flash, VM, audio, power policy,
Pet2D, or LVGL flush changes. Hardware callbacks should return `PET_RESULT_NOT_READY`,
`PET_RESULT_UNSUPPORTED`, or fixed test values until their later phases explicitly wire them.

## PetEgg P3 Display/Input POC Rule

P3 may refine the Jieli display profile and add a controlled raw-key mapping POC under
`apps/watch/pet_platform_jieli/`, but it must not enable real LCD flush, Pet2D rendering, VM/Flash,
BLE, NFC, or audio hardware paths. Display profile constants must state their SDK source or board-test
TODOs; self-tests only validate profile/layout and owner/input state machines.

P3 input mapping is a private PetEgg HAL POC. It must not consume the real Jieli key queue, must not
replace `mvp_a_ui_handle_system_key()` or `mvp_a_app_key_event()`, and must not take over LVGL/MVP-A
input. Test raw-event injection must stay local to `pet_platform_jieli` and must not be treated as a
real hardware binding.

## PetEgg P4 Render Owner Boundary Rule

P4 establishes only the render-owner boundary between the current MVP-A LVGL shell and a future Pet2D
renderer. LVGL and PET2D must coordinate through `pet_display_owner_t` acquire/release before any real
LCD flush is enabled. `apps/watch/pet2d_boundary/` is a placeholder boundary for owner handoff and
self-test only; it is not the Pet2D engine and must not import simulator Pet2D code.

P4 may hook MVP-A LVGL shell create/render paths to acquire or release `PET_DISPLAY_OWNER_LVGL_SYSTEM_UI`,
but it must not modify the LVGL low-level flush callback, write RGB565 pixels to the LCD, enable real
Pet2D runtime, take over the key path, write storage, or connect BLE/NFC. P4 self-tests validate owner
state-machine behavior only and are not evidence of real hardware display ownership.

## PetEgg P5 Resource Manifest Adapter Rule

P5 is a read-only resource manifest compatibility phase. `apps/watch/pet_resource_jieli/` may parse the
P1/simulator resource manifest ABI from an in-memory blob, validate CRCs, and perform entry lookup, but
it must not read external Flash, write VM/Flash, decode PNG/JPG/GIF/JSON at runtime, import final art,
or enable Pet2D rendering.

The P5 static test blob is only a small fixture for compile/self-test coverage and is not the production
resource route. Formal resources should later be produced by the Jieli resource tools / `image_dll`,
packaged as manifest/resource binaries, and mapped to local or external Flash only after ownership,
offset, size, byte order, compression and CRC policy are confirmed.

## PetEgg P6 Save Transaction Adapter Rule

P6 is an A/B save transaction and rollback adapter phase. `apps/watch/pet_save_jieli/` may validate
the P1/simulator 64-byte save slot header, choose the latest valid slot by counter, write the inactive
slot first, verify the written slot, and rely on the counter to select the latest save. It must not
erase the old slot before the new slot verifies.

P6 must not write real VM, Flash, syscfg, files or external storage, and must not replace the existing
`mvp_a_save` default path. The memory backend and fault injection are for self-test only; they do not
prove real power-fail atomicity, wear leveling, erase policy or Flash page alignment. `pet_platform`
storage callbacks stay stubbed unless a later phase explicitly confirms storage ownership and power
rules.

## PetEgg P7 Protocol Debug Adapter Rule

P7 is a BLE/NFC stub and debug-injection phase. Packet helpers may validate the P1/simulator packet and
NFC pair payload ABI, and `apps/watch/pet_platform_jieli/` may expose BLE loopback and fake NFC queues
only under `PET_DEBUG` or `PET_PLATFORM_JIELI_TEST`. The default product path must continue to return
`PET_RESULT_NOT_READY` for real BLE/NFC callbacks.

P7 must not start BLE advertising, scanning, connecting, GATT services, NFC reader scans or RF
transactions. Protocol code must not depend on Jieli private BLE/NFC structs, must not write VM/Flash
or syscfg, and must not change MVP-A page, input or save behavior. Debug injection is a self-test and
bring-up aid only; it is not evidence that real BLE/NFC hardware paths are wired.

## PetEgg P8 Platform Self-Test Snapshot Rule

P8 is a platform self-test aggregator and integration snapshot phase. `apps/watch/pet_selftest/` may
summarize and call existing P1-P7 compile/self-test entry points, but it must not create new real
hardware side effects, modify MVP-A defaults, enable LCD flush, start Pet2D runtime, write VM/Flash or
syscfg, or start BLE/NFC RF activity.

The P8 capability snapshot must distinguish test/stub readiness from real hardware readiness. Shared
ABI, HAL skeleton, display profile, owner boundary, resource parser, save transaction, protocol helper,
BLE loopback, fake NFC and debug injection may be marked present; real LCD flush, real key queue, real
Flash storage, real BLE, real NFC and Pet2D runtime must remain disabled until later hardware phases
explicitly wire and validate them.

## PetEgg P9 Display Flush Owner POC Rule

P9 is a display-flush owner guard and no-op-to-real POC phase. `pet_display_jieli_flush()` may validate
RGB565 rectangle parameters, enforce the current `pet_display_owner_t`, record diagnostic stats and
provide self-test coverage, but the default path must not write the LCD or replace the LVGL flush
callback.

`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` must default to 0. Any tiny real LCD flush experiment must be
manual, macro-gated and authorized by a later hardware task. P9 must not enable Pet2D runtime, allocate
a full framebuffer, change MVP-A default pages, write VM/Flash/syscfg, connect BLE/NFC or submit build
byproducts.

## PetEgg P10 Tiny Real LCD Flush POC Rule

P10 is a tiny, manual-only LCD flush POC phase. `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` must still default
to 0 in committed source. Any real flush attempt must be explicitly macro-gated, manually triggered, and
guarded by display owner state; it must not run from boot, the default LVGL page, or platform run-all
self-tests.

Do not replace the LVGL flush callback, change `lv_port_disp.c` default behavior, enable Pet2D runtime,
allocate a full framebuffer, or render HOME/Observe scenes in P10. Manual board tests must first confirm
owner state and use only a tiny caller-owned RGB565 rectangle.

## PetEgg P11 Pet2D Minimal Real Flush POC Rule

P11 is a Pet2D-boundary minimal visual POC phase, not the Pet2D runtime. It may generate a small
caller-owned RGB565 test surface, such as 16x16, and expose a manual boundary probe that uses the P10
gated real-flush path only when `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` is explicitly enabled for a local
board test. The committed default must keep that macro at 0.

Do not render HOME/Observe, load formal resources, parse PNG/JPG/GIF/JSON, allocate a full framebuffer,
replace the LVGL flush callback, or change MVP-A default pages in P11. Temporary Debug UI triggers for
manual board tests must be removed before commit, and build byproducts must stay out of commits.

## PetEgg P12 Repeated Flush / Dirty Rect POC Rule

P12 is a repeated tiny flush and dirty-rect alignment POC phase. It may add small 16x16, 32x32 and 64x64
RGB565 test patterns plus finite repeated-probe helpers, but it must still keep full Pet2D runtime,
HOME/Observe, formal resources, full framebuffer allocation and default MVP-A page behavior disabled.

`PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` must remain 0 in committed source. Any repeated real flush board
test must be local, manually triggered, owner-guarded, finite, and removed from Debug UI before commit.
The current board has no NFC, no speaker and only one unit, so real NFC, audio and BLE two-board link
tests are Future Scope; keep only fake/stub/self-test coverage for those paths in this hardware cycle.

## PetEgg P13 Resource Sprite Surface POC Rule

P13 is a resource-sprite-to-minimal-surface POC phase. It may read only the P5 in-memory test resource
blob, view RGB565 fixture entries as raw pixels, blit them into caller-owned 16/32/64 surfaces, and expose
a manual resource-derived visual probe through the existing gated real-flush path.

P13 must not load formal resource packages, read external Flash/NOR/SFC/`flash_file_info`, decode
PNG/JPG/GIF/JSON, allocate a full framebuffer, enable full Pet2D runtime, render HOME/Observe, replace
the LVGL flush callback, or change MVP-A default pages. `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` must remain
0 in committed source, any Debug UI trigger for board testing must be temporary, and NFC/audio/real BLE
two-board work remains Future Scope on the current hardware.

## PetEgg P14 Key Calibration + Minimal Sprite Movement POC Rule

P14 is a raw-key calibration and minimal sprite movement POC phase. It may add passive key-calibration
record buffers and small Pet2D-boundary movement helpers, but it must not consume the real Jieli key
queue, change `mvp_a_app_key_event()` defaults, or alter the board key table without board-log evidence.
Physical key labels, SDK key values, raw codes and `PetKey` mapping must be documented together before
any permanent mapping correction is made.

P14 does not enable full Pet2D runtime, HOME/Observe, formal resources, external Flash, full framebuffer
allocation, LVGL flush replacement, VM/Flash/syscfg writes, NFC, audio or real BLE. Any Debug UI trigger
or app-common key logging used for board tests must be temporary and removed before commit. The committed
source must keep `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, `real_lcd_flush_enabled` at 0 and
`pet2d_runtime_enabled` at 0. NFC/audio/real BLE two-board tests remain Future Scope on the current
hardware.

## PetEgg P15 Key Latency + Movement Stats POC Rule

P15 is a key-to-render latency and movement repeated-flush statistics phase. It may extend the
Pet2D-boundary movement POC with coarse key/logic/render/flush timestamps, dirty-rect old/new union
tracking, bounded repeated movement steps and stats snapshots, but it must not route the real Jieli key
queue into Pet2D or make movement run automatically.

P15 does not enable full Pet2D runtime, HOME/Observe, formal resources, external Flash, full framebuffer
allocation, LVGL flush replacement, VM/Flash/syscfg writes, NFC, audio or real BLE. Any Debug UI trigger
used for local board latency tests must be temporary and removed before commit. The committed source
must keep `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, `real_lcd_flush_enabled` at 0 and
`pet2d_runtime_enabled` at 0. NFC/audio/real BLE two-board tests remain Future Scope on the current
hardware.

## PetEgg P16 Real Resource Package / External Flash Read POC Rule

P16 is a read-only real resource package and external-Flash resource-path POC. It may audit Jieli SDK
resource APIs and add a read-only adapter that probes an existing `manifest.bin` or equivalent resource
entry through the SDK resource file path, but it must not write external Flash, VM, syscfg or files and
must not modify resource download/packaging tool defaults.

P16 must not enable the full Pet2D runtime, HOME/Observe, background scrolling, image decoding,
full-framebuffer allocation, LVGL flush replacement, NFC, audio or real BLE. If a resource package is
not present, the adapter must return `NOT_FOUND`/`NOT_READY` rather than hardcoding an address or faking
bytes. Any temporary Debug UI trigger used for board probing must be removed before commit. The committed
source must keep `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, `real_lcd_flush_enabled` at 0 and
`pet2d_runtime_enabled` at 0. NFC/audio/real BLE two-board tests remain Future Scope on the current
hardware.

## PetEgg P17S External Flash Pause Rule

P17S stops the external Flash / virfat / raw NOR resource-package route. P17 board tests showed that
the PetEgg package file can be present on `storage/virfat_flash/C/` with the expected size, but the
runtime payload read from both `res_fopen/res_fread` and ordinary `fopen/fread` did not match the local
plain `MRTP` package header. A direct raw NOR read experiment also caused a soft reset, so raw NOR reads
must not be continued as a default PetEgg path.

Until a later Future Scope task confirms the Jieli `fat_comm`/virfat encryption or mapping policy and a
safe raw-binary read channel, do not add PetEgg external Flash package generators, do not modify
`download/watch` or `fat_comm` inputs for PetEgg resources, do not add raw NOR read probes, and do not
submit external Flash package artifacts such as `petegg.pkg`, `manifest.bin`, `sprites.pak` or
`anim_table.bin`. Current development continues within the 2M internal Flash environment using compile
time fixtures, the P5 resource test blob, and other explicitly bounded small-resource paths. NFC,
audio/speaker and real BLE two-board validation remain Future Scope.

## PetEgg P18 Pet2D Scene Mode / LVGL Handoff POC Rule

P18 introduces only a controlled Pet2D test-scene handoff between the MVP-A LVGL shell and the existing
minimal Pet2D-boundary movement/resource fixture. The scene may be entered manually from the Debug page,
release LVGL display ownership, acquire `PET_DISPLAY_OWNER_PET2D`, handle bounded LEFT_UP/RIGHT_DOWN/OK/
CANCEL input, then release PET2D ownership and request LVGL refresh on cancel, timeout or error.

P18 must not enable HOME/Observe, the full Pet2D runtime, background scrolling, formal resources,
external Flash / virfat / raw NOR, PNG/JPG/GIF/JSON decoding, full-framebuffer allocation, LVGL flush
callback replacement, VM/Flash/syscfg writes, NFC, audio or real BLE. Any retained Debug entry must be
manual-only and safe when `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` is 0; the committed source must keep that
macro at 0, `real_lcd_flush_enabled` at 0 and `pet2d_runtime_enabled` at 0. External Flash, NFC,
audio/speaker and real BLE two-board validation remain Future Scope.

## PetEgg P19 High-res Motion / Performance POC Rule

P19 adds only a bounded high-res motion/performance POC on top of the P18 handoff. It may measure
manual 32x32, 64x64 and 96x96 finite-frame dirty-rect movement, with optional 128x128 mode kept
unsupported unless it can obey the 128x128 maximum scratch-buffer limit. It must record coarse logic,
render, flush and frame timing stats without producing per-frame log spam.

P19 must not enable HOME/Observe, the full Pet2D runtime, background maps, external Flash / virfat / raw
NOR resources, formal resource packages, PNG/JPG/GIF/JSON decoding, full-framebuffer allocation, LVGL
flush callback replacement, VM/Flash/syscfg writes, NFC, audio or real BLE. The committed source must
keep `PET_JIELI_ENABLE_REAL_LCD_FLUSH_POC` at 0, `real_lcd_flush_enabled` at 0 and
`pet2d_runtime_enabled` at 0. Any P19 Debug entries are engineering-only, manual-triggered, finite, and
safe when the real flush gate is disabled.

## 项目基线

这是杰理 AC701N / BR28 手表类 SDK 工程，当前根目录是 `D:\0-jieli_sdk\sdk`。主应用位于 `apps/watch`，公共业务和驱动适配位于 `apps/common`，芯片相关实现、库和后处理工具位于 `cpu/br28`，对外头文件与预编译库接口位于 `include_lib`。

本目录已初始化为 Git 仓库，当前主线分支使用 `master`/`origin/master`。修改前后优先用 `git status --short` 和必要的 `git diff` 检查变更；用户明确要求忽略某个 feature 分支时，不要把该分支上的未合并实现当作主线事实。

## 默认构建

- Windows 默认构建命令：`.vscode\winmk.bat all`
- 该脚本会把 `tools\utils` 加入 `PATH`，并执行 `make "all" -j %NUMBER_OF_PROCESSORS%`。
- 直接构建命令：`make`
- 详细构建：`make VERBOSE=1`
- 清理构建产物：`make clean`
- 如果旧版 `make` 不支持 `$(file ...)` 函数，按 `Makefile` 注释使用：`LINK_AT=0 make`

Windows 下 `Makefile` 默认工具链在 `C:/JL/pi32/bin`，使用 `clang.exe`、`pi32v2-lto-wrapper.exe`、`llvm-ar.exe`。Linux 下默认工具链在 `/opt/jieli/pi32v2/bin`，并需要确认 `/opt/jieli/common/bin/clang` 或对应 pi32v2 工具存在；Linux 构建前还应确认 `ulimit -n` 足够大。

构建输出主文件是 `cpu/br28/tools/sdk.elf`。构建会在 `pre_build` 阶段预处理并生成 `cpu/br28/sdk.ld`、`cpu/br28/sdk_used_list.used`、`cpu/br28/tools/download.bat` 或 `cpu/br28/tools/download.sh`、`cpu/br28/tools/isd_config.ini`，随后运行下载/打包后处理脚本。

运行编译时不要把完整日志直接输出到对话上下文。优先把 stdout/stderr 重定向到 `build_logs/` 下的文件，再从日志文件中摘取错误、警告摘要和最后结果：

```powershell
New-Item -ItemType Directory -Force build_logs | Out-Null
.\.vscode\winmk.bat all *> build_logs\winmk-all.log
Select-String -Path build_logs\winmk-all.log -Pattern 'error|warning|failed|undefined|No such file|cannot|Error' -CaseSensitive:$false
Get-Content -Path build_logs\winmk-all.log -Tail 80
```

## 入口与配置

- 应用入口：`apps/watch/app_main.c`，`app_main()` 初始化 UI、电源、升级资源下载逻辑后进入 `app_task_loop()`。
- 任务分发：`apps/watch/app_main.c` 根据 `app_curr_task` 切换 `APP_POWERON_TASK`、`APP_BT_TASK`、`APP_MUSIC_TASK`、`APP_RTC_TASK`、`APP_SMARTBOX_ACTION_TASK`、`APP_IDLE_TASK` 等。
- 板级选择：`apps/watch/board/br28/board_config.h` 当前主线启用 `CONFIG_BOARD_701N_LVGL_DEMO`。
- 当前板级配置：`apps/watch/board/br28/board_701n_lvgl_demo/board_701n_lvgl_demo_cfg.h`，其中 `LVGL_TEST_ENABLE` 为 1。
- 链接脚本源：`cpu/br28/sdk_ld.c` 预处理生成 `cpu/br28/sdk.ld`；最终链接还使用 `apps/watch/board/br28/app.ld`、`apps/watch/board/br28/app_overlay.ld` 相关配置。

切换板级时优先改 `board_config.h` 的 `CONFIG_BOARD_*` 选择，并同时检查对应 `board_*_cfg.h`、`key_table`、显示屏、触摸、存储和蓝牙宏，不要只改某一个驱动文件。

## 目录地图

- `apps/watch`: 手表应用、任务管理、UI、运动、SmartBox、LTE/CAT1、产品测试和用户 API。
- `apps/watch/mvp_a`: 主线 MVP-A LVGL Demo 框架，包含核心状态、存档、资源索引、平台 stub、LVGL 页面 shell 和页面 widgets。
- `apps/watch/board/br28`: BR28 板级配置集合，含 `board_701n_demo`、`board_701n_lvgl_demo`、`board_701n_nandflash_demo` 等。
- `apps/watch/ui/lcd/STYLE_WATCH_NEW`: 传统 LCD 手表 UI action 和 demo 页面；主线 MVP-A 页面当前不在这里实现。
- `apps/watch/ui/lua_ui`: Lua UI 绑定层；当前 LVGL demo 板级中 `TCFG_LUA_ENABLE` 为关闭状态，启用前要同步资源和内存配置。
- `apps/common`: 公共音频、设备、文件、更新、网络、支付、LVGL、第三方协议与配置。
- `cpu/br28`: 芯片相关音频、功耗、UI driver、P11/充电/时钟/IIC 等实现。
- `cpu/br28/liba`: 预编译库，覆盖蓝牙、音频编解码、运动健康算法、QuickJS、网络、UI、升级等。
- `include_lib`: SDK 头文件接口，包括 driver、system、btstack、btctrler、media、net、update。
- `tools` 与 `cpu/br28/tools`: 编译、下载、UI 资源、JTAG、升级资源、配置工具。

## 功能宏与依赖

`Makefile` 定义了大量全局宏，例如 `CONFIG_CPU_BR28`、`CONFIG_UCOS_ENABLE`、`CONFIG_SOUNDBOX`、`CONFIG_WATCH_CASE_ENABLE`、`CONFIG_UPDATA_ENABLE`、`CONFIG_OTA_UPDATA_ENABLE`、`LV_LVGL_H_INCLUDE_SIMPLE` 等。功能开关更多集中在板级 `board_*_cfg.h`。

当前 `board_701n_lvgl_demo_cfg.h` 中可见的长期约束：

- `TCFG_APP_BT_EN`、`TCFG_APP_MUSIC_EN`、`TCFG_APP_RTC_EN` 默认开启；`TCFG_APP_LINEIN_EN`、`TCFG_APP_PC_EN`、`TCFG_APP_RECORD_EN`、`TCFG_APP_CAT1_EN` 默认关闭。
- `CONFIG_APP_BT_ENABLE` 启用后会开启 `SMART_BOX_EN`，并进一步强制打开 BLE。
- LVGL demo 板级启用 `LVGL_TEST_ENABLE`，当前主线 UI 验证路径走 `cpu/br28/ui_driver/lvgl/lvgl_main.c` 和 `apps/watch/mvp_a/ui/mvp_a_lvgl_shell.c`。
- UI 总开关和 LCD 屏相关宏仍可能被板级条件块 `#undef`/重定义。检查最终宏值时要看完整条件块。
- 触摸屏 `TCFG_TOUCH_PANEL_ENABLE` 开启；具体触摸型号和 IIC 时序要在同一板级配置文件内核对。
- 存储相关宏存在多处 `#undef`/重定义，改 NOR/NAND/SFC/VIRFAT 时必须从板级配置顶部一路检查到后续条件块。

## UI 与资源

本工程同时包含传统 `ui_new`/LCD UI、LVGL v8.1.0、Lua UI 绑定、UI 资源工具和多套表盘资源。当前主线 MVP-A Demo 已接入 `apps/watch/mvp_a` 的 LVGL shell；传统 LCD UI 代码仍在 `apps/watch/ui/lcd/STYLE_WATCH_NEW`，资源侧常在 `cpu/br28/tools/UI工程/ui_454x454_watch`。

UI 资源目录内有大量工具生成文件、预览文件、`.bat` 脚本和中文路径。修改 UI 功能时先判断是代码逻辑、资源工程、字体/语言表、还是打包脚本问题；不要把资源生成输出当作手写源码随意重排。

MVP-A 主线代码状态：

- `mvp_a_app.c` 负责初始化、场景切换和 4 键事件分发。
- `mvp_a_save.c` 通过 `syscfg_read/write` 保存 `mvp_a_save_data_t`，带 magic/version/data_len/checksum。
- `mvp_a_platform.c` 中 NFC/BLE 当前仍是 `MVP_A_RESULT_NOT_READY` stub。
- `mvp_a_assets.c` 是资源 metadata/manifest 映射层；`mvp_a_image_assets.c` 是少量过渡用 C 数组图像，不是正式资源路线。
- LVGL 页面由 `mvp_a_lvgl_shell.c` 统一创建，页面文件位于 `apps/watch/mvp_a/ui/pages/`。

## Pet2D / LVGL 渲染约束

以下是后续 Pet2D 架构的长期口径。注意：这些约束不代表 `master` 当前已经实现 Pet2D；当前主线仍是 MVP-A LVGL demo skeleton。实现 Pet2D 时必须遵守：

1. HOME / Observe 仍由 Pet2D 实现，不得改成杰理 UI 页面。
2. LVGL 继续用于系统菜单、卡包、状态、Debug HUD。
3. 后续 Pet2D 实屏渲染优先使用 IMB 2D 硬件加速。
4. 图像资源优先使用杰理 SDK 资源工具 / image_dll 生成硬件可识别压缩格式。
5. 正式图像资源必须放 16M 外置 Flash，不得转 C 数组。
6. 运行时禁止解 PNG / JSON。
7. IMB 可直接从 NOR Flash 读取的资源，优先通过 `flash_file_info` / 地址映射交给 IMB。
8. 旋转资源原则上不要直接从 Flash 做，实时旋转只用于小图或先加载到 SRAM。
9. 不使用 full framebuffer，不使用双全屏 framebuffer。
10. LVGL 与 Pet2D 仍通过 Render Owner 互斥。
11. 任何实屏 smoke 必须宏控制，默认关闭或受控执行。

## 生成物与搜索排除

默认搜索源码时优先排除以下生成物或重型输出：

- `objs/`
- `obj/`
- `*.depend`
- `*.layout`
- `cpu/br28/tools/sdk.elf`
- `cpu/br28/tools/sdk.map`
- `cpu/br28/tools/sdk.elf.*.txt`
- `cpu/br28/sdk.ld`
- `cpu/br28/sdk_used_list.used`
- `cpu/br28/tools/isd_config.ini`
- `cpu/br28/tools/download.bat`
- `cpu/br28/tools/download.sh`
- `cpu/br28/tools/app.bin`
- `cpu/br28/tools/sdk.lst`
- `cpu/br28/tools/symbol_tbl.txt`
- `cpu/br28/tools/download/watch/*.bin`
- `cpu/br28/tools/download/watch/*.fw`
- `cpu/br28/tools/download/watch/*.ufw`
- `cpu/br28/tools/download/watch/*.zip`
- `build_logs/`
- `cpu/br28/tools/UI工程/**/project/preview/`

推荐搜索示例：

```powershell
rg -n "SYMBOL_OR_MACRO" apps cpu include_lib -g "*.c" -g "*.h" -g "*.S"
rg --files apps/watch apps/common cpu/br28 include_lib
```

## 编码与文件风格

仓库内大量中文注释、中文路径和 Windows 批处理脚本。部分文件在 PowerShell 中可能显示为乱码；这通常是编码显示问题，不代表源码本身损坏。编辑前先确认目标文件编码，避免把 GBK/ANSI 文件整文件改写成另一种编码。

C 代码风格以现有 SDK 为准：宏控制功能、板级头文件集中配置、`ENABLE_THIS_MOUDLE`/`DISABLE_THIS_MOUDLE` 拼写保持原样。不要为了纠正拼写或格式做跨 SDK 的大范围机械改动。

## 修改规则

- 优先在当前板级配置、当前功能模块或明确入口附近做小范围修改。
- 改功能宏时同步检查 `Makefile` 源文件列表、对应 `include_lib` 头文件、预编译库依赖和后续 `#undef` 条件块。
- 改板级外设时同时检查 `.c` 初始化、`board_*_cfg.h` 宏、`key_table`、管脚、时钟、电源和低功耗条件。
- 改 UI 时同时检查 action 文件、页面/窗口 ID、资源工程、字体/语言资源、显示屏与触摸宏。
- 不要直接编辑 `cpu/br28/liba/*.a`；这些是预编译库。
- 不要把 `Makefile` 自动生成的 `sdk.ld`、`download.bat/sh`、`isd_config.ini` 当作长期源文件，源头通常在 `cpu/br28/sdk_ld.c`、`cpu/br28/tools/download.c`、`cpu/br28/tools/isd_config_rule.c`。

## 验证建议

普通代码修改后至少运行：

```powershell
.\.vscode\winmk.bat all *> build_logs\winmk-all.log
```

涉及构建系统、链接、后处理脚本或资源打包时运行：

```powershell
make clean
make VERBOSE=1
```

如果本机缺少 `C:/JL/pi32` 工具链，记录为环境缺失，不要把工具链路径改成临时本机路径提交到项目文件。硬件相关修改还需要用户在目标板上下载验证，尤其是 LCD、触摸、按键、功耗、充电、蓝牙、升级和外部 Flash。

## 常见陷阱

- `board_config.h` 只选择板型；真正大部分功能宏在对应 `board_*_cfg.h`，并可能被后面的条件块再次改写。
- `SMART_BOX_EN` 会影响 BLE/RCSP 相关路径，关闭或开启时要查 `apps/common/config/include/bt_profile_cfg.h` 和 SmartBox 目录。
- UI 代码、LVGL、Lua UI 和资源工具并存；不要看到 `lvgl` 目录就默认所有界面全走 LVGL，也不要把 feature 分支上的 Pet2D 代码当作 `master` 已有能力。
- `cpu/br28/tools` 下既有源头 `.c`，也有构建生成 `.bat/.ini` 和下载输出；改前先确认文件角色。
- 中文路径和脚本对 Windows 工具链友好，跨平台脚本改动要同时看 `Makefile` 中 Windows/Linux 分支。
- 根目录 `AC701N.cbp` 是 IDE 工程文件，`AC701N.depend` 和 `AC701N.layout` 更像本地/生成状态；不要优先从它们推断源码结构。

## 信息来源

本文件基于以下项目文件生成：根 `Makefile`，`apps/watch/app_main.c`，`apps/watch/board/br28/board_config.h`，`apps/watch/board/br28/board_701n_lvgl_demo/board_701n_lvgl_demo_cfg.h`，`apps/watch/mvp_a`，目录扫描结果，以及 `cpu/br28/tools`、`cpu/br28/liba`、`include_lib`、`apps/common`、`apps/watch` 的结构。更新时以 `master`/`origin/master` 为准，明确忽略未合入的 feature 分支实现。
