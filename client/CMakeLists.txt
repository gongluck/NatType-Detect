# CMake最低版本要求
cmake_minimum_required(VERSION 3.0)

# 添加版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# 获取当前文件夹名
STRING(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})

# 项目名称
project(${CURRENT_FOLDER})

# 使用C++11
set(CMAKE_CXX_STANDARD 11)

# 目标平台判断
if(WIN32)
# 分别设置了Debug版本和Release版本可执行文件的输出目录(经测试,windows环境有效)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/generated/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/generated/bin)
elseif(UNIX)
# 设置可执行文件的输出目录(经测试,linux环境有效)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/generated/bin)
endif()

#头文件包含目录
include_directories(../common)

# 自动添加当前源码目录和生成目录到包含目录
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 查找当前目录下所有源文件并保存到变量
aux_source_directory(. SRCS)

# 指定生成可执行文件
add_executable(${PROJECT_NAME} ${SRCS})

# 添加链接库
if(WIN32)
target_link_libraries(${PROJECT_NAME} "WSOCK32")
elseif(UNIX)
target_link_libraries(${PROJECT_NAME} "pthread")
endif()

# 设置调试版本文件后缀名
set_property(TARGET ${PROJECT_NAME} PROPERTY DEBUG_POSTFIX _d)
