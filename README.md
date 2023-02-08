![preview-pattern](https://user-images.githubusercontent.com/7984500/217584108-c7e99530-d409-4a47-84aa-e807b820e665.png)

# Kaleidoscope

A **kaleidoscope** style screen filter!

## Introduction

This program uses an equilateral triangle box to pick patterns, and fill the entire windows desktop with the generated graphics. By moving the selection box around, we can get event more strange images like the picture below:

![preview-code](https://user-images.githubusercontent.com/7984500/217586805-81230b0d-ff5b-4b64-9bf9-ec3629b37c4d.png)

### Requirements

- Operating system: Windows (version >= 10) with DirectX 12

### Usage

Open the `kaleidoscope.exe`, program will automatically fill the screen with a small triangle pattern in the middle of your desktop. Then:

- Drag with the mouse to move the selection box.
- Rotate your mouse wheel to resize the selection box.
- Press ESC or use the right-click menu to exit.

https://user-images.githubusercontent.com/7984500/217632201-b8e297cf-3540-4726-b808-772ee35adf3d.mp4

Notes:

- If you want to take a screenshot, please **DISABLE** `Exclude from capture` in the right-click menu.
- By enabling `Exclude from capture`, graphics will be smoother. But it will NOT be captured by other screenshot program any more.

## Build

Requirements:

- Visual Studio (version >= 2022) with the C++ desktop development tools.

Compile:

1. Clone this project.
2. Open the root folder with Visual Studio, or just configure it with CMake.
3. Wait until CMake configuration finished (during this step, a third-party library will be cloned).
4. Compile it with Visual Studio or CMake (still use msbuild).
5. The compiled program may be output to `out\build\x64-Release\app\kaleidoscope` or `build\app\kaleidoscope\MinSizeRel`. (depends on your CMake configuration)

## Miscellaneous

It may be more appropriate to use DirectX 11, as [Desktop Duplication API](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api) doesn't support DirectX 12 (current implementation has one unnecessary copy operation).
