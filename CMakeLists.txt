cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

# 核心：强制忽略所有未定义符号，链接器再也不报错
target_link_options(zygisk PRIVATE "-Wl,--unresolved-symbols=ignore-all")

target_link_libraries(zygisk PRIVATE log)
