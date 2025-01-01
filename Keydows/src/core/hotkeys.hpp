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
