using System;
using System.Runtime.InteropServices;

namespace Launcher
{
    public static class Injector
    {
        private const uint PROCESS_CREATE_THREAD = 0x0002;
        private const uint PROCESS_QUERY_INFORMATION = 0x0400;
        private const uint PROCESS_VM_OPERATION = 0x0008;
        private const uint PROCESS_VM_WRITE = 0x0020;
        private const uint PROCESS_VM_READ = 0x0010;

        private const uint MEM_COMMIT = 0x1000;
        private const uint MEM_RESERVE = 0x2000;
        private const uint MEM_RELEASE = 0x8000;
        private const uint PAGE_READWRITE = 0x04;

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out IntPtr lpThreadId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool VirtualFreeEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint dwFreeType);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        public static bool InjectClassic(int pid, string dllPath)
        {
            IntPtr process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, pid);
            if (process == IntPtr.Zero)
                return false;

            byte[] dllBytes = System.Text.Encoding.ASCII.GetBytes(dllPath + '\0');
            IntPtr remoteMem = VirtualAllocEx(process, IntPtr.Zero, (uint)dllBytes.Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (remoteMem == IntPtr.Zero)
                return false;

            if (!WriteProcessMemory(process, remoteMem, dllBytes, (uint)dllBytes.Length, out _))
                return false;

            IntPtr kernel32 = GetModuleHandle("kernel32.dll");
            IntPtr loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");
            if (loadLibrary == IntPtr.Zero)
                return false;

            IntPtr thread = CreateRemoteThread(process, IntPtr.Zero, 0, loadLibrary, remoteMem, 0, out _);
            bool result = thread != IntPtr.Zero;
            if (thread != IntPtr.Zero)
                CloseHandle(thread);

            VirtualFreeEx(process, remoteMem, 0, MEM_RELEASE);
            CloseHandle(process);
            return result;
        }

        private static bool InjectManual(int pid, string dllPath)
        {
            // Call into the bridge DLL which performs manual mapping. The
            // bridge may not be present or might fail, so wrap the call in a
            // try/catch and surface success as a boolean.
            try
            {
                return ManualMapBridge.ManualMapper.Inject(pid, dllPath);
            }
            catch (DllNotFoundException)
            {
                return false;
            }
            catch (BadImageFormatException)
            {
                return false;
            }
        }

        public static bool InjectDLL(int pid, string dllPath)
        {
            // Manual mapping is not supported; fall back to classic injection.
            return InjectClassic(pid, dllPath);
        }

        public static bool InjectDLL(int pid, string dllPath, bool manual)
        {
            return manual ? InjectManual(pid, dllPath) : InjectClassic(pid, dllPath);
        }
    }
}
