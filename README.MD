# Pet Egg Demo / AC701N Watch SDK

这是基于杰理 AC701N / BR28 手表类 SDK 整理的项目仓库，当前主要工程位于 `D:\0-jieli_sdk\sdk`。仓库目标是保留可复现构建所需的源码、配置、资源和工具输入，同时避免把编译过程文件、打包结果文件纳入版本管理。

## 项目概览

- 主应用：`apps/watch`
- 公共业务与驱动适配：`apps/common`
- BR28 芯片相关实现、链接配置、后处理工具：`cpu/br28`
- 对外 SDK 头文件与预编译库接口：`include_lib`
- Windows 构建脚本：`.vscode/winmk.bat`
- 当前主线板级：`CONFIG_BOARD_701N_LVGL_DEMO`
- MVP-A Demo 代码：`apps/watch/mvp_a`

应用入口在 `apps/watch/app_main.c`，`app_main()` 会初始化 UI、电源、升级资源下载逻辑，然后进入 `app_task_loop()` 进行任务分发。

## 目录结构

```text
apps/watch/                         手表应用、任务、UI、运动、产品测试、用户 API
apps/watch/mvp_a/                   MVP-A LVGL Demo 框架、状态、存档、资源映射和页面
apps/watch/board/br28/              BR28 板级配置集合
apps/watch/ui/lcd/STYLE_WATCH_NEW/  传统 LCD 手表 UI action 和 demo 页面
apps/watch/ui/lua_ui/               Lua UI 绑定层
apps/common/                        公共音频、设备、文件、更新、网络、LVGL、第三方协议
cpu/br28/                           芯片相关实现、库、链接脚本源和工具
cpu/br28/liba/                      杰理预编译库
cpu/br28/tools/                     下载、打包、UI 资源、JTAG、配置工具
include_lib/                        SDK 头文件接口
tools/                              通用构建辅助工具
```

## 构建环境

Windows 下默认使用杰理 PI32 工具链：

```text
C:/JL/pi32/bin
```

常见工具包括 `clang.exe`、`pi32v2-lto-wrapper.exe`、`llvm-ar.exe` 等。请先确认工具链已经安装，并且仓库中的 `tools/utils` 工具未被安全软件隔离。

## 构建方法

推荐使用项目内的 Windows 构建脚本：

```powershell
.\.vscode\winmk.bat all
```

该脚本会把 `tools\utils` 加入 `PATH`，再执行并行 `make all`。

为了避免终端输出过长，建议把构建日志重定向到本地日志目录：

```powershell
New-Item -ItemType Directory -Force build_logs | Out-Null
.\.vscode\winmk.bat all *> build_logs\winmk-all.log
Select-String -Path build_logs\winmk-all.log -Pattern 'error|warning|failed|undefined|No such file|cannot|Error' -CaseSensitive:$false
Get-Content -Path build_logs\winmk-all.log -Tail 80
```

直接构建命令：

```powershell
make
```

详细构建：

```powershell
make VERBOSE=1
```

清理构建：

```powershell
make clean
```

## 构建产物

主构建输出通常位于：

```text
cpu/br28/tools/sdk.elf
```

构建和后处理过程还会生成 `sdk.lst`、`symbol_tbl.txt`、`app.bin`、`jl_isd.fw`、`update.ufw`、`upgrade.zip` 等文件。这些属于过程文件或结果文件，已通过 `.gitignore` 排除，不应提交到 Git。

典型生成物包括：

```text
build_logs/
obj/
objs/
*.depend
*.layout
cpu/br28/sdk.ld
cpu/br28/sdk_used_list.used
cpu/br28/tools/app.bin
cpu/br28/tools/sdk.elf
cpu/br28/tools/sdk.lst
cpu/br28/tools/sdk.map
cpu/br28/tools/symbol_tbl.txt
cpu/br28/tools/download/watch/update.ufw
cpu/br28/tools/download/watch/upgrade.zip
```

## 板级与配置入口

板级选择入口：

```text
apps/watch/board/br28/board_config.h
```

当前板级配置：

```text
apps/watch/board/br28/board_701n_lvgl_demo/board_701n_lvgl_demo_cfg.h
```

当前 `board_config.h` 选择 `CONFIG_BOARD_701N_LVGL_DEMO`，该板级中 `LVGL_TEST_ENABLE` 为 1。若切回传统表盘 demo，需要同步检查 `board_701n_demo_cfg.h`、Makefile 源文件列表、显示屏、触摸、按键和存储宏。

切换板级时不要只改某一个驱动文件，需要同时检查：

- `CONFIG_BOARD_*` 选择
- 对应 `board_*_cfg.h`
- LCD 屏幕配置
- 触摸配置
- key table
- 存储介质配置
- 蓝牙和 SmartBox 相关宏
- 低功耗、电源、充电相关配置

## UI 与资源

本项目同时包含传统杰理 LCD UI、LVGL v8.1.0、Lua UI 绑定、UI 资源工具和多套表盘资源。

常用入口：

```text
apps/watch/mvp_a
cpu/br28/ui_driver/lvgl
apps/watch/ui/lcd/STYLE_WATCH_NEW
cpu/br28/tools/UI工程/ui_454x454_watch
cpu/br28/tools/ui_resource
```

当前主线的 MVP-A 页面不是传统杰理 UI 页面，而是通过 `apps/watch/mvp_a/ui/mvp_a_lvgl_shell.c` 创建 LVGL 页面。`apps/watch/mvp_a/core/mvp_a_app.c` 负责场景和 4 键事件分发，`apps/watch/mvp_a/core/mvp_a_save.c` 通过 `syscfg` 保存 `mvp_a_save_data_t`。

UI 修改时先判断问题属于哪一类：

- C 代码逻辑
- UI 资源工程
- 字体或多国语言资源
- 屏幕驱动配置
- 触摸事件
- 下载/打包脚本

传统 UI 接口常用函数包括：

- 页面显示/隐藏：`ui_show_main()`、`ui_hide_main()`、`ui_hide_curr_main()`
- 控件显示/隐藏：`ui_show()`、`ui_hide()`
- 控件事件注册：`REGISTER_UI_EVENT_HANDLER(id)`

注意：`onchange` 回调中通常不要直接调用 `ui_show()` / `ui_hide()`，控件显示状态更适合在 `onkey` 或 `ontouch` 中处理。

## MVP-A 当前主线状态

- LVGL 版本：当前 Makefile 使用 `apps/common/ui/lvgl_v810`，版本宏为 8.1.0。
- 当前页面范围：Boot、Home、Growth/Care、Training、CardBag、NFC、Coop、Boss、Diary、Debug。
- NFC/BLE：`apps/watch/mvp_a/services/mvp_a_platform.c` 仍返回 `MVP_A_RESULT_NOT_READY`，属于待接硬件/协议 stub。
- 资源：`mvp_a_assets.c` 是资源 metadata/manifest 映射；`mvp_a_image_assets.c` 和少量 PNG 只是当前 main 的过渡验证资源。
- 存档：当前使用 `syscfg_read/write`，带 magic、version、data_len、checksum。

后续 Pet2D 架构口径：HOME / Observe 将由 Pet2D 实现，LVGL 继续用于系统菜单、卡包、状态和 Debug HUD；二者通过 Render Owner 互斥。当前 `master` 尚未包含 feature 分支上的 Pet2D 实现，不能把未合入代码当成主线能力。

## 资源策略

正式图像资源不要继续转成 C 数组。后续应优先使用杰理 SDK 资源工具 / `image_dll` 生成硬件可识别压缩格式，并放入 16M 外置 Flash。运行时禁止解 PNG / JSON；能让 IMB 从 NOR Flash 直接读取的资源，优先通过 `flash_file_info` / 地址映射交给 IMB。实时旋转只用于小图或先加载到 SRAM 的资源，不使用 full framebuffer 或双全屏 framebuffer。

## LVGL 开发提示

仓库中已有多份 LVGL 相关目录。当前 Makefile 使用 `apps/common/ui/lvgl_v810` 和 `cpu/br28/ui_driver/lvgl`；另一个 `apps/common/ui/lvgl` 目录存在但不是当前主线编译路径。新增 LVGL 页面时需要重点确认：

- 板级是否仍启用 `CONFIG_BOARD_701N_LVGL_DEMO` 和 `LVGL_TEST_ENABLE`
- 显示驱动和 flush 回调是否正确对接
- 输入设备是否接入 LVGL indev
- 是否会与传统 UI 或未来 Pet2D Render Owner 冲突
- 资源、内存、PSRAM、刷新 buffer 是否满足新页面需求
- Makefile 是否包含新增 LVGL 页面源文件

## 常见注意事项

- 仓库包含大量中文路径、中文注释和 Windows 批处理脚本，编辑前注意文件编码。
- 不要直接编辑 `cpu/br28/liba/*.a`，这些是预编译库。
- `cpu/br28/tools` 中既有构建输入工具，也有构建生成物，提交前要用 `git status --short` 和 `git diff` 核对。
- UI 工程资源和工具生成文件很多，不要把预览缓存或本地构建输出当作长期源码提交。
- 如果安全软件隔离了 `tools/utils` 下的构建辅助工具，构建可能会失败。

## 推荐开发流程

```powershell
git status --short
.\.vscode\winmk.bat all *> build_logs\winmk-all.log
Select-String -Path build_logs\winmk-all.log -Pattern 'error|warning|failed|undefined|No such file|cannot|Error' -CaseSensitive:$false
git diff
```

提交前确认只包含源码、配置、资源源文件和必要工具输入，不包含本地日志、构建中间物或固件打包结果。
