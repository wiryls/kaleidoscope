#include <system_error>
#include <Windows.h>
#undef min
#undef max

#include "tool.h"
#include "view.h"

auto constexpr must_nonzero = [](auto && value)
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

struct window::impl
{
public:
    auto static inline constexpr title = "kaleidoscope";
    auto static inline constexpr class_name = "kaleidoscope";

public:
    HMODULE module_handle {};
    WNDCLASS clazz {};
    HWND window_instance{};
};

window::window()
	: m(std::make_unique<impl>())
{
	using namespace xerror;

    auto & module_handle = m->module_handle;
	module_handle = ::GetModuleHandle(nullptr) >> must_nonzero;

    auto & clazz = m->clazz;
    clazz.lpszClassName = TEXT(impl::class_name);
    clazz.hInstance = module_handle;
    clazz.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    clazz.hCursor = LoadCursor(nullptr, IDC_ARROW);
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
            SetDCBrushColor(hdc, RGB(233, 233, 233));
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

    // create window
    auto & window_instance = m->window_instance;
    window_instance = CreateWindowEx(
        WS_EX_TOPMOST,
        TEXT(clazz.lpszClassName),
        TEXT(impl::title),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        clazz.hInstance,
        nullptr) >> must_nonzero;

    // enable fullscreen mode
    // - https://stackoverflow.com/q/2382464
    // click through
    // - https://stackoverflow.com/a/1524047
    // - https://www.codeproject.com/Articles/12877/Transparent-Click-Through-Forms
    auto monitor_instance = ::MonitorFromWindow(window_instance, MONITOR_DEFAULTTONEAREST);
    auto monitor_info = MONITORINFO{ /* cbSize */ sizeof(MONITORINFO) };
    ::GetMonitorInfo(monitor_instance, &monitor_info) >> must_nonzero;
    ::SetWindowLongPtr(window_instance, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_THICKFRAME));
    auto extended_style = ::GetWindowLongPtr(window_instance, GWL_EXSTYLE) | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT;
    ::SetWindowLongPtr(window_instance, GWL_EXSTYLE, extended_style & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
    ::SetLayeredWindowAttributes(window_instance, RGB(255, 255, 255), 255, LWA_COLORKEY) >> must_nonzero;
    auto left = monitor_info.rcMonitor.left;
    auto top = monitor_info.rcMonitor.top;
    auto width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    auto height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    ::SetWindowPos(window_instance, 0, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >> must_nonzero;

    // show window
    ::ShowWindow(window_instance, SW_SHOW);
}

window::~window()
{
    if (m)
    {
        if (m->clazz.hInstance)
            UnregisterClass(m->clazz.lpszClassName, m->module_handle);
    }
}

auto window::run() -> void
{
    auto loop = true;
    while (loop)
    {
        auto msg = MSG{};
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            loop = msg.message != WM_QUIT;
            if (!loop)
                break;

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
}
