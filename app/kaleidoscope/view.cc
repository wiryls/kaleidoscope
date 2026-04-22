#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "error.h"
#include "render.h"
#include "resource.h"
#include "tool.h"
#include "viewmodel.h"

namespace ext
{

auto static inline to_aligned_regular_triangle(viewmodel::state<LONG> const & state) -> mirror::aligned_regular_triangle
{
    auto top   = state.triangle_top();
    auto out   = mirror::aligned_regular_triangle{};
    out.top_x  = static_cast<float>(top[0]);
    out.top_y  = static_cast<float>(top[1]);
    out.length = static_cast<float>(state.triangle_side_length());
    return out;
}

auto static inline switch_menu_item(HMENU menu, UINT item, bool checked) -> void
{
    using namespace aux;
    auto info = MENUITEMINFO{sizeof(MENUITEMINFO), MIIM_STATE};
    GetMenuItemInfo(menu, item, FALSE, &info) >> must::done;
    info.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(menu, item, FALSE, &info) >> must::done;
}

auto static inline set_exclude_from_capture(HWND hwnd, bool on) -> BOOL
{
    // Exclude current window from screen capture (require version >= Windows 10 Version 2004)
    return SetWindowDisplayAffinity(hwnd, on ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
}

auto static inline set_top_most(HWND hwnd, bool on) -> LONG_PTR
{
    auto option = on ? HWND_TOPMOST : HWND_NOTOPMOST;
    return SetWindowPos(hwnd, option, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

auto static inline update_window_region(HWND hwnd, model::triangle_vertices<LONG> const & vertices) -> void
{
    using namespace aux;
    auto rect = RECT{};
    GetClientRect(hwnd, &rect);
    auto full   = CreateRectRgnIndirect(&rect) >> must::non_null;
    auto points = std::array<POINT, 3>{};
    for (auto i = 0; i < 3; ++i)
    {
        points[i].x = vertices[i][0];
        points[i].y = vertices[i][1];
    }
    auto tri    = CreatePolygonRgn(points.data(), 3, ALTERNATE) >> must::non_null;
    auto result = CreateRectRgn(0, 0, 0, 0) >> must::non_null;
    CombineRgn(result, full, tri, RGN_DIFF) >> must::done;
    SetWindowRgn(hwnd, result, FALSE) >> must::done;

    DeleteObject(full);
    DeleteObject(tri);
}

} // namespace ext

namespace app
{

using state_type = viewmodel::state<LONG>;

struct monitor_info
{
    HMONITOR     handle{};
    RECT         rect{};
    std::wstring name{};
};

struct extended_data
{
    state_type                state{};
    std::unique_ptr<mirror>   render{};
    HMENU                     menu{};
    HMENU                     monitor_submenu{};
    std::vector<monitor_info> monitors{};
    int                       current_monitor_index{};
};

auto static constexpr title      = TEXT("Kaleidoscope");
auto static constexpr class_name = TEXT("kaleidoscope window");

auto static constexpr menu_item_exit            = UINT_PTR{1000};
auto static constexpr menu_item_exit_text       = TEXT("Exit");
auto static constexpr menu_item_no_capture      = UINT_PTR{1001};
auto static constexpr menu_item_no_capture_text = TEXT("Exclude from capture");
auto static constexpr menu_item_top_most        = UINT_PTR{1002};
auto static constexpr menu_item_top_most_text   = TEXT("Keep top most");
auto static constexpr monitor_menu_id_first     = UINT_PTR{2000};

auto static CALLBACK enum_monitors_callback(HMONITOR monitor, HDC, LPRECT rect, LPARAM param) -> BOOL
{
    using namespace aux;
    auto & monitors = *reinterpret_cast<std::vector<monitor_info> *>(param);

    auto info   = MONITORINFOEXW{};
    info.cbSize = sizeof(info);
    GetMonitorInfoW(monitor, &info) >> must::done;

    auto name = std::wstring{info.szDevice};
    // Strip "\\." prefix (e.g. "\\.\DISPLAY1" -> "DISPLAY1")
    if (name.starts_with(L"\\\\.\\"))
        name.erase(0, 4);

    monitors.push_back({monitor, *rect, std::move(name)});
    return TRUE;
}

auto static inline enumerate_monitors() -> std::vector<monitor_info>
{
    using namespace aux;
    auto monitors = std::vector<monitor_info>{};
    EnumDisplayMonitors(nullptr, nullptr, enum_monitors_callback, reinterpret_cast<LPARAM>(&monitors)) >> must::done;
    return monitors;
}

auto static inline find_monitor_index(std::vector<monitor_info> const & monitors, HMONITOR handle) -> int
{
    for (int i = 0; i < static_cast<int>(monitors.size()); ++i)
    {
        if (monitors[i].handle == handle)
            return i;
    }
    return 0;
}

auto static inline build_monitor_submenu(HMENU submenu, std::vector<monitor_info> const & monitors, int current) -> void
{
    using namespace aux;
    for (int i = 0; i < static_cast<int>(monitors.size()); ++i)
    {
        auto & m     = monitors[i];
        auto   w     = m.rect.right - m.rect.left;
        auto   h     = m.rect.bottom - m.rect.top;
        auto   text  = std::format(L"{} ({}x{})", m.name, w, h);
        auto   flags = MF_STRING | (i == current ? MF_CHECKED : 0);
        AppendMenuW(submenu, flags, monitor_menu_id_first + i, text.c_str()) >> must::done;
    }
}

auto static inline rebuild_monitor_submenu(extended_data & data) -> void
{
    // Destroy old submenu items
    for (int i = static_cast<int>(data.monitors.size()) - 1; i >= 0; --i)
        DeleteMenu(data.monitor_submenu, monitor_menu_id_first + i, MF_BYCOMMAND);

    build_monitor_submenu(data.monitor_submenu, data.monitors, data.current_monitor_index);
}

auto static inline switch_to_monitor(HWND hwnd, extended_data & data, int index) -> void
{
    using namespace aux;
    if (index < 0 || index >= static_cast<int>(data.monitors.size()))
        return;
    if (index == data.current_monitor_index)
        return;

    auto & monitor = data.monitors[index];
    auto   width   = static_cast<UINT>(monitor.rect.right - monitor.rect.left);
    auto   height  = static_cast<UINT>(monitor.rect.bottom - monitor.rect.top);
    auto   left    = static_cast<int>(monitor.rect.left);
    auto   top     = static_cast<int>(monitor.rect.top);

    // Move window to target monitor FIRST so that MonitorFromWindow
    // returns the correct output when the renderer is created
    SetWindowPos(hwnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >>
        must::done;

    // Recreate renderer (safest approach — HDR state may differ between monitors)
    data.render.reset();
    data.state.on_monitor_size_changed(width, height);
    data.render = std::make_unique<mirror>(hwnd, width, height);
    data.render->on_update(ext::to_aligned_regular_triangle(data.state));
    ext::update_window_region(hwnd, data.state.triangle_vertices());

    // Update menu
    data.current_monitor_index = index;
    rebuild_monitor_submenu(data);
}

auto static inline handle_lifetime(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> extended_data *
{
    using namespace aux;
    using user_data_pointer = extended_data *;
    (void)wparam;

    auto user_data = user_data_pointer{};
    switch (umsg)
    {
    case WM_CREATE:
    {
        auto param = reinterpret_cast<CREATESTRUCT *>(lparam) >> must::non_null;
        auto udata = reinterpret_cast<user_data_pointer>(param->lpCreateParams) >> must::non_null;

        // Enumerate monitors
        udata->monitors              = enumerate_monitors();
        auto current_monitor         = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        udata->current_monitor_index = find_monitor_index(udata->monitors, current_monitor);

        // Prepare parameters from target monitor
        auto & m      = udata->monitors[udata->current_monitor_index];
        auto   width  = static_cast<UINT>(m.rect.right - m.rect.left);
        auto   height = static_cast<UINT>(m.rect.bottom - m.rect.top);
        auto   left   = static_cast<int>(m.rect.left);
        auto   top    = static_cast<int>(m.rect.top);

        // Update members
        udata->state.on_monitor_size_changed(width, height);
        udata->render = std::make_unique<mirror>(hwnd, width, height);
        udata->render->on_update(ext::to_aligned_regular_triangle(udata->state));
        udata->menu = CreatePopupMenu() >> must::non_null;

        // Save udata as user data of current window
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(udata));

        // Resize current window to fullscreen
        SetWindowPos(hwnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >>
            must::done;

        // Set initial window region (excludes triangle for click-through)
        ext::update_window_region(hwnd, udata->state.triangle_vertices());

        // Setup menu
        //
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/menurc/using-menus
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createpopupmenu
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenua
        // https://stackoverflow.com/a/68845977
        auto menu = udata->menu;

        // Monitor submenu
        udata->monitor_submenu = CreatePopupMenu() >> must::non_null;
        build_monitor_submenu(udata->monitor_submenu, udata->monitors, udata->current_monitor_index);
        AppendMenuW(menu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(udata->monitor_submenu), L"Monitor") >>
            must::done;

        AppendMenu(menu, MF_STRING, menu_item_top_most, menu_item_top_most_text) >> must::done;
        AppendMenu(menu, MF_STRING, menu_item_no_capture, menu_item_no_capture_text) >> must::done;
        AppendMenu(menu, MF_SEPARATOR, 0, nullptr) >> must::done;
        AppendMenu(menu, MF_STRING, menu_item_exit, menu_item_exit_text) >> must::done;
        {
            auto option = udata->state.option_keep_top_most();
            ext::set_top_most(hwnd, option) >> must::done;
            ext::switch_menu_item(menu, menu_item_top_most, option);
        }
        {
            auto option = udata->state.option_exclude_from_capture();
            ext::set_exclude_from_capture(hwnd, option) >> must::done;
            ext::switch_menu_item(menu, menu_item_no_capture, option);
        }
        return nullptr;
    }
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return nullptr;
    }
    case WM_DESTROY:
    {
        user_data = reinterpret_cast<user_data_pointer>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (user_data && user_data->menu)
            DestroyMenu(user_data->menu);

        PostQuitMessage(0);
        return nullptr;
    }
    default:
        user_data = reinterpret_cast<user_data_pointer>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        return user_data;
    }
}

auto static inline handle_common_events(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data)
    -> std::optional<LRESULT>
{
    auto & state  = data.state;
    auto & render = *data.render;
    (void)wparam;

    switch (umsg)
    {
    case WM_PAINT:
        ValidateRect(hwnd, nullptr);
        return 0;
    case WM_SIZE:
    {
        auto width  = LOWORD(lparam);
        auto height = HIWORD(lparam);
        if (width != 0 && height != 0)
        {
            state.on_monitor_size_changed(width, height);
            render.on_update(ext::to_aligned_regular_triangle(state));
            render.on_resize(width, height);
            ext::update_window_region(hwnd, state.triangle_vertices());
        }
        return 0;
    }
    default:
    {
        return std::nullopt;
    }
    }
}

auto static inline handle_inputs(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data)
    -> std::optional<LRESULT>
{
    using namespace aux;
    auto & state  = data.state;
    auto & render = *data.render;

    switch (umsg)
    {
    case WM_MOUSEWHEEL:
    {
        auto delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        state.on_length_changed(delta);
        render.on_update(ext::to_aligned_regular_triangle(state));

        // About mouse events
        // - https://learn.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations
        // -
        // https://github.com/MicrosoftDocs/win32/blob/e82557891475f35c505f90f2aa0f76bebb4e190c/desktop-src/inputdev/about-mouse-input.md

        ext::update_window_region(hwnd, state.triangle_vertices());
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
            auto x = GET_X_LPARAM(lparam);
            auto y = GET_Y_LPARAM(lparam);
            state.on_moving(x, y);
            render.on_update(ext::to_aligned_regular_triangle(state));
            ext::update_window_region(hwnd, state.triangle_vertices());
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        if (state.is_moving())
        {
            state.on_stop_moving();
            ReleaseCapture();
            ext::update_window_region(hwnd, state.triangle_vertices());
        }
        return 0;
    }
    case WM_CONTEXTMENU:
    {
        auto x = GET_X_LPARAM(lparam);
        auto y = GET_Y_LPARAM(lparam);
        TrackPopupMenu(data.menu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hwnd, nullptr) >> must::done;
        return 0;
    }
    case WM_KEYDOWN:
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
    default:
    {
        return std::nullopt;
    }
    }
}

auto static inline handle_menu(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data)
    -> std::optional<LRESULT>
{
    using namespace aux;
    (void)lparam;

    if (umsg != WM_COMMAND)
        return std::nullopt;

    auto id = LOWORD(wparam);

    // Monitor submenu items
    if (id >= monitor_menu_id_first && id < monitor_menu_id_first + static_cast<UINT_PTR>(data.monitors.size()))
    {
        switch_to_monitor(hwnd, data, static_cast<int>(id - monitor_menu_id_first));
        return 0;
    }

    auto & state = data.state;
    switch (id)
    {
    case menu_item_top_most:
    {
        auto option = state.option_keep_top_most(true);
        ext::set_top_most(hwnd, option) >> must::done;
        ext::switch_menu_item(data.menu, menu_item_top_most, option);
        return 0;
    }
    case menu_item_no_capture:
    {
        auto option = state.option_exclude_from_capture(true);
        ext::set_exclude_from_capture(hwnd, option) >> must::done;
        ext::switch_menu_item(data.menu, menu_item_no_capture, option);
        return 0;
    }
    case menu_item_exit:
    {
        DestroyWindow(hwnd);
        return 0;
    }
    default:
    {
        return std::nullopt;
    }
    }
}

auto static CALLBACK window_message_handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> LRESULT
{
    auto user_data = handle_lifetime(hwnd, umsg, wparam, lparam);
    if (user_data == nullptr)
        return DefWindowProc(hwnd, umsg, wparam, lparam);

    auto code = std::optional<LRESULT>{};

    if (not code.has_value())
        code = handle_common_events(hwnd, umsg, wparam, lparam, *user_data);
    if (not code.has_value())
        code = handle_inputs(hwnd, umsg, wparam, lparam, *user_data);
    if (not code.has_value())
        code = handle_menu(hwnd, umsg, wparam, lparam, *user_data);

    if (code.has_value())
        return code.value();

    return DefWindowProc(hwnd, umsg, wparam, lparam);
}
} // namespace app

auto static inline handle_messages() -> bool
{
    auto message = MSG{};
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
            return false;
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return true;
}

auto static inline to_wstring(std::string_view narrow) -> std::wstring
{
    auto length = MultiByteToWideChar(CP_UTF8, 0, narrow.data(), static_cast<int>(narrow.size()), nullptr, 0);
    auto result = std::wstring(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow.data(), static_cast<int>(narrow.size()), result.data(), length);
    return result;
}

auto CALLBACK run(HINSTANCE instance, int show) -> void
{
    // Prepare some declarations
    using namespace aux;

    // Create the user data
    auto user_data = app::extended_data{};

    // Create a window class
    //
    // Refer to
    // https://learn.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
    auto clazz          = WNDCLASS{};
    clazz.style         = CS_HREDRAW | CS_VREDRAW;
    clazz.lpfnWndProc   = app::window_message_handler;
    clazz.hInstance     = instance;
    clazz.hIcon         = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
    clazz.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    clazz.lpszClassName = app::class_name;
    RegisterClass(&clazz) >> must::done;

    // Create a fullscreen window
    //
    // WS_EX_NOREDIRECTIONBITMAP eliminates the GDI redirection surface, so there is
    // no separate GDI surface that can desync with the DX12 frame (which was causing
    // black flickering inside the triangle during dragging). Click-through inside the
    // triangle is handled by SetWindowRgn (subtracting the triangle from the window
    // region) instead of the previous LWA_COLORKEY + GDI white polygon approach.
    auto style          = WS_POPUP;
    auto extended_style = WS_EX_NOREDIRECTIONBITMAP;
    auto window         = CreateWindowEx(
                      extended_style, clazz.lpszClassName, app::title, style, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, CW_USEDEFAULT,
                      nullptr, // No parent window
                      nullptr, // No menu
                      clazz.hInstance, &user_data
                  ) >>
                  must::non_null;

    // Show the window
    ShowWindow(window, show);

    // Run the message loop.
    //
    // Use PeekMessage instead of GetMessage + WM_TIMER for rendering.
    // GetMessage blocks until a message arrives, and WM_TIMER has low priority
    // (~15.6ms resolution), resulting in unreliable frame rates.
    // PeekMessage is non-blocking; we render in the idle time, and the actual
    // frame rate is controlled by swap chain VSync (Present interval = 1).
    while (handle_messages())
    {
        auto data = reinterpret_cast<app::extended_data *>(GetWindowLongPtr(window, GWLP_USERDATA));
        if (data && data->render)
            data->render->on_render();
    }

    // Cleaning
    UnregisterClass(clazz.lpszClassName, clazz.hInstance);
}

auto CALLBACK
wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE /* prev_instance */, _In_ PWSTR /* cmd_line */, _In_ int show)
    -> int
{
    auto message = std::wstring{};

    try
    {
        run(instance, show);
    }
    catch (std::system_error const & err)
    {
        message = to_wstring(err.what());
    }
    catch (_com_error const & err)
    {
        message = to_wstring(err.ErrorMessage());
    }

    if (!message.empty())
    {
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw
        MessageBoxW(nullptr, message.c_str(), L"Oops!", MB_OK | MB_ICONWARNING | MB_TOPMOST);
    }
    return 0;
}
