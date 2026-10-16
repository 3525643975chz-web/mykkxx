cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

# 补上了 nativehelper 库，专门用于解决 JNI_GetCreatedJavaVMs 找不到的问题
target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log 
    nativehelper
)
