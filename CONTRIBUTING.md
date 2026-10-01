# 开发与验证

采用 X-Gen 工程规范 1.0.0，规则来源和本仓补充见 [规范采用](docs/standards.md)。
新增行为按 TDD：实际验证 RED，再实现和验证 GREEN。生产以 C11 编译，测试链接真实 target 并以 C++17 使用 GoogleTest。

依赖由开发环境显式准备：GoogleTest 1.16.0、xgen-status 0.1.0 ABI 1，以及固定 xgen-quality 0.1.0。
来源见 [PROVENANCE](PROVENANCE.md)。不允许 CMake 自动下载。以下路径是已准备依赖的变量，不要求存在某个兄弟目录：

```sh
cmake --preset host -DXGCT_STATUS_SOURCE_DIR="$STATUS_SOURCE" -DXGS_BUILD_STRINGS=OFF -DGTest_DIR="$GTEST_CONFIG_DIR"
cmake --build --preset host
python tools/quality.py test --build-dir out/host
python tools/quality.py text
python tools/quality.py format
python tools/quality.py cppcheck --build-dir out/host
python tools/quality.py tidy --build-dir out/host
python tools/quality.py docs
python -m pre_commit run --all-files
```

Windows 选择 GCC 时显式传 `CMAKE_C_COMPILER` 和 `CMAKE_CXX_COMPILER`；MSVC 在开发者命令环境中配置。
clang-tidy 必须使用与分析环境匹配的数据库；不要用未配置 MinGW 系统头的 MSVC clang-tidy 解释 GCC 数据库。

覆盖率使用 coverage preset，运行测试后调用 `python tools/quality.py coverage --build-dir out/coverage`。
行、函数、分支各自至少 80%，不取平均。例子实现同样纳入生产分析及覆盖率；测试和依赖代码不计入。
CI 显式从 `X-Gen-Lab/xgen-quality` 获取配置中固定的工具提交，允许用 `XGEN_QUALITY_REPOSITORY` 覆盖为受控镜像。状态组件同样默认从 `X-Gen-Lab/xgen-status` 获取固定提交，可用 `XGEN_STATUS_REPOSITORY` 覆盖。仓库覆盖、固定提交不可获取或安装版本不符时失败；不跟随依赖主分支，远端验证与本地结果分别记录。
