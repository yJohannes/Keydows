#pragma once

#include <windows.h>
#include <iostream>


enum Directions
{
    LEFT  = VK_F9,
    RIGHT = VK_F12,
    UP    = VK_F10,
    DOWN  = VK_F11,
};

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
    W  = MOD_WIN,
    WA = MOD_WIN | MOD_ALT
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
    register_key(CLOSE,  WA, VK_F4);
    register_key(RELOAD, WA, VK_F5);
    
    register_key(MOVE_LEFT,  W, LEFT);
    register_key(MOVE_RIGHT, W, RIGHT);
    register_key(MOVE_UP,    W, UP);
    register_key(MOVE_DOWN,  W, DOWN);

    register_key(RESZ_LEFT,  WA, LEFT);
    register_key(RESZ_RIGHT, WA, RIGHT);
    register_key(RESZ_UP,    WA, UP);
    register_key(RESZ_DOWN,  WA, DOWN);
}

void unregister_hotkeys()
{
    for (int hk_id = 0; hk_id < 9; ++hk_id)
    {
        UnregisterHotKey(NULL, hk_id);
    }
}