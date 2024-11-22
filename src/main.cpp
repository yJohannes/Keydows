#include <windows.h>

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#define MOD_CWA MOD_CONTROL | MOD_WIN | MOD_ALT
#define MOVE_SPEED 10

enum Hotkeys
{
    Close,
    Left,
    Right,
    Up,
    Down
};

int main()
{
    if (!RegisterHotKey(NULL, Hotkeys::Right, MOD_CWA, VK_RIGHT))
    {
        std::cerr << "Failed to register hotkey!" << std::endl;
    }

    if (!RegisterHotKey(NULL, Hotkeys::Close, MOD_CWA, VK_F4))
    {
        std::cerr << "Failed to register hotkey!" << std::endl;
    }


    MSG msg;
    while (true)
    {
        if (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_HOTKEY)
            {
                if (msg.wParam == Hotkeys::Close)
                {
                    break;
                }

                else if (msg.wParam == Hotkeys::Right)
                {
                    std::string window_name = "SpeedCrunch";

                    std::wstring wide_window_name(window_name.begin(), window_name.end());

                    HWND hwnd = FindWindowW(NULL, wide_window_name.c_str());
                    if (hwnd)
                    {
                        std::cout << "JOO";
                        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowrect
                        //
                        RECT rect;
                        GetWindowRect(hwnd, &rect);
                        LONG width = rect.right - rect.left;
                        LONG height = rect.bottom - rect.top;

                        MoveWindow(hwnd, rect.left - MOVE_SPEED, rect.top - MOVE_SPEED, width, height, TRUE);
                    }


                }
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnregisterHotKey(NULL, Hotkeys::Right);
    UnregisterHotKey(NULL, Hotkeys::Close);
    return 0;
}