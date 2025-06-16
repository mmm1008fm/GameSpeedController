#include <Windows.h>
#include <MinHook.h>
#include "TimeController.h"
#include "HookFunctions.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MH_Initialize() != MH_OK)
            return FALSE;

        if (!InitializeHooks())
            return FALSE;

        DisableThreadLibraryCalls(hModule);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        CleanupHooks();
        MH_Uninitialize();
    }

    return TRUE;
}
