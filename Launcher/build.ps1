# Создаем директории для сборки
New-Item -ItemType Directory -Force -Path "build"
New-Item -ItemType Directory -Force -Path "build/x86"
New-Item -ItemType Directory -Force -Path "build/x64"

# Собираем x86 версию
cmake -B build/x86 -S HookDLL -A Win32
cmake --build build/x86 --config Release

# Собираем x64 версию
cmake -B build/x64 -S HookDLL -A x64
cmake --build build/x64 --config Release

# Копируем DLL в папку Plugins
New-Item -ItemType Directory -Force -Path "Plugins/HookDLL/x86"
New-Item -ItemType Directory -Force -Path "Plugins/HookDLL/x64"

Copy-Item "build/x86/Release/HookDLL_x86.dll" -Destination "Plugins/HookDLL/x86/"
Copy-Item "build/x64/Release/HookDLL_x64.dll" -Destination "Plugins/HookDLL/x64/"

# Собираем WPF приложение
dotnet build Launcher/Launcher.csproj -c Release 