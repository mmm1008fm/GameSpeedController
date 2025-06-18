cmake_minimum_required(VERSION 3.10)
project(HookDLL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/MinHook/CMakeLists.txt)
    add_subdirectory(MinHook)
else()
    message(STATUS "Downloading MinHook...")
    FetchContent_Declare(
        MinHook
        GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(MinHook)
endif()

set(SOURCES
    src/HookMain.cpp
    src/HookFunctions.cpp
    src/IPC.cpp
    src/TimeController.cpp
)

add_library(HookDLL SHARED ${SOURCES})
target_include_directories(HookDLL PRIVATE src)
target_link_libraries(HookDLL PRIVATE libMinHook)
