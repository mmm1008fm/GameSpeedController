#include <Windows.h>
#include <MinHook.h>
#include "HookFunctions.h"
#include "TimeController.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            if (MH_Initialize() != MH_OK)
                return FALSE;
            
            if (!InitializeHooks())
                return FALSE;
            
            DisableThreadLibraryCalls(hModule);
            break;

        case DLL_PROCESS_DETACH:
            MH_Uninitialize();
            break;
    }
    return TRUE;
} 