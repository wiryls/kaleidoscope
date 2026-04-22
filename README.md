![preview-pattern](https://user-images.githubusercontent.com/7984500/217584108-c7e99530-d409-4a47-84aa-e807b820e665.png)

# Kaleidoscope

[English](README.md) | [中文](README.zh-CN.md)

A **kaleidoscope** style screen filter for Windows 10/11.

The program captures the desktop using an equilateral triangle selection box and fills the entire screen with kaleidoscope-style reflected graphics.

## Usage

- **Drag** the triangle to move the selection box.
- **Scroll** the mouse wheel to resize the selection box.
- **Right-click** to open the context menu.
- **Press ESC** or use the context menu to exit.

> **Note:** Enable `Exclude from capture` for smoother rendering. When enabled, the window will not appear in screenshots — disable it first if you need to capture the kaleidoscope.

Moving the selection box around:

<https://user-images.githubusercontent.com/7984500/217632201-b8e297cf-3540-4726-b808-772ee35adf3d.mp4>

## Build

### Requirements

- Windows 10+ with DirectX 12
- Visual Studio 2022+ with C++ desktop development workload

### Steps

1. Clone the project.
2. Open the root folder with Visual Studio.
3. Wait for CMake configuration to finish.
4. Add `x64-Release` to CMake Settings and select it (optional).
5. Build the project.
6. The output is in `out\build\x64-Release\app\kaleidoscope` (depends on your CMake configuration).

## Notes

Using DirectX 11 may be more appropriate, as the [Desktop Duplication API](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api) doesn't support DirectX 12 natively (the current implementation requires an extra copy).
