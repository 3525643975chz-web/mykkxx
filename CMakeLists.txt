cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

# 把标志放在这里，链接器绝对会乖乖听话
target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log
    -Wl,--allow-shlib-undefined
)
