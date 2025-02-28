
#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <functional>

class HotkeyManager
{
public:
    /// @brief Register a hotkey
    /// @param h_wnd 
    /// @param mod 
    /// @param vk 
    /// @param callback Callback function which can be nullptr
    /// @return Hotkey ID
    [[nodiscard]]
    static int register_hotkey(HWND h_wnd, int mod, int vk);
    static void unregister_hotkey(HWND h_wnd, int hotkey_id);
    static void unregister_all_hotkeys();
    
private:
    static std::unordered_map<int, HWND> m_hotkey_map;
};

#endif // HOTKEY_MANAGER_H