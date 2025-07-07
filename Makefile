# 编译器和标志
CXX = g++
CXXFLAGS = -g -std=c++11 -DPLATFORM_PC
LDFLAGS = -lkernel32 -lwinmm -lole32

# 源文件和目标文件
SRCS = main.cpp buzzer_sim.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = buzzer_simulator.exe

# 默认目标：构建可执行文件
all: $(TARGET)

# 链接规则：从对象文件创建可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 编译规则：从 .cpp 文件创建 .o 文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则：删除生成的文件
clean:
	del /Q $(TARGET) $(OBJS)

# 伪目标
.PHONY: all clean