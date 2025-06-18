#include "MinHook.h"

MH_STATUS WINAPI MH_Initialize(void) {
    return MH_OK;
}

MH_STATUS WINAPI MH_Uninitialize(void) {
    return MH_OK;
}

MH_STATUS WINAPI MH_CreateHook(LPVOID, LPVOID, LPVOID*) {
    return MH_OK;
}

MH_STATUS WINAPI MH_EnableHook(LPVOID) {
    return MH_OK;
}

MH_STATUS WINAPI MH_DisableHook(LPVOID) {
    return MH_OK;
}
