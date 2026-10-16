cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

# 核心：强制链接器忽略所有未定义符号（最稳妥的写法）
set_target_properties(zygisk PROPERTIES LINK_FLAGS "-Wl,--unresolved-symbols=ignore-all")

target_link_libraries(zygisk PRIVATE log)
