cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log
)

# 关键：告诉链接器允许未定义的符号（这些符号在系统运行时会自动找到）
target_link_options(zygisk PRIVATE "-Wl,--allow-shlib-undefined")
