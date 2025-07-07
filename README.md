# ESP32 蜂鸣器模拟器与驱动程序 (miniaudio 增强版)

本项目提供了一个用于控制无源蜂鸣器的硬件抽象层（HAL），使得同一套应用程序逻辑代码可以在PC（用于模拟）和真实的ESP32设备上运行。

PC端的模拟器经过了功能增强，使用轻量级的 **`miniaudio`** 库来生成高质量的**方波**声音，从而能更真实地模拟物理蜂鸣器的音色，且运行稳定可靠。

## 文件结构

-   `buzzer_hal.h`: 硬件抽象层接口，定义了控制蜂鸣器的标准函数。
-   `main.cpp`: 主应用程序逻辑，使用HAL来创建音效和旋律，平台无关。
-   `buzzer_sim.cpp`: **PC端**的HAL实现，使用 `miniaudio` 库播放声音。
-   `buzzer_esp32.cpp`: **ESP32端**的HAL实现，使用 `ledc` 驱动控制物理蜂鸣器。
-   `miniaudio.h`: **（必需）** 第三方单头文件音频库。
-   `.vscode/`: 包含为 Visual Studio Code 配置好的构建和调试环境。
    -   `tasks.json`: 定义了如何编译PC模拟器。
    -   `launch.json`: 定义了如何运行和调试PC模拟器。
-   `README.md`: 本说明文件。

## PC端模拟器环境准备

要在PC上成功编译和运行模拟器，你需要准备以下两样东西：

### 1. C++ 编译器 (MinGW-w64)

你需要一个C++编译器。本项目推荐使用 **MinGW-w64** (通过 MSYS2 安装)。如果你尚未安装，请参照相关教程完成安装，并确保已将编译器的 `bin` 目录（例如 `C:\msys64\mingw64\bin`）添加到了系统的 `PATH` 环境变量中。

### 2. `miniaudio.h` 音频库文件

本项目依赖 `miniaudio` 这个单头文件库。
1.  请从其官方GitHub仓库下载：[https://github.com/mackron/miniaudio](https://github.com/mackron/miniaudio)
2.  在页面中找到并点击 `miniaudio.h` 文件。
3.  在新页面右上角点击 **"Raw"** 按钮，然后在打开的纯文本页面上右键选择“另存为...”。
4.  将 `miniaudio.h` 文件保存到本项目的根目录下，与 `main.cpp` 等文件放在一起。

## 如何编译和运行

### 推荐方式：使用 VS Code (一键编译运行)

本项目已经为你配置好了完整的VS Code环境，这是最简单、最可靠的运行方式。

1.  确保你已完成上述的“环境准备”步骤。
2.  使用VS Code打开本项目文件夹。
3.  确保VS Code已经安装了微软官方的“C/C++”扩展。
4.  直接按下 **`F5`** 键。

VS Code会自动执行 `.vscode/tasks.json` 中定义的编译任务，然后启动 `.vscode/launch.json` 中定义的调试会话。

**注意**: 本项目中的 `.vscode` 配置包含了**环境隔离**设置，可以临时将MinGW的路径置于`PATH`变量顶端，从而避免与系统中其他程序（如 `pyenv-win`）可能存在的库版本冲突。因此，强烈推荐使用此方式。

### 手动方式：使用命令行

如果你希望手动编译，可以在项目根目录下打开终端（如PowerShell或CMD），然后执行以下命令：

```bash
g++ -g -std=c++11 -DPLATFORM_PC main.cpp buzzer_sim.cpp -o buzzer_simulator.exe -lkernel32 -lwinmm -lole32
```

**命令解释**:
*   `g++ ...`: 调用编译器，编译 `main.cpp` 和 `buzzer_sim.cpp`。
*   `-DPLATFORM_PC`: 定义宏以启用PC平台的代码。
*   `-o buzzer_simulator.exe`: 指定输出的可执行文件名为 `buzzer_simulator.exe`。
*   `-lkernel32 -lwinmm -lole32`: 链接 `miniaudio` 在Windows上所需的系统库。

如果手动执行时遇到库冲突（例如关于 `__clock_gettime64` 的错误），这通常意味着你的 `PATH` 环境变量中存在其他程序的干扰。此时，更建议你使用VS Code的F5一键运行方式，因为它能自动处理这种环境隔离问题。

## ESP32端说明

ESP32端的编译和部署方式保持不变，请参考你所使用的ESP-IDF版本的标准流程，并确保在 `CMakeLists.txt` 中定义了 `PLATFORM_ESP32` 宏。