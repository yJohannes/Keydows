#pragma once

#include <windows.h>
#include <iostream>
#include <unordered_map>

namespace hotkey
{
    // Simple unique hotkey ID 
    extern unsigned int register_counter;
    unsigned int register_hotkey(HWND h_wnd, int hk_id, int mod, int vk);
    void unregister_key(HWND h_wnd, int hk_id);
    void unregister_all_hotkeys(HWND h_wnd);
};