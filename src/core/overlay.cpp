#include "overlay.h"
#include "defines.h"
#include "application.h"

Overlay::Overlay()
    : m_input_data({-1, -1})
    , m_input_char_1(NULL_CHAR)
    , m_input_char_2(NULL_CHAR)
    , m_default_mem_dc(nullptr)
    , m_default_mem_bitmap(nullptr)
{
}

Overlay::~Overlay()
{
    delete_cached_default_overlay();
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

void Overlay::set_charset(const wchar_t *charset)
{
    m_charset = charset;
}

void Overlay::set_click_direction_charset(const wchar_t *charset)
{
    m_click_direction_charset = charset;
}

// Returns bool whether to block the key input for further receivers
bool Overlay::keyboard_proc_receiver(int n_code, WPARAM w_param, LPARAM l_param)
{
    // w_param contains event type
    // l_param contains event data
    if (n_code >= 0)
    {
        KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;
        WPARAM vk_code = p_keydata->vkCode;

        switch (vk_code) {
        case VK_ESCAPE:
            activate(false);
            return true;

        case VK_BACK:  // Remove an input
            undo_input();
            Application::repaint();
            return true;

        case VK_RETURN: // Clear inputs
            clear_input();
            Application::repaint();
            return true;

        // Allow certain mod keys to pass, shift for right click
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_LWIN:
        case VK_RWIN:
            return false;
        }

        switch (w_param) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            ::PostMessage(Application::h_wnd, (UINT)w_param, vk_code, l_param);
            return true;  // Block the key input for further receivers
        }
    }
    
    return false;
}

void Overlay::activate(bool on)
{
    if (on)
    {
        Application::attach_hooks();
        Application::show_window(true);
    }
    else
    {
        clear_input();
        Application::detach_hooks();
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

// Expects capitalized letters 
int Overlay::enter_input(wchar_t in_char)
{

    // Accept only valid chars for input
    if (is_valid_char(in_char))
    {
        int max_horizontal_index = m_size.cx / m_block_width;
        int max_vertical_index = m_size.cy / m_block_height;

        if (m_input_char_1 == NULL_CHAR && get_char_index(in_char) < max_horizontal_index)
        {
            m_input_char_1 = in_char;
            return FIRST_INPUT;
        }
        
        if (m_input_char_2 == NULL_CHAR && get_char_index(in_char) < max_vertical_index)
        {
            m_input_char_2 = in_char;
            return SECOND_INPUT;
        }
    }

    if (m_input_char_1 && m_input_char_2 && in_char == VK_SPACE) // Should make VK codes to unicode as well?
    {

    }


    // Any third key will trigger (maybe change to a separate function?)
    if (m_input_char_1 && m_input_char_2)
    {
        int id1 = get_char_index(m_input_char_1);
        int id2 = get_char_index(m_input_char_2);


        int x, y;
        char_ids_to_coordinates(id1, id2, &x, &y);

        auto& d = m_click_direction_charset;

        x -= m_block_width / 2 * (in_char == d[2] || in_char == d[4] || in_char == d[6]);
        x += m_block_width / 2 * (in_char == d[3] || in_char == d[5] || in_char == d[7]);

        y -= m_block_height / 2 * (in_char == d[0] || in_char == d[4] || in_char == d[5]);
        y += m_block_height / 2 * (in_char == d[1] || in_char == d[6] || in_char == d[7]);

        // Reset pressed chars (will change with multi-click)
        clear_input();

        m_input_data.x = x;
        m_input_data.y = y;
        m_input_data.click_key = in_char;
        return CLICKED;
    }

    return NO_INPUT;
}

int Overlay::undo_input()
{
    if (m_input_char_2)
    {
        m_input_char_2 = NULL_CHAR;
        return FIRST_INPUT;
    }
    else if (m_input_char_1)
    {
        m_input_char_1 = NULL_CHAR;
        return NO_INPUT;
    }

    return NO_INPUT;
}

void Overlay::clear_input()
{
    m_input_char_1 = NULL_CHAR;
    m_input_char_2 = NULL_CHAR;
}

#pragma region Helpers

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
#pragma endregion