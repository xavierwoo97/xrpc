# 设置cmake最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)

# 生成debug版本,可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")
add_compile_options(-DTHREADED)

# 设置输出文件目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 项目项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/usr/local/lib)

# src包含了mprpc框架所有的相关代码
add_subdirectory(src)

# example包含了mprpc框架使用的示例代码
add_subdirectory(example)