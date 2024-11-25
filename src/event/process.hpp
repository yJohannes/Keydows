#include "hotkeys.hpp"

#define MOVE_SPEED 25

bool running = true;

void handle_hotkey(MSG &msg)
{
    if (msg.wParam == CLOSE)
    {
        running = false;
        return;
    }

    WPARAM wParam = msg.wParam;
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

        MoveWindow(hwnd, x, y, width, height, TRUE);
    }
}


void run()
{
    register_hotkeys();

    MSG msg;
    while (running)
    {

        HWND hwnd = GetForegroundWindow();
        if (!hwnd)
        {
            continue;
        }
        
        if (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_HOTKEY)
            {
                handle_hotkey(msg);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    unregister_hotkeys();
}