cmake_minimum_required(VERSION 3.22)
project(meovra CXX)

add_library(meovra SHARED meovra.cpp)
target_compile_features(meovra PRIVATE cxx_std_17)
target_compile_options(meovra PRIVATE -fvisibility=hidden -fno-rtti -Oz)
target_link_options(meovra PRIVATE -Wl,--gc-sections -s)
