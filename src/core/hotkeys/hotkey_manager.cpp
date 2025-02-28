#include "hotkey_manager.h"

std::unordered_map<int, HWND> HotkeyManager::m_hotkey_map;

// void f(void (*f)(void)) {(**(&f))();}

int HotkeyManager::register_hotkey(HWND h_wnd, int mod, int vk)
{
    int id = 1; // 0 is invalid

    for (const auto& kv : m_hotkey_map)
    {
        if (kv.first == id)
            id++;
        else
            break;
    }

    if (!::RegisterHotKey(h_wnd, id, mod, vk))
    {
        std::cerr << "Failed to register hotkey ID " << id << " with VK " << (char)vk << "!" << std::endl;
        throw std::runtime_error("Hotkey registration failed");
    }

    std::cout << "Registered hotkey ID " << id << " with VK " << (char)vk << " for hwnd " << h_wnd << ".\n";

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

// void HotkeyManager::invoke_hotkey_callback(WPARAM triggered_hotkey_id)
// {
//     std::cout << "ID: " << triggered_hotkey_id << " invoked\n";

//     if (auto it = m_hotkey_map.find(triggered_hotkey_id); it != m_hotkey_map.end())
//     {
//         auto& [_, callback] = it->second;
//         if (callback != nullptr)
//             callback(triggered_hotkey_id);
//     }
//     else
//     {
//         std::cerr << "Invalid hotkey ID " << triggered_hotkey_id << "!" << std::endl;
//     }
// }
