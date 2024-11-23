#include "hotkeys.hpp"

#define MOVE_SPEED 25

bool running = true;

enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

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
        
        width -= MOVE_SPEED * (wParam == RESZ_LEFT);
        width += MOVE_SPEED * (wParam == RESZ_RIGHT);
        height -= MOVE_SPEED * (wParam == RESZ_UP);
        height += MOVE_SPEED * (wParam == RESZ_DOWN);

        MoveWindow(hwnd, x, y, width, height, TRUE);
    }
}


void run()
{
    HotKeyManager hk_manager;

    MSG msg;
    while (running)
    {
        if (GetMessage(&msg, NULL, 0, 0) && msg.message == WM_HOTKEY)
        {
            handle_hotkey(msg);
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}