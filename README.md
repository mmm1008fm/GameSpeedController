# GameSpeedController

GameSpeedController is a small experiment for adjusting the speed of Windows applications. It contains a C++ DLL built with CMake and a WPF launcher that injects the DLL into a chosen process. The DLL hooks `Sleep` and `QueryPerformanceCounter` via MinHook and applies a global time multiplier.

## Prerequisites

- **Visual Studio 2022** with the C++ and .NET desktop workloads
- **.NET 7 SDK** (required for the WPF launcher)
- **CMake 3.10** or newer
- **MinHook** sources placed in `HookDLL/MinHook`

## Building

### Using build scripts

At the repository root run:

```powershell
./build.ps1
```

This compiles the 64‑bit DLL and copies it to `Launcher/Plugins/HookDLL/x64`, then restores and builds the launcher.

For building both 32‑bit and 64‑bit versions you can run `Launcher/build.ps1`:

```powershell
cd Launcher
./build.ps1
```

### Manual build

1. Configure and build the DLL:

```powershell
cmake -B build/x64 -S HookDLL -A x64
cmake --build build/x64 --config Release
```

2. Copy `build/x64/Release/HookDLL_x64.dll` to `Launcher/Plugins/HookDLL/x64`.
3. Build the launcher:

```powershell
dotnet restore Launcher
cd Launcher
dotnet build -c Release
```

## Using the launcher

Run the built `Launcher.exe`. Select a process with a window and press **Inject** to load the DLL (optionally with manual mapping). Adjust the **Time Multiplier** slider or use the hotkeys below to change the game speed.

### Hotkeys

The launcher registers global hotkeys:

- **Ctrl + F9** – increase the multiplier
- **Ctrl + F10** – decrease the multiplier
- **Ctrl + F11** – toggle pause/resume

### IPC commands

The DLL includes an IPC client that can parse commands received through a named pipe. Supported commands are:

- `SET <value>` – set a new multiplier
- `RESET` – restore the multiplier to `1.0`

A simple `PipeServer` implementation is provided in the launcher for sending these commands.
