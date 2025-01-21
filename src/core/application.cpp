#include "defines.h"
#include "application.h"

WNDCLASSEXW  Application::m_wcex;
HWND Application::h_wnd = nullptr;

Overlay Application::m_overlay;
SmoothScroll Application::m_smooth_scroll;

Application::Application(HINSTANCE h_instance)
{
    const wchar_t class_name[] = L"Keydows";
    WNDCLASSEXW m_wcex = {0};
    m_wcex.cbSize         = sizeof(m_wcex);
    m_wcex.style          = CS_HREDRAW | CS_VREDRAW;
    m_wcex.lpfnWndProc    = wnd_proc;
    m_wcex.hInstance      = h_instance;
    m_wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
    m_wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    m_wcex.lpszClassName  = class_name;
    ::RegisterClassExW(&m_wcex);

    h_wnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to mouse press
        class_name,
        L"Keydows Overlay Window",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL,
        h_instance,
        this
    );
    ::SetLayeredWindowAttributes(h_wnd, RGB(0, 0, 0), 200, LWA_ALPHA | LWA_COLORKEY);

    hotkey::register_hotkey(h_wnd, Hotkeys::QUIT, MOD_CONTROL | MOD_ALT, L'Q');
    timer::initialize();

    load_config();
    m_overlay.activate(false);
    m_smooth_scroll.activate(true);
}

Application::~Application()
{
    ::UnregisterClassW(m_wcex.lpszClassName, m_wcex.hInstance);
}

int Application::run()
{
    MSG msg;
    while (::GetMessageW(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

void Application::load_config()
{
    std::ifstream config_file("../config.json");
    
    if (!config_file.is_open())
    {
        std::cerr << "Failed to open config.json" << std::endl;
        hotkey::register_hotkey(h_wnd, Application::Hotkeys::OVERLAY, MOD_CONTROL, VK_OEM_PERIOD);
        m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
        m_overlay.set_resolution(24, 19);
        return;
    }

    json json;
    config_file >> json;

    // Overlay
    {
        auto overlay = json.at("overlay");

        auto resolution = overlay.at("resolution");
        int x = resolution.at(0);
        int y = resolution.at(1);

        m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
        m_overlay.set_resolution(x, y);
        
        std::string charset = overlay.at("charset");
        std::string dir_charset = overlay.at("click_direction_charset");
        
        m_overlay.set_charset(std::wstring(charset.begin(), charset.end()).c_str());
        m_overlay.set_click_direction_charset(std::wstring(dir_charset.begin(), dir_charset.end()).c_str());

        // Hotkeys
        auto hk = overlay.at("hotkeys");
        auto activate = hk.at("activate");
        hotkey::register_hotkey(h_wnd, Hotkeys::OVERLAY, activate.at("mod"), activate.at("key"));
    }
}

LRESULT CALLBACK Application::wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
    case WM_HOTKEY:
        handle_hotkey(w_param);
        return 0;

    case WM_PAINT:
        paint_event();
        return 0;

    case WM_DESTROY:
        shutdown();
        return 0;

    default:
        return ::DefWindowProc(h_wnd, message, w_param, l_param);
    }
}

void Application::shutdown()
{
    HookManager::detach_hooks();
    hotkey::unregister_key(h_wnd, QUIT);
    hotkey::unregister_key(h_wnd, OVERLAY);
    ::PostQuitMessage(0);
}

void Application::handle_hotkey(WPARAM w_param)
{
    switch (w_param) {
    case Hotkeys::QUIT:
        ::DestroyWindow(h_wnd);   // Send WM_DESTROY message
        break;

    case Hotkeys::OVERLAY:
        // Prevent control from getting stuck. For whatever reason
        // right mod keys won't release. Make dynamic later.
        release_key(VK_LCONTROL);
        m_overlay.activate(!::IsWindowVisible(h_wnd));
        break;
    }
}

void Application::show_window(bool show)
{
    if (show)
    {
        // Because the window never has focus, it can't receive keydown events
        ::ShowWindow(h_wnd, SW_SHOWNOACTIVATE);
        ::SetWindowPos(h_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        // Repaint so that the old bitmap is not shown
        repaint();
        ::ShowWindow(h_wnd, SW_HIDE);
    }
}

void Application::move_cursor(int x, int y)
{
    ::SetCursorPos(x, y);
}

void Application::paint_event()
{
    m_overlay.render(h_wnd);
}

void Application::repaint()
{
    ::InvalidateRect(h_wnd, NULL, FALSE);  // NULL means the entire client area, TRUE means erase background
    ::UpdateWindow(h_wnd);                 // Post WM_PAINT event
}

void Application::click(int n, int x, int y, bool right_click)
{
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[1].type = INPUT_MOUSE;

    DWORD down;
    DWORD up;
    if (right_click)
    {
        down = MOUSEEVENTF_RIGHTDOWN;
        up   = MOUSEEVENTF_RIGHTUP;
    }
    else
    {
        down = MOUSEEVENTF_LEFTDOWN;
        up   = MOUSEEVENTF_LEFTUP;
    }

    inputs[0].mi.dwFlags = down;
    inputs[1].mi.dwFlags = up;

    ::SetCursorPos(x, y);

    for (int i = 0; i < n; ++i)
    {
        // Send the inputs with a delay because some apps may not register them otherwise
        ::SendInput(1, &inputs[0], sizeof(INPUT));
        ::Sleep(25);
        ::SendInput(1, &inputs[1], sizeof(INPUT));
    }
}

void Application::click_async(int n, int x, int y, bool right_click)
{
    std::thread(&Application::click, n, x, y, right_click).detach();
}

// Used for releasing keys so they don't get left on hold after overlay is activated
void Application::release_key(int vk_code)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk_code;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    ::SendInput(1, &input, sizeof(INPUT));
}

bool Application::is_key_down(int virtual_key)
{
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
    // If the high-order bit is 1, the key is down; otherwise, it is up.
    //
    return ::GetAsyncKeyState(virtual_key) & 0x8000;
}