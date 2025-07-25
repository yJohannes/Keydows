#include "overlay.h"
#include "defines.h"

namespace overlay
{

Overlay::Overlay()
    : m_input_char_1(NULL_CHAR)
    , m_input_char_2(NULL_CHAR)
    , m_default_mem_dc(nullptr)
    , m_default_mem_bitmap(nullptr)
{
    const wchar_t class_name[] = L"Keydows Overlay";
    WNDCLASSEXW m_wcex = {0};
    m_wcex.cbSize         = sizeof(m_wcex);
    m_wcex.style          = CS_HREDRAW | CS_VREDRAW;
    m_wcex.lpfnWndProc    = wnd_proc;
    m_wcex.hInstance      = GetModuleHandle(NULL);
    m_wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
    m_wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    m_wcex.lpszClassName  = class_name;
    ::RegisterClassExW(&m_wcex);

    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    set_size(dm.dmPelsWidth, dm.dmPelsHeight);
    set_resolution(24, 18);

    m_hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to mouse press
        class_name,
        L"Keydows Overlay",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        dm.dmPelsWidth, dm.dmPelsHeight,
        NULL, NULL,
        m_wcex.hInstance,
        this
    );

    ::SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 200, LWA_ALPHA | LWA_COLORKEY);
    ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    m_keybinds = {
        {Event::HIDE,          VK_ESCAPE},
        {Event::REMOVE,        VK_BACK},
        {Event::CLEAR,         VK_RETURN},
        {Event::MOVE,          L'C'},
        {Event::DOUBLE_CLICK,  L'V'},
        {Event::TRIPLE_CLICK,  L'N'},
        {Event::QUAD_CLICK,    L'M'}
    };

    m_hotkeys[Event::ACTIVATE] = HotkeyManager::register_hotkey(
        m_hwnd, MOD_CONTROL, VK_OEM_PERIOD
    );
}

Overlay::~Overlay()
{
    delete_cached_default_overlay();
    ::UnregisterClassW(m_wcex.lpszClassName, m_wcex.hInstance);
}

int Overlay::run()
{
    MSG msg;
    while (::GetMessageW(&msg, m_hwnd, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

void Overlay::shutdown()
{
    HotkeyManager::unregister_hotkey(m_hwnd, m_hotkeys[Event::ACTIVATE]);
    ::PostQuitMessage(0);
}

void Overlay::toggle(bool on)
{
    static int keyboard_id;
    static int mouse_id;

    if (on)
    {
        keyboard_id = LLInput::register_listener(
            WH_KEYBOARD_LL,
            CREATE_LISTENER(keyboard_hook_listener)
        );

        mouse_id = LLInput::register_listener(
            WH_MOUSE_LL,
            CREATE_LISTENER(mouse_hook_listener)
        );

        show_overlay(true);
    }
    else
    {
        clear_input();
        LLInput::unregister_listener(WH_KEYBOARD_LL, keyboard_id);
        LLInput::unregister_listener(WH_MOUSE_LL, mouse_id);
        show_overlay(false);
    }
}

void Overlay::render(HWND h_wnd)
{    
    bool use_default_overlay = (m_input_char_1 == NULL_CHAR);

    // Get maximized window rect
    RECT rect = {0};
    ::GetClientRect(h_wnd, &rect);

    PAINTSTRUCT ps = {0};
    HDC h_dc = ::BeginPaint(h_wnd, &ps);   // Begin painting on overlay
    HDC h_mem_dc = ::CreateCompatibleDC(h_dc);
    ::SetBkMode(h_mem_dc, OPAQUE);

    // Check if default overlay is still valid
    if (m_default_mem_dc)
    {
        BITMAP def = {0};
        ::GetObject(m_default_mem_bitmap, sizeof(BITMAP), &def);
        if (def.bmWidth != rect.right - rect.left || def.bmHeight != rect.bottom - rect.top)
        {
            delete_cached_default_overlay();
        }
    }

    // Initialize default overlay
    if (m_default_mem_dc == nullptr)
    {
        m_default_mem_dc = ::CreateCompatibleDC(h_dc);
        m_default_mem_bitmap = ::CreateCompatibleBitmap(h_dc, rect.right, rect.bottom);
        ::SelectObject(m_default_mem_dc, m_default_mem_bitmap);
        render_overlay_bitmap(m_default_mem_dc);
    }

    if (use_default_overlay)
    {
        // Copy pre-rendered bitmap to the screen 
        ::BitBlt(h_dc, 0, 0, rect.right, rect.bottom, m_default_mem_dc, 0, 0, SRCCOPY);
    }
    else
    {
        HBITMAP h_mem_bitmap = ::CreateCompatibleBitmap(h_dc, rect.right, rect.bottom);
        HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_mem_dc, h_mem_bitmap);
        render_overlay_bitmap(h_mem_dc);

        ::BitBlt(h_dc, 0, 0, rect.right, rect.bottom, h_mem_dc, 0, 0, SRCCOPY);
        ::SelectObject(h_mem_dc, h_old_bitmap);
        ::DeleteObject(h_mem_bitmap);
    }

    ::DeleteDC(h_mem_dc);
    ::EndPaint(h_wnd, &ps);
}

void Overlay::repaint()
{
    ::InvalidateRect(m_hwnd, NULL, FALSE);  // NULL means the entire client area, TRUE means erase background
    ::UpdateWindow(m_hwnd);                 // Post WM_PAINT event
}

void Overlay::show_overlay(bool show)
{
    if (show)
    {
        // Because the window never has focus, it can't receive keydown events
        ::ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        // Repaint so that the old bitmap is not shown
        repaint();
        ::ShowWindow(m_hwnd, SW_HIDE);
    }
}

// Returns bool whether to block the key input for further receivers
// (overlay processes the input)
bool CALLBACK Overlay::keyboard_hook_listener(WPARAM w_param, LPARAM l_param)
{
    // w_param contains event type
    // l_param contains event data        
    KBDLLHOOKSTRUCT* keydata = (KBDLLHOOKSTRUCT*)l_param;
    WPARAM key = keydata->vkCode;
    LPARAM press_type = w_param;

#ifdef OVERLAY_DEBUG
    std::cout << "VK pressed:  " << key << "\n";
    std::cout << "-> VK char:  " << (char)key << "\n";
#endif
    // Allow certain mod keys to pass, shift for right click
    switch (key) {
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return false;
    }

    // Ignore keydowns, only keyups get processed.
    switch (press_type) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        return true;
    }

    auto& kb = m_keybinds;

    // Special keys
    if (key == kb[Event::HIDE])
    {
        toggle(false);
        return true;
    }
    else if (key == kb[Event::REMOVE])
    {
        undo_input();
        repaint();
        return true;        
    }
    else if (key == kb[Event::CLEAR])
    {
        clear_input();
        repaint();
        return true;
    }

    // Finally process keyups
    switch (press_type) {
    case WM_KEYUP:
    case WM_SYSKEYUP:
        process_key(key, l_param);
        return true;
    }
    
    return false;
}

// Make determine whether click was synthetic or no
bool CALLBACK Overlay::mouse_hook_listener(WPARAM w_param, LPARAM l_param)
{
    if (w_param == WM_LBUTTONDOWN || w_param == WM_RBUTTONDOWN)
    {
        toggle(false);
    }

    return false;
}

/// @brief Expects capitalized letters
/// @param input_char 
/// @return number of clicks or -1 for moving mouse only
int Overlay::enter_input(wchar_t c)
{
    // Any third key will trigger (maybe change to a separate function?)
    if (m_input_char_1 && m_input_char_2)
    {

        int id1 = get_char_index(m_input_char_1);
        int id2 = get_char_index(m_input_char_2);

        int x, y;
        char_ids_to_coordinates(id1, id2, &x, &y);
        apply_direction(c, &x, &y);
        m_click_pos.x = x;
        m_click_pos.y = y;

        clear_input();

        if (c == m_keybinds[Event::MOVE])   return INT32_MAX;
        if (c == m_keybinds[Event::DOUBLE_CLICK]) return 2;
        if (c == m_keybinds[Event::TRIPLE_CLICK]) return 3;
        if (c == m_keybinds[Event::QUAD_CLICK])   return 4;
        return 1;
    }

    // Finally log keystrokes
    if (is_valid_char(c))
    {
        int max_horizontal_index = m_size.cx / m_block_width;
        int max_vertical_index = m_size.cy / m_block_height;

        if (m_input_char_1 == NULL_CHAR && get_char_index(c) < max_horizontal_index)
        {
            m_input_char_1 = c;
            return -1;
        }
        
        if (m_input_char_2 == NULL_CHAR && get_char_index(c) < max_vertical_index)
        {
            m_input_char_2 = c;
            return -2;
        }
    }

    return 0;
}

void Overlay::undo_input()
{
    if (m_input_char_2)
    {
        m_input_char_2 = NULL_CHAR;
    }
    else if (m_input_char_1)
    {
        m_input_char_1 = NULL_CHAR;
    }
}

void Overlay::clear_input()
{
    m_input_char_1 = NULL_CHAR;
    m_input_char_2 = NULL_CHAR;
}

void Overlay::set_size(int x, int y)
{
    m_size.cx = x;
    m_size.cy = y;
}

void Overlay::set_resolution(int x, int y)
{
    m_resolution.cx = x;
    m_resolution.cy = y;

    m_block_width = m_size.cx / x;
    m_block_height = m_size.cy / y;
}

void Overlay::set_charset(const wchar_t* charset)
{
    m_charset = charset;
}

void Overlay::set_click_direction_charset(const wchar_t* charset)
{
    m_click_direction_charset = charset;
}

LRESULT Overlay::wnd_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    Overlay* self = reinterpret_cast<Overlay*>(::GetWindowLongPtr(h_wnd, GWLP_USERDATA));
    if (self) return self->handle_message(msg, w_param, l_param);

    return ::DefWindowProc(h_wnd, msg, w_param, l_param);
}

LRESULT Overlay::handle_message(UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg) {
    case WM_HOTKEY:
        if (w_param == m_hotkeys[Event::ACTIVATE])
        {
            // Prevent control from getting stuck. For whatever reason
            // right mod keys won't release. Make dynamic later.
            HLInput::set_key(VK_LCONTROL, false);
            toggle(!::IsWindowVisible(m_hwnd));
        }
        return 0;

    case WM_PAINT:
        render(m_hwnd);
        return 0;

    case WM_DESTROY:
        shutdown();
        return 0;

    default:
        return ::DefWindowProc(m_hwnd, msg, w_param, l_param);
    }
}

// Returns -1 for characters not in char list
int Overlay::get_char_index(wchar_t c) const
{
    for (int i = 0; i < m_charset.size(); ++i)
    {
        if (m_charset[i] == c) return i;
    }
    return -1; // Not found
}

// Returns -1 for invalid char id (-1)
void Overlay::char_ids_to_coordinates(int char_id1, int char_id2, int* x_out, int* y_out) const
{
    *x_out = char_id1 * m_block_width + m_block_width / 2;
    *y_out = char_id2 * m_block_height + m_block_height / 2;

    if (char_id1 == -1) *x_out = -1;
    if (char_id2 == -1) *y_out = -1;
}

// Returns -1 for characters not in char list
void Overlay::chars_to_coordinates(wchar_t c1, wchar_t c2, int* x_out, int* y_out) const
{
    int id1 = get_char_index(c1);
    int id2 = get_char_index(c2);
    char_ids_to_coordinates(id1, id2, x_out, y_out);
}


void Overlay::apply_direction(wchar_t c, int *x, int *y) const
{
    auto& d = m_click_direction_charset;
    *x -= m_block_width / 2 * (c == d[2] || c == d[4] || c == d[6]);
    *x += m_block_width / 2 * (c == d[3] || c == d[5] || c == d[7]);

    *y -= m_block_height / 2 * (c == d[0] || c == d[4] || c == d[5]);
    *y += m_block_height / 2 * (c == d[1] || c == d[6] || c == d[7]);
}

void Overlay::render_overlay_bitmap(HDC h_dc)
{
    static HPEN h_pen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    static HFONT h_font = ::CreateFont(
        20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial"
    );

    ::SelectObject(h_dc, h_pen);
    ::SelectObject(h_dc, h_font);

    int sel_x, sel_y;
    chars_to_coordinates(m_input_char_1, m_input_char_2, &sel_x, &sel_y);

    for (int x = 0; x < m_size.cx; x += m_block_width) {
        for (int y = 0; y < m_size.cy; y += m_block_height)
        {
            // Highlight selected row & col
            if (x == sel_x - m_block_width / 2 || y == sel_y - m_block_height / 2)
            {
                ::SetBkColor(h_dc, RGB(255, 255, 255));
                ::SetTextColor(h_dc, RGB(1, 1, 1));
            }
            else
            {
                ::SetBkColor(h_dc, RGB(1, 1, 1));   // True black rgb
                ::SetTextColor(h_dc, RGB(255, 255, 255));
            }

            wchar_t cell_chars[3] = {
                m_charset[x / m_block_width % m_charset.size()],
                m_charset[y / m_block_height % m_charset.size()],
                L'\0'
            };
            
            RECT text_rect = { x, y, x + m_block_width, y + m_block_height };
            ::DrawTextW(h_dc, cell_chars, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
}

void Overlay::delete_cached_default_overlay()
{
    if (m_default_mem_bitmap)
    {
        ::DeleteObject(m_default_mem_bitmap);
        m_default_mem_bitmap = nullptr;
    }

    if (m_default_mem_dc)
    {
        ::DeleteDC(m_default_mem_dc);
        m_default_mem_dc = nullptr;
    }
}

// Key events
void Overlay::process_key(WPARAM key, LPARAM details)
{
    // Map input virtual key to actual key (fails for certain keys like öäå for some reason)
    wchar_t uni_key = key;  // Fail-safe
    {
        UINT scan_code = (details >> 16) & 0xFF;
        BYTE kb_state[256];
        ::GetKeyboardState(kb_state);
        kb_state[VK_SHIFT] = 0;   // Remove shift

        ::ToUnicode(key, scan_code, kb_state, &uni_key, 1, 0);
        uni_key = towupper(uni_key);
    }

#ifdef OVERLAY_DEBUG
    std::cout << (char)uni_key << "\n";
#endif
    int result = enter_input(uni_key);
    if (result == -1 || result == -2)
    {
        repaint();  // Force repaint to update highlights
        return;
    }

    if (result == INT32_MAX)
    {

        HLInput::move_cursor(m_click_pos.x, m_click_pos.y);
        toggle(false);
        return;
    }

    if (result > 0)
    {
        HLInput::click_async(result, m_click_pos.x, m_click_pos.y, HLInput::keydown(VK_SHIFT));
        toggle(false);
        return;
    }

}

} // namespace overlay