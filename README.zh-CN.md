![preview-pattern](https://user-images.githubusercontent.com/7984500/217584108-c7e99530-d409-4a47-84aa-e807b820e665.png)

# Kaleidoscope

[English](README.md) | [中文](README.zh-CN.md)

一款 **万花筒** 风格的屏幕滤镜，适用于 Windows 10/11。

程序通过一个等边三角形选区捕捉桌面画面，并将万花筒风格的反射图形铺满整个屏幕。

## 使用

- **拖拽** 三角形以移动选区。
- **滚动** 鼠标滚轮以调整选区大小。
- **右键** 打开上下文菜单。
- **按 ESC** 或通过上下文菜单退出。

> **提示：** 启用 `Exclude from capture` 可获得更流畅的渲染效果。启用后窗口将不会出现在截图中——如需截取万花筒画面，请先关闭该选项。

移动选区效果：

<https://user-images.githubusercontent.com/7984500/217632201-b8e297cf-3540-4726-b808-772ee35adf3d.mp4>

## 构建

### 环境要求

- Windows 10 及以上，支持 DirectX 12
- Visual Studio 2022 及以上，安装 C++ 桌面开发工作负载

### 步骤

1. 克隆项目。
2. 使用 Visual Studio 打开根目录。
3. 等待 CMake 配置完成。
4. 在 CMake 设置中添加 `x64-Release` 并选中（可选）。
5. 构建项目。
6. 输出位于 `out\build\x64-Release\app\kaleidoscope`（取决于 CMake 配置）。

## 备注

使用 DirectX 11 可能更为合适，因为 [Desktop Duplication API](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api) 原生不支持 DirectX 12（当前实现需要一次额外的拷贝）。
