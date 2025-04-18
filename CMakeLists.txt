cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(WebServer)

# 设置头文件路径
include_directories(
    src/tcp
    src/http
)

# 设置编译器和编译选项
set(CMAKE_CXX_COMPILER "/usr/bin/c++")  # 显式设置 C++ 编译器路径
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

# jsoncpp
include(FetchContent)
set(JSONCPP_WITH_TEST OFF)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
set(JSONCPP_WITH_WARNING_AS_ERROR OFF)
set(JSONCPP_WITH_EXAMPLE OFF)
set(JSONCPP_STATIC_WINDOWS_RUNTIME  OFF)
set(BUILD_SHARED_LIBS  OFF)
set(BUILD_STATIC_LIBS  ON)
set(BUILD_OBJECT_LIBS  ON)
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://gitee.com/AnPilgrim/jsoncpp.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(jsoncpp)
if(NOT TARGET jsoncpp_static)
  message(FATAL_ERROR "Target jsoncpp_static not available.")
endif()
if(NOT TARGET jsoncpp_object)
  message(FATAL_ERROR "Target jsoncpp_object not available.")
endif()

# 查找线程库
find_package(Threads REQUIRED)
# 查找OpenCV（仅包含必要模块）
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

# 添加源文件
aux_source_directory(src/tcp SRC_LIST1)
aux_source_directory(src/http SRC_LIST2)
aux_source_directory(src/retrieval SRC_LIST3)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/test)

# 添加可执行文件
add_executable(httpserver src/test/http_server.cpp ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3})

# 链接线程库
target_link_libraries(httpserver jsoncpp_object ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS})

# 添加头文件路径
target_include_directories(httpserver PRIVATE ${OpenCV_INCLUDE_DIRS})
