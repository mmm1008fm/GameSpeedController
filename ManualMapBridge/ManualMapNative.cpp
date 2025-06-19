#include "ManualMapNative.h"
#include <windows.h>

namespace
{
    struct LoaderData
    {
        LPVOID imageBase;
        PIMAGE_NT_HEADERS ntHeaders;
        PIMAGE_BASE_RELOCATION baseReloc;
        PIMAGE_IMPORT_DESCRIPTOR importDir;
        FARPROC loadLibraryA;
        FARPROC getProcAddress;
    };

    DWORD WINAPI LoaderStub(LPVOID param)
    {
        auto data = reinterpret_cast<LoaderData*>(param);
        BYTE* base = static_cast<BYTE*>(data->imageBase);

        // Apply relocations
        SIZE_T delta = reinterpret_cast<SIZE_T>(base) -
                       data->ntHeaders->OptionalHeader.ImageBase;
        if (delta && data->baseReloc)
        {
            auto reloc = data->baseReloc;
            while (reloc->VirtualAddress)
            {
                DWORD count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
                               sizeof(WORD);
                auto list = reinterpret_cast<WORD*>(reinterpret_cast<BYTE*>(reloc) +
                                                    sizeof(IMAGE_BASE_RELOCATION));
                for (DWORD i = 0; i < count; ++i)
                {
                    if ((list[i] >> 12) == IMAGE_REL_BASED_DIR64)
                    {
                        auto patch = reinterpret_cast<SIZE_T*>(base + reloc->VirtualAddress +
                                                             (list[i] & 0xfff));
                        *patch += delta;
                    }
                }
                reloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
                    reinterpret_cast<BYTE*>(reloc) + reloc->SizeOfBlock);
            }
        }

        // Resolve imports
        if (data->importDir)
        {
            auto importDesc = data->importDir;
            while (importDesc->Name)
            {
                const char* modName = reinterpret_cast<const char*>(base + importDesc->Name);
                HMODULE mod = reinterpret_cast<HMODULE(WINAPI*)(const char*)>(data->loadLibraryA)(modName);
                ULONG_PTR* thunkRef = reinterpret_cast<ULONG_PTR*>(base + importDesc->OriginalFirstThunk);
                ULONG_PTR* funcRef = reinterpret_cast<ULONG_PTR*>(base + importDesc->FirstThunk);
                if (!importDesc->OriginalFirstThunk)
                    thunkRef = funcRef;

                for (; *thunkRef; ++thunkRef, ++funcRef)
                {
                    if (*thunkRef & IMAGE_ORDINAL_FLAG64)
                    {
                        *funcRef = reinterpret_cast<ULONG_PTR>(
                            reinterpret_cast<FARPROC(WINAPI*)(HMODULE, DWORD)>(data->getProcAddress)(
                                mod, static_cast<DWORD>(*thunkRef & 0xffff)));
                    }
                    else
                    {
                        auto thunkData = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(base + *thunkRef);
                        *funcRef = reinterpret_cast<ULONG_PTR>(
                            reinterpret_cast<FARPROC(WINAPI*)(HMODULE, const char*)>(data->getProcAddress)(
                                mod, thunkData->Name));
                    }
                }
                ++importDesc;
            }
        }

        // Call entry point
        auto dllMain = reinterpret_cast<BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID)>(
            base + data->ntHeaders->OptionalHeader.AddressOfEntryPoint);
        if (dllMain)
            dllMain(reinterpret_cast<HINSTANCE>(base), DLL_PROCESS_ATTACH, nullptr);

        return 0;
    }

    // Marker used to calculate the stub size when copying to the remote process
    DWORD WINAPI LoaderStubEnd()
    {
        return 0;
    }
}

bool ManualMapNative(int pid, const wchar_t* path)
{
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY | SEC_IMAGE,
                                        0, 0, nullptr);
    if (!mapping)
    {
        CloseHandle(file);
        return false;
    }

    LPVOID localImage = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!localImage)
    {
        CloseHandle(mapping);
        CloseHandle(file);
        return false;
    }

    auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(localImage);
    auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(
        static_cast<BYTE*>(localImage) + dos->e_lfanew);

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process)
    {
        UnmapViewOfFile(localImage);
        CloseHandle(mapping);
        CloseHandle(file);
        return false;
    }

    LPVOID remoteImage = VirtualAllocEx(process, nullptr,
                                        nt->OptionalHeader.SizeOfImage,
                                        MEM_COMMIT | MEM_RESERVE,
                                        PAGE_EXECUTE_READWRITE);
    if (!remoteImage)
    {
        CloseHandle(process);
        UnmapViewOfFile(localImage);
        CloseHandle(mapping);
        CloseHandle(file);
        return false;
    }

    // Copy headers
    SIZE_T bytes = 0;
    WriteProcessMemory(process, remoteImage, localImage,
                       nt->OptionalHeader.SizeOfHeaders, &bytes);

    // Copy sections
    auto section = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i)
    {
        if (section[i].SizeOfRawData == 0)
            continue;
        LPVOID dest = static_cast<BYTE*>(remoteImage) + section[i].VirtualAddress;
        LPVOID src = static_cast<BYTE*>(localImage) + section[i].PointerToRawData;
        WriteProcessMemory(process, dest, src, section[i].SizeOfRawData, &bytes);
    }

    LoaderData data{};
    data.imageBase = remoteImage;
    data.ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        static_cast<BYTE*>(remoteImage) + dos->e_lfanew);
    data.baseReloc = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
        static_cast<BYTE*>(remoteImage) +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    data.importDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        static_cast<BYTE*>(remoteImage) +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    data.loadLibraryA = GetProcAddress(kernel32, "LoadLibraryA");
    data.getProcAddress = GetProcAddress(kernel32, "GetProcAddress");

    SIZE_T stubSize = 0x1000; // allocate a page for stub and data
    LPVOID remoteStub = VirtualAllocEx(process, nullptr, stubSize,
                                       MEM_COMMIT | MEM_RESERVE,
                                       PAGE_EXECUTE_READWRITE);
    if (!remoteStub)
    {
        VirtualFreeEx(process, remoteImage, 0, MEM_RELEASE);
        CloseHandle(process);
        UnmapViewOfFile(localImage);
        CloseHandle(mapping);
        CloseHandle(file);
        return false;
    }

    // Write loader data
    WriteProcessMemory(process, remoteStub, &data, sizeof(data), &bytes);
    SIZE_T offset = sizeof(data);

    // Copy the loader stub to the remote process
    SIZE_T stubSize = reinterpret_cast<SIZE_T>(&LoaderStubEnd) -
                      reinterpret_cast<SIZE_T>(&LoaderStub);
    WriteProcessMemory(process, static_cast<BYTE*>(remoteStub) + offset,
                       reinterpret_cast<LPCVOID>(&LoaderStub), stubSize, &bytes);

    // Calculate remote stub address
    LPTHREAD_START_ROUTINE func = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        static_cast<BYTE*>(remoteStub) + offset);

    HANDLE thread = CreateRemoteThread(process, nullptr, 0, func,
                                       remoteStub, 0, nullptr);
    bool result = thread != nullptr;
    if (thread)
        CloseHandle(thread);

    CloseHandle(process);
    UnmapViewOfFile(localImage);
    CloseHandle(mapping);
    CloseHandle(file);
    return result;
}
