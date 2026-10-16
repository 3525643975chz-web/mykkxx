cmake_minimum_required(VERSION 3.10)
project(zygisk)

# 强制忽略未定义符号
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--unresolved-symbols=ignore-all")

add_library(zygisk SHARED main.cpp)

# 千万注意：这里绝对不要写 -static-libstdc++ 和 -static-libgcc ！
target_link_libraries(zygisk PRIVATE log)
