#include "defines.h"
#include "application.h"

WNDCLASSEXW  Application::m_wcex;
HWND Application::m_hwnd = nullptr;
HHOOK Application::m_keyboard_hook = nullptr;
HHOOK Application::m_mouse_hook = nullptr;
Overlay Application::m_overlay;

/*
 * Creates the Keydows overlay application that uses a low-level
 * mouse and keyboard hook to catch keystrokes. The overlay never
 * obtains focus and therefore only relies on said hooks.
 */
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

    m_hwnd = ::CreateWindowExW(
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
    ::SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);
    hotkey::register_key(m_hwnd, hotkey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
    hotkey::register_key(m_hwnd, hotkey::OVERLAY, MOD_ALT, VK_OEM_PERIOD);

    m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
    m_overlay.set_resolution(28, 22);

    show_overlay(false);
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
    // w_param contains event type
    // l_param contains event data
    if (n_code >= 0)
    {
        KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;
        WPARAM vk_code = p_keydata->vkCode;

        // Allow certain mod keys to pass, shift for right click
        if (vk_code == VK_SHIFT || vk_code == VK_LSHIFT || vk_code == VK_RSHIFT ||
            vk_code == VK_LWIN || vk_code == VK_RWIN)
        {
            return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
        }

        switch (w_param) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            ::PostMessage(m_hwnd, (UINT)w_param, vk_code, l_param);
            return 1;  // Block the key input for further receivers
        }
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
            show_overlay(false);
        }
    }

    // Pass the input to further receivers
    return CallNextHookEx(m_mouse_hook, n_code, w_param, l_param);
}

void Application::destroy_proc()
{
    detach_hooks();
    hotkey::unregister_hotkeys(m_hwnd);
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
        ::DestroyWindow(m_hwnd);
        return;

    case VK_ESCAPE:
        show_overlay(false);
        return;

    case VK_BACK:  // Remove a typed key
        m_overlay.undo_input();
        force_repaint();
        return;

    case VK_RETURN:
        m_overlay.clear_input();
        force_repaint();
        return;
    }

    std::cout << "-> VK char:\t" << (char)key << "\n";

    // COULD TURN THIS INTO A VALIDATOR FOR ALSO THE GUI
    // Map input virtual key to actual key (fails for certain keys for some reason) 
    wchar_t uni_key = key;  // Fail-safe
    {
        BYTE kb_state[256];
        ::GetKeyboardState(kb_state);

        UINT scan_code = (details >> 16) & 0xFF;
        ::ToUnicode(key, scan_code, kb_state, &uni_key, 1, 0);
        uni_key = towupper(uni_key);  // Uppercase letters, also could use lowercase to detect shift. Just a thought
    }

    int result = m_overlay.enter_input(uni_key);
    switch (result)
    {
    case Overlay::FIRST_INPUT:
    case Overlay::SECOND_INPUT:
        force_repaint();    // Force repaint to update highlights
        break;
    case Overlay::TRIGGERED:
        int x = m_overlay.input_data()->x;
        int y = m_overlay.input_data()->y;
        click_at(x, y, is_key_down(VK_SHIFT));
        show_overlay(false);
        force_repaint();
        break;
    }
}

void Application::handle_hotkey(WPARAM w_param)
{
    switch (w_param) {
    case hotkey::CLOSE:
        ::DestroyWindow(m_hwnd); // Send WM_DESTROY message
        break;
    case hotkey::OVERLAY:
        release_key(VK_MENU);
        show_overlay(!::IsWindowVisible(m_hwnd));
        break;
    }
}

void Application::show_overlay(bool show)
{
    if (show)
    {
        attach_hooks();

        // Because the window never has focus, it can't receive keydown events; only uses global keyboard hook
        ::ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        detach_hooks();
        m_overlay.clear_input();

        // Repaint so that the old bitmap is not shown
        force_repaint();
        ::ShowWindow(m_hwnd, SW_HIDE);
    }
}

void Application::paint_event()
{
    m_overlay.render(m_hwnd);
}

void Application::force_repaint()
{
    // Force a repaint of the window by invalidating its client area
    ::InvalidateRect(m_hwnd, NULL, FALSE);  // NULL means the entire client area, TRUE means erase background
    ::UpdateWindow(m_hwnd);                 // Force the window to repaint immediately
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