#include "hotkey_manager.h"

std::unordered_map<int, HWND> HotkeyManager::m_hotkey_map;

int HotkeyManager::register_hotkey(HWND h_wnd, int mod, int vk)
{
    int id = 0;

    for (const auto& kv : m_hotkey_map)
    {
        if (kv.first == id)
            id++;
        else
            break;
    }

    if (!::RegisterHotKey(h_wnd, id, mod, vk))
    {
        std::cerr << "Failed to register hotkey " << id << "!" << std::endl;
        throw std::runtime_error("Hotkey registration failed");
    }

    m_hotkey_map[id] = h_wnd;
    return id;
}

void HotkeyManager::unregister_hotkey(HWND h_wnd, int hotkey_id)
{
    if (!::UnregisterHotKey(h_wnd, hotkey_id))
    {
        std::cerr << "Failed to unregister hotkey ID " << hotkey_id << "!" << std::endl;
        throw std::runtime_error("Hotkey unregistration failed");
    }

    m_hotkey_map.erase(hotkey_id);
}

void HotkeyManager::unregister_all_hotkeys()
{
    for (const auto& kv : m_hotkey_map)
    {
        unregister_hotkey(kv.second, kv.first);
    }
}
