cmake_minimum_required(VERSION 3.10)
project(zygisk)

# 核心：在最全局的地方强制设置忽略未定义符号
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--unresolved-symbols=ignore-all")

add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk PRIVATE log)
