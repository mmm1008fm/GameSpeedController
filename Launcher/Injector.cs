using System;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Launcher
{
    public class Injector
    {
        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll")]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll")]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        private const uint MEM_COMMIT = 0x1000;
        private const uint MEM_RESERVE = 0x2000;
        private const uint PAGE_READWRITE = 0x04;

        public bool InjectDLL(int processId, string dllPath, bool useManualMapping)
        {
            try
            {
                var process = Process.GetProcessById(processId);
                if (process == null)
                    return false;

                if (useManualMapping)
                {
                    return ManualMap(process, dllPath);
                }
                else
                {
                    return LoadLibraryInject(process, dllPath);
                }
            }
            catch
            {
                return false;
            }
        }

        private bool LoadLibraryInject(Process process, string dllPath)
        {
            var loadLibraryAddr = GetProcAddress(LoadLibrary("kernel32.dll"), "LoadLibraryA");
            if (loadLibraryAddr == IntPtr.Zero)
                return false;

            var dllPathBytes = System.Text.Encoding.ASCII.GetBytes(dllPath);
            var dllPathAddr = VirtualAllocEx(process.Handle, IntPtr.Zero, (uint)dllPathBytes.Length + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (dllPathAddr == IntPtr.Zero)
                return false;

            if (!WriteProcessMemory(process.Handle, dllPathAddr, dllPathBytes, (uint)dllPathBytes.Length, out _))
                return false;

            var threadHandle = CreateRemoteThread(process.Handle, IntPtr.Zero, 0, loadLibraryAddr, dllPathAddr, 0, IntPtr.Zero);
            return threadHandle != IntPtr.Zero;
        }

        private bool ManualMap(Process process, string dllPath)
        {
            // TODO: Implement manual mapping
            throw new NotImplementedException("Manual mapping not implemented yet");
        }
    }
} 