#pragma once
#include <Windows.h>

bool InitializeHooks();
void CleanupHooks();

// Оригинальные функции
extern VOID (WINAPI *TrueSleep)(DWORD dwMilliseconds);
extern DWORD (WINAPI *TrueSleepEx)(DWORD dwMilliseconds, BOOL bAlertable);
extern BOOL (WINAPI *TrueQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount);
extern BOOL (WINAPI *TrueQueryPerformanceFrequency)(LARGE_INTEGER* lpFrequency);
extern DWORD (WINAPI *TrueGetTickCount)();
extern ULONGLONG (WINAPI *TrueGetTickCount64)();

// Хуки
VOID WINAPI HookedSleep(DWORD dwMilliseconds);
DWORD WINAPI HookedSleepEx(DWORD dwMilliseconds, BOOL bAlertable);
BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
BOOL WINAPI HookedQueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
DWORD WINAPI HookedGetTickCount();
ULONGLONG WINAPI HookedGetTickCount64();
