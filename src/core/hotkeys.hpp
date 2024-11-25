#pragma once

#include <windows.h>
#include <iostream>

enum HotKeyID
{
    CLOSE,
    RELOAD,
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
    SWA  = MOD_SHIFT | MOD_WIN | MOD_ALT,
    SCW = MOD_SHIFT | MOD_CONTROL | MOD_WIN
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
    register_key(CLOSE, SCW, VK_F4);
    register_key(RELOAD, SCW, VK_F5);
    
    register_key(MOVE_LEFT,  SWA, VK_LEFT);
    register_key(MOVE_RIGHT, SWA, VK_RIGHT);
    register_key(MOVE_UP,    SWA, VK_UP);
    register_key(MOVE_DOWN,  SWA, VK_DOWN);

    register_key(RESZ_LEFT,  SCW, VK_LEFT);
    register_key(RESZ_RIGHT, SCW, VK_RIGHT);
    register_key(RESZ_UP,    SCW, VK_UP);
    register_key(RESZ_DOWN,  SCW, VK_DOWN);
}

void unregister_hotkeys()
{
    for (int hk_id = 0; hk_id < 9; ++hk_id)
    {
        UnregisterHotKey(NULL, hk_id);
    }
}