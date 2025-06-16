# GameSpeedController

This repository contains a launcher and hook DLL used for manipulating game speed.

## Manual Mapping

The original UI included a toggle for manual DLL mapping. This feature is not
implemented and is currently unsupported. The launcher now always uses the
standard `LoadLibrary` injection method.
