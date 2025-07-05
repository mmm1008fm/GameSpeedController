#include "MinHook.h"
#include <windows.h>
#include <vector>
#include <cstring>

// Simple internal structure representing a hook
struct HookItem
{
    LPVOID  target;       // Address of the function to hook
    LPVOID  detour;       // Address of the replacement function
    LPVOID  trampoline;   // Pointer to the trampoline calling the original
    BYTE    original[16]; // Original bytes overwritten by the hook
    SIZE_T  patchSize;    // Size of the patch (depends on architecture)
    bool    enabled;
};

static std::vector<HookItem> g_hooks;

static HookItem* FindHook(LPVOID target)
{
    for (auto& h : g_hooks)
    {
        if (h.target == target)
            return &h;
    }
    return nullptr;
}

// Helper to write memory with the appropriate permissions
static void WriteMemory(LPVOID address, LPCVOID data, SIZE_T size)
{
    DWORD oldProt;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProt);
    std::memcpy(address, data, size);
    VirtualProtect(address, size, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), address, size);
}

MH_STATUS WINAPI MH_Initialize(void)
{
    return MH_OK;
}

MH_STATUS WINAPI MH_Uninitialize(void)
{
    for (auto& h : g_hooks)
    {
        if (h.enabled)
            WriteMemory(h.target, h.original, h.patchSize);
        if (h.trampoline)
            VirtualFree(h.trampoline, 0, MEM_RELEASE);
    }
    g_hooks.clear();
    return MH_OK;
}

MH_STATUS WINAPI MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal)
{
    if (!pTarget || !pDetour || !ppOriginal)
        return MH_ERROR_FUNCTION_NOT_FOUND;

    if (FindHook(pTarget))
        return MH_ERROR_ALREADY_CREATED;

#if defined(_M_X64) || defined(__x86_64__)
    const SIZE_T patchSize = 12; // mov rax, imm64; jmp rax
#else
    const SIZE_T patchSize = 5; // jmp rel32
#endif

    HookItem item{};
    item.target = pTarget;
    item.detour = pDetour;
    item.patchSize = patchSize;
    item.enabled = false;

    std::memcpy(item.original, pTarget, patchSize);

    SIZE_T trampSize = patchSize + patchSize;
    item.trampoline = VirtualAlloc(nullptr, trampSize, MEM_COMMIT | MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
    if (!item.trampoline)
        return MH_ERROR_MEMORY_ALLOC;

    // Copy original bytes to trampoline
    std::memcpy(item.trampoline, item.original, patchSize);

    // Jump from trampoline back to target + patchSize
    BYTE* p = static_cast<BYTE*>(item.trampoline) + patchSize;
#if defined(_M_X64) || defined(__x86_64__)
    p[0] = 0x48; p[1] = 0xB8;
    *reinterpret_cast<void**>(p + 2) = static_cast<BYTE*>(pTarget) + patchSize;
    p[10] = 0xFF; p[11] = 0xE0;
#else
    p[0] = 0xE9;
    *reinterpret_cast<int32_t*>(p + 1) =
        static_cast<int32_t>(static_cast<BYTE*>(pTarget) - (p + 5) + patchSize);
#endif

    *ppOriginal = item.trampoline;
    g_hooks.push_back(item);
    return MH_OK;
}

// Writes a jump from src to dst
static void WriteJump(void* src, void* dst, SIZE_T patchSize)
{
#if defined(_M_X64) || defined(__x86_64__)
    BYTE patch[12];
    patch[0] = 0x48; patch[1] = 0xB8;
    *reinterpret_cast<void**>(patch + 2) = dst;
    patch[10] = 0xFF; patch[11] = 0xE0;
#else
    BYTE patch[5];
    patch[0] = 0xE9;
    *reinterpret_cast<int32_t*>(patch + 1) =
        static_cast<int32_t>(static_cast<BYTE*>(dst) - (static_cast<BYTE*>(src) + 5));
#endif
    WriteMemory(src, patch, patchSize);
}

MH_STATUS WINAPI MH_EnableHook(LPVOID pTarget)
{
    if (pTarget == MH_ALL_HOOKS)
    {
        for (auto& h : g_hooks)
        {
            if (!h.enabled)
            {
                WriteJump(h.target, h.detour, h.patchSize);
                h.enabled = true;
            }
        }
        return MH_OK;
    }

    HookItem* h = FindHook(pTarget);
    if (!h)
        return MH_ERROR_NOT_CREATED;
    if (!h->enabled)
    {
        WriteJump(h->target, h->detour, h->patchSize);
        h->enabled = true;
    }
    return MH_OK;
}

MH_STATUS WINAPI MH_DisableHook(LPVOID pTarget)
{
    if (pTarget == MH_ALL_HOOKS)
    {
        for (auto& h : g_hooks)
        {
            if (h.enabled)
            {
                WriteMemory(h.target, h.original, h.patchSize);
                h.enabled = false;
            }
        }
        return MH_OK;
    }

    HookItem* h = FindHook(pTarget);
    if (!h)
        return MH_ERROR_NOT_CREATED;
    if (h->enabled)
    {
        WriteMemory(h->target, h->original, h->patchSize);
        h->enabled = false;
    }
    return MH_OK;
}
