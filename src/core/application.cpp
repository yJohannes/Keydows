#include "windowsdefs.h"
#include "application.h"

HWND  Application::m_hwnd;
HHOOK Application::m_keyboard_hook;
HHOOK Application::m_mouse_hook;

LONG Application::m_display_width;
LONG Application::m_display_height;

LONG Application::m_block_width;
LONG Application::m_block_height;
LONG Application::m_horizontal_blocks = 28;
LONG Application::m_vertical_blocks = 22;

wchar_t Application::m_input_char_1 = NULL_CHAR;
wchar_t Application::m_input_char_2 = NULL_CHAR;
bool Application::m_overlay_active = false;

Application::Application(HINSTANCE h_instance)
{
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    m_display_width = ::GetSystemMetrics(SM_CXSCREEN);
    m_display_height = ::GetSystemMetrics(SM_CYSCREEN);

    m_block_width = m_display_width / m_horizontal_blocks;
    m_block_height = m_display_height / m_vertical_blocks;

    // Window creation
    {
        const wchar_t class_name[] = L"Keydows";
        WNDCLASSEXW wcex = {0};
        wcex.cbSize = sizeof(wcex);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = wnd_proc;
        wcex.hInstance      = h_instance;
        wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
        wcex.lpszClassName  = class_name;
        ::RegisterClassExW(&wcex);

        // Create window handler
        m_hwnd = ::CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to keypresses
            class_name,
            L"Keydows Overlay Window",
            WS_POPUP | WS_VISIBLE,
            0, 0,
            m_display_width, m_display_height,
            NULL, NULL,
            h_instance,
            this
        );
    }

    ::SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);
    HotKey::register_key(m_hwnd, HotKey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
    HotKey::register_key(m_hwnd, HotKey::OVERLAY, MOD_ALT, VK_OEM_PERIOD);

    show_overlay(false);
}

Application::~Application()
{
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

#pragma region Procedures, hooks
LRESULT CALLBACK Application::wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
    case WM_HOTKEY: // Top priority
        handle_hotkey(w_param);
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        handle_keydown(w_param, l_param);
        return 0;

    case WM_PAINT:
        paint_event(h_wnd);
        return 0;

    case WM_DESTROY:
        destroy_proc();
        return 0;

    default:
        return ::DefWindowProc(h_wnd, message, w_param, l_param);
    }
}

// Posts keyboard event messages
LRESULT CALLBACK Application::keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    // w_param contains event type
    // l_param contains event data

    if (m_overlay_active && n_code >= 0)
    {
        KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;

        WPARAM vk_code = p_keydata->vkCode;         // New w_param
        LPARAM out_key_data = (LPARAM)p_keydata;    // New l_param

        // Allow certain mod keys to pass
        if ((vk_code == VK_SHIFT) || (vk_code == VK_LSHIFT) || (vk_code == VK_RSHIFT) ||
            (vk_code == VK_LWIN) || (vk_code == VK_RWIN))
            return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);

        std::cout
        << "Key caught:\t" << vk_code << " char: " << (char)vk_code << "\n"
        << "m_overlay_active:\t" << m_overlay_active << "\n";

        switch (w_param) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            ::PostMessage(m_hwnd, (UINT)w_param, vk_code, out_key_data);
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
    HotKey::unregister_hotkeys(m_hwnd);
    ::PostQuitMessage(0);
}

void Application::attach_hooks()
{
    m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
    if (!m_keyboard_hook)
        std::cerr << "Failed to install keyboard hook!" << std::endl;

    m_mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, NULL, 0);
    if (!m_mouse_hook)
        std::cerr << "Failed to install mouse hook!" << std::endl;
}

void Application::detach_hooks()
{
    ::UnhookWindowsHookEx(m_keyboard_hook);
    ::UnhookWindowsHookEx(m_mouse_hook);
}
#pragma endregion

#pragma region Window handlers
void Application::handle_keydown(WPARAM w_param, LPARAM l_param) // l_param may contain press count !!
{
    std::cout << "VK pressed:\t" << w_param << "\n";

    switch (w_param) {
    case VK_F4:
        ::DestroyWindow(m_hwnd);
        return;

    case VK_ESCAPE:
        show_overlay(false);
        return;

    case VK_BACK:  // Remove a typed key
        if (m_overlay_active)
        {
            if (m_input_char_2)
                m_input_char_2 = NULL_CHAR;
            else if (m_input_char_1)
                m_input_char_1 = NULL_CHAR;
        }
        force_repaint(m_hwnd);
        return;

    case VK_RETURN:
        m_input_char_1 = NULL_CHAR;
        m_input_char_2 = NULL_CHAR;
        force_repaint(m_hwnd);
        return;
    }

    if (!m_overlay_active) // Redundant checks after unhooking?
        return;

    wchar_t key_char = towupper(get_key_char(w_param, l_param));
    std::cout << "-> VK char:\t" << (char)key_char << "\n";

    if (key_char == NULL_CHAR)
        return;

    // Accept only valid chars for input
    if (is_valid_char(key_char))
    {
        // Laske max char
        int max_horizontal_index = m_display_width / m_block_width;
        int max_vertical_index = m_display_height / m_block_height;

        std::cout
        << (get_char_index(m_input_char_1)) << "\n"
        << (get_char_index(m_input_char_2)) << "\nEND INDICES\n";
        
        // if (get_char_index(m_input_char_1) > max_horizontal_index)
        // {
        //     std::cout << "in1\n";
        //     return;
        // }
        // if (get_char_index(m_input_char_2) > max_vertical_index)
        // {
        //     std::cout << "in2\n";
        //     return;
        // }

        if (m_input_char_1 == NULL_CHAR)
        {
            m_input_char_1 = key_char;
            force_repaint(m_hwnd);  // Force repaint after to apply/remove highlights
            return;
        }
        else if (m_input_char_2 == NULL_CHAR)
        {
            m_input_char_2 = key_char;
            force_repaint(m_hwnd);
            return;
        }
    }

    // Any third key will trigger
    if (m_input_char_1 && m_input_char_2)
    {
        int id1 = get_char_index(m_input_char_1);
        int id2 = get_char_index(m_input_char_2);

        LONG x, y;
        char_ids_to_coordinates(id1, id2, &x, &y);

        x -= m_block_width / 2 * (w_param == 'A' || w_param == 'Q' || w_param == 'X');
        x += m_block_width / 2 * (w_param == 'D' || w_param == 'E' || w_param == 'C');

        y -= m_block_height / 2 * (w_param == 'W' || w_param == 'Q' || w_param == 'E');
        y += m_block_height / 2 * (w_param == 'S' || w_param == 'X' || w_param == 'C');

        // Reset pressed chars (will be changed with multi-click being added)
        m_input_char_1 = NULL_CHAR;
        m_input_char_2 = NULL_CHAR;

        if (is_valid_coordinate(x, y)) 
        {
            click_at(x, y, is_key_down(VK_SHIFT));
            show_overlay(false);
            force_repaint(m_hwnd);
        }
    }
}

void Application::handle_hotkey(WPARAM w_param)
{
    switch (w_param) {
    case HotKey::CLOSE:
        ::DestroyWindow(m_hwnd); // Send WM_DESTROY message
        break;
    case HotKey::OVERLAY:
        release_key(VK_MENU);
        show_overlay(!::IsWindowVisible(m_hwnd));
        break;
    }
}

void Application::paint_event(HWND h_wnd)
{
    // Get maximized window rect
    RECT rc = {0};
    ::GetClientRect(h_wnd, &rc);

    PAINTSTRUCT ps = {0};
    HDC h_DC = ::BeginPaint(h_wnd, &ps);

    static HPEN h_pen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    static HFONT h_font = ::CreateFont(
        20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial"
    );

            // Draw vertical lines
            // ::MoveToEx(h_mem_dc, x-1, 0, NULL);
            // ::LineTo(h_mem_dc, x-1, m_display_height);

            // Draw horizontal lines
            // ::MoveToEx(h_mem_dc, 0, y-1, NULL);
            // ::LineTo(h_mem_dc, m_display_width, y-1);

    HDC h_mem_dc = ::CreateCompatibleDC(h_DC);
    HBITMAP h_mem_bitmap = ::CreateCompatibleBitmap(h_DC, rc.right, rc.bottom);
    HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_mem_dc, h_mem_bitmap);
    ::SetBkMode(h_mem_dc, OPAQUE);     // OPAQUE, TRANSPARENT

    ::SelectObject(h_mem_dc, h_pen);
    ::SelectObject(h_mem_dc, h_font);

    LONG selx, sely;
    chars_to_coordinates(m_input_char_1, m_input_char_2, &selx, &sely);

    for (LONG x = 0; x < m_display_width; x += m_block_width) {
        for (LONG y = 0; y < m_display_height; y += m_block_height)
        {
            // Selected row & col
            if (x == selx - m_block_width / 2 || y == sely - m_block_height / 2)
            {
                ::SetBkColor(h_mem_dc, RGB(255, 255, 255));
                ::SetTextColor(h_mem_dc, RGB(1, 1, 1));
            }
            else
            {
                ::SetBkColor(h_mem_dc, RGB(1, 1, 1));            // True black rgb
                ::SetTextColor(h_mem_dc, RGB(255, 255, 255));
            }

            wchar_t cell_chars[3] = {
                m_chars[x / m_block_width % wcslen(m_chars)],
                m_chars[y / m_block_height % wcslen(m_chars)],
                L'\0'
            };
            
            RECT text_rect = { x, y, x + m_block_width, y + m_block_height };
            ::DrawTextW(h_mem_dc, cell_chars, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // Copy the memory bitmap to the screen
    ::BitBlt(h_DC, 0, 0, rc.right, rc.bottom, h_mem_dc, 0, 0, SRCCOPY);

    // Cleanup
    ::SelectObject(h_mem_dc, h_old_bitmap);
    ::DeleteObject(h_mem_bitmap);
    ::DeleteObject(h_mem_dc);
    ::EndPaint(h_wnd, &ps);
}

void Application::show_overlay(bool show)
{
    if (show)
    {
        m_overlay_active = true;
        attach_hooks();

        // Because the window never has focus, it can't receive keydown events; only uses global keyboard hook
        ::ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        m_overlay_active = false;
        detach_hooks();

        m_input_char_1 = NULL_CHAR; // Reset input
        m_input_char_2 = NULL_CHAR;

        // test to repaint before showing so that the old bitmap is not shown
        // There is a better way though
        force_repaint(m_hwnd);
        ::ShowWindow(m_hwnd, SW_HIDE);
    }
}

void Application::force_repaint(HWND h_wnd)
{
    // Force a repaint of the window by invalidating its client area
    ::InvalidateRect(h_wnd, NULL, FALSE);  // NULL means the entire client area, TRUE means erase background
    ::UpdateWindow(h_wnd);                 // Force the window to repaint immediately
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

void Application::release_key(int vk_code)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk_code;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    ::SendInput(1, &input, sizeof(INPUT));
}
#pragma endregion

#pragma region Helpers
// Returns -1 for characters not in char list
int Application::get_char_index(wchar_t c)
{
    for (int i = 0; i < wcslen(m_chars); ++i)
    {
        if (m_chars[i] == c) return i;
    }
    return -1; // Not found
}

bool Application::is_key_down(int virtual_key)
{
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
    // If the high-order bit is 1, the key is down; otherwise, it is up.
    //
    return ::GetAsyncKeyState(virtual_key) & 0x8000;
}

wchar_t Application::get_key_char(WPARAM w_param, LPARAM l_param)
{
    // This function will return the character corresponding to the key pressed
    BYTE kb_state[256];   // Array that represents the keyboard state
    ::GetKeyboardState(kb_state);

    UINT scan_code = (l_param >> 16) & 0xFF;
    UINT vk = (UINT)w_param;
    
    wchar_t ch = 0;
    if (::ToAscii(vk, scan_code, kb_state, (LPWORD)&ch, 0) == 1)
        return ch;
    
    return NULL_CHAR;
}

// Returns -1 for invalid char id (-1)
void Application::char_ids_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out)
{
    *x_out = char_id1 * m_block_width + m_block_width / 2;
    *y_out = char_id2 * m_block_height + m_block_height / 2;

    if (char_id1 == -1) *x_out = -1;
    if (char_id2 == -1) *y_out = -1;
}

// Returns -1 for characters not in char list
void Application::chars_to_coordinates(wchar_t c1, wchar_t c2, LONG* x_out, LONG* y_out)
{
    int id1 = get_char_index(c1);
    int id2 = get_char_index(c2);
    char_ids_to_coordinates(id1, id2, x_out, y_out);
}
#pragma endregion