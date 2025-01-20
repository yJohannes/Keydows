#include "hotkeys.h"

namespace hotkey
{
    void register_hotkey(HWND h_wnd, int hk_id, int mod, int vk)
    {
        if (!::RegisterHotKey(h_wnd, hk_id, mod, vk))
        {
            std::cerr << "Failed to register hotkey " << hk_id << "!" << std::endl;
            throw std::runtime_error("Hotkey registration failed");
        }
    }

    void unregister_key(HWND h_wnd, int hk_id)
    {
        if (!::UnregisterHotKey(h_wnd, hk_id))
        {
            std::cerr << "Failed to unregister hotkey ID " << hk_id << "!" << std::endl;
            throw std::runtime_error("Hotkey unregistration failed");
        }
    }
} // namespace hotkey