#include "HookFunctions.h"
#include "TimeController.h"
#include <MinHook.h>

// Оригинальные функции
VOID (WINAPI *TrueSleep)(DWORD dwMilliseconds) = nullptr;
BOOL (WINAPI *TrueQueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount) = nullptr;

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

    // Хук QueryPerformanceCounter
    if (MH_CreateHook(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryPerformanceCounter"),
        &HookedQueryPerformanceCounter,
        reinterpret_cast<LPVOID*>(&TrueQueryPerformanceCounter)) != MH_OK)
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