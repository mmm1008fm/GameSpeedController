# Ensure MinHook is available
$minHookPath = "HookDLL/MinHook/MinHook.h"
if (-not (Test-Path $minHookPath)) {
    Write-Host "MinHook not found. Cloning from GitHub..."
    git clone https://github.com/TsudaKageyu/minhook.git "HookDLL/MinHook"
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
