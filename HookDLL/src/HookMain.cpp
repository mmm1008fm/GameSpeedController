#include <Windows.h>
#include <MinHook.h>
#include "TimeController.h"

// Original function pointers
using SleepFunc = VOID (WINAPI*)(DWORD);
using QpcFunc = BOOL (WINAPI*)(LARGE_INTEGER*);

static SleepFunc TrueSleep = nullptr;
static QpcFunc TrueQueryPerformanceCounter = nullptr;

// Hooked implementations adjust timing based on the multiplier
static VOID WINAPI HookedSleep(DWORD dwMilliseconds)
{
    if (TrueSleep)
    {
        DWORD scaled = static_cast<DWORD>(dwMilliseconds * GetTimeMultiplier());
        TrueSleep(scaled);
    }
}

static BOOL WINAPI HookedQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    BOOL result = TrueQueryPerformanceCounter(lpPerformanceCount);
    if (result)
    {
        lpPerformanceCount->QuadPart = static_cast<LONGLONG>(
            lpPerformanceCount->QuadPart * GetTimeMultiplier());
    }
    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MH_Initialize() != MH_OK)
            return FALSE;

        if (MH_CreateHookApi(L"kernel32", "Sleep", &HookedSleep,
                             reinterpret_cast<LPVOID*>(&TrueSleep)) != MH_OK)
            return FALSE;

        if (MH_CreateHookApi(L"kernel32", "QueryPerformanceCounter",
                             &HookedQueryPerformanceCounter,
                             reinterpret_cast<LPVOID*>(&TrueQueryPerformanceCounter)) != MH_OK)
            return FALSE;

        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
            return FALSE;

        DisableThreadLibraryCalls(hModule);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    return TRUE;
}
