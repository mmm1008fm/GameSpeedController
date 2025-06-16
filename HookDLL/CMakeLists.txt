cmake_minimum_required(VERSION 3.10)
project(HookDLL LANGUAGES CXX)

# Use C++17 for the DLL
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include MinHook library
add_subdirectory(MinHook)

# Source files for the hook DLL
set(SOURCES
    src/HookMain.cpp
    src/IPC.cpp
    src/TimeController.cpp
)

# Build a single shared library
add_library(HookDLL SHARED ${SOURCES})
target_include_directories(HookDLL PRIVATE src)
target_link_libraries(HookDLL PRIVATE libMinHook)
