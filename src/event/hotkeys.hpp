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

class HotKeyManager
{
public:
    HotKeyManager()
    {
        register_hotkeys();
    }

    ~HotKeyManager()
    {
        unregister_hotkeys();
    }

private:
    int m_num_registered = 0;

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

    void register_key(int k_id, int mod, int vk)
    {
        if (!RegisterHotKey(NULL, k_id, mod, vk))
        {
            std::cerr << "Failed to register hotkey 0x"
            << std::hex << std::uppercase << vk
            << "!" << std::endl;
            return;
        }
        m_num_registered++;
    }

    void unregister_hotkeys()
    {
        for (int k_id = 0; k_id < m_num_registered; ++k_id)
        {
            UnregisterHotKey(NULL, k_id);
        }
    }


};