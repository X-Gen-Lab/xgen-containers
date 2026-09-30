# 更新记录

## 0.1.0（本地，未发布）

- 从 core 提取无拥有权的 list 与 hash，保留旧行为回归并补充清空和结构诊断。
- 增加固定容量 bitset 和字节 ring，支持带身份的 DMA RX/TX 预约、部分完成与取消。
- 增加循环 DMA 累计事件与 overrun 接入示例。
- 提供四个独立 CMake target、组件安装、版本/ABI 检查、GoogleTest、质量与文档门禁。
