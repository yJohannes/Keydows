#pragma once

#include "hotkeys.hpp"

struct Application
{
    bool running = true;
    const int MOVE_SPEED = 25;
    
    void handle_hotkey(WPARAM wParam, BOOL bRepaint=TRUE)
    {
        if (wParam == CLOSE)
        {
            running = false;
            return;
        }

        HWND hwnd = GetForegroundWindow();
        if (hwnd)
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            LONG x = rect.left;
            LONG y = rect.top;
            LONG width = rect.right - rect.left;
            LONG height = rect.bottom - rect.top;

            x -= MOVE_SPEED * (wParam == MOVE_LEFT);
            x += MOVE_SPEED * (wParam == MOVE_RIGHT);
            y -= MOVE_SPEED * (wParam == MOVE_UP);
            y += MOVE_SPEED * (wParam == MOVE_DOWN);
            
            width  -= MOVE_SPEED * (wParam == RESZ_LEFT);
            width  += MOVE_SPEED * (wParam == RESZ_RIGHT);
            height -= MOVE_SPEED * (wParam == RESZ_UP);
            height += MOVE_SPEED * (wParam == RESZ_DOWN);

            MoveWindow(hwnd, x, y, width, height, bRepaint);
        }
    }

    void run()
    {
        register_hotkeys();

        MSG msg;
        while (running)
        {
            if (!GetMessage(&msg, NULL, 0, 0))
            {
                continue;
            }

            if (msg.message == WM_HOTKEY)
            {
                int last_key = -1;
                static int keys[] = {VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN};

                for (int key : keys)
                {
                    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
                    // If the high-order bit is 1, the key is down; otherwise, it is up.
                    //
                    if (GetKeyState(key) & 0x8000)
                    {
                        last_key = key;
                        break;
                    }
                }

                while (GetKeyState(last_key) & 0x8000)
                {
                    handle_hotkey(msg.wParam);
                    Sleep(50);
                }
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        unregister_hotkeys();
    }
};