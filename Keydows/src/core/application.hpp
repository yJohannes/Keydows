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
- Key that removes input characters 
*/

class Application
{
private:
    const int MOVE_SPEED = 15;
    const int MOVE_DURATION = 25;
    const int MOVE_MARGIN = 200; // pixels

    bool m_listening = true;  // Opens as listening
    bool m_shift_down = false;

    HWND m_HWND;
    LONG DISPLAY_W;
    LONG DISPLAY_H;

    LONG BLOCK_W;
    LONG BLOCK_H;
    LONG X_BLOCKS = 20;
    LONG Y_BLOCKS = 16;

    wchar_t m_inchar1 = 0;
    wchar_t m_inchar2 = 0;
    
    static constexpr const wchar_t* m_chars = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";
public:
    Application(HINSTANCE hInstance, int nCmdShow)
    {
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

        DISPLAY_W = ::GetSystemMetrics(SM_CXSCREEN);
        DISPLAY_H = ::GetSystemMetrics(SM_CYSCREEN);

        BLOCK_W = DISPLAY_W / X_BLOCKS;
        BLOCK_H = DISPLAY_H / Y_BLOCKS;

        // Register program related stuff
        {
            const wchar_t class_name[] = L"Keydows";
            WNDCLASSEXW wcex = {0};
            wcex.cbSize = sizeof(wcex);
            wcex.style          = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc    = WndProcWrapper;
            wcex.hInstance      = hInstance;
            wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
            wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
            wcex.lpszClassName  = class_name;
            ::RegisterClassExW(&wcex);

            // Create h_wnd
            m_HWND = ::CreateWindowExW(
                WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to keypresses
                class_name,
                L"Overlay Window",
                WS_POPUP | WS_VISIBLE,
                0, 0,
                DISPLAY_W, DISPLAY_H,
                NULL, NULL,
                hInstance,
                this
            );

            // Store the Application instance in the window's user data field
            ::SetWindowLongPtr(m_HWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        }
        
        ::SetLayeredWindowAttributes(m_HWND, RGB(0, 0, 0), 220, LWA_ALPHA | LWA_COLORKEY);

        HKeys::register_key(m_HWND, HKeys::CLOSE, MOD_CONTROL | MOD_ALT, 'Q');
        HKeys::register_key(m_HWND, HKeys::OVERLAY, MOD_ALT, VK_OEM_PERIOD);

        ::ShowWindow(m_HWND, nCmdShow);
        ::UpdateWindow(m_HWND);
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

private:
    void handle_keydown(WPARAM wParam, LPARAM lParam) // lParam may contain press count !!
    {
        switch (wParam)
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
            return;
        }

        wchar_t key_char = towupper(get_key_char(wParam, lParam));
        if (is_null_char(key_char)) return;

        // Log keystrokes and in the end handle input
        if (!m_inchar1)
        {
            m_inchar1 = key_char;
        }
        else if (!m_inchar2)
        {
            m_inchar2 = key_char;
        }
        else if (m_inchar1 && m_inchar2)
        {
            int id1 = get_char_index(m_inchar1);
            int id2 = get_char_index(m_inchar2);

            LONG x, y;
            char_id_to_coordinates(id1, id2, &x, &y);

            x -= BLOCK_W / 2 * (wParam == 'A' || wParam == 'Q' || wParam == 'X');
            x += BLOCK_W / 2 * (wParam == 'D' || wParam == 'E' || wParam == 'C');

            y -= BLOCK_H / 2 * (wParam == 'W' || wParam == 'Q' || wParam == 'E');
            y += BLOCK_H / 2 * (wParam == 'S' || wParam == 'X' || wParam == 'C');

            if (is_valid_coordinate(x, y)) 
            {
                click_at(x, y, m_shift_down);
            }

            // Reset pressed chars (will be changed with multi-click being added)
            m_inchar1 = 0;
            m_inchar2 = 0;
        }
    }

    void handle_hotkey(WPARAM wParam, BOOL bRepaint=TRUE)
    {
        switch (wParam)
        {
        case HKeys::CLOSE:
            close();
            return;
        case HKeys::OVERLAY:
            show_window(!::IsWindowVisible(m_HWND));
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
                // ::MoveToEx(h_memDC, x-1, 0, NULL);
                // ::LineTo(h_memDC, x-1, DISPLAY_H);

                // Draw horizontal lines
                // ::MoveToEx(h_memDC, 0, y-1, NULL);
                // ::LineTo(h_memDC, DISPLAY_W, y-1);

        HDC h_memDC = ::CreateCompatibleDC(h_DC);
        HBITMAP h_mem_bitmap = ::CreateCompatibleBitmap(h_DC, rc.right, rc.bottom);
        HBITMAP h_old_bitmap = (HBITMAP)::SelectObject(h_memDC, h_mem_bitmap);

        // Set the memory DC for text and line drawing
        // ::SetTextColor(h_memDC, RGB(255, 255, 255));    // White text color
        ::SetBkMode(h_memDC, OPAQUE);                   // OPAQUE, TRANSPARENT

        ::SelectObject(h_memDC, h_pen);
        ::SelectObject(h_memDC, h_font);

        LONG selx, sely;
        chars_to_coordinates(m_inchar1, m_inchar2, &selx, &sely);

        for (LONG x = 0; x < DISPLAY_W; x += BLOCK_W) {
            for (LONG y = 0; y < DISPLAY_H; y += BLOCK_H)
            {
                // Selected row & col
                if (x == selx - BLOCK_W / 2 || y == sely - BLOCK_H / 2)
                {
                    ::SetBkColor(h_memDC, RGB(255, 255, 255));
                    ::SetTextColor(h_memDC, RGB(1, 1, 1));
                }
                else
                {
                    ::SetBkColor(h_memDC, RGB(1, 1, 1));            // True black rgb
                    ::SetTextColor(h_memDC, RGB(255, 255, 255));
                }

                wchar_t cell_chars[3] = {
                    m_chars[x / BLOCK_W % wcslen(m_chars)],
                    m_chars[y / BLOCK_H % wcslen(m_chars)],
                    L'\0'
                };
                
                RECT text_rect = { x, y, x + BLOCK_W, y + BLOCK_H };
                ::DrawTextW(h_memDC, cell_chars, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }

        // Copy the memory bitmap to the screen
        ::BitBlt(h_DC, 0, 0, rc.right, rc.bottom, h_memDC, 0, 0, SRCCOPY);

        // Cleanup
        ::SelectObject(h_memDC, h_old_bitmap);
        ::DeleteObject(h_mem_bitmap);
        ::DeleteObject(h_memDC);
        ::EndPaint(h_wnd, &ps);
    }

    void show_window(bool show)
    {
        if (show)
        {
            m_listening = true;

            ::ShowWindow(m_HWND, SW_SHOWNORMAL); // SW_SHOWNOACTIVATE Doesn't get focus, SW_SHOWNORMAL gets focus
            ::SetForegroundWindow(m_HWND); // This forces focus anyway
            // ::ShowWindow(m_HWND, SW_SHOWNOACTIVATE);
            // ::SetWindowPos(m_HWND, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        }
        else
        {
            m_listening = false;
            m_inchar1 = 0; // Reset input
            m_inchar2 = 0;

            force_window_repaint(m_HWND);
            ::ShowWindow(m_HWND, SW_HIDE);
        }
    }

    void close()
    {
        HKeys::unregister_hotkeys(m_HWND);
        ::PostQuitMessage(0); 
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

    wchar_t get_key_char(WPARAM wParam, LPARAM lParam)
    {
        // This function will return the character corresponding to the key pressed
        BYTE keyboardState[256];   // Array that represents the keyboard state
        ::GetKeyboardState(keyboardState);

        UINT scanCode = (lParam >> 16) & 0xFF;
        UINT virtualKey = (UINT)wParam;
        
        wchar_t ch = 0;

        // ToAscii converts the virtual key code into a character
        if (::ToAscii(virtualKey, scanCode, keyboardState, (LPWORD)&ch, 0) == 1) {
            return ch;
        }

        return 0; // null if no character was generated
    }


    // Returns -1 for invalid char id (-1)
    void char_id_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out)
    {
        *x_out = char_id1 * BLOCK_W + BLOCK_W / 2;
        *y_out = char_id2 * BLOCK_H + BLOCK_H / 2;

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
        return x >= 0 && x < DISPLAY_W && y >= 0 && y < DISPLAY_H;
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
    static LRESULT CALLBACK WndProcWrapper(HWND h_wnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Application* app = reinterpret_cast<Application*>(::GetWindowLongPtr(h_wnd, GWLP_USERDATA));  // Get the Application instance
        
        // Default handling
        if (!app)
        {
            return ::DefWindowProc(h_wnd, message, wParam, lParam);
        }
        
        // Dispatch messages to the window proc
        return app->WndProc(h_wnd, message, wParam, lParam);
    }

    LRESULT CALLBACK WndProc(HWND h_wnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_KEYDOWN:
            handle_keydown(wParam, lParam);
            force_window_repaint(h_wnd);
            return 0;

        case WM_KEYUP:
            if (wParam == VK_SPACE && m_listening)
            {
                return 0;  // Do nothing if not input both
            }
            else if (wParam == VK_SHIFT)
            {
                m_shift_down = false;
            }

        case WM_HOTKEY:
            handle_hotkey(wParam, 0);
            return 0;

        case WM_PAINT:
            paint_event(h_wnd);
            return 0;

        case WM_KILLFOCUS:
            show_window(false);

        case WM_NCHITTEST:
            return HTCAPTION;

        case WM_DESTROY:
            close();
            return 0;

        default:
            break;
        }
        return ::DefWindowProc(h_wnd, message, wParam, lParam);
    }
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

};

/*
    void handle_message(const MSG *msg)
    {
        if (msg->message == WM_HOTKEY)
        {
            static int holdables[1] = {};// {LEFT, RIGHT, UP, DOWN};
            int held_key = -1;

            for (int key : holdables)
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
                // If the high-order bit is 1, the key is down; otherwise, it is up.
                //
                if (::GetKeyState(key) & 0x8000) // & 0x8000)
                {
                    held_key = key;
                    break;
                }
            }

            if (held_key == -1)
            {
                // For keys other than those listed in `holdables`
                //
                handle_hotkey(msg->wParam);
            }

            else while (::GetKeyState(held_key) & 0x8000)
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

                handle_hotkey(msg->wParam);
                ::Sleep(MOVE_DURATION);
            }
        }
    }
*/