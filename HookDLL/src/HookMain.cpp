#include <Windows.h>
#include <MinHook.h>
#include "TimeController.h"
#include "HookFunctions.h"
#include "IPC.h"
#include <atomic>

static HANDLE g_ipcThread = nullptr;
static std::atomic<bool> g_ipcRunning{false};
static IPCClient g_ipcClient;

DWORD WINAPI IPCThreadProc(LPVOID)
{
    const std::wstring pipeName = L"\\\\.\\pipe\\GameSpeedController";

    while (g_ipcRunning)
    {
        if (!g_ipcClient.IsConnected())
        {
            if (!g_ipcClient.Connect(pipeName))
            {
                Sleep(1000);
                continue;
            }
        }

        std::string message;
        if (g_ipcClient.ReadMessage(message))
        {
            ParseIPCCommand(message);
        }
        else
        {
            g_ipcClient.Disconnect();
            Sleep(1000);
        }
    }

    g_ipcClient.Disconnect();
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MH_Initialize() != MH_OK)
            return FALSE;

        if (!InitializeHooks())
            return FALSE;

        g_ipcRunning = true;
        g_ipcThread = CreateThread(nullptr, 0, IPCThreadProc, nullptr, 0, nullptr);

        DisableThreadLibraryCalls(hModule);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        g_ipcRunning = false;
        if (g_ipcThread)
        {
            WaitForSingleObject(g_ipcThread, INFINITE);
            CloseHandle(g_ipcThread);
            g_ipcThread = nullptr;
        }

        g_ipcClient.Disconnect();

        CleanupHooks();
        MH_Uninitialize();
    }

    return TRUE;
}
