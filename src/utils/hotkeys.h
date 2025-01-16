#pragma once

#include <windows.h>
#include <iostream>

namespace hotkey
{
    extern unsigned int registered_key_count;

    enum HotKeyID
    {
        CLOSE,
        OVERLAY,
    };

    void register_key(HWND h_wnd, int hk_id, int mod, int vk);
    void unregister_hotkeys(HWND h_wnd);
};