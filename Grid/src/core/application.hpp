// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <cwchar>

#include "hotkeys.hpp"

class Application
{
private:
    const int MOVE_SPEED = 15;
    const int MOVE_DURATION = 25;
    const int MOVE_MARGIN = 200; // pixels

    bool m_listening = false;
    wchar_t char1 = 0;
    wchar_t char2 = 0;
  
    HWND m_HWND;
    LONG DISPLAY_W;
    LONG DISPLAY_H;

    // LONG X_BLOCKS = 24;
    // LONG Y_BLOCKS = 18;


    LONG X_BLOCKS = 38;
    LONG Y_BLOCKS = 32;

    LONG BLOCK_W;
    LONG BLOCK_H;

    // static constexpr const wchar_t* m_chars = L"QWERTYUIOPASDFGHJKLZXCVBNM1234567890";
    static constexpr const wchar_t* m_chars = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";
    // static constexpr const wchar_t* m_chars = L"12345ABCDEFGHIJKLMNOPQRTSUVWXYZ67890,.-";
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

            // Create hWnd
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

        HKeys::register_key(m_HWND, HKeys::CLOSE, HKeys::WA, VK_F4);
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
    static LRESULT CALLBACK WndProcWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        Application* app = reinterpret_cast<Application*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));  // Get the Application instance
        
        // Default handling
        if (!app)
        {
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }
        
        // Dispatch messages to the window proc
        return app->WndProc(hWnd, message, wParam, lParam);
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_KEYDOWN:
            handle_keydown(wParam, lParam);
            return 0;

        case WM_KEYUP:
            if (wParam == VK_SPACE && m_listening)
            {
                return 0;
            }

        case WM_HOTKEY:
            handle_hotkey(wParam, 0);
            return 0;

        case WM_PAINT:
            paint_event(hWnd);
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
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    void close()
    {
        HKeys::unregister_hotkeys(m_HWND);
        ::PostQuitMessage(0); 
    }

    void handle_keydown(WPARAM wParam, LPARAM lParam) // lParam may contain press count !!
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            std::cout << m_listening;
            if (m_listening)
            {
                show_window(false);
            }
            return;

        case VK_BACK:
            if (m_listening)
            {
                if (char2) char2 = 0;
                else if (char1) char1 = 0; 
            }
            return;
        }
        
        wchar_t key_down = towupper(get_key_char(wParam, lParam));
        if (key_down == 0) return;

        std::cout << (char)key_down << "\n";

        if (char1 && char2)
        {
            int id1 = get_char_index(char1);
            int id2 = get_char_index(char2);

            if (id1 == -1 || id2 == -1) 
            {
                if (!id1) char1 = 0;
                if (!id2) char2 = 0;
                return;
            }

            LONG x, y;
            chars_to_coordinates(id1, id2, &x, &y);

            x -= BLOCK_W / 2 * (wParam == 'A' || wParam == 'Q' || wParam == 'X');
            x += BLOCK_W / 2 * (wParam == 'D' || wParam == 'E' || wParam == 'C');

            y -= BLOCK_H / 2 * (wParam == 'W' || wParam == 'Q' || wParam == 'E');
            y += BLOCK_H / 2 * (wParam == 'S' || wParam == 'X' || wParam == 'C');

            if (x < DISPLAY_W && y < DISPLAY_H) // xy cannot be negative
            {
                click_at(x, y);
            }

            // Reset pressed chars (will be changed with multi-click being added)
            char1 = 0;
            char2 = 0;
        }
        else if (!char1) char1 = key_down;
        else if (char1) char2 = key_down;

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

    void paint_event(HWND hWnd)
    {
        PAINTSTRUCT ps = {0};
        HDC hDC = ::BeginPaint(hWnd, &ps);
        RECT rc = {0};
        ::GetClientRect(hWnd, &rc);

        // Create a memory DC for off-screen rendering
        HDC hMemDC = ::CreateCompatibleDC(hDC);
        HBITMAP hMemBitmap = ::CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
        HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hMemBitmap);

        // Reuse pen and font instead of creating them on each paint
        static HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); // White color
        static HFONT hFont = ::CreateFont(
            20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial"
        );

        static HFONT hFont_border = ::CreateFont(
            36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial"
        );

        // Set the memory DC for text and line drawing
        ::SetTextColor(hMemDC, RGB(255, 255, 255));  // White text color
        ::SetBkMode(hMemDC, OPAQUE);            // OPAQUE, TRANSPARENT
        ::SetBkColor(hMemDC, RGB(1, 1, 1)); // Background color, 0 is invisible

        ::SelectObject(hMemDC, hPen);
        ::SelectObject(hMemDC, hFont);
        
        for (LONG x = 0; x < DISPLAY_W; x += BLOCK_W) {
            for (LONG y = 0; y < DISPLAY_H; y += BLOCK_H)
            {
                // // Draw vertical lines
                // ::MoveToEx(hMemDC, x-1, 0, NULL);
                // ::LineTo(hMemDC, x-1, DISPLAY_H);

                // // Draw horizontal lines
                // ::MoveToEx(hMemDC, 0, y-1, NULL);
                // ::LineTo(hMemDC, DISPLAY_W, y-1);

                wchar_t combination[3] = {
                    m_chars[x / BLOCK_W % wcslen(m_chars)],
                    m_chars[y / BLOCK_H % wcslen(m_chars)],
                    L'\0'
                };

                // Draw text at the center of each block
                RECT textRect = { x, y, x + BLOCK_W, y + BLOCK_H };
                ::DrawTextW(hMemDC, combination, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }

        // Copy the memory bitmap to the screen
        ::BitBlt(hDC, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

        // Cleanup
        ::SelectObject(hMemDC, hOldBitmap);
        ::DeleteObject(hMemBitmap);
        ::DeleteObject(hMemDC);

        ::EndPaint(hWnd, &ps);
    }

    void show_window(bool show)
    {
        if (show)
        {
            ::ShowWindow(m_HWND, SW_SHOWNORMAL); // SW_SHOWNOACTIVATE Doesn't get focus, SW_SHOWNORMAL gets focus
            ::SetForegroundWindow(m_HWND); // This forces focus anyway
            m_listening = true;
        }
        else
        {
            ::ShowWindow(m_HWND, SW_HIDE);
            m_listening = false;
            char1 = 0;
            char2 = 0;
        }
    }

    void chars_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out)
    {
        *x_out = char_id1 * BLOCK_W + BLOCK_W / 2;
        *y_out = char_id2 * BLOCK_H + BLOCK_H / 2;
    }

    int get_char_index(wchar_t c) {
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

    void click_at(int x, int y)
    {
        SetCursorPos(x, y);

        INPUT inputs[2] = {};
        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

        inputs[1].type = INPUT_MOUSE;
        inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

        SendInput(2, inputs, sizeof(INPUT));
    }

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
};






        /*
        PAINTSTRUCT ps = {0};
        HDC hDC = ::BeginPaint(hWnd, &ps);
        RECT rc = {0};
        ::GetClientRect(hWnd, &rc);

        static HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255));  // Blue color, 1px width
        HPEN hPenOld = (HPEN)::SelectObject(hDC, hPen);

        ::SetTextColor(hDC, RGB(255, 255, 255));  // White text color
        ::SetBkMode(hDC, TRANSPARENT);            // Transparent background

        // Create a custom font
        static HFONT hFont = ::CreateFont(
            20,                        // Height of the font in pixels (adjust to your needs)
            0,                         // Width of the font (0 = proportional width)
            0,                         // Escapement angle
            0,                         // Orientation angle
            FW_BOLD,                   // Font weight (FW_NORMAL or FW_BOLD)
            FALSE,                     // Italic attribute
            FALSE,                     // Underline attribute
            FALSE,                     // Strikeout attribute
            DEFAULT_CHARSET,           // Character set
            OUT_TT_PRECIS,             // Output precision
            CLIP_DEFAULT_PRECIS,       // Clipping precision
            DEFAULT_QUALITY,           // Output quality
            DEFAULT_PITCH,             // Pitch and family
            L"Arial"                   // Font face name
        );

        // Select the font into the device context
        HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);

        LONG X_BLOCKS = 24;
        LONG Y_BLOCKS = 18;

        LONG BLOCK_W = DISPLAY_W / X_BLOCKS;
        LONG BLOCK_H = DISPLAY_H / Y_BLOCKS;
        
        for (LONG x = 0; x < DISPLAY_W; x += BLOCK_W) {
            for (LONG y = 0; y < DISPLAY_H; y += BLOCK_H)
            {
                wchar_t combination[3] = {
                    chars[x / BLOCK_W % wcslen(chars)],
                    chars[y / BLOCK_H % wcslen(chars)],
                    L'\0'
                };

                ::MoveToEx(hDC, x-1, 0, NULL);
                ::LineTo(hDC, x-1, DISPLAY_H);

                ::MoveToEx(hDC, 0, y-1, NULL);
                ::LineTo(hDC, DISPLAY_W, y-1);

                // Draw text at the intersection or center of the cell
                RECT textRect = { x, y, x + BLOCK_W, y + BLOCK_H }; // Cell bounds
                ::DrawTextW(hDC, combination, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }

        ::SelectObject(hDC, hOldFont);
        // ::DeleteObject(hFont);

        // Clean up by selecting the old pen back
        ::SelectObject(hDC, hPenOld);
        // ::DeleteObject(hPen);

        ::EndPaint(hWnd, &ps);
        */