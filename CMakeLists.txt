# CMake最低版本要求
cmake_minimum_required(VERSION 3.15)

# 添加版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# 获取当前文件夹名
STRING(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})

# 项目名称
project(${CURRENT_FOLDER})

# 添加子项目
add_subdirectory(server)
add_subdirectory(client)
