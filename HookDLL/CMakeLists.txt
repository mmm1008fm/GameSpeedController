cmake_minimum_required(VERSION 3.10)
project(HookDLL)

# Добавляем MinHook как подмодуль
add_subdirectory(MinHook)

# Настройка для x86 и x64 сборок
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Общие исходные файлы
set(SOURCES
    src/HookMain.cpp
    src/HookFunctions.cpp
    src/IPC.cpp
    src/TimeController.cpp
)

# Создаем x86 версию
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:IA32")
add_library(HookDLL_x86 SHARED ${SOURCES})
target_include_directories(HookDLL_x86 PRIVATE src)
target_link_libraries(HookDLL_x86 PRIVATE libMinHook.x86)

# Создаем x64 версию
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
add_library(HookDLL_x64 SHARED ${SOURCES})
target_include_directories(HookDLL_x64 PRIVATE src)
target_link_libraries(HookDLL_x64 PRIVATE libMinHook.x64) 