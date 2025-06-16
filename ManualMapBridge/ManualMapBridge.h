#pragma once

using namespace System;

namespace ManualMapBridge
{
    public ref class ManualMapper
    {
    public:
        static bool Inject(int pid, String^ dllPath);
    };
}
