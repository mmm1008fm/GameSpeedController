cmake_minimum_required(VERSION 3.10)
project(MinHook LANGUAGES C)

add_library(libMinHook STATIC MinHook.cpp)

target_include_directories(libMinHook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
