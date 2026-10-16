cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log
)

# 这行是为了让 NDK 链接器别那么死板，允许在运行时从系统找 JNI 函数
target_link_options(zygisk PRIVATE "-Wl,--allow-shlib-undefined")
