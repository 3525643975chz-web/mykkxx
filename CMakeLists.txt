cmake_minimum_required(VERSION 3.10)
project(zygisk)

# 允许链接器忽略系统未定义的符号（解决 JNI_GetCreatedJavaVMs 报错）
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

add_library(zygisk SHARED main.cpp)

# 注意！这里绝对不要写 -static-libstdc++ 和 -static-libgcc
target_link_libraries(zygisk PRIVATE log)
