#include "hl_input.h"

void HLInput::move_cursor(int x, int y)
{
    ::SetCursorPos(x, y);
}

void HLInput::click(int n, int x, int y, bool right_click)
{
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[1].type = INPUT_MOUSE;

    DWORD down;
    DWORD up;
    if (right_click)
    {
        down = MOUSEEVENTF_RIGHTDOWN;
        up   = MOUSEEVENTF_RIGHTUP;
    }
    else
    {
        down = MOUSEEVENTF_LEFTDOWN;
        up   = MOUSEEVENTF_LEFTUP;
    }

    inputs[0].mi.dwFlags = down;
    inputs[1].mi.dwFlags = up;

    ::SetCursorPos(x, y);

    for (int i = 0; i < n; ++i)
    {
        // Send the inputs with a delay because some apps may not register them otherwise
        ::SendInput(1, &inputs[0], sizeof(INPUT));
        ::Sleep(25);
        ::SendInput(1, &inputs[1], sizeof(INPUT));
    }
}

void HLInput::click_async(int n, int x, int y, bool right_click)
{
    std::thread(&HLInput::click, n, x, y, right_click).detach();
}

void HLInput::scroll(double delta)
{
    int d = static_cast<int>(delta * 120);  // Scale by the standard delta unit

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (d > 0) ? MOUSEEVENTF_WHEEL : MOUSEEVENTF_WHEEL;
    input.mi.mouseData = d;

    ::SendInput(1, &input, sizeof(INPUT));
}


void HLInput::set_key(int vk_code, bool pressed)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk_code;
    input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;
    ::SendInput(1, &input, sizeof(INPUT));
}

bool HLInput::is_key_down(int virtual_key)
{
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
    // If the high-order bit is 1, the key is down; otherwise, it is up.
    //
    return ::GetAsyncKeyState(virtual_key) & 0x8000;
}