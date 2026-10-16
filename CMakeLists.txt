cmake_minimum_required(VERSION 3.10)
project(zygisk)

# 核心：强制全局忽略所有未定义符号，链接器再也不会因为找不到 JNI_GetCreatedJavaVMs 而报错
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--unresolved-symbols=ignore-all")

add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk PRIVATE log)
