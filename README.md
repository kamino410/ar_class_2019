# ar_class_2019

## Required

* Visual Stuio 2019 (older versions might be okey)
* OpenCV

## Setup project

1. Open `ar_class_2019.sln` with Visual Studio 2019.
2. Solution Explorer -> Right click `ar_class_2019` -> Manage NuGet Packages -> Restore
3. Solution Explorer -> Right click `ar_class_2019` -> Properties
    * C/C++ -> All Options -> Additional Include Directories -> Edit -> Set your OpenCV include directory
    * Linker -> All Options -> Additional Dependencies -> Edit -> Set your OpenCV library directory
    * Debugging -> Working Directory -> Edit to `$(TargetDir)`
4. You might have to copy `opencv_worldXXX.dll` in your OpenCV binary directory into `ar_class_2019/x64/Release` (where `main.exe` exists)
