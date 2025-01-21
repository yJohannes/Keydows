#include "overlay.h"
#include "defines.h"
#include "application.h"

Overlay::Overlay()
    : m_input_char_1(NULL_CHAR)
    , m_input_char_2(NULL_CHAR)
    , m_default_mem_dc(nullptr)
    , m_default_mem_bitmap(nullptr)
{
    m_keybinds[Action::HIDE] = VK_ESCAPE;
    m_keybinds[Action::REMOVE_INPUT] = VK_BACK;
    m_keybinds[Action::CLEAR_INPUTS] = VK_RETURN;
    m_keybinds[Action::MOVE_MOUSE]   = L'C';
    m_keybinds[Action::DOUBLE_CLICK] = L'V';
    m_keybinds[Action::TRIPLE_CLICK] = L'N';
    m_keybinds[Action::QUAD_CLICK]   = L'M';
}

Overlay::~Overlay()
{
    delete_cached_default_overlay();
}

void Overlay::activate(bool on)
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

        Application::show_window(true);
    }
    else
    {
        clear_input();
        LLInput::unregister_listener(WH_KEYBOARD_LL, keyboard_id);
        LLInput::unregister_listener(WH_MOUSE_LL, mouse_id);
        Application::show_window(false);
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

// Returns bool whether to block the key input for further receivers
// (overlay processes the input)
bool CALLBACK Overlay::keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param)
{
    // w_param contains event type
    // l_param contains event data
    if (n_code < 0)
        return false;
        
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
    if (key == kb[Action::HIDE]) {
        activate(false);
        return true;
    } else if (key == kb[Action::REMOVE_INPUT]) {
        undo_input();
        Application::repaint();
        return true;        
    } else if (key == kb[Action::CLEAR_INPUTS]) {
        clear_input();
        Application::repaint();
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
bool CALLBACK Overlay::mouse_hook_listener(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code == HC_ACTION)
    {
        if ((w_param == WM_LBUTTONDOWN) || (w_param == WM_RBUTTONDOWN))
        {
            activate(false);
        }
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

        if (c == Action::MOVE_MOUSE)   return INT32_MAX;
        if (c == Action::DOUBLE_CLICK) return 2;
        if (c == Action::TRIPLE_CLICK) return 3;
        if (c == Action::QUAD_CLICK)   return 4;
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

#pragma endregion Setters & Getters
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
    x -= m_block_width / 2 * (c == d[2] || c == d[4] || c == d[6]);
    x += m_block_width / 2 * (c == d[3] || c == d[5] || c == d[7]);

    y -= m_block_height / 2 * (c == d[0] || c == d[4] || c == d[5]);
    y += m_block_height / 2 * (c == d[1] || c == d[6] || c == d[7]);
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

        Application::repaint();  // Force repaint to update highlights
        return;
    }

    if (result > 0)
    {

        Application::click_async(result, m_click_pos.x, m_click_pos.y, Application::is_key_down(VK_SHIFT));
        activate(false);
        return;
    }

    if (result == INT32_MAX)
    {

        Application::move_cursor(m_click_pos.x, m_click_pos.y);
        activate(false);
        return;
    }
}