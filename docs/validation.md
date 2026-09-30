# 实施与验证记录

本地分支 `feat/independent-containers`。版本 0.1.0 / ABI 1；未推送、未创建发布 tag。

## TDD 证据

| 内容 | RED 提交与实际结果 | GREEN 提交与实际结果 |
| --- | --- | --- |
| bitset、DMA ring | 38079ba：测试编译成功，新增已声明接口未实现导致链接失败 | de9259d：8 个 GoogleTest 通过 |
| list clear、list/hash 诊断 | b18f019：新增接口 undefined reference | ec566c8：10 个 GoogleTest 通过 |
| 循环 DMA 事件计数与 overrun 示例 | 370a504：dma_cycle_drain 未实现的链接失败 | 017c2b3：12 个 GoogleTest 通过 |

命令均为 `cmake --build out/host`；GREEN 随后运行 `ctest --test-dir out/host --output-on-failure --no-tests=error`。
完整 RED 原始输出保存在本地 `out/red-build.log`、`out/red-diagnostics.log`、`out/red-cycle.log`，对应提交可重放。
纯迁移前 `out/core-baseline` 运行 core 的 list/hash CTest 2/2 通过，没有故意破坏旧实现制造 RED。

后续整理补充非法参数、碰撞链删除、容器复用、双向 DMA 完成顺序，以及 C/C++ 单头编译验证。
固定种子 ring 复制模型与 std::deque 比较 6000 步，覆盖容量 1、非二次幂 7、容量 16。

## 当前验证范围

| 项目 | 结果与证据 |
| --- | --- |
| GCC 13.2.0 | 33/33 CTest：16 个 GoogleTest、17 个集成消费者；C11 Release 与公开头独立编译通过 |
| MSVC 19.40.33811 | 33/33 CTest；与 GCC 使用不同的 GoogleTest 二进制，生产 C11、测试 C++17 |
| 行覆盖 | 356/358，99.4% |
| 函数覆盖 | 49/49，100% |
| 分支覆盖 | 297/302，98.3% |
| 覆盖分母 | src 四项实现与 examples/circular_dma.c；不含测试、GoogleTest、status、生成文件 |
| 静态分析 | 固定 Cppcheck 2.21.0 与 clang-tidy 19.1.0；真实 GCC/MSVC 编译数据库，精确例外见 standards.md |
| API 文档 | Doxygen 1.16.0 严格公开头模式；告警导致失败 |
| 格式与文本 | clang-format 19.1.5，pre-commit 与 CI 共用 xgen-quality 薄入口 |

工具失败阻断也已实际观察：缺少 gcovr 时 coverage 非零退出；MSVC 环境优先找到 clang-tidy 19.1.5 时，
固定版本门禁拒绝执行（要求 19.1.0）。显式选择已准备的固定工具后检查通过，未放宽版本规则。

每次检查的源码提交、脏状态、工具版本、命令和状态由 `out/reports/quality-*.json` 自动记录。
详细覆盖未命中行与分支保留在 `out/reports/coverage.json`；统计不以四舍五入结果替代门槛判定。

组件消费测试包括四项源码消费、四项安装消费、可选缺失组件、必需缺失组件、错误包版本、
错误 ABI、错误 target 类型、重复提供兼容 target 及不完整预提供集合。
list-only 源码消费断言没有 xgs::status；安装消费显式禁用 xgen_status 查找，确保没有隐藏依赖。

## 资源与平台边界

根项目使用 ARM GCC 15.2.1、Cortex-M0 编译常量并提取目标对象布局：
list 12 字节、list node 8、hash 16、hash node 16、bitset 8、ring 56、读写 span 各 16。
证据为产品工作区 `xgen-link/build/independent-migration/component-layout.json`；这是交叉编译对象事实，不是上板运行。
bitset 可直接用临时描述符借用既有存储，不要求每个消费者常驻额外 8 字节。

未在本仓执行真实 DMA、缓存一致性、中断并发、RTOS、整机 RAM/栈预算和 sanitizer 验证。
本仓没有锁或原子实现，主机模型不能证明板卡 DMA 时序；硬件任务由板包与产品承担。
本地 CI 配置已建立；固定依赖未发布前，不声明远端 CI 已运行或可读取相应提交。
