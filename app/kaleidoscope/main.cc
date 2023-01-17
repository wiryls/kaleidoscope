#include <cstdint>
#include <cmath>
#include <concepts>
#include <type_traits>
#include <algorithm>
#include <system_error>

#include <windows.h>
#undef min
#undef max

namespace aux
{
    template<typename T, typename U>
    auto inline operator >>(T && value, U && handle) -> std::invoke_result_t<U, T>
    {
        return std::forward<U>(handle)(std::forward<T>(value));
    }
}

namespace must
{
    auto constexpr nonzero = [](auto && value)
    {
        if (!static_cast<bool>(value))
        {
            auto code = ::GetLastError();
            auto desc = std::system_category().message(code);
            ::OutputDebugString(desc.c_str());
            throw std::system_error(code, std::system_category());
        }
        return std::forward<decltype(value)>(value);
    };
}

namespace model
{
    class tile
    {
    public:
        tile(std::integral auto screen_width, std::integral auto screen_height)
            : tile(static_cast<std::int32_t>(screen_width), static_cast<std::int32_t>(screen_height))
        {}

        tile(std::int32_t screen_width, std::int32_t screen_height)
            : top{screen_width / 2}
            , left{screen_height / 2}
            , length{std::min(top, static_cast<std::int32_t>(static_cast<double>(screen_height) / std::sqrt(3.)))}
        {}

    private:
        std::int32_t top;
        std::int32_t left;
        std::int32_t length;
    };
}

auto wWinMain(
    _In_     HINSTANCE instance,
    _In_opt_ HINSTANCE /* prev_instance */,
    _In_     PWSTR /* cmd_line */,
    _In_     int show) -> int
{
    // define some variables
    auto static constexpr title = TEXT("Kaleidoscope");
    auto static constexpr class_name = TEXT("kaleidoscope");

    // prepare some declarations
    using namespace ::aux;

    // create a window class
    // - https://learn.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
    auto clazz = WNDCLASS{};
    clazz.hInstance = instance;
    clazz.hIcon = ::LoadIcon(nullptr, IDI_WINLOGO);
    clazz.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    clazz.lpszClassName = class_name;
    clazz.lpfnWndProc = +[](HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> LRESULT
    {
        switch (umsg)
        {
        case WM_KEYDOWN:
            switch (wparam)
            {
            case VK_ESCAPE:
                ::DestroyWindow(hwnd);
                break;
            }
            break;

        case WM_ERASEBKGND:
        {
            auto ps = PAINTSTRUCT{};
            auto rc = RECT{};
            auto hdc = ::BeginPaint(hwnd, &ps); // experiment
            GetClientRect(hwnd, &rc);
            SetDCBrushColor(hdc, RGB(0, 0, 0));
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));

            rc.left = 1000;
            rc.right = 1200;
            rc.top = 100;
            rc.bottom = 200;
            SetDCBrushColor(hdc, RGB(255, 255, 255));
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));

            ::EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE:
            ::DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            return {};
        }

        return ::DefWindowProc(hwnd, umsg, wparam, lparam);
    };
    ::RegisterClass(&clazz);

    // create the window
    auto window = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED /* click through */,
        clazz.lpszClassName,
        title,
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr, // no parent window
        nullptr, // no menu
        clazz.hInstance,
        nullptr /* TODO */) >> must::nonzero;

    // enable fullscreen mode
    // - https://stackoverflow.com/q/2382464
    auto monitor_info = ::MONITORINFO{ /* cbSize */ sizeof(::MONITORINFO)};
    ::GetMonitorInfo(::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor_info) >> must::nonzero;
    ::SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_THICKFRAME)) >> must::nonzero;
    auto extended_style = ::GetWindowLongPtr(window, GWL_EXSTYLE);
    auto disabled_style = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE;
    ::SetWindowLongPtr(window, GWL_EXSTYLE, extended_style & ~disabled_style) >> must::nonzero;
    auto left = monitor_info.rcMonitor.left;
    auto top = monitor_info.rcMonitor.top;
    auto width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    auto height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    ::SetWindowPos(window, 0, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >> must::nonzero;

    // allow to click through
    // - https://stackoverflow.com/a/1524047
    // - https://www.codeproject.com/Articles/12877/Transparent-Click-Through-Forms
    ::SetLayeredWindowAttributes(window, RGB(255, 255, 255), 0, LWA_COLORKEY) >> must::nonzero;

    // show the main window
    ::ShowWindow(window, show);

    // run the message loop.
    for (auto message = MSG{}; ::GetMessage(&message, nullptr, 0, 0) > 0; )
    {
        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
    }

    // cleaning
    ::UnregisterClass(clazz.lpszClassName, clazz.hInstance);
    return 0;
}
