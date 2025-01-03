#include "hotkeys.h"

namespace HotKey
{
    unsigned int registered_key_count = 0; // Define the static variable here

    void register_key(HWND h_wnd, int hk_id, int mod, int vk)
    {
        if (!::RegisterHotKey(h_wnd, hk_id, mod, vk))
        {
            std::cerr << "Failed to register hotkey 0x"
            << std::hex << std::uppercase << vk
            << "!" << std::endl;
            throw std::runtime_error("Hotkey registration failed");
        }
        registered_key_count++;
    }

    void unregister_hotkeys(HWND h_wnd)
    {
        for (int hk_id = 0; hk_id < registered_key_count; ++hk_id)
        {
            if (!::UnregisterHotKey(h_wnd, hk_id))
            {
                std::cerr << "Failed to unregister hotkey ID " << hk_id << "!" << std::endl;
                throw std::runtime_error("Hotkey unregistration failed");
            }
        }
        registered_key_count = 0;
    }
} // namespace HotKey