# xgen-containers

固定容量、调用者持有存储的 C11 容器。没有堆、RTOS、平台或 xgen-memory 依赖。

| 能力 | CMake target | 特性 |
| --- | --- | --- |
| 双向链表 | `xgct::list` | 侵入式、任意节点 O(1) 删除、解除全部链接、按需结构诊断 |
| 整数键索引 | `xgct::hash` | uint32 键、固定桶和容量、不分配或释放节点/值 |
| 位集合 | `xgct::bitset` | 外部存储、范围检查、允许临时借用描述符 |
| 字节环形缓冲 | `xgct::ring_buffer` | 任意容量、批量复制、RX/TX 连续区 claim/finish/cancel |

仅 list 不需要 xgen-status。其余 target 只使用 `xgs::status` 类型，不链接错误字符串。
每项通过 `XGCT_BUILD_LIST/HASH/BITSET/RING_BUFFER` 选择，默认全部启用。
包版本为 0.1.0，ABI 为 1。生产始终以 C11 编译；GoogleTest 仅用于显式主机测试。

## 集成

由项目显式提供 xgen-status 0.1.0 ABI 1 的 target、安装包或源码路径。配置不联网。

```cmake
set(XGCT_STATUS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/modules/status" CACHE PATH "")
set(XGS_BUILD_STRINGS OFF CACHE BOOL "")
add_subdirectory(modules/containers)
target_link_libraries(application PRIVATE xgct::list xgct::bitset)
```

安装包消费：

```cmake
find_package(xgen_containers 0.1.0 EXACT CONFIG REQUIRED COMPONENTS list bitset)
target_link_libraries(application PRIVATE xgct::list xgct::bitset)
```

单独 list 的构建无需安装或检出 status：

```sh
cmake -S . -B out/list -DXGCT_BUILD_HASH=OFF -DXGCT_BUILD_BITSET=OFF -DXGCT_BUILD_RING_BUFFER=OFF
cmake --build out/list
cmake --install out/list --prefix out/install-list
```

## 使用契约

调用者负责所有共享访问的串行化，ISR 使用同样遵守该要求。对象、节点和借用缓冲在使用期间不能移动。
list 插入要求节点未链接，删除和定位要求节点属于该链表；SAFE 遍历仅允许删除当前节点。
hash 的 owner 支持跨表重复插入检测，但查找最坏复杂度仍为 O(N)。诊断函数不能验证任意无效地址。

ring 的读写各允许一个在途 claim，两个方向可同时持有不重叠区域。写完成前数据不可读，读完成前空间不可复用。
取消先停止硬件并同步完成回调，再调用 cancel；reset/deinit 在有在途 claim 时返回 BUSY。
具体平台职责、部分完成和循环 DMA 模型见 [DMA 接入](docs/dma.md)。

开发入口：[贡献指南](CONTRIBUTING.md)、[规范采用](docs/standards.md)、[验证记录](docs/validation.md)、[来源](PROVENANCE.md)。
