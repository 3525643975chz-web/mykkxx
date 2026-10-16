cmake_minimum_required(VERSION 3.10)
project(zygisk)

add_library(zygisk SHARED main.cpp)

target_link_libraries(zygisk PRIVATE 
    -static-libstdc++ 
    -static-libgcc 
    log
)
