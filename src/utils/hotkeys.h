#pragma once
#include <windows.h>
#include <iostream>

namespace hotkey
{
    // Simple unique hotkey ID 
    void register_hotkey(HWND h_wnd, int hk_id, int mod, int vk);
    void unregister_key(HWND h_wnd, int hk_id);
};