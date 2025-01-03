// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <cwchar>

#include "hotkeys.hpp"

/*
TODO:
- Key that removes all input characters 
- Don't do all repaint rendering on show, use old DC 
- Start on run instead of what line 41 says
*/

class Application
{
private:
    static HWND m_hwnd;
    static HHOOK m_keyboard_hook;
    static bool m_keyboard_state[256];

    LONG m_display_w;
    LONG m_display_h;

    LONG m_cell_w;
    LONG m_cell_h;
    LONG m_num_cells_x = 20;
    LONG m_num_cells_y = 16;

    wchar_t m_inchar1 = 0;
    wchar_t m_inchar2 = 0;
    
    static constexpr const wchar_t* m_chars = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";

    bool m_listening = true;  // Opens as listening
    bool m_shift_down = false;
    
public:
    Application(HINSTANCE h_instance, int n_cmd_show)
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
        {
            m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
            if (!m_keyboard_hook)
                std::cerr << "Failed to install hook!" << std::endl;
        }

        HotKey::register_key(m_hwnd, HotKey::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
        HotKey::register_key(m_hwnd, HotKey::OVERLAY, MOD_ALT, VK_OEM_PERIOD);

        ::SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);
        ::ShowWindow(m_hwnd, n_cmd_show);
        ::UpdateWindow(m_hwnd);
    }

    int run()
    {
        MSG msg;
        while (::GetMessageW(&msg, NULL, 0, 0))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }

        return (int)msg.wParam;
    }

    void destroy_proc()
    {
            HotKey::unregister_hotkeys(m_hwnd);
            ::UnhookWindowsHookEx(m_keyboard_hook);
            ::PostQuitMessage(0);
    }

private:
    void handle_keydown(WPARAM w_param, LPARAM l_param) // l_param may contain press count !!
    {
        switch (w_param)
        {
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

        case VK_SHIFT:
            m_shift_down = true;
            std :: cout << "SHIFT\n";
            return;
        }

        std :: cout << "after switch\n";

        if (!m_listening)
            return;

        wchar_t key_char = towupper(get_key_char(w_param, l_param));
        if (is_null_char(key_char))
            return;

        std::cout << "Key pressed: " << (char)key_char << "\n"
        << "Shift down: " << m_shift_down << "\n";
        
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
                click_at(x, y, m_shift_down);
            }

            // Reset pressed chars (will be changed with multi-click being added)
            m_inchar1 = 0;
            m_inchar2 = 0;
        }
    }

    void handle_hotkey(WPARAM w_param)
    {
        switch (w_param)
        {
        case HotKey::CLOSE:
            ::DestroyWindow(m_hwnd); // Send WM_DESTROY message
            std::cout << "HOTKEY CLOSE TRIGGERED\n";
            return;
        case HotKey::OVERLAY:
            show_window(!::IsWindowVisible(m_hwnd));
            return;
        default:
            break;
        }
    }

    void paint_event(HWND h_wnd)
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

    void show_window(bool show)
    {
        if (show)
        {
            m_listening = true;
            // ::ShowWindow(m_hwnd, SW_SHOWNORMAL); // SW_SHOWNOACTIVATE Doesn't get focus, SW_SHOWNORMAL gets focus
            // ::SetForegroundWindow(m_hwnd); // This forces focus anyway

            // Because the window never has focus, it can't receive keydown events; only uses global keyboard hook
            ::ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
            ::SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else
        {
            m_listening = false;
            m_inchar1 = 0; // Reset input
            m_inchar2 = 0;

            force_window_repaint(m_hwnd);
            ::ShowWindow(m_hwnd, SW_HIDE);
        }
    }

    // Returns -1 for characters not in char list
    int get_char_index(wchar_t c) const
    {
        for (int i = 0; i < wcslen(m_chars); ++i)
        {
            if (m_chars[i] == c) return i;
        }
        return -1; // Not found
    }

    wchar_t get_key_char(WPARAM w_param, LPARAM l_param)
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
    void char_id_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out)
    {
        *x_out = char_id1 * m_cell_w + m_cell_w / 2;
        *y_out = char_id2 * m_cell_h + m_cell_h / 2;

        if (char_id1 == -1) *x_out = -1;
        if (char_id2 == -1) *y_out = -1;
    }

    // Returns -1 for characters not in char list
    void chars_to_coordinates(wchar_t c1, wchar_t c2, LONG* x_out, LONG* y_out)
    {
        int id1 = get_char_index(c1);
        int id2 = get_char_index(c2);
        char_id_to_coordinates(id1, id2, x_out, y_out);
    }



    bool is_valid_coordinate(LONG x, LONG y)
    {
        return x >= 0 && x < m_display_w && y >= 0 && y < m_display_h;
    }

    bool is_valid_char(wchar_t c)
    {
        return (get_char_index(c) != -1);
    }

    bool is_null_char(wchar_t c)
    {
        return c == L'\0';
    }

    void click_at(int x, int y, bool right_click=false) const
    {
        ::SetCursorPos(x, y);

        INPUT inputs[2] = {};
        inputs[0].type = INPUT_MOUSE;
        inputs[1].type = INPUT_MOUSE;

        if (right_click)
        {
            inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        }
        else
        {
            inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        }

        ::SendInput(2, inputs, sizeof(INPUT));
    }

    void force_window_repaint(HWND h_wnd)
    {
        BOOL erase = FALSE;
        // Force a repaint of the window by invalidating its client area
        ::InvalidateRect(h_wnd, NULL, erase);     // NULL means the entire client area, TRUE means erase background
        ::UpdateWindow(h_wnd);                   // Force the window to repaint immediately
}
    //// WINDOW PROCEDURE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    static LRESULT CALLBACK wnd_proc_wrapper(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
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

    LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
    {
        switch (message)
        {
        case WM_KEYDOWN:
            handle_keydown(w_param, l_param);
            force_window_repaint(h_wnd);
            return 0;

        case WM_KEYUP:
            if (w_param == VK_SPACE && m_listening)
            {
                return 0;  // Do nothing if not input both
            }
            else if (w_param == VK_SHIFT)
            {
                m_shift_down = false;
            }

        case WM_HOTKEY:
            handle_hotkey(w_param);
            return 0;

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
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param) {
        /* 
         * w_param contains event type
         * l_param contains event data
        */
        if (n_code >= 0 && w_param == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;

            WPARAM vk_code = p_keydata->vkCode;         // New w_param
            LPARAM out_key_data = (LPARAM)p_keydata;    // New l_param

            ::PostMessage(m_hwnd, WM_KEYDOWN, vk_code, out_key_data);
            return 1;  // Block the key input for the window that has focus
        }
        return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
    }

    // static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param) {
    //     if (n_code >= 0) {
    //         KBDLLHOOKSTRUCT* p_keydata = (KBDLLHOOKSTRUCT*)l_param;
    //         WPARAM vk_code = p_keydata->vkCode;

    //         // Check if the key is being pressed or released
    //         if (w_param == WM_KEYDOWN) {
    //             m_keyboard_state[vk_code] = true;  // Mark the key as pressed
    //         } else if (w_param == WM_KEYUP) {
    //             m_keyboard_state[vk_code] = false;  // Mark the key as released
    //         }

    //         // Post all currently pressed keys (you can filter for specific keys or use this logic)
    //         for (int i = 0; i < 256; ++i) {
    //             if (m_keyboard_state[i]) {
    //                 ::PostMessage(m_hwnd, WM_KEYDOWN, i, 0);  // Send keydown event for the currently pressed keys
    //             }
    //         }

    //         // Block the key input for the window that has focus
    //         return 1;
    //     }

    //     return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
    // }


    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

};

HWND  Application::m_hwnd;
HHOOK Application::m_keyboard_hook;
bool Application::m_keyboard_state[256] = { false };  // Static raw C array



/*
    void handle_message(const MSG *msg)
    {
        if (msg->message == WM_HOTKEY)
        {
            static int holdables[1] = {};// {LEFT, RIGHT, UP, DOWN};
            int held_key = -1;

            for (int key : holdables)
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getm_keyboard_state
                // If the high-order bit is 1, the key is down; otherwise, it is up.
                //
                if (::Getm_keyboard_state(key) & 0x8000) // & 0x8000)
                {
                    held_key = key;
                    break;
                }
            }

            if (held_key == -1)
            {
                // For keys other than those listed in `holdables`
                //
                handle_hotkey(msg->w_param);
            }

            else while (::Getm_keyboard_state(held_key) & 0x8000)
            {
                // If new hotkey is activated switch to that
                //
                MSG new_msg;
                if (::PeekMessage(&new_msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (new_msg.message != msg->message)
                    {
                        break;
                    }
                }

                handle_hotkey(msg->w_param);
                ::Sleep(MOVE_DURATION);
            }
        }
    }
*/