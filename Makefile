# 编译器和标志
CXX = g++
CXXFLAGS = -g -std=c++11 -DPLATFORM_PC
LDFLAGS = -lkernel32 -lwinmm -lole32

# 源文件
SRCS = main.cpp esp32_tone_api.cpp esp32-hal-ledc-sim.cpp

# 构建目录和目标文件
BUILD_DIR = build
TARGET = buzzer_simulator.exe
TARGET_PATH = $(BUILD_DIR)/$(TARGET)

# 自动生成对象文件列表 (e.g., build/main.o)
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# .PHONY 定义伪目标，这些目标不代表真实文件
.PHONY: all clean

# 默认目标：构建所有内容
all: $(TARGET_PATH)

# 链接规则：从所有对象文件创建最终的可执行文件
$(TARGET_PATH): $(OBJS)
	@echo Linking target: $@
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# 编译规则：将每个 .cpp 文件编译成 build/ 目录下的 .o 对象文件
# | $(BUILD_DIR) 表示 $(BUILD_DIR) 是此规则的“order-only”依赖，
# 确保在编译任何对象文件之前，构建目录一定存在。
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo Compiling $<...
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 规则：创建构建目录
$(BUILD_DIR):
	@if not exist $(subst /,\,$(BUILD_DIR)) mkdir $(subst /,\,$(BUILD_DIR))

# 清理规则：删除整个 build 目录
clean:
	@echo Cleaning build directory...
	@if exist $(subst /,\,$(BUILD_DIR)) rmdir /S /Q $(subst /,\,$(BUILD_DIR))