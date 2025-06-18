#include "ManualMapNative.h"
#include <windows.h>

bool ManualMapNative(int pid, const wchar_t* path)
{
    // Manual mapping is not implemented. Returning false informs the managed
    // launcher that this feature is unavailable so it can fall back to the
    // classic LoadLibrary injection method.
    (void)pid;  // Unused
    (void)path; // Unused
    return false;
}
