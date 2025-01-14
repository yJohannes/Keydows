#include "defines.h"
#include "application.h"

WNDCLASSEXW  Application::m_wcex;
HWND Application::h_wnd = nullptr;
HHOOK Application::m_keyboard_hook = nullptr;
HHOOK Application::m_mouse_hook = nullptr;
Overlay Application::m_overlay;

Application::Application(HINSTANCE h_instance)
{
#if NTDDI_VERSION >= NTDDI_WINBLUE
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#else
    ::SetProcessDPIAware();
#endif

    // Window creation
    const wchar_t class_name[] = L"Keydows";
    WNDCLASSEXW m_wcex = {0};
    m_wcex.cbSize = sizeof(m_wcex);
    m_wcex.style          = CS_HREDRAW | CS_VREDRAW;
    m_wcex.lpfnWndProc    = wnd_proc;
    m_wcex.hInstance      = h_instance;
    m_wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
    m_wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    m_wcex.lpszClassName  = class_name;
    ::RegisterClassExW(&m_wcex);

    h_wnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to keypresses
        class_name,
        L"Keydows Overlay Window",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL,
        h_instance,
        this
    );

    // This does nothing at the moment for text
    ::SetLayeredWindowAttributes(h_wnd, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);

    load_config();
    m_overlay.activate(false);
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
        hotkey::register_key(h_wnd, hotkey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
        hotkey::register_key(h_wnd, hotkey::OVERLAY, MOD_RIGHT | MOD_CONTROL, VK_OEM_PERIOD);
        m_overlay.set_resolution(24, 19);
        return;
    }

    json json;
    config_file >> json;

    // Overlay
    {
        auto resolution = json.at("resolution");
        int x = resolution.at(0);
        int y = resolution.at(1);

        m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
        m_overlay.set_resolution(x, y);
        
        std::string charset = json.at("charset");
        std::string dir_charset = json.at("click_direction_charset");
        
        m_overlay.set_charset(std::wstring(charset.begin(), charset.end()).c_str());
        m_overlay.set_click_direction_charset(std::wstring(dir_charset.begin(), dir_charset.end()).c_str());
    }

    // Hotkeys
    {
        auto hks = json.at("hotkeys");
        auto hk_overlay = hks.at("overlay");
        hotkey::register_key(h_wnd, hotkey::OVERLAY, hk_overlay.at("mod"), hk_overlay.at("key"));
        hotkey::register_key(h_wnd, hotkey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
    }
    

}

#pragma region Proc, hook
LRESULT CALLBACK Application::wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
    case WM_HOTKEY:
        handle_hotkey(w_param);
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        handle_keydown(w_param, l_param);
        return 0;

    case WM_PAINT:
        paint_event();
        return 0;

    case WM_DESTROY:
        destroy_proc();
        return 0;

    default:
        return ::DefWindowProc(h_wnd, message, w_param, l_param);
    }
}

// Posts keyboard event messages for wnd_proc to process
LRESULT CALLBACK Application::keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (m_overlay.keyboard_proc_receiver(n_code, w_param, l_param))
    {
        return 1;
    }
    // Pass the input to further receivers
    return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
}

LRESULT CALLBACK Application::mouse_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code == HC_ACTION)
    {
        if ((w_param == WM_LBUTTONDOWN) || (w_param == WM_RBUTTONDOWN))
        {
            m_overlay.activate(false);
        }
    }

    // Pass the input to further receivers
    return CallNextHookEx(m_mouse_hook, n_code, w_param, l_param);
}

void Application::destroy_proc()
{
    detach_hooks();
    hotkey::unregister_hotkeys(h_wnd);
    ::PostQuitMessage(0);
}


void Application::attach_hooks()
{
    m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
    if (!m_keyboard_hook)
    {
        std::cerr << "Failed to install keyboard hook!" << std::endl;
        ::MessageBox(NULL, L"Failed to install keyboard hook!", L"Error", MB_ICONERROR | MB_OK);
    }

    m_mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, NULL, 0);
    if (!m_mouse_hook)
    {
        std::cerr << "Failed to install mouse hook!" << std::endl;
        ::MessageBox(NULL, L"Failed to install mouse hook!", L"Error", MB_ICONERROR | MB_OK);
    }
}

void Application::detach_hooks()
{
    ::UnhookWindowsHookEx(m_keyboard_hook);
    ::UnhookWindowsHookEx(m_mouse_hook);
}
#pragma endregion

#pragma region Event
void Application::handle_keydown(WPARAM key, LPARAM details)
{
    std::cout << "VK pressed:\t" << key << "\n";

    switch (key) {
    case VK_F4:
        ::DestroyWindow(h_wnd);
        return;
    }

    std::cout << "-> VK char:\t" << (char)key << "\n";

    // Map input virtual key to actual key (fails for certain keys like öäå for some reason)
    wchar_t uni_key = key;  // Fail-safe
    {
        BYTE kb_state[256];
        ::GetKeyboardState(kb_state);
        kb_state[VK_SHIFT] = 0;   // Remove shift

        UINT scan_code = (details >> 16) & 0xFF;

        ::ToUnicode(key, scan_code, kb_state, &uni_key, 1, 0);
        uni_key = towupper(uni_key);
    }

    std::cout << (char)uni_key << "\n";

    int result = m_overlay.enter_input(uni_key);
    switch (result)
    {
    case Overlay::FIRST_INPUT:
    case Overlay::SECOND_INPUT:
        repaint();    // Force repaint to update highlights
        break;
    case Overlay::CLICKED:
        int x = m_overlay.input_data()->x;
        int y = m_overlay.input_data()->y;
        click_at(x, y, is_key_down(VK_SHIFT));
        m_overlay.activate(false);
        repaint();
        break;
    }
}

void Application::handle_hotkey(WPARAM w_param)
{
    switch (w_param) {
    case hotkey::CLOSE:
        ::DestroyWindow(h_wnd);   // Send WM_DESTROY message
        break;

    case hotkey::OVERLAY:
        // Prevent control from getting stuck. For whatever reason
        // right mod keys won't release. Make dynamic later.
        release_key(VK_LCONTROL);
        m_overlay.activate(!::IsWindowVisible(h_wnd));
        // show_window(!::IsWindowVisible(h_wnd));

        break;
    }
}

void Application::show_window(bool show)
{
    if (show)
    {
        // Because the window never has focus, it can't receive keydown events; only uses global keyboard hook
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

void Application::paint_event()
{
    m_overlay.render(h_wnd);
}

void Application::repaint()
{
    ::InvalidateRect(h_wnd, NULL, FALSE);  // NULL means the entire client area, TRUE means erase background
    ::UpdateWindow(h_wnd);                 // Post WM_PAINT event
}

void Application::click_at(int x, int y, bool right_click)
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
    ::SendInput(2, inputs, sizeof(INPUT));
}

// Used for releasing specifially the alt key so it doesn't get
// left on hold after overlay is activated
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

#pragma endregion