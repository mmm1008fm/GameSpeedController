# Ensure MinHook is available
$minHookPath = "HookDLL/MinHook/MinHook.h"
if (-not (Test-Path $minHookPath)) {
    Write-Error "MinHook submodule not found. Run 'git submodule update --init' first."
    exit 1
}

# Build HookDLL x64
cmake -B build/x64 -S HookDLL -A x64
cmake --build build/x64 --config Release

# Copy DLL to Launcher plugins
$pluginDir = "Launcher/Plugins/HookDLL/x64"
New-Item -ItemType Directory -Force -Path $pluginDir
Copy-Item "build/x64/Release/HookDLL_x64.dll" -Destination $pluginDir

# Restore and build the launcher
cd Launcher

(dotnet restore)

dotnet build -c Release
