#include "hotkeys.h"

namespace hotkey
{
    unsigned int register_counter = 0;

    unsigned int register_hotkey(HWND h_wnd, int hk_id, int mod, int vk)
    {
        if (::RegisterHotKey(h_wnd, hk_id, mod, vk))
        {
            return register_counter++;
        }
        else
        {
            std::cerr << "Failed to register hotkey " << hk_id << "!" << std::endl;
            throw std::runtime_error("Hotkey registration failed");
            return UINT_MAX;
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

    void unregister_all_hotkeys(HWND h_wnd)
    {
        for (int hk_id = 0; hk_id < register_counter; ++hk_id)
        {
            unregister_key(h_wnd, hk_id);
        }
    }

} // namespace hotkey