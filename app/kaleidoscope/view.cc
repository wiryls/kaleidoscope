#include <cstdint>
#include <cmath>
#include <system_error>
#include <string>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "tool.h"
#include "viewmodel.h"
#include "render.h"

namespace must
{
    auto constexpr done = [](std::convertible_to<BOOL> auto && value)
    {
        if (static_cast<bool>(value))
            return std::forward<decltype(value)>(value);

        auto code = ::GetLastError();
        auto desc = std::system_category().message(code);
        ::OutputDebugString(desc.c_str());

        // Note: to use OutputDebugString in our cmake target,
        // we may need to add the following env to "launch.vs.json" file
        //
        // ```json
        // "env": {
        //     "DEBUG_LOGGING_LEVEL": "trace;info",
        //     "ENABLE_TRACING" : "true"
        // }
        // ```
        //
        // see: https://learn.microsoft.com/en-us/cpp/build/configure-cmake-debugging-sessions?view=msvc-170#launchvsjson-reference

        throw std::system_error(code, std::system_category());
    };

    auto constexpr non_null = []<typename T>(T * value)
    {
        done(value != nullptr);
        return value;
    };
}

namespace ext
{
    auto to_aligned_regular_triangle(viewmodel::state<LONG> const & state) -> mirror::aligned_regular_triangle
    {
        auto top = state.triangle_top();
        auto out = mirror::aligned_regular_triangle{};
        out.top_x = static_cast<float>(top[0]);
        out.top_y = static_cast<float>(top[1]);
        out.length = static_cast<float>(state.triangle_side_length());
        return out;
    }
}

namespace app
{
    struct data
    {
        viewmodel::state<LONG> state{};
        std::unique_ptr<mirror> render{ nullptr };
    };
}

auto main_window_message_handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> LRESULT
{
    // Preparation
    using namespace ::aux;
    using user_data_pointer = app::data *;

    // Life-time related message
    auto user_data = user_data_pointer{};
    switch (umsg)
    {
    case WM_CREATE:
    {
        auto param = reinterpret_cast<CREATESTRUCT *>(lparam) >> must::non_null;
        auto state = reinterpret_cast<user_data_pointer>(param->lpCreateParams) >> must::non_null;
        {
            // Init members
            state->render = std::make_unique<mirror>(hwnd);
        }
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return 0;
    }
    case WM_CLOSE:
    {
        ::DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        return 0;
    }}

    // Ignore if not ready
    if (user_data = reinterpret_cast<user_data_pointer>(::GetWindowLongPtr(hwnd, GWLP_USERDATA)); user_data == nullptr)
    {
        return ::DefWindowProc(hwnd, umsg, wparam, lparam);
    }

    // Event related message
    auto & state = user_data->state;
    auto & render = *user_data->render.get();
    switch (umsg)
    {
    case WM_MOUSEWHEEL:
    {
        auto & v = state.viewport_vertices();
        auto rect = RECT{v[2][0], v[0][1], v[1][0], v[1][1]};

        auto delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        state.on_length_changed(delta);

        // About mouse events
        // - https://learn.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations
        // - https://github.com/MicrosoftDocs/win32/blob/e82557891475f35c505f90f2aa0f76bebb4e190c/desktop-src/inputdev/about-mouse-input.md

        ::InvalidateRect(hwnd, &rect, true /* Send WM_ERASEBKGND */);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        auto x = GET_X_LPARAM(lparam);
        auto y = GET_Y_LPARAM(lparam);
        state.on_start_moving(x, y);

        ::SetCapture(hwnd); // Support moving outside our window
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        if (state.is_moving())
        {
            auto & v = state.viewport_vertices();
            auto rect = RECT{v[2][0], v[0][1], v[1][0], v[1][1]};

            auto x = GET_X_LPARAM(lparam);
            auto y = GET_Y_LPARAM(lparam);
            state.on_moving(x, y);

            ::InvalidateRect(hwnd, &rect, true);
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        state.on_stop_moving();

        ::ReleaseCapture();
        return 0;
    }
    case WM_KEYDOWN: // Handle keyboard input
    {
        switch (wparam)
        {
        case VK_ESCAPE:
        {
            ::DestroyWindow(hwnd);
            break;
        }}
        return 0;
    }
    case WM_PAINT: // Paint
    {
        // Prepare a triangle
        auto & positions = state.viewport_vertices();
        auto vertices = std::array<POINT, 3>{};
        static_assert(sizeof positions == sizeof vertices);
        std::memcpy(vertices.data(), positions.data(), sizeof(positions));

        // Repaint mask (click through area)
        auto ps = PAINTSTRUCT{};
        auto hdc = ::BeginPaint(hwnd, &ps);
        ::SetDCBrushColor(hdc, RGB(255, 255, 255));
        ::Polygon(hdc, vertices.data(), static_cast<int>(vertices.size()));
        ::EndPaint(hwnd, &ps);

        // Repaint screen
        render.on_render();

        return 0;
    }
    case WM_SIZE:
    {
        auto width = LOWORD(lparam);
        auto height = HIWORD(lparam);
        if (width != 0 && height != 0)
        {
            // Update
            state.update_monitor_size(width, height);
            render.on_update(ext::to_aligned_regular_triangle(state));
            render.on_resize();
            // Repaint
            ::InvalidateRect(hwnd, nullptr, true);
        }
        return 0;
    }
    default:
    {
        return ::DefWindowProc(hwnd, umsg, wparam, lparam);
    }}
}

auto wWinMain(
    _In_     HINSTANCE instance,
    _In_opt_ HINSTANCE /* prev_instance */,
    _In_     PWSTR /* cmd_line */,
    _In_     int show) -> int
{
    // Define some variables
    auto static constexpr title = TEXT("Kaleidoscope");
    auto static constexpr class_name = TEXT("kaleidoscope window");

    // Prepare some declarations
    using namespace ::aux;

    // Create the user data
    auto user_data = app::data{};

    // Create a window class
    // https://learn.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
    auto clazz = WNDCLASS{};
    clazz.hInstance = instance;
    clazz.hIcon = ::LoadIcon(nullptr, IDI_WINLOGO);
    clazz.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    clazz.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)); // Handle WM_ERASEBKGND with black
    clazz.lpszClassName = class_name;
    clazz.lpfnWndProc = main_window_message_handler;
    ::RegisterClass(&clazz);

    // Create the window
    auto window = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED /* click through */,
        clazz.lpszClassName,
        title,
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr, // No parent window
        nullptr, // No menu
        clazz.hInstance,
        &user_data) >> must::non_null;

    // Enable fullscreen mode
    // https://stackoverflow.com/q/2382464
    ::SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_THICKFRAME)) >> must::done;
    auto extended_style = ::GetWindowLongPtr(window, GWL_EXSTYLE);
    auto disabled_style = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE;
    ::SetWindowLongPtr(window, GWL_EXSTYLE, extended_style & ~disabled_style) >> must::done;

    auto monitor_info = ::MONITORINFO{ /* cbSize */ sizeof(::MONITORINFO)};
    ::GetMonitorInfo(::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor_info) >> must::done;
    auto monitor_left = static_cast<int>(monitor_info.rcMonitor.left);
    auto monitor_top = static_cast<int>(monitor_info.rcMonitor.top);
    auto monitor_width = static_cast<int>(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
    auto monitor_height = static_cast<int>(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);
    ::SetWindowPos(window, 0, monitor_left, monitor_top, monitor_width, monitor_height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >> must::done;

    // Allow to click through
    //
    // - https://stackoverflow.com/a/1524047
    // - https://www.codeproject.com/Articles/12877/Transparent-Click-Through-Forms
    //
    // WHITE \ RGB(255, 255, 255) is selected as click-through-able color
    ::SetLayeredWindowAttributes(window, RGB(255, 255, 255), 0, LWA_COLORKEY) >> must::done;

    // Show the main window
    ::ShowWindow(window, show);

    // Run the message loop.
    for (auto message = MSG{}; ::GetMessage(&message, nullptr, 0, 0) > 0; )
    {
        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
    }

    // Cleaning
    ::UnregisterClass(clazz.lpszClassName, clazz.hInstance);
    return 0;
}