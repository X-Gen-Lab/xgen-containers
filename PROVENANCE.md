# 来源与授权

list/hash 的源代码、公开契约及迁移测试来自 xgen-core 提交
`dc5eb1167ba21de384e7a760a8586b87502b73b2`，路径为 `src/list.c`、`src/hash.c`、
`include/xgc/list.h`、`include/xgc/hash.h` 和 `tests/test_core.c`。
迁移前运行原 CTest 的 list/hash 用例通过；迁移后原断言改成 GoogleTest，Release 不依赖 assert。
公开前缀调整为 xgct，状态类型消费独立 xgen-status。bitset、ring、诊断接口、循环 DMA 示例及工程设施为新增。

来源仓库未确定完整顶层许可证，本次不擅自加入新授权或 SPDX 标识。公开发布前由仓库所有者确认授权。

开发依赖与受控配置：

- GoogleTest 1.16.0：`ff6133ab49b364a883a55ba75c39e520fea6245b`，仅主机依赖，保留上游授权。
- xgen-status 0.1.0：`28bc7be369b81e9b9b9d7379743dd0da6fa4647b`，类型 target ABI 1。
- xgen-quality 0.1.0：初次接入的历史基线为 `9c957d406d935d27959babe7f01172151f9d26c1`；当前版本和固定来源唯一维护于 [tools/quality.json](tools/quality.json)，不以历史提交替代当前配置。
- 工程规范 1.0.0：xgen-roadmap 本地文档基线，提交 `9ca20873878b0de4893241d4c8c08cce01b40408`。
- 格式和 EditorConfig 经工程规范模板分发；原始 Nexus 来源为 `7a203266082ad1f655b6686713b2b7950ba31ec6`。
- 构建、安装、pre-commit、严格 Doxygen、质量薄入口和 CI 以 xgen-crc/bytes/status 已验证结构为参考，按本仓依赖调整。

未声明本轮源码或依赖提交已发布到远端。版本字符串不是许可证或发布证明。

## 2026-10-01 格式配置增补

按 xgen-roadmap 尚未发布的工程规范 1.0.0 local 增补 C-020、C-021、DOC-013，受控 `.clang-format` 增加 `SeparateDefinitionBlocks: Always`、`KeepEmptyLines` 三项 false 和 `LineEnding: LF`，保留 `MaxEmptyLinesToKeep: 1`。当前格式模板 SHA-256 为 `ffdb331b03ae4f6c5f75ee54d4afaa6d4741f5ac3ec57f55b0f04ad8ec396a2a`；这些是初始来源之后的受控调整，`.editorconfig` 的来源不变。

本轮源码迁移仅调整空行和 LF，不改变代码行为；来源、作者和许可证事实保持。格式检查边界见 [规范采用记录](docs/standards.md)，实际工具版本和源码固定值读取当前配置。这份记录不代表远端 CI 已执行。
