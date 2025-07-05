# GameSpeedController

GameSpeedController is a small experiment for adjusting the speed of Windows applications. It contains a C++ DLL built with CMake and a WPF launcher that injects the DLL into a chosen process. The DLL hooks `Sleep` and `QueryPerformanceCounter` via MinHook and applies a global time multiplier.

## Prerequisites

- **Visual Studio 2022** with the C++ and .NET desktop workloads
- **.NET 7 SDK** (required for the WPF launcher)
- **CMake 3.10** or newer
- **MinHook** library bundled in `HookDLL/MinHook`

After cloning the repository, initialize the submodule:

```powershell
git submodule update --init
```

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

### Manual mapping

The launcher can attempt to load the DLL without calling `LoadLibrary` in the
target process. This manual mapping technique is useful when the normal
injection method fails due to security software or when `LoadLibrary` is
restricted. The repository now includes a native implementation, so manual
mapping is performed whenever possible and the launcher falls back to the
classic method only on failure.

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

## Limitations and troubleshooting

- The injector may require administrator privileges to open the target process.
  Security software can also block DLL injection. If injection fails, check the
  debug output for a Win32 error code.
- The time multiplier is clamped to a maximum of `10.0` to avoid extreme timer
  values that could destabilize applications.
- This repository does not yet include a fully configured test environment. A
  small MSTest project is provided as a starting point, but building it requires
  the .NET SDK.
