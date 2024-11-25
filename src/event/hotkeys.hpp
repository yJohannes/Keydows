#pragma once

#include <windows.h>
#include <iostream>

enum HotKeyID
{
    CLOSE,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    RESZ_LEFT,
    RESZ_RIGHT,
    RESZ_UP,
    RESZ_DOWN
};

enum HotKeyMod
{
    WA  = MOD_WIN | MOD_ALT,
    CWA = MOD_CONTROL | MOD_WIN | MOD_ALT
};

void register_key(int hk_id, int mod, int vk)
{
    if (!RegisterHotKey(NULL, hk_id, mod, vk))
    {
        std::cerr << "Failed to register hotkey 0x"
        << std::hex << std::uppercase << vk
        << "!" << std::endl;
        return;
    }
}

void register_hotkeys()
{
    register_key(CLOSE, WA, VK_F4);
    
    register_key(MOVE_LEFT,  WA, VK_LEFT);
    register_key(MOVE_RIGHT, WA, VK_RIGHT);
    register_key(MOVE_UP,    WA, VK_UP);
    register_key(MOVE_DOWN,  WA, VK_DOWN);

    register_key(RESZ_LEFT,  CWA, VK_LEFT);
    register_key(RESZ_RIGHT, CWA, VK_RIGHT);
    register_key(RESZ_UP,    CWA, VK_UP);
    register_key(RESZ_DOWN,  CWA, VK_DOWN);
}

void unregister_hotkeys()
{
    for (int hk_id = 0; hk_id < 9; ++hk_id)
    {
        UnregisterHotKey(NULL, hk_id);
    }
}