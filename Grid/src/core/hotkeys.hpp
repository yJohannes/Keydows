#pragma once

#include <windows.h>
#include <iostream>

class HKeys
{
private:
    static unsigned int registered_key_count;
public:
    enum HotKeyID
    {
        CLOSE,
        OVERLAY,
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

    static void register_key(HWND hWnd, int hk_id, int mod, int vk)
    {
        if (!::RegisterHotKey(hWnd, hk_id, mod, vk))
        {
            std::cerr << "Failed to register hotkey 0x"
            << std::hex << std::uppercase << vk
            << "!" << std::endl;
            throw std::runtime_error("Hotkey registration failed");

            return;
        }

        registered_key_count++;
    }

    // void register_hotkeys()
    // {
    //     register_key(CLOSE,  WA, VK_F4);
        
    //     register_key(MOVE_LEFT,  W, LEFT);
    //     register_key(MOVE_RIGHT, W, RIGHT);
    //     register_key(MOVE_UP,    W, UP);
    //     register_key(MOVE_DOWN,  W, DOWN);

    //     register_key(RESZ_LEFT,  WA, LEFT);
    //     register_key(RESZ_RIGHT, WA, RIGHT);
    //     register_key(RESZ_UP,    WA, UP);
    //     register_key(RESZ_DOWN,  WA, DOWN);
    // }

    static void unregister_hotkeys(HWND hWnd)
    {
        for (int hk_id = 0; hk_id < registered_key_count; ++hk_id)
        {
            if (!::UnregisterHotKey(hWnd, hk_id))
            {
                std::cerr << "Failed to unregister hotkey!" << std::endl;
                throw std::runtime_error("Hotkey unregistration failed");

            }
        }
    }
};

unsigned int HKeys::registered_key_count = 0;
