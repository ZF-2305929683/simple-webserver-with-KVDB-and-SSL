# 设置编译器
CC = g++

# 设置编译选项
CXXFLAGS = -std=c++20 -Wall -g

# 指定搜索头文件的目录
INCLUDES = -I./net_src/

# 指定源文件和目标文件
SERVER_SOURCES = server.cpp $(wildcard net_src/*.cpp)
SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
SERVER_EXECUTABLE = server_app

CLIENT_SOURCES = client.cpp $(wildcard net_src/*.cpp) # 添加 client.cpp 到源文件列表
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o) # 创建对应的 .o 文件目标
CLIENT_EXECUTABLE = client_app # 定义客户端可执行文件名

# 规则：构建 server 可执行文件
$(SERVER_EXECUTABLE): $(SERVER_OBJECTS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# 规则：构建 client 可执行文件
$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ $^
	

# 规则：编译.cpp文件到.o文件
%.o: %.cpp
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 清理规则
clean:
	rm -f $(SERVER_EXECUTABLE) $(SERVER_OBJECTS) $(CLIENT_EXECUTABLE) $(CLIENT_OBJECTS)

# 默认目标（运行make时如果没有指定目标，则执行这个）
.DEFAULT_GOAL := all
all: $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)