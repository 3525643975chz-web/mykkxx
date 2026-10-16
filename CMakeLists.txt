cmake_minimum_required(VERSION 3.10)
project(zygisk)

# 允许未定义符号，解决 JNI_GetCreatedJavaVMs 报错
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--unresolved-symbols=ignore-all")

add_library(zygisk SHARED main.cpp)

# 千万注意：不要写 -static-libstdc++ 和 -static-libgcc
target_link_libraries(zygisk PRIVATE log)
