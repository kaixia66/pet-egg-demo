# AGENTS.md

## 项目基线

这是杰理 AC701N / BR28 手表类 SDK 工程，当前根目录是 `D:\0-jieli_sdk\sdk`。主应用位于 `apps/watch`，公共业务和驱动适配位于 `apps/common`，芯片相关实现、库和后处理工具位于 `cpu/br28`，对外头文件与预编译库接口位于 `include_lib`。

本目录不是 Git 仓库；修改前后不要假定可以用 `git status` 或 `git diff` 做完整追踪。

## 默认构建

- 默认构建命令：`make`
- 详细构建：`make VERBOSE=1`
- 清理构建产物：`make clean`
- 如果旧版 `make` 不支持 `$(file ...)` 函数，按 `Makefile` 注释使用：`LINK_AT=0 make`

Windows 下 `Makefile` 默认工具链在 `C:/JL/pi32/bin`，使用 `clang.exe`、`pi32v2-lto-wrapper.exe`、`llvm-ar.exe`。Linux 下默认工具链在 `/opt/jieli/pi32v2/bin`，并需要确认 `/opt/jieli/common/bin/clang` 或对应 pi32v2 工具存在；Linux 构建前还应确认 `ulimit -n` 足够大。

构建输出主文件是 `cpu/br28/tools/sdk.elf`。构建会在 `pre_build` 阶段预处理并生成 `cpu/br28/sdk.ld`、`cpu/br28/sdk_used_list.used`、`cpu/br28/tools/download.bat` 或 `cpu/br28/tools/download.sh`、`cpu/br28/tools/isd_config.ini`，随后运行下载/打包后处理脚本。

## 入口与配置

- 应用入口：`apps/watch/app_main.c`，`app_main()` 初始化 UI、电源、升级资源下载逻辑后进入 `app_task_loop()`。
- 任务分发：`apps/watch/app_main.c` 根据 `app_curr_task` 切换 `APP_POWERON_TASK`、`APP_BT_TASK`、`APP_MUSIC_TASK`、`APP_RTC_TASK`、`APP_SMARTBOX_ACTION_TASK`、`APP_IDLE_TASK` 等。
- 板级选择：`apps/watch/board/br28/board_config.h` 当前启用 `CONFIG_BOARD_701N_DEMO`。
- 当前板级配置：`apps/watch/board/br28/board_701n_demo/board_701n_demo_cfg.h`。
- 链接脚本源：`cpu/br28/sdk_ld.c` 预处理生成 `cpu/br28/sdk.ld`；最终链接还使用 `apps/watch/board/br28/app.ld`、`apps/watch/board/br28/app_overlay.ld` 相关配置。

切换板级时优先改 `board_config.h` 的 `CONFIG_BOARD_*` 选择，并同时检查对应 `board_*_cfg.h`、`key_table`、显示屏、触摸、存储和蓝牙宏，不要只改某一个驱动文件。

## 目录地图

- `apps/watch`: 手表应用、任务管理、UI、运动、SmartBox、LTE/CAT1、产品测试和用户 API。
- `apps/watch/board/br28`: BR28 板级配置集合，含 `board_701n_demo`、`board_701n_lvgl_demo`、`board_701n_nandflash_demo` 等。
- `apps/watch/ui/lcd/STYLE_WATCH_NEW`: 当前 Makefile 中大量编译的 LCD 手表 UI action 和 demo 页面。
- `apps/watch/ui/lua_ui`: Lua UI 绑定层；当前 `board_701n_demo_cfg.h` 中 `TCFG_LUA_ENABLE` 为关闭状态，启用前要同步资源和内存配置。
- `apps/common`: 公共音频、设备、文件、更新、网络、支付、LVGL、第三方协议与配置。
- `cpu/br28`: 芯片相关音频、功耗、UI driver、P11/充电/时钟/IIC 等实现。
- `cpu/br28/liba`: 预编译库，覆盖蓝牙、音频编解码、运动健康算法、QuickJS、网络、UI、升级等。
- `include_lib`: SDK 头文件接口，包括 driver、system、btstack、btctrler、media、net、update。
- `tools` 与 `cpu/br28/tools`: 编译、下载、UI 资源、JTAG、升级资源、配置工具。

## 功能宏与依赖

`Makefile` 定义了大量全局宏，例如 `CONFIG_CPU_BR28`、`CONFIG_UCOS_ENABLE`、`CONFIG_SOUNDBOX`、`CONFIG_WATCH_CASE_ENABLE`、`CONFIG_UPDATA_ENABLE`、`CONFIG_OTA_UPDATA_ENABLE`、`LV_LVGL_H_INCLUDE_SIMPLE` 等。功能开关更多集中在板级 `board_*_cfg.h`。

当前 `board_701n_demo_cfg.h` 中可见的长期约束：

- `TCFG_APP_BT_EN`、`TCFG_APP_MUSIC_EN`、`TCFG_APP_RTC_EN` 默认开启；`TCFG_APP_LINEIN_EN`、`TCFG_APP_PC_EN`、`TCFG_APP_RECORD_EN`、`TCFG_APP_CAT1_EN` 默认关闭。
- `CONFIG_APP_BT_ENABLE` 启用后会开启 `SMART_BOX_EN`，并进一步强制打开 BLE。
- UI 总开关 `TCFG_UI_ENABLE` 开启；LCD 相关默认包含 `TCFG_LCD_SPI_SH8601A_ENABLE`，后续又可能被条件 `#undef` 改写为其他屏配置。检查最终宏值时要看完整条件块。
- 触摸屏 `TCFG_TOUCH_PANEL_ENABLE` 开启；具体触摸型号和 IIC 时序要在同一板级配置文件内核对。
- 存储相关宏存在多处 `#undef`/重定义，改 NOR/NAND/SFC/VIRFAT 时必须从板级配置顶部一路检查到后续条件块。

## UI 与资源

本工程同时包含传统 `ui_new`/LCD UI、LVGL v8.1.0、Lua UI 绑定、UI 资源工具和多套表盘资源。代码侧入口常在 `apps/watch/ui/lcd/STYLE_WATCH_NEW`，资源侧常在 `cpu/br28/tools/UI工程/ui_454x454_watch`。

UI 资源目录内有大量工具生成文件、预览文件、`.bat` 脚本和中文路径。修改 UI 功能时先判断是代码逻辑、资源工程、字体/语言表、还是打包脚本问题；不要把资源生成输出当作手写源码随意重排。

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
make
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
- UI 代码、LVGL、Lua UI 和资源工具并存；不要看到 `lvgl` 目录就默认当前界面全走 LVGL。
- `cpu/br28/tools` 下既有源头 `.c`，也有构建生成 `.bat/.ini` 和下载输出；改前先确认文件角色。
- 中文路径和脚本对 Windows 工具链友好，跨平台脚本改动要同时看 `Makefile` 中 Windows/Linux 分支。
- 根目录 `AC701N.cbp` 是 IDE 工程文件，`AC701N.depend` 和 `AC701N.layout` 更像本地/生成状态；不要优先从它们推断源码结构。

## 信息来源

本文件基于以下项目文件生成：根 `Makefile`，`apps/watch/app_main.c`，`apps/watch/board/br28/board_config.h`，`apps/watch/board/br28/board_701n_demo/board_701n_demo_cfg.h`，目录扫描结果，以及 `cpu/br28/tools`、`cpu/br28/liba`、`include_lib`、`apps/common`、`apps/watch` 的结构。
