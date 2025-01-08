#include "windowsdefs.h"
#include "application.h"

WNDPROC Application::m_old_wnd_proc = nullptr;
HWND  Application::m_hwnd;
HHOOK Application::m_keyboard_hook;
bool Application::m_listening = false;
bool Application::m_shift_down = false;

Application::Application(HINSTANCE h_instance)
{
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    m_display_w = ::GetSystemMetrics(SM_CXSCREEN);
    m_display_h = ::GetSystemMetrics(SM_CYSCREEN);

    m_cell_w = m_display_w / m_num_cells_x;
    m_cell_h = m_display_h / m_num_cells_y;

    // Window creation
    {
        const wchar_t class_name[] = L"Keydows";
        WNDCLASSEXW wcex = {0};
        wcex.cbSize = sizeof(wcex);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = wnd_proc_wrapper;
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
            m_display_w, m_display_h,
            NULL, NULL,
            h_instance,
            this
        );

        // Store the Application instance in the window's user data field
        ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    // Keyboard hook
    // if (false)
    {
        m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
        if (!m_keyboard_hook)
            std::cerr << "Failed to install keyboard hook!" << std::endl;
    }

    HotKey::register_key(m_hwnd, HotKey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
    HotKey::register_key(m_hwnd, HotKey::OVERLAY, MOD_ALT, VK_OEM_PERIOD);

    ::SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);
    show_window(true);
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


void Application::handle_keydown(WPARAM w_param, LPARAM l_param) // l_param may contain press count !!
{
    std::cout << "VK pressed:\t" << w_param << "\n";

    switch (w_param) {
    case VK_F4:
        ::DestroyWindow(m_hwnd);
        return;
    case VK_ESCAPE:
        if (m_listening)
        {
            show_window(false);
        }
        return;

    // Remove a typed key
    case VK_BACK:
        if (m_listening)
        {
            if (m_inchar2) m_inchar2 = 0;
            else if (m_inchar1) m_inchar1 = 0;
        }
        return;

    // Go above windows tray (maybe)
    case VK_RWIN:
    case VK_LWIN:
        show_window(true);
    }

    if (!m_listening)
        return;

    wchar_t key_char = towupper(get_key_char(w_param, l_param));
    std::cout << "VK char:\t" << (char)key_char << "\n";

    if (is_null_char(key_char))
        return;
    
    // Log keystrokes and in the end handle input characters
    if (!m_inchar1)
    {
        m_inchar1 = key_char;
    }
    else if (!m_inchar2)
    {
        m_inchar2 = key_char;
    }
    else if (m_inchar1 && m_inchar2) // KEEP IN MIND w_param == WM_KEYDOWN
    {
        int id1 = get_char_index(m_inchar1);
        int id2 = get_char_index(m_inchar2);

        LONG x, y;
        char_id_to_coordinates(id1, id2, &x, &y);

        x -= m_cell_w / 2 * (w_param == 'A' || w_param == 'Q' || w_param == 'X');
        x += m_cell_w / 2 * (w_param == 'D' || w_param == 'E' || w_param == 'C');

        y -= m_cell_h / 2 * (w_param == 'W' || w_param == 'Q' || w_param == 'E');
        y += m_cell_h / 2 * (w_param == 'S' || w_param == 'X' || w_param == 'C');

        if (is_valid_coordinate(x, y)) 
        {
            // HWND h_fg = ::GetForegroundWindow();
            // block_window(h_fg, FALSE);

            std::cout << "Clicked, shift down: " << m_shift_down << "\n";
            click_at(x, y, m_shift_down);
        }

        // Reset pressed chars (will be changed with multi-click being added)
        m_inchar1 = 0;
        m_inchar2 = 0;
    }
}

void Application::handle_hotkey(WPARAM w_param)
{
    switch (w_param) {
    case HotKey::CLOSE:
        ::DestroyWindow(m_hwnd); // Send WM_DESTROY message
        break;
    case HotKey::OVERLAY:
        show_window(!::IsWindowVisible(m_hwnd));
        break;
    default:
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
            // ::LineTo(h_mem_dc, x-1, m_display_h);

            // Draw horizontal lines
            // ::MoveToEx(h_mem_dc, 0, y-1, NULL);
            // ::LineTo(h_mem_dc, m_display_w, y-1);

    HDC h_mem_dc = ::CreateCompatibleDC(h_DC);
    HBITMAP h_mem_bitmap = ::CreateCompatibleBitmap(h_DC, rc.right, rc.bottom);
    HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_mem_dc, h_mem_bitmap);
    ::SetBkMode(h_mem_dc, OPAQUE);     // OPAQUE, TRANSPARENT

    ::SelectObject(h_mem_dc, h_pen);
    ::SelectObject(h_mem_dc, h_font);

    LONG selx, sely;
    chars_to_coordinates(m_inchar1, m_inchar2, &selx, &sely);

    for (LONG x = 0; x < m_display_w; x += m_cell_w) {
        for (LONG y = 0; y < m_display_h; y += m_cell_h)
        {
            // Selected row & col
            if (x == selx - m_cell_w / 2 || y == sely - m_cell_h / 2)
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
                m_chars[x / m_cell_w % wcslen(m_chars)],
                m_chars[y / m_cell_h % wcslen(m_chars)],
                L'\0'
            };
            
            RECT text_rect = { x, y, x + m_cell_w, y + m_cell_h };
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

void Application::show_window(bool show)
{
    static auto hwnd_title = [](HWND hwnd) -> std::string {
        const int bufferSize = 256;
        wchar_t windowTitle[bufferSize];

        if (GetWindowTextW(hwnd, windowTitle, bufferSize)) {
            auto wideStr = std::wstring(windowTitle);
            return std::string(wideStr.begin(), wideStr.end());
        }
        return "Failed to get window title.";
    };

    if (show)
    {

        HWND h_fg = ::GetForegroundWindow();
        std::cout << "hwnd: " << m_hwnd  << " " << hwnd_title(m_hwnd) << "\n "
            << "foreground: " << h_fg    << " " << hwnd_title(h_fg)   << "\n"
        ;

        m_listening = true;
 
        // Because the window never has focus, it can't receive keydown events; only uses global keyboard hook
        ::ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        // ::SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        m_listening = false;
        m_inchar1 = 0; // Reset input
        m_inchar2 = 0;

        // test to repaint before showing so that the old bitmap is not shown
        // There is a better way though
        force_repaint(m_hwnd);
        ::ShowWindow(m_hwnd, SW_HIDE);
    }
}

void Application::force_repaint(HWND h_wnd) const
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
    BYTE keyboardState[256];   // Array that represents the keyboard state
    ::GetKeyboardState(keyboardState);

    UINT scan_code = (l_param >> 16) & 0xFF;
    UINT virtualKey = (UINT)w_param;
    
    wchar_t ch = 0;

    // ToAscii converts the virtual key code into a character
    if (::ToAscii(virtualKey, scan_code, keyboardState, (LPWORD)&ch, 0) == 1) {
        return ch;
    }

    return 0; // null if no character was generated
}

// Returns -1 for invalid char id (-1)
void Application::char_id_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out) const
{
    *x_out = char_id1 * m_cell_w + m_cell_w / 2;
    *y_out = char_id2 * m_cell_h + m_cell_h / 2;

    if (char_id1 == -1) *x_out = -1;
    if (char_id2 == -1) *y_out = -1;
}

// Returns -1 for characters not in char list
void Application::chars_to_coordinates(wchar_t c1, wchar_t c2, LONG* x_out, LONG* y_out) const
{
    int id1 = get_char_index(c1);
    int id2 = get_char_index(c2);
    char_id_to_coordinates(id1, id2, x_out, y_out);
}

#pragma region WND & KB

LRESULT CALLBACK Application::wnd_proc_wrapper(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    Application* app = reinterpret_cast<Application*>(::GetWindowLongPtr(h_wnd, GWLP_USERDATA));  // Get the Application instance

    // Default handling
    if (!app)
    {
        return ::DefWindowProc(h_wnd, message, w_param, l_param);
    }
    
    // Dispatch messages to the window proc
    return app->wnd_proc(h_wnd, message, w_param, l_param);
}

LRESULT CALLBACK Application::wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
    case WM_HOTKEY: // Top priority
        handle_hotkey(w_param);
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        handle_keydown(w_param, l_param);
        force_repaint(h_wnd);
        return 0;

    case WM_KEYUP:
        if (w_param == VK_SPACE && m_listening)
            return 0;  // Do nothing if not input both

    case WM_PAINT:
        paint_event(h_wnd);
        return 0;

    case WM_KILLFOCUS:
        show_window(false);

    case WM_NCHITTEST:
        return HTCAPTION;

    case WM_DESTROY:
        destroy_proc();
        return 0;

    default:
        break;
    }
    return ::DefWindowProc(h_wnd, message, w_param, l_param);
}

// Posts keyboard event messages
LRESULT CALLBACK Application::keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    // w_param contains event type
    // l_param contains event data

    if (m_listening && n_code >= 0)
    {
        KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;

        WPARAM vk_code = p_keydata->vkCode;         // New w_param
        LPARAM out_key_data = (LPARAM)p_keydata;    // New l_param

        #define IS_SHIFT(vk) ((vk == VK_SHIFT) || (vk == VK_LSHIFT) | (vk == VK_RSHIFT))

        std::cout
        << "Key caught:\t" << vk_code << " char: " << (char)vk_code << "\n"
        << "Shift vk:\t" << VK_SHIFT << "\n"
        << "m_listening:\t" << m_listening << "\n"
        << "shift before:\t" << m_shift_down << "\n\n"
        << "m_shift_down:\t" << ((w_param == WM_KEYDOWN) && IS_SHIFT(vk_code)) << "\n\n"
        ;

        switch (w_param) {
        case WM_KEYDOWN:
            if (IS_SHIFT(vk_code)) m_shift_down = true;
        case WM_KEYUP:
            if (IS_SHIFT(vk_code)) m_shift_down = false;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            ::PostMessage(m_hwnd, (UINT)w_param, vk_code, out_key_data);
            return 1;  // Block the key input for further receivers
        }
    }
    return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
}

void Application::destroy_proc() const
{
        HotKey::unregister_hotkeys(m_hwnd);
        ::UnhookWindowsHookEx(m_keyboard_hook);
        ::PostQuitMessage(0);
}

#pragma endregion