#pragma once
#include <cstdint>
#include <memory>

// forward declaration
struct HWND__;
using HWND = HWND__ *;

// mirror
class mirror
{
public:
    struct aligned_regular_triangle
    {
        float top_x;
        float top_y;
        float length;
    };

public:
    ~mirror();
    mirror(HWND window, std::uint32_t width, std::uint32_t height);

public:
    auto on_resize(std::uint32_t width, std::uint32_t height) -> void;
    auto on_render() -> void;
    auto on_update(aligned_regular_triangle const & triangle) -> void;

private:
    struct core;
    std::unique_ptr<core> o;
};
