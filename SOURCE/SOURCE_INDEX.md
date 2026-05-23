# SOURCE_INDEX.md

> 本文件说明 `sdk/source/` 目录中各类文件的用途、阅读顺序和适用任务。  
> CodeX 每次任务开始前应先读 `CODEX_CONTEXT.md`，再读本索引，并按任务类型选择专题文件。

---

## 1. 目录定位

`sdk/source/` 是碰碰宠物蛋 MVP-A 的工程知识库，用于给 CodeX、开发者、测试、UI 资源整理任务提供稳定上下文。

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
sdk/source/CODEX_CONTEXT.md
sdk/source/SOURCE_INDEX.md
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

该目录保存 PRD 摘要和完整 Markdown 转写。

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

该目录用于 UI、交互、页面、视觉规格任务。

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

该目录用于资源和动画索引，不建议存放大型资源本体。

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
apps/watch/ui/lcd/STYLE_WATCH_NEW
cpu/br28/tools/UI工程/ui_454x454_watch
CONFIG_BOARD_701N_DEMO
```

适合任务：

- 环境扫描；
- 页面注册；
- 资源导入；
- 构建入口查找。

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
| 环境扫描 | `CODEX_CONTEXT.md`, `SOURCE_INDEX.md`, `04_engineering_mapping/sdk_paths.md` |
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
3. 当前任务相关专题文件；
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
