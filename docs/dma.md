# DMA 与环形缓冲接入

本仓提供字节存储的所有权状态，不控制 DMA。两种接入方式要分别选择。

## 单次或可重新装载 DMA：零拷贝 claim

RX 顺序为 write_claim → 平台准备缓冲和启动 DMA → 硬件停止访问并完成缓存同步 → write_finish。
TX 顺序为 read_claim → 平台准备缓存并启动 DMA → 传输停止访问 → read_finish。

claim 返回连续区域，长度可能因物理尾部或容量而缩短。不能让硬件访问返回区域之外的地址。
finish 的 completed 是实际完成的前缀字节数；剩余预约全部解除。RX 未完成区域不发布；TX 未发送前缀之后的数据仍留队列。
零完成等价于取消。错误或主动停止时，先停止硬件、排空相关完成事件，再 cancel；cancel 本身不会停止硬件。

每方向只允许一个活动 claim，双向 claim 可共存。普通复制 API 在同方向有活动 claim 时返回零。
查询 readable 包括正在 TX 的数据；writable 排除 RX 已预约空间。reset/deinit 遇到任一 claim 返回 BUSY，状态不变。

每次 claim 返回 uint64 token。finish/cancel 必须带对应 token，错误长度、重复完成和本生命周期的过期 token 不改变状态。
token 不回绕：达到 UINT64_MAX 后拒绝新 claim；普通复制不依赖 token。reset 不重置计数。
重新 init 建立新生命周期，必须事先保证旧回调完全停止，不能用 token 代替硬件生命周期隔离。

## 自由运行循环 DMA：显式累计事件与稳定快照

[circular_dma.c](../examples/circular_dma.c) 是可编译、可测试的接入示例。它从独立硬件循环缓冲复制到软件 ring。
该模型有一次复制；不要把它误写成任意循环 DMA 直接写 ring 的零拷贝实现。

平台提供 produced 的单调绝对字节计数，并保证本次调用读到的整个未消费范围保持稳定：停止 DMA，或提供经验证的不可追尾覆盖窗口。
仅检查调用开始时的 producer-consumer 距离不能防止复制期间 DMA 追上读指针。

绝对计数需要不会丢失的半传输/全传输事件及一致的位置快照。仅用当前 modulo 写位置减上一次位置，无法区分零进度与多次完整回绕。
如果平台无法保证事件无丢失，就必须上报未知/丢失并重新同步，不能猜测累计数。

示例在 produced-consumed 大于硬件容量时锁存 overrun 并拒绝继续消费，不悄悄覆盖数据。
软件队列满时允许先复制可容纳部分，返回 CAPACITY 并保留尚未消费计数；平台需要及时背压或停止 DMA。
恢复由平台显式停止硬件、记录丢失、重置计数及软件队列，再重新启动。

## 平台责任与资源

平台负责 DMA 可达地址、地址/长度对齐、缓存行隔离、clean/invalidate、内存屏障、硬件状态、ISR/任务串行化和超时。
只有字节对齐的缓冲区不自动满足 DMA；一个缓存行同时包含 DMA 与 CPU 仍在修改的其他数据也不安全。
本库未使用原子操作或内建锁，不声明 lock-free、跨核安全或特定板卡硬件验证。

ring 状态包含 1 个指针、6 个 size_t 和 3 个 uint64_t；token 字段合计 24 字节。
ARM GCC 15.2.1 Cortex-M0 对象布局实测 ring 状态 56 字节，读/写 span 各 16 字节；
这是交叉编译常量区提取结果，不是 MCU 运行结果。claim/finish 为 O(1)，复制为 O(n)。
64 位计数在小 MCU 上的指令和状态成本要在实际 ELF 测量；本仓不以主机结构体大小替代 MCU 预算。
