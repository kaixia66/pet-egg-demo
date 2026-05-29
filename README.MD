# Pet Egg Demo / AC701N Watch SDK

这是基于杰理 AC701N / BR28 手表类 SDK 整理的项目仓库，当前主要工程位于 `D:\0-jieli_sdk\sdk`。仓库目标是保留可复现构建所需的源码、配置、资源和工具输入，同时避免把编译过程文件、打包结果文件纳入版本管理。

## 项目概览

- 主应用：`apps/watch`
- 公共业务与驱动适配：`apps/common`
- BR28 芯片相关实现、链接配置、后处理工具：`cpu/br28`
- 对外 SDK 头文件与预编译库接口：`include_lib`
- 碰碰宠物蛋最小移植骨架：`apps/watch/petegg`
- Windows 构建脚本：`.vscode/winmk.bat`
- 当前默认板级：`CONFIG_BOARD_701N_DEMO`

应用入口在 `apps/watch/app_main.c`，`app_main()` 会初始化 UI、电源、升级资源下载逻辑，然后进入 `app_task_loop()` 进行任务分发。

## 目录结构

```text
apps/watch/                         手表应用、任务、UI、运动、产品测试、用户 API
apps/watch/petegg/                  碰碰宠物蛋 P13 portable vendor copy 与 Jieli port skeleton
apps/watch/board/br28/              BR28 板级配置集合
apps/watch/ui/lcd/STYLE_WATCH_NEW/  当前 LCD 手表 UI action 和 demo 页面
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
apps/watch/board/br28/board_701n_demo/board_701n_demo_cfg.h
```

切换板级时不要只改某一个驱动文件，需要同时检查：

- `CONFIG_BOARD_*` 选择
- 对应 `board_*_cfg.h`
- LCD 屏幕配置
- 触摸配置
- key table
- 存储介质配置
- 蓝牙和 SmartBox 相关宏
- 低功耗、电源、充电相关配置

## PetEgg P13 Minimal Port

`apps/watch/petegg` 保存碰碰宠物蛋 Jieli 701N 最小移植骨架。P13 只建立
portable vendor copy、`jieli_port` stub、fake input/NFC/battery injection 和 compile smoke，
不替换现有 watch app，也不默认接真实 LCD/NFC/BLE/Flash。

关键规则：

- portable modules 不依赖 Jieli SDK、LVGL、SDL 或 simulator adapter；
- `jieli_port` 是后续唯一接触 Jieli LCD、Key、VM/Flash、NFC、BLE、电源、音频的层；
- SD 卡内容扩展已取消，内容卡指 NFC 实体卡；
- NFC 内容卡只激活本地或外置 Flash 预置内容，不承载图片、动画、宠物资源或道具资源；
- 设备本地最多 18 只宠物，宠物数据、卡包、装备、家园资产和 NFC 激活记录保存在设备本地。

更多说明见：

```text
apps/watch/petegg/README.md
SOURCE/petegg/PETEGG_JIELI_MINIMAL_PORT_PLAN.md
SOURCE/petegg/PETEGG_PORTABLE_VENDOR_COPY.md
SOURCE/petegg/PETEGG_JIELI_BUILD_NOTES.md
```

## UI 与资源

本项目同时包含传统杰理 LCD UI、LVGL v8.1.0、Lua UI 绑定、UI 资源工具和多套表盘资源。

常用入口：

```text
apps/watch/ui/lcd/STYLE_WATCH_NEW
cpu/br28/tools/UI工程/ui_454x454_watch
cpu/br28/tools/ui_resource
```

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

## LVGL 开发提示

仓库中已有 LVGL 相关目录和宏，但不能仅凭 `lvgl` 目录判断当前界面已经完全走 LVGL。若希望用 LVGL 替代原表盘表针页面，需要重点确认：

- 板级是否启用 LVGL 相关配置
- 显示驱动和 flush 回调是否正确对接
- 输入设备是否接入 LVGL indev
- 原传统 UI 主页面是否仍被启动
- 资源、内存、PSRAM、刷新 buffer 是否满足新页面需求
- Makefile 是否包含新增 LVGL 页面源文件

建议先以 `apps/watch/board/br28/board_701n_lvgl_demo` 作为参考，再逐步把当前板级迁移到自定义 LVGL 首页。

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
