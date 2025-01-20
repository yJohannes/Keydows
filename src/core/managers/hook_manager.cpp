#include "hook_manager.h"


int HookManager::register_listener(int hook_type, HookListener listener)
{
    auto* map = get_hook_listener_map(hook_type);
    int id = 0;

    for (const auto& kv : *map)
    {
        if (kv.first == id)
            id++;
        else
            break;
    }

    attach_hook(hook_type);
    (*map)[id] = listener;
    std::cout << "Registered listener\n";
    return id;
}

void HookManager::unregister_listener(int hook_type, int id)
{
    auto* map = get_hook_listener_map(hook_type);
    map->erase(id);
    std::cout << "Unregistered listener\n";

    // If no hooks are in use
    if (map->empty())
    {
        detach_hook(hook_type);
    }
}


void HookManager::attach_hook(int hook_type)
{
    static auto kb_proc = static_wrap(this, &HookManager::keyboard_proc);
    static auto ms_proc = static_wrap(this, mouse_proc);

    switch (hook_type) {
    case KEYBOARD:
        if (m_keyboard_hook != nullptr)
            return;
        

        m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
        if (!m_keyboard_hook)
        {
            std::cerr << "Failed to install keyboard hook!" << std::endl;
            ::MessageBox(NULL, L"Failed to install keyboard hook!", L"Error", MB_ICONERROR | MB_OK);
        }
        return;

    case MOUSE:
        if (m_mouse_hook != nullptr)
            return;

        m_mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, ms_proc, NULL, 0);
        if (!m_mouse_hook)
        {
            std::cerr << "Failed to install mouse hook!" << std::endl;
            ::MessageBox(NULL, L"Failed to install mouse hook!", L"Error", MB_ICONERROR | MB_OK);
        }
        return;
    }
}

void HookManager::detach_hook(int hook_type)
{
    switch (hook_type) {
    case KEYBOARD:
        ::UnhookWindowsHookEx(m_keyboard_hook);
        m_keyboard_hook = nullptr;
        return;

    case MOUSE:
        ::UnhookWindowsHookEx(m_mouse_hook);
        m_mouse_hook = nullptr;
        return;
    }
}

void HookManager::attach_hooks()
{
    attach_hook(KEYBOARD);
    attach_hook(MOUSE);
}

void HookManager::detach_hooks()
{
    detach_hook(KEYBOARD);
    detach_hook(MOUSE);
}

LRESULT CALLBACK HookManager::keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    for (auto& listener : m_keyboard_listeners)
    {
        if (listener.second(n_code, w_param, l_param))
        {
            return 1;
        }
    }
    // Pass the input to further receivers
    return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
}

LRESULT CALLBACK HookManager::mouse_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    for (auto& listener : m_mouse_listeners)
    {
        if (listener.second(n_code, w_param, l_param))
        {
            return 1;
        }
    }

    // Pass the input to further receivers
    return CallNextHookEx(m_mouse_hook, n_code, w_param, l_param);
}

std::unordered_map<int, HookListener> *HookManager::get_hook_listener_map(int hook_type)
{
    switch (hook_type) {
    case KEYBOARD:
        return &m_keyboard_listeners;
    case MOUSE:
        return &m_mouse_listeners;
    default:
        return nullptr;
    }
}