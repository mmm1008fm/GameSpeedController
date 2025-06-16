#include "ManualMapBridge.h"
#include "ManualMapNative.h"
#include <msclr/marshal.h>

using namespace System::Runtime::InteropServices;

namespace ManualMapBridge
{
    bool ManualMapper::Inject(int pid, System::String^ dllPath)
    {
        IntPtr ptr = Marshal::StringToHGlobalUni(dllPath);
        bool result = ManualMapNative(pid, static_cast<const wchar_t*>(ptr.ToPointer()));
        Marshal::FreeHGlobal(ptr);
        return result;
    }
}
