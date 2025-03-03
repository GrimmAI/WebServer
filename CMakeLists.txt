cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(WebServer)

# 设置头文件路径
include_directories(
    src/tcp
    src/http
)

# 设置编译器和编译选项
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# 查找线程库
find_package(Threads REQUIRED)

# 添加源文件
aux_source_directory(src/tcp SRC_LIST1)
aux_source_directory(src/http SRC_LIST2)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/test)

# 添加可执行文件
add_executable(httpserver src/http_server.cpp ${SRC_LIST1} ${SRC_LIST2})

# 链接线程库
target_link_libraries(httpserver ${CMAKE_THREAD_LIBS_INIT})