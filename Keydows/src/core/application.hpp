#pragma once

#include <algorithm>

#include "hotkeys.hpp"

class Application
{
private:
    bool running = true;
    bool reload = false;
    const int MOVE_SPEED = 15;
    const int MOVE_DURATION = 25;
    const int MOVE_MARGIN = 200; // pixels
  
public:
    void run()
    {
        register_hotkeys();

        MSG msg;
        while (running)
        {
            if (GetMessage(&msg, NULL, 0, 0))
            {
                handle_message(&msg);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        unregister_hotkeys();
    }

private:
    void handle_message(const MSG *msg)
    {
        if (msg->message == WM_HOTKEY)
        {
            static int holdables[] = {LEFT, RIGHT, UP, DOWN};
            int held_key = -1;

            for (int key : holdables)
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
                // If the high-order bit is 1, the key is down; otherwise, it is up.
                //
                if (GetKeyState(key) & 0x8000) // & 0x8000)
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

            else while (GetKeyState(held_key) & 0x8000)
            {
                // If new hotkey is activated switch to that
                //
                MSG new_msg;
                if (PeekMessage(&new_msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (new_msg.message != msg->message)
                    {
                        break;
                    }
                }

                handle_hotkey(msg->wParam);
                Sleep(MOVE_DURATION);
            }
        }
    }

    void handle_hotkey(WPARAM wParam, BOOL bRepaint=TRUE)
    {
        if (wParam == CLOSE)
        {
            running = false;
            return;
        }

        if (wParam == RELOAD)
        {
            running = false;
            return;
        }

        HWND hwnd = GetForegroundWindow();
        if (hwnd)
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            int x = rect.left;
            int y = rect.top;
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            x -= MOVE_SPEED * (wParam == MOVE_LEFT);
            x += MOVE_SPEED * (wParam == MOVE_RIGHT);
            y -= MOVE_SPEED * (wParam == MOVE_UP);
            y += MOVE_SPEED * (wParam == MOVE_DOWN);
            
            width  -= MOVE_SPEED * (wParam == RESZ_LEFT);
            width  += MOVE_SPEED * (wParam == RESZ_RIGHT);
            height -= MOVE_SPEED * (wParam == RESZ_UP);
            height += MOVE_SPEED * (wParam == RESZ_DOWN);

            static const int max_width = GetSystemMetrics(SM_CXSCREEN);
            static const int max_height = GetSystemMetrics(SM_CYSCREEN);

            int max_x = max_width - width;
            int max_y = max_height - height;

            x = std::clamp(x, -MOVE_MARGIN, max_x + MOVE_MARGIN);
            y = std::clamp(y, -MOVE_MARGIN, max_y + MOVE_MARGIN);

            MoveWindow(hwnd, x, y, width, height, bRepaint);
        }
    }
};