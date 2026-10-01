# 工程规范采用

采用 X-Gen 工程规范 1.0.0，权威正文位于 xgen-roadmap 的 `docs/standards/`。
当前为 [PROVENANCE](../PROVENANCE.md) 记录的本地基线；离线独立开发可使用带该提交标识的只读规范包。
不假设兄弟仓库存在，不虚构远端 tag。发布前补齐可获取规范来源。

适用范围：include、src、examples、tests 的自有 C/C++，以及 CMake、Python 薄入口和文档。
生成版本头及构建产物不在源码格式分母中；GoogleTest 和 xgen-status 由各自仓库负责。
质量工具版本和范围只在 tools/quality.json 与固定 xgen-quality 策略维护；本地、hook、CI 使用同一入口。

## 有界分析例外

以下是精确诊断例外，不关闭全仓规则。维护责任为本组件维护者；更改对应接口或内存边界时必须重新评审。

| 规则 | 精确范围 | 理由和补偿验证 |
| --- | --- | --- |
| bugprone-easily-swappable-parameters | xgct_bitset_init、私有 advance | 公共接口分别接收字节容量和位数；私有函数分别接收位置、步长和容量。参数有单位契约和边界测试。 |
| clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling | xgct_bitset_init/clear_all 的 memset；xgct_ring_write/read 的 memcpy | MSVC 模型建议 Annex K 函数，但这些并非所有 C11 平台提供。实现先限制容量并分解连续区，测试覆盖耗尽、回绕和边界；调用者保证公开存储契约。 |

对应 NOLINT 仅围住相关函数的指定规则，其他诊断仍阻断。接口改变或分析器能识别容量证明时删除例外。
结构诊断不能验证任意地址；DMA 硬件一致性、ISR 时限和最终资源预算仍由平台/产品验证。

公开 API 使用严格 Doxygen；输出无告警不能替代行为评审。测试和自动检查结果见 [验证记录](validation.md)。

## 2026-10-01 空行规范采用

采用上述初始来源之后、尚未发布的工程规范 1.0.0 local 增补 C-020、C-021、DOC-013。本轮仅迁移空行和 LF，不改变生产或测试代码行为。根 `.clang-format` 对应受控模板 SHA-256 `ffdb331b03ae4f6c5f75ee54d4afaa6d4741f5ac3ec57f55b0f04ad8ec396a2a`；配置来源和实际增补见 [来源记录](../PROVENANCE.md)。

`python tools/quality.py format` 是只读检查入口，由固定 clang-format 19.1.5 检查排版，并由共享工具检查其支持的公共 C 头结构分隔、独立 API 注释与声明组，以及 Doxygen 紧邻直接声明的形式。它不声称覆盖任意 C++ AST；条件包装、复杂宏、函数内语义阶段和字段分组仍需人工评审。

共享工具当前版本和固定源码来源只在 [tools/quality.json](../tools/quality.json) 维护；初始规范提交不代表已经包含这次增补。采用记录与实际执行证据分开，本次同步不声明远端 CI 已运行。
