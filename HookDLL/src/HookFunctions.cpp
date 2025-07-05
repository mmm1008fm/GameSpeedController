#include "HookFunctions.h"
#include "TimeController.h"
#include <MinHook.h>

// Оригинальные функции
VOID (WINAPI *TrueSleep)(DWORD dwMilliseconds) = nullptr;
DWORD (WINAPI *TrueSleepEx)(DWORD dwMilliseconds, BOOL bAlertable) = nullptr;
BOOL (WINAPI *TrueQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount) = nullptr;
BOOL (WINAPI *TrueQueryPerformanceFrequency)(LARGE_INTEGER* lpFrequency) = nullptr;
DWORD (WINAPI *TrueGetTickCount)() = nullptr;
ULONGLONG (WINAPI *TrueGetTickCount64)() = nullptr;

bool InitializeHooks()
{
    // Хук Sleep
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "Sleep"),
        &HookedSleep,
        reinterpret_cast<LPVOID*>(&TrueSleep)) != MH_OK)
    {
        return false;
    }

    // Хук SleepEx
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "SleepEx"),
        &HookedSleepEx,
        reinterpret_cast<LPVOID*>(&TrueSleepEx)) != MH_OK)
    {
        return false;
    }

    // Хук GetTickCount
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetTickCount"),
        &HookedGetTickCount,
        reinterpret_cast<LPVOID*>(&TrueGetTickCount)) != MH_OK)
    {
        return false;
    }

    // Хук GetTickCount64
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetTickCount64"),
        &HookedGetTickCount64,
        reinterpret_cast<LPVOID*>(&TrueGetTickCount64)) != MH_OK)
    {
        return false;
    }

    // Хук QueryPerformanceCounter
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryPerformanceCounter"),
        &HookedQueryPerformanceCounter,
        reinterpret_cast<LPVOID*>(&TrueQueryPerformanceCounter)) != MH_OK)
    {
        return false;
    }

    // Хук QueryPerformanceFrequency
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryPerformanceFrequency"),
        &HookedQueryPerformanceFrequency,
        reinterpret_cast<LPVOID*>(&TrueQueryPerformanceFrequency)) != MH_OK)
    {
        return false;
    }

    return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
}

void CleanupHooks()
{
    MH_DisableHook(MH_ALL_HOOKS);
}

VOID WINAPI HookedSleep(DWORD dwMilliseconds)
{
    TrueSleep(static_cast<DWORD>(dwMilliseconds * GetTimeMultiplier()));
}

DWORD WINAPI HookedSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
    return TrueSleepEx(static_cast<DWORD>(dwMilliseconds * GetTimeMultiplier()), bAlertable);
}

BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    BOOL result = TrueQueryPerformanceCounter(lpPerformanceCount);
    if (result)
    {
        lpPerformanceCount->QuadPart = static_cast<LONGLONG>(
            lpPerformanceCount->QuadPart * GetTimeMultiplier());
    }
    return result;
}

BOOL WINAPI HookedQueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
    return TrueQueryPerformanceFrequency(lpFrequency);
}

DWORD WINAPI HookedGetTickCount()
{
    return static_cast<DWORD>(TrueGetTickCount() * GetTimeMultiplier());
}

ULONGLONG WINAPI HookedGetTickCount64()
{
    return static_cast<ULONGLONG>(TrueGetTickCount64() * GetTimeMultiplier());
}
