# SOURCE_INDEX.md

## P1 Shared Interface Pack Addendum

The P1 shared interface files live under `apps/watch/pet_shared/include/` and are indexed here because
they define the simulator/Jieli contract boundary while `source/` remains documentation-only.
The simulator reference used for ABI alignment is `D:/0-jieli_sdk/simulator/shared_portable/include`.

Interface files:
- `apps/watch/pet_shared/include/pet_types.h`: fixed-width aliases, bool compatibility, shared result
  codes, version macros, static assert and packed layout helpers.
- `apps/watch/pet_shared/include/pet_display_profile.h`: screen shape, RGB565 display profile,
  display owner, flush mode, safe area and rotation fields.
- `apps/watch/pet_shared/include/pet_key.h`: four-key logical input, key actions, event timestamps,
  hold/repeat/raw fields and input snapshots.
- `apps/watch/pet_shared/include/pet_protocol.h`: BLE-independent packet framing and NFC pair payload.
- `apps/watch/pet_shared/include/pet_save_format.h`: packed A/B save slot header, counters, payload
  length, CRC32 and transaction/status fields.
- `apps/watch/pet_shared/include/pet_resource_format.h`: packed resource manifest/header entries,
  resource type enum, CRC fields and canonical resource file names.
- `apps/watch/pet_shared/include/pet_platform.h`: callback table for time, identity, display, input,
  storage, audio, BLE packet, NFC scan/poll and power APIs.
- `apps/watch/pet_shared/pet_shared_compile_check.c`: minimal compile-only include check for the
  shared interface pack.

Simulator name mapping:
- `pet_key.h` maps simulator `pet_input.h`.
- `pet_protocol.h` maps simulator `pet_packet.h` and `pet_nfc_pair_payload.h`.
- `pet_resource_format.h` maps simulator `pet_resource_manifest.h`.
- `pet_platform.h` extends simulator `PetPlatformCallbacks` for future Jieli port callbacks.

Related P1 docs:
- `source/00_project_brief/jieli_701n_customization_baseline.md`
- `source/00_project_brief/open_questions_and_risks.md`

## P2 Jieli Platform HAL Skeleton Addendum

`apps/watch/pet_platform_jieli/` is the Jieli-specific adapter directory for the P1 shared ABI. It is
compiled into the SDK for boundary checking, but P2 keeps all real hardware paths disabled.

P2 files:
- `apps/watch/pet_platform_jieli/pet_platform_jieli.h`: public getter/init entry for the Jieli platform
  callback table.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_internal.h`: internal prototypes and P2 version
  constants for the Jieli adapter modules.
- `apps/watch/pet_platform_jieli/pet_platform_jieli.c`: owns `g_pet_platform_jieli`, fills every
  `pet_platform_t` callback, and provides millis/device identity stubs.
- `apps/watch/pet_platform_jieli/pet_display_jieli.c`: display profile and owner acquire/release stub;
  does not touch the real LCD or LVGL flush path.
- `apps/watch/pet_platform_jieli/pet_input_jieli.c`: key event and snapshot stub; does not alter MVP-A
  key handling.
- `apps/watch/pet_platform_jieli/pet_storage_jieli.c`: storage read/write stub; does not write VM,
  Flash, files or MVP-A save data.
- `apps/watch/pet_platform_jieli/pet_audio_jieli.c`: SFX/audio state stub.
- `apps/watch/pet_platform_jieli/pet_ble_jieli.c`: packet send/poll stub; no real BLE GATT or loopback.
- `apps/watch/pet_platform_jieli/pet_nfc_jieli.c`: card/pair scan stub; no real NFC or fake injection.
- `apps/watch/pet_platform_jieli/pet_power_jieli.c`: fixed test battery values and power TODOs.
- `apps/watch/pet_platform_jieli/pet_debug_jieli.c`: `PET_DEBUG`-guarded injection stubs.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_compile_check.c`: minimal compile-only use of
  `pet_platform_jieli_get()`.

## P3 Display Profile + Input Mapping POC Addendum

P3 stays inside `apps/watch/pet_platform_jieli/` and keeps real hardware paths disabled. It refines the
P2 display/profile and input stubs into auditable POC code:
- `apps/watch/pet_platform_jieli/pet_platform_jieli.h`: now declares
  `pet_platform_jieli_display_self_test()` and `pet_platform_jieli_input_self_test()`.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_internal.h`: declares P3 display constants
  (`PET_JIELI_DISPLAY_*`), placeholder raw key/event constants, and the raw-event mapping helper.
- `apps/watch/pet_platform_jieli/pet_display_jieli.c`: returns the named 454x454 RGB565 profile,
  validates safe area/owner behavior in display self-test, and still never writes to the LCD.
- `apps/watch/pet_platform_jieli/pet_input_jieli.c`: maps placeholder IO raw codes/events to
  `PetKeyEvent`, owns a small private POC queue, returns `PET_RESULT_AGAIN` when no event is queued,
  and provides input self-test coverage.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_compile_check.c`: references both self-test
  entry points without performing real hardware calls.
- `SOURCE/10_engineering_reports/p3_display_input_poc.md`: records display and key-source audit
  findings, mapping table, self-test expectations, and P3 hardware boundaries.

## P4 Render Owner Boundary Addendum

P4 adds the LVGL/Pet2D display-owner boundary while keeping all real rendering hardware paths disabled:
- `apps/watch/pet_platform_jieli/pet_platform_jieli.h`: declares `pet_display_jieli_get_owner()` and
  `pet_display_jieli_owner_self_test()` for controlled owner-state checks.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_internal.h`: exposes the owner getter internally for
  Jieli boundary modules.
- `apps/watch/pet_platform_jieli/pet_display_jieli.c`: factors owner validation into
  `pet_display_jieli_owner_self_test()` and documents the same-owner re-acquire stub policy.
- `apps/watch/mvp_a/ui/mvp_a_lvgl_shell.c`: acquires the LVGL display owner at shell create/render time,
  verifies owner before rendering, and provides `mvp_a_lvgl_shell_release_display_owner()` for future
  handoff. It does not change LVGL flush or page creation logic.
- `apps/watch/mvp_a/ui/mvp_a_lvgl_shell.h`: declares the controlled LVGL owner release hook.
- `apps/watch/pet2d_boundary/pet2d_boundary.h`: public placeholder boundary API for future Pet2D entry,
  exit and self-test.
- `apps/watch/pet2d_boundary/pet2d_boundary.c`: validates PET2D owner acquisition/release without
  allocating a framebuffer or calling LCD flush.
- `apps/watch/pet2d_boundary/pet2d_boundary_compile_check.c`: compile-only reference for the boundary.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_compile_check.c`: references display-owner and
  Pet2D boundary self-tests without hardware calls.
- `SOURCE/10_engineering_reports/p4_render_owner_boundary.md`: records owner state machine, LVGL hook
  points, placeholder behavior and P4 boundaries.

## P5 Resource Manifest Adapter Addendum

P5 adds a read-only resource manifest parser and fixture backend while keeping real Flash/resource
loading disabled:
- `apps/watch/pet_resource_jieli/pet_resource_jieli.h`: public read-only blob parser API, manifest info
  struct, lookup/read helpers and self-test declaration.
- `apps/watch/pet_resource_jieli/pet_resource_jieli.c`: parses the P1/simulator manifest ABI from a
  little-endian memory blob, validates magic/version/header size/entry size/table CRC/entry CRC and
  performs lookup without malloc, file IO or Flash IO.
- `apps/watch/pet_resource_jieli/pet_resource_jieli_test_blob.h`: fixture IDs and test blob export.
- `apps/watch/pet_resource_jieli/pet_resource_jieli_test_blob.c`: small P5-only manifest fixture with
  three entries and fixed CRCs; not a production art/resource route.
- `apps/watch/pet_resource_jieli/pet_resource_jieli_compile_check.c`: compile-only reference for the
  resource parser and self-test.
- `apps/watch/pet2d_boundary/pet2d_boundary.h`: declares `pet2d_boundary_resource_probe_self_test()`.
- `apps/watch/pet2d_boundary/pet2d_boundary.c`: lightly probes the resource self-test without loading
  sprites, entering Pet2D runtime or writing the display.
- `apps/watch/pet2d_boundary/pet2d_boundary_compile_check.c`: includes the resource probe in the
  boundary compile/self-test path.
- `SOURCE/10_engineering_reports/p5_resource_manifest_adapter.md`: records SDK resource audit,
  manifest ABI alignment, test blob contents, parser API and P5 boundaries.

## P6 Save Transaction Adapter Addendum

P6 adds an isolated A/B save adapter under `apps/watch/pet_save_jieli/`. It is compiled for
ABI/transaction checking, but it does not replace MVP-A syscfg save and does not write real VM/Flash:
- `apps/watch/pet_save_jieli/pet_save_jieli.h`: public A/B save adapter API for opening the memory
  backend, loading the latest slot, transaction writes, slot validation, latest-slot selection, CRC32
  and self-test.
- `apps/watch/pet_save_jieli/pet_save_jieli.c`: validates the P1/simulator 64-byte save slot header,
  computes payload CRC32, chooses the higher-counter valid slot, writes inactive slots through a
  staged header, verifies writes and exercises rollback self-tests.
- `apps/watch/pet_save_jieli/pet_save_jieli_backend.h`: shared backend fault enum for P6 failure
  simulation.
- `apps/watch/pet_save_jieli/pet_save_jieli_memory_backend.h`: caller-owned A/B slot buffers and
  test-only write-fault / low-battery flags.
- `apps/watch/pet_save_jieli/pet_save_jieli_memory_backend.c`: no-malloc memory backend init, clear and
  test fault controls.
- `apps/watch/pet_save_jieli/pet_save_jieli_compile_check.c`: compile-only reference for the save
  adapter and self-test.
- `SOURCE/10_engineering_reports/p6_save_transaction_adapter.md`: records MVP-A save audit, ABI
  alignment, transaction/rollback behavior, memory backend fault injection and P6 storage boundaries.

## P7 Protocol Debug Adapter Addendum

P7 adds packet/NFC-pair helpers and test-only BLE/NFC/debug injection paths while keeping real BLE/NFC
hardware disabled:
- `apps/watch/pet_protocol_jieli/pet_protocol_jieli.h`: packet and NFC pair helper API for CRC16,
  build/finalize/validate and self-test.
- `apps/watch/pet_protocol_jieli/pet_protocol_jieli.c`: C99, no-malloc helper implementation aligned
  with simulator packet and 24-byte NFC pair payload semantics.
- `apps/watch/pet_protocol_jieli/pet_protocol_jieli_compile_check.c`: compile-only reference for
  packet and NFC pair ABI checks.
- `apps/watch/pet_platform_jieli/pet_platform_jieli.h`: declares P7 BLE/NFC/debug self-tests and
  `PET_DEBUG` / `PET_PLATFORM_JIELI_TEST` debug injection controls.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_internal.h`: declares test-mode BLE loopback,
  fake NFC injection and fake debug state helpers.
- `apps/watch/pet_platform_jieli/pet_ble_jieli.c`: adds a fixed-size packet loopback queue that is
  available only in test/debug mode; default sends/polls still return `NOT_READY`.
- `apps/watch/pet_platform_jieli/pet_nfc_jieli.c`: adds fake NFC card and NFC pair queues only in
  test/debug mode; real scans remain disabled.
- `apps/watch/pet_platform_jieli/pet_debug_jieli.c`: adds macro-isolated fake time, fake battery,
  BLE packet injection, NFC card injection and NFC pair payload injection.
- `apps/watch/pet_platform_jieli/pet_platform_jieli.c`: reads fake debug time only when test/debug
  mode is compiled.
- `apps/watch/pet_platform_jieli/pet_power_jieli.c`: reads fake debug battery only when test/debug
  mode is compiled.
- `apps/watch/pet_platform_jieli/pet_platform_jieli_compile_check.c`: references protocol, BLE, NFC
  and debug self-tests without real hardware calls.
- `SOURCE/10_engineering_reports/p7_protocol_debug_adapter.md`: records BLE/NFC path audit, ABI
  alignment, loopback/fake queue design, debug injection boundaries and P7 risks.

> 本文件说明 `SOURCE/` 目录中各类文件的用途、阅读顺序和适用任务。
> CodeX 每次任务开始前应先读 `CODEX_CONTEXT.md`，再读本索引，并按任务类型选择专题文件。

---

## 1. 目录定位

`SOURCE/` 是碰碰宠物蛋 MVP-A 的工程知识库，用于给 CodeX、开发者、测试、UI 资源整理任务提供稳定上下文。

它不是 SDK 源码目录，不参与编译，不承载固件业务逻辑。

### 1.1 这个目录用于

- 保存 PRD 摘要；
- 保存 MVP-A 范围锁定；
- 保存 UI 设计原则；
- 保存页面状态机；
- 保存资源命名和 QA 规则；
- 保存工程路径映射；
- 保存 CodeX 常用 prompt；
- 记录需求冲突和执行口径。

### 1.2 这个目录不用于

- 不放固件 C 代码；
- 不放 SDK 构建脚本；
- 不放编译产物；
- 不放大型图片序列帧；
- 不放临时调试日志；
- 不放未经压缩的大 ZIP 资源包；
- 不被 Makefile / 构建系统引用。

---

## 2. 首读文件

每个 CodeX 任务都必须优先读取以下两个文件：

```text
SOURCE/CODEX_CONTEXT.md
SOURCE/SOURCE_INDEX.md
```

### 2.1 `CODEX_CONTEXT.md`

用途：项目最高优先级上下文。

内容包括：

- 项目一句话定义；
- MVP-A 工程目标；
- SDK 路径；
- 4 键 / NFC / BLE / 存档约束；
- UI 圆屏约束；
- 禁止事项；
- 文档优先级；
- CodeX 执行规则。

适用于所有任务。

### 2.2 `SOURCE_INDEX.md`

用途：告诉 CodeX “什么任务该读哪些文件”。

适用于所有任务。

---

## 3. `00_project_brief/`

### 3.1 `mvp_a_scope.md`

用途：锁定 MVP-A 范围。

适合任务：

- 判断某个功能是否应该进入 MVP-A；
- 开始新开发阶段；
- 评估 CodeX 是否扩大范围；
- 与完整商业版 PRD 发生冲突时；
- 拆分 Sprint / 阶段任务。

重点内容：

- MVP-A 目标；
- 必做内容；
- 暂缓内容；
- 页面范围；
- 资源范围；
- 联机范围；
- 存档范围；
- 验收标准。

### 3.2 `codex_rules.md`

用途：CodeX 执行规则。

适合任务：

- 开始大规模工程任务；
- 多文件修改；
- 需要提交 PR；
- 需要进行环境扫描；
- 需要生成报告。

建议内容：

- 修改前必须扫描；
- 禁止误改 SDK 主框架；
- 禁止提交产物；
- 输出格式；
- commit message 规则；
- 风险分级。

### 3.3 `engineering_boundaries.md`

用途：工程边界。

适合任务：

- 判断能否改某个目录；
- 判断是否可以引入依赖；
- 判断是否可以修改板级配置；
- 判断是否要接入构建系统。

建议内容：

- 可修改目录；
- 谨慎修改目录；
- 禁止修改目录；
- 外部依赖规则；
- 编译产物规则。

### 3.4 `open_questions_and_conflicts.md`

用途：记录 PRD 和执行口径冲突。

适合任务：

- 发现 SD 卡 / 取消 SD 卡冲突；
- 发现单宠 / 多宠冲突；
- 发现 MVP-A / 商业版冲突；
- 发现卡牌体系版本冲突；
- 发现 UI 文档与 PRD 不一致；
- 发现素材命名与工程命名不一致。

这是 CodeX 避免乱开发的关键文件。

---

## 4. `01_prd/`

该目录是规划中的 PRD 摘要和完整 Markdown 转写位置；当前 `master` 未提交该目录时，不要假定这些文件已经存在。

### 4.1 推荐结构

```text
01_prd/
  01_investor_prd_summary.md
  02_hardware_prd_summary.md
  03_software_prd_summary.md
  04_pet_system_summary.md
  05_growth_system_summary.md
  06_card_bag_system_summary.md
  prd_full_text/
    01_投资人版_V1.1.md
    02_硬件版_V1.1.md
    03_软件版_V1.1.md
    04_宠物体系_V1.1.md
    05_养成体系_V1.0.md
    06_卡牌卡包体系_V1.0.md
```

### 4.2 摘要文件优先级

CodeX 通常应先读 summary，不要一开始读 full text。

| 文件 | 何时读取 |
|---|---|
| `02_hardware_prd_summary.md` | 硬件输入、NFC、BLE、电源、SD、按键、BOM、结构约束 |
| `03_software_prd_summary.md` | 软件系统、数据归属、卡包、NFC、对战、QTE、存档 |
| `04_pet_system_summary.md` | 宠物、属性、灵兽池、Stage、视觉规则 |
| `05_growth_system_summary.md` | 喂食、陪伴、任务、沉睡、成长算法 |
| `06_card_bag_system_summary.md` | 卡牌、卡包、赠送、图鉴、家园卡、纸卡玩法 |
| `01_investor_prd_summary.md` | 商业叙事、整体价值、Roadmap，不直接作为工程实现依据 |

### 4.3 `prd_full_text/`

用途：

- 追溯原始 PRD；
- 查找完整表格；
- 查找未摘录细节；
- 处理需求争议。

CodeX 只有在 summary 不足时才读取 full text。

---

## 5. `02_mvp_a_ui/`

该目录是规划中的 UI、交互、页面、视觉规格任务位置；当前主线代码事实以 `apps/watch/mvp_a/ui` 和 `apps/watch/mvp_a/core/mvp_a_def.h` 为准。

### 5.1 `ui_design_principles.md`

用途：MVP-A UI 总原则。

应包含：

- 454×454 主画布；
- 360×360 兼容；
- 圆屏安全区；
- 顶部短标题；
- 中央主视觉；
- 底部唯一操作气泡；
- 不做长列表；
- 不做触摸；
- 不做手机式 UI；
- 儿童可读性优先。

适合任务：

- 新增页面；
- 优化页面；
- 资源导入；
- 页面视觉 QA。

### 5.2 `page_inventory.md`

用途：页面清单。

适合任务：

- 判断页面是否已存在；
- 页面注册；
- 页面跳转；
- MVP-A 范围检查；
- 页面资源需求拆分。

### 5.3 `page_state_machine.md`

用途：页面状态机和跳转关系。

适合任务：

- 实现页面切换；
- 梳理状态跳转；
- 异常恢复；
- 设计首页到 Boss / NFC / 卡包路径。

### 5.4 `four_key_interaction_table.md`

用途：每页 4 键交互表。

适合任务：

- 写按键事件；
- 判断确认 / 返回逻辑；
- 优化 QTE；
- 处理长按。

### 5.5 `visual_spec.md`

用途：每页视觉规格。

适合任务：

- 给设计资源命名；
- 导入页面背景；
- 指定主视觉；
- 检查安全区；
- 生成美术任务。

### 5.6 `copy_keys.md`

用途：中文文案 Key。

适合任务：

- 避免硬编码中文；
- 统一短文案；
- 替换页面提示；
- 做多语言预留。

### 5.7 `engineering_handoff_fields.md`

用途：UI / 美术 / CodeX 交接字段。

适合任务：

- 从设计文档转工程资源；
- 建立资源 manifest；
- 标准化页面元数据。

---

## 6. `03_assets/`

该目录是规划中的资源和动画索引位置，不建议存放大型资源本体。当前主线资源 metadata 在 `apps/watch/mvp_a/services/mvp_a_assets.c`，资源落点说明在 `apps/watch/mvp_a/resources/MVP-A_Final_Engineering_Resources_V0.1/README.md`。

### 6.1 `asset_manifest.md`

用途：图片资源清单。

字段建议：

- asset_id；
- file_name；
- type；
- page；
- state；
- size；
- transparent；
- safe_area；
- source；
- status；
- notes。

### 6.2 `animation_manifest.md`

用途：动画资源清单。

字段建议：

- animation_id；
- page；
- actor；
- state；
- frame_prefix；
- fps；
- loop；
- duration_ms；
- sprite_sheet；
- frame_index_csv；
- fallback_png。

### 6.3 `resource_index.md`

用途：工程资源索引。

适合任务：

- 从资源包导入 SDK；
- 检查路径；
- 确认是否漏资源；
- CodeX 自动匹配页面资源。

### 6.4 `asset_naming_rules.md`

用途：资源命名规则。

建议规则：

```text
pet_qinglong_stage0_idle_0001.png
pet_qinglong_stage5_happy_0001.png
boss_yanling_idle_0001.png
ui_btn_confirm_default.png
bg_home_wood_day.png
fx_nfc_success_0001.png
```

### 6.5 `asset_qa_checklist.md`

用途：资源质量检查。

重点：

- 图片居中；
- 圆屏安全区；
- 透明背景；
- 不留白边；
- 不串图；
- 尺寸一致；
- 序列帧命名连续；
- Sprite Sheet 与 frame index 对齐；
- QTE 组件可读；
- 低分辨率可识别。

### 6.6 `references/README.md`

用途：记录参考图来源、Google Drive 文件名、用途。

不建议直接塞大量原始设计图，除非文件很少且确实需要。

### 6.7 `generated_assets/README.md`

用途：记录生成资源包、ZIP、导出版本和下载位置。

---

## 7. `04_engineering_mapping/`

该目录用于把产品规则映射到 SDK。

### 7.1 `sdk_paths.md`

用途：记录 SDK 关键路径。

必须包含：

```text
apps/watch
apps/watch/mvp_a
cpu/br28/ui_driver/lvgl
apps/common/ui/lvgl_v810
apps/watch/ui/lcd/STYLE_WATCH_NEW
cpu/br28/tools/UI工程/ui_454x454_watch
CONFIG_BOARD_701N_LVGL_DEMO
```

适合任务：

- 环境扫描；
- 页面注册；
- 资源导入；
- 构建入口查找。

当前 `master` 的 MVP-A 主线事实：

- 板级选择：`apps/watch/board/br28/board_config.h` 启用 `CONFIG_BOARD_701N_LVGL_DEMO`。
- LVGL 版本：Makefile 使用 `apps/common/ui/lvgl_v810`，版本宏为 8.1.0。
- MVP-A 应用层：`apps/watch/mvp_a/core/mvp_a_app.c`。
- MVP-A LVGL shell：`apps/watch/mvp_a/ui/mvp_a_lvgl_shell.c`。
- 存档：`apps/watch/mvp_a/core/mvp_a_save.c` 通过 `syscfg` 保存。
- NFC/BLE：`apps/watch/mvp_a/services/mvp_a_platform.c` 当前仍为 `MVP_A_RESULT_NOT_READY` stub。

### 7.2 `ui_resource_mapping.md`

用途：页面与资源路径映射。

适合任务：

- 把 PNG / Sprite Sheet 接入 UI 工程；
- 检查资源是否已注册；
- 页面引用资源。

### 7.3 `state_to_code_mapping.md`

用途：状态机与代码模块映射。

适合任务：

- 实现页面状态跳转；
- 查找按键处理；
- 处理异常恢复。

### 7.4 `storage_model.md`

用途：设备卡包、宠物状态、本地存档模型。

适合任务：

- 存档结构；
- 版本迁移；
- 数据恢复；
- Debug 页。

### 7.5 `nfc_ble_flow.md`

用途：NFC 和 BLE 流程映射。

适合任务：

- NFC 读卡；
- 碰一碰；
- BLE 建连；
- 合作 Boss；
- 断线恢复。

### 7.6 `save_recovery_rules.md`

用途：存档异常恢复规则。

适合任务：

- 低电保护；
- 写入失败；
- 重启恢复；
- 数据损坏处理。

---

## 8. `05_codex_prompts/`

该目录保存可复用 CodeX Prompt。

### 8.1 `00_read_source_first.md`

用途：每次任务前提醒 CodeX 先读 source。

### 8.2 `01_environment_scan.md`

用途：工程环境扫描。

### 8.3 `02_ui_state_machine_task.md`

用途：UI 页面和状态机任务。

### 8.4 `03_resource_import_task.md`

用途：资源导入任务。

### 8.5 `04_nfc_ble_task.md`

用途：NFC / BLE 任务。

### 8.6 `05_save_recovery_task.md`

用途：存档 / 异常恢复任务。

---

## 9. `99_archive/`

### 9.1 `deprecated_or_conflicting_notes.md`

用途：

- 放弃的旧方案；
- 已冲突但保留记录的规则；
- 旧版 PRD 摘要；
- 不进入 MVP-A 的商业版设想。

不要让 CodeX 从这里直接实现功能。

---

## 10. 任务到文件的快速索引

| 任务类型 | 必读文件 |
|---|---|
| 环境扫描 | `CODEX_CONTEXT.md`, `SOURCE_INDEX.md`; 若 `04_engineering_mapping/sdk_paths.md` 尚不存在，直接查 `Makefile`、`board_config.h`、`apps/watch/mvp_a` |
| UI 页面开发 | `CODEX_CONTEXT.md`, `02_mvp_a_ui/ui_design_principles.md`, `02_mvp_a_ui/page_inventory.md`, `02_mvp_a_ui/four_key_interaction_table.md` |
| 页面状态机 | `02_mvp_a_ui/page_state_machine.md`, `04_engineering_mapping/state_to_code_mapping.md` |
| 资源导入 | `03_assets/asset_manifest.md`, `03_assets/animation_manifest.md`, `03_assets/asset_naming_rules.md`, `03_assets/asset_qa_checklist.md` |
| NFC 读卡 | `04_engineering_mapping/nfc_ble_flow.md`, `01_prd/02_hardware_prd_summary.md`, `01_prd/03_software_prd_summary.md` |
| BLE 联机 | `04_engineering_mapping/nfc_ble_flow.md`, `01_prd/03_software_prd_summary.md` |
| 合作 Boss | `mvp_a_scope.md`, `page_state_machine.md`, `nfc_ble_flow.md` |
| 存档恢复 | `04_engineering_mapping/storage_model.md`, `04_engineering_mapping/save_recovery_rules.md`, `01_prd/03_software_prd_summary.md` |
| 判断是否进 MVP-A | `00_project_brief/mvp_a_scope.md`, `00_project_brief/open_questions_and_conflicts.md` |
| 处理需求冲突 | `00_project_brief/open_questions_and_conflicts.md` |
| 生成 CodeX 新任务 | `05_codex_prompts/` |

---

## 11. 阅读优先级规则

如果时间有限，CodeX 按以下顺序阅读：

1. `CODEX_CONTEXT.md`
2. `SOURCE_INDEX.md`
3. 当前任务相关专题文件；若索引列出的专题文件尚未提交，则直接读取对应代码路径；
4. 对应 PRD summary；
5. 对应 PRD full text；
6. archive 文件只作历史参考。

---

## 12. 更新规则

每当 PRD、UI、资源或工程口径发生变化，应同步更新：

- `CODEX_CONTEXT.md`：只更新最高优先级口径；
- `SOURCE_INDEX.md`：只更新文件索引和阅读规则；
- `mvp_a_scope.md`：更新 MVP-A 范围；
- `open_questions_and_conflicts.md`：记录冲突、决策和弃用内容；
- 对应专题文件：记录详细规则。

每次更新建议 commit message：

```text
docs(source): update codex project context
docs(source): update MVP-A scope
docs(source): record PRD conflict resolution
docs(source): add UI source index
```
