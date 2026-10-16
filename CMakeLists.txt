cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

# 核心：告诉链接器允许未定义的符号，运行时系统会提供
target_link_options(zygisk PRIVATE "-Wl,--allow-shlib-undefined")

target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log
)
