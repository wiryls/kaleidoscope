#include <cstdint>
#include <cmath>
#include <system_error>
#include <optional>
#include <string>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "resource.h"
#include "tool.h"
#include "error.h"
#include "viewmodel.h"
#include "render.h"

namespace ext
{
    auto static inline calculate_extended_bounding_rect(viewmodel::state<LONG> const & state) -> RECT
    {
        auto constexpr static border = LONG{64};
        auto & v = state.triangle_vertices();
        return RECT
        {
            std::max(v[2][0], border) - border, // left
            std::max(v[0][1], border) - border, // top
            v[1][0] + border,                   // right
            v[1][1] + border,                   // bottom
        };
    }

    auto static inline expand_triangle(std::array<POINT, 3> & input, bool is_moving) -> std::array<POINT, 3> &
    {
        auto constexpr static border = LONG{16};
        auto extra = (is_moving ? 3 : 1) * border;

        input[0].y -= extra;
        input[1].x += extra;
        input[1].y += extra;
        input[2].x -= extra;
        input[2].y += extra;

        return input;
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

    auto static inline switch_menu_item(HMENU menu, UINT item, bool checked) -> void
    {
        auto info = MENUITEMINFO{sizeof(MENUITEMINFO), MIIM_STATE};
        GetMenuItemInfo(menu, item, false, &info);
        info.fState = checked ? MFS_CHECKED : MFS_UNCHECKED;
        SetMenuItemInfo(menu, item, false, &info);
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
}

namespace app
{
    using state_type = viewmodel::state<LONG>;

    struct extended_data
    {
        state_type state{};
        std::unique_ptr<mirror> render{};
        HMENU menu{};
    };

    auto static constexpr title = TEXT("Kaleidoscope");
    auto static constexpr class_name = TEXT("kaleidoscope window");

    auto static constexpr render_timer_id = UINT_PTR{0x2333};
    auto static constexpr render_timer_interval = UINT{1000 / 60};

    auto static constexpr menu_item_exit = UINT_PTR{1000};
    auto static constexpr menu_item_exit_text = TEXT("Exit");
    auto static constexpr menu_item_no_capture = UINT_PTR{1001};
    auto static constexpr menu_item_no_capture_text = TEXT("Exclude from capture");
    auto static constexpr menu_item_top_most = UINT_PTR{1002};
    auto static constexpr menu_item_top_most_text = TEXT("Keep top most");

    auto static inline handle_liftime(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> extended_data *
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

            // Prepare parameters
            auto monitor_info = ::MONITORINFO{ /* cbSize */ sizeof(::MONITORINFO)};
            auto & rect = monitor_info.rcMonitor;
            GetMonitorInfo(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitor_info) >> must::done;
            auto top = static_cast<int>(rect.top);
            auto left = static_cast<int>(rect.left);
            auto width = static_cast<UINT>(rect.right - rect.left);
            auto height = static_cast<UINT>(rect.bottom - rect.top);

            // Update members
            udata->state.on_monitor_size_changed(width, height);
            udata->render = std::make_unique<mirror>(hwnd, width, height);
            udata->render->on_update(ext::to_aligned_regular_triangle(udata->state));
            udata->menu = CreatePopupMenu() >> must::non_null;

            // Save udata as user data of current window
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(udata));

            // Mark white (255, 255, 255) as transparent color (our background is black)
            SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), 0, LWA_COLORKEY) >> must::done;

            // Resize current window to fullscreen
            SetWindowPos(hwnd, 0, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) >> must::done;

            // Setup menu
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/menurc/using-menus
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createpopupmenu
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenua
            // https://stackoverflow.com/a/68845977
            auto menu = udata->menu;
            AppendMenu(menu, MF_STRING, menu_item_top_most, menu_item_top_most_text);
            AppendMenu(menu, MF_STRING, menu_item_no_capture, menu_item_no_capture_text);
            AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
            AppendMenu(menu, MF_STRING, menu_item_exit, menu_item_exit_text);
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

            // Setup a timer to render
            //
            // Refer to
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-settimer
            SetTimer(hwnd, render_timer_id, render_timer_interval, nullptr);
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

            KillTimer(hwnd, render_timer_id);
            PostQuitMessage(0);
            return nullptr;
        }
        default:
            user_data = reinterpret_cast<user_data_pointer>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
            return user_data;
        }
    }

    auto static inline handle_common_events(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data) -> std::optional<LRESULT>
    {
        auto & state = data.state;
        auto & render = *data.render.get();

        switch (umsg)
        {
        case WM_TIMER:
        {
            switch (wparam)
            {
            case render_timer_id:
                render.on_render();
                return 0;
            }
            return std::nullopt;
        }
        case WM_PAINT: // Paint
        {
            // Prepare a triangle
            auto & vertices = state.triangle_vertices();
            auto points = std::array<POINT, 3>{};
            static_assert(sizeof vertices == sizeof points);
            std::memcpy(points.data(), vertices.data(), sizeof(vertices));

            // Because I want allow not to enable WDA_EXCLUDEFROMCAPTURE, as it forbids
            // other programs capturing this window, I choose to extend the transparents area.
            ext::expand_triangle(points, state.is_moving());

            // Prepare to repaint transparent area with gdi
            auto ps = PAINTSTRUCT{};
            auto hdc = BeginPaint(hwnd, &ps);
            SetDCBrushColor(hdc, RGB(255, 255, 255));
            Polygon(hdc, points.data(), static_cast<int>(points.size()));
            EndPaint(hwnd, &ps);

            // Repaint
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
                state.on_monitor_size_changed(width, height);
                render.on_update(ext::to_aligned_regular_triangle(state));
                render.on_resize(width, height);
                // Repaint
                InvalidateRect(hwnd, nullptr, true);
            }
            return 0;
        }
        default:
        {
            return std::nullopt;
        }
        }
    }

    auto static inline handle_inputs(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data) -> std::optional<LRESULT>
    {
        using namespace aux;
        auto & state = data.state;
        auto & render = *data.render.get();

        switch (umsg)
        {
        case WM_MOUSEWHEEL:
        {
            auto rect = ext::calculate_extended_bounding_rect(state);
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
                auto rect = ext::calculate_extended_bounding_rect(state);

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

                auto rect = ext::calculate_extended_bounding_rect(state);
                InvalidateRect(hwnd, &rect, true) >> must::done;
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

    auto static inline handle_menu(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam, extended_data & data) -> std::optional<LRESULT>
    {
        using namespace aux;
        (void)hwnd;
        (void)lparam;

        if (umsg != WM_COMMAND)
            return std::nullopt;

        auto & state = data.state;
        switch (wparam)
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
        auto user_data = handle_liftime(hwnd, umsg, wparam, lparam);
        if (user_data == nullptr)
            return DefWindowProc(hwnd, umsg, wparam, lparam);

        auto code = std::optional<LRESULT>{};

        if (code.has_value() == false)
            code = handle_common_events(hwnd, umsg, wparam, lparam, *user_data);
        if (code.has_value() == false)
            code = handle_inputs(hwnd, umsg, wparam, lparam, *user_data);
        if (code.has_value() == false)
            code = handle_menu(hwnd, umsg, wparam, lparam, *user_data);

        if (code.has_value())
            return code.value();

        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
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
    auto clazz = WNDCLASS{};
    clazz.style = CS_HREDRAW | CS_VREDRAW; // Repaint if window got moved or size changed (optional)
    clazz.lpfnWndProc = app::window_message_handler;
    clazz.hInstance = instance;
    clazz.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
    clazz.hCursor = LoadCursor(nullptr, IDC_ARROW);
    clazz.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)); // Handle WM_ERASEBKGND with black
    clazz.lpszClassName = app::class_name;
    RegisterClass(&clazz);

    // Create a fullscreen window
    //
    // Note: finally i give up using WS_EX_NOREDIRECTIONBITMAP in order to allow
    // partial click-through-able.
    auto style = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME);
    auto extended_style = WS_EX_LAYERED & ~ (WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    auto window = CreateWindowEx(
        extended_style,
        clazz.lpszClassName,
        app::title,
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
}

auto CALLBACK wWinMain(
    _In_     HINSTANCE instance,
    _In_opt_ HINSTANCE /* prev_instance */,
    _In_     PWSTR /* cmd_line */,
    _In_     int show) -> int
{
    auto message = std::string{};

    try
    {
        run(instance, show);
    }
    catch (std::system_error const & err)
    {
        message = err.what();
    }
    catch (_com_error const & err)
    {
        message = err.ErrorMessage();
    }

    if (!message.empty())
    {
        // Refer to
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox
        MessageBox(nullptr, message.c_str(), ":-(", MB_OK | MB_ICONWARNING | MB_TOPMOST);
    }
    return 0;
}
