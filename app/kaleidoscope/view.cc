#include <cstdint>
#include <cmath>
#include <system_error>
#include <string>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "tool.h"
#include "error.h"
#include "viewmodel.h"
#include "render.h"

namespace ext
{
    auto static inline to_bounding_rect(viewmodel::state<LONG> const & state) -> RECT
    {
        auto & v = state.viewport_vertices();
        return RECT{v[2][0], v[0][1], v[1][0], v[1][1]};
    }

    auto static inline to_aligned_regular_triangle(viewmodel::state<LONG> const & state) -> mirror::aligned_regular_triangle
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
        std::unique_ptr<mirror> render{nullptr};
    };
}

auto static message_handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> LRESULT
{
    // Preparation
    using namespace aux;
    using user_data_pointer = app::data *;

    /////////////////////////////////////////////////////////////////////
    /// Life-time related
    auto user_data = user_data_pointer{};
    switch (umsg)
    {
    case WM_CREATE:
    {
        auto param = reinterpret_cast<CREATESTRUCT *>(lparam) >> must::non_null;
        auto udata = reinterpret_cast<user_data_pointer>(param->lpCreateParams) >> must::non_null;

        // Prepare parameters
        auto monitor_info = ::MONITORINFO{ /* cbSize */ sizeof(::MONITORINFO)};
        auto & rect = monitor_info.rcMonitor;
        GetMonitorInfo(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitor_info) >> must::done;
        auto top = static_cast<int>(rect.top);
        auto left = static_cast<int>(rect.left);
        auto width = static_cast<UINT>(rect.right - rect.left);
        auto height = static_cast<UINT>(rect.bottom - rect.top);

        // Update members
        udata->state.update_monitor_size(width, height);
        udata->render = std::make_unique<mirror>(hwnd, width, height);
        udata->render->on_update(ext::to_aligned_regular_triangle(udata->state));

        // Save udata as user data of current window
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(udata));

        // Mark white (255, 255, 255) as transparent color (our background is black)
        SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), 0, LWA_COLORKEY) >> must::done;

        // Resize current window to fullscreen
        SetWindowPos(hwnd, 0, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >> must::done;
        return 0;
    }
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }

    // Ignore if not ready
    if (user_data = reinterpret_cast<user_data_pointer>(::GetWindowLongPtr(hwnd, GWLP_USERDATA)); user_data == nullptr)
    {
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }

    /////////////////////////////////////////////////////////////////////
    /// Life-time related
    auto & state = user_data->state;
    auto & render = *user_data->render.get();
    switch (umsg)
    {
    case WM_MOUSEWHEEL:
    {
        auto rect = ext::to_bounding_rect(state);
        auto delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        state.on_length_changed(delta);
        render.on_update(ext::to_aligned_regular_triangle(state));

        // About mouse events
        // - https://learn.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations
        // - https://github.com/MicrosoftDocs/win32/blob/e82557891475f35c505f90f2aa0f76bebb4e190c/desktop-src/inputdev/about-mouse-input.md

        InvalidateRect(hwnd, &rect, true /* Send WM_ERASEBKGND */);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        if (!state.is_moving())
        {
            auto x = GET_X_LPARAM(lparam);
            auto y = GET_Y_LPARAM(lparam);
            state.on_start_moving(x, y);
            SetCapture(hwnd); // Allow cursor moving outside our window
        }
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        if (state.is_moving())
        {
            auto rect = ext::to_bounding_rect(state);

            auto x = GET_X_LPARAM(lparam);
            auto y = GET_Y_LPARAM(lparam);
            state.on_moving(x, y);
            render.on_update(ext::to_aligned_regular_triangle(state));

            InvalidateRect(hwnd, &rect, true) >> must::done;
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        if (state.is_moving())
        {
            state.on_stop_moving();
            ReleaseCapture();

            auto rect = ext::to_bounding_rect(state);
            InvalidateRect(hwnd, &rect, true) >> must::done;
        }
        return 0;
    }
    case WM_RBUTTONUP:
    {
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_KEYDOWN: // Handle keyboard input
    {
        switch (wparam)
        {
        case VK_ESCAPE:
        {
            DestroyWindow(hwnd);
            break;
        }
        }
        return 0;
    }
    case WM_PAINT: // Paint
    {
        // Prepare a triangle
        auto & vertices = state.viewport_vertices();
        auto points = std::array<POINT, 3>{};
        static_assert(sizeof vertices == sizeof points);
        std::memcpy(points.data(), vertices.data(), sizeof(vertices));

        // Prepare to repaint transparent area with gdi
        auto ps = PAINTSTRUCT{};
        auto hdc = BeginPaint(hwnd, &ps);
        SetDCBrushColor(hdc, RGB(255, 255, 255));
        Polygon(hdc, points.data(), static_cast<int>(points.size()));
        EndPaint(hwnd, &ps);

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
            render.on_resize(width, height);
            // Repaint
            InvalidateRect(hwnd, nullptr, true);
        }
        return 0;
    }
    default:
    {
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    }
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
    using namespace aux;

    // Create the user data
    auto user_data = app::data{};

    // Create a window class
    //
    // Refer to
    // https://learn.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
    auto clazz = WNDCLASS{};
    clazz.style = CS_HREDRAW | CS_VREDRAW; // Repaint if window got moved or size changed (optional)
    clazz.lpfnWndProc = message_handler;
    clazz.hInstance = instance;
    clazz.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    clazz.hCursor = LoadCursor(nullptr, IDC_ARROW);
    clazz.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)); // Handle WM_ERASEBKGND
    clazz.lpszClassName = class_name;
    RegisterClass(&clazz);

    // Create a fullscreen window
    //
    // Note: finally i give up using WS_EX_NOREDIRECTIONBITMAP in order to allow
    // partial click-through-able.
    auto style = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME);
    auto extended_style
        = (WS_EX_LAYERED | WS_EX_TOPMOST) &
        ~ (WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    auto window = CreateWindowEx(
        extended_style,
        clazz.lpszClassName,
        title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr, // No parent window
        nullptr, // No menu
        clazz.hInstance,
        &user_data) >> must::non_null;

    // Show the window
    ShowWindow(window, show);

    // Run the message loop.
    for (auto message = MSG{}; GetMessage(&message, nullptr, 0, 0) > 0; )
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    // Cleaning
    UnregisterClass(clazz.lpszClassName, clazz.hInstance);
    return 0;
}
