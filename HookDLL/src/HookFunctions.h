#pragma once
#include <Windows.h>

bool InitializeHooks();
void CleanupHooks();

// Оригинальные функции
extern VOID (WINAPI *TrueSleep)(DWORD dwMilliseconds);
extern BOOL (WINAPI *TrueQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount);
extern DWORD (WINAPI *TrueGetTickCount)();
extern ULONGLONG (WINAPI *TrueGetTickCount64)();

// Хуки
VOID WINAPI HookedSleep(DWORD dwMilliseconds);
BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
DWORD WINAPI HookedGetTickCount();
ULONGLONG WINAPI HookedGetTickCount64();
