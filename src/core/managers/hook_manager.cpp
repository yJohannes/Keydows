#include "hook_manager.h"

HHOOK HookManager::m_keyboard_hook = nullptr;
HHOOK HookManager::m_mouse_hook = nullptr;

std::unordered_map<int, HookManager::Listener> HookManager::m_keyboard_listeners;
std::unordered_map<int, HookManager::Listener> HookManager::m_mouse_listeners;

void HookManager::initialize()
{
}

int HookManager::register_listener(int hook_type, Listener listener)
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
#ifdef HOOK_DEBUG
    std::cout << "Registered listener\n";
#endif
    return id;

}

void HookManager::unregister_listener(int hook_type, int id)
{
    auto* map = get_hook_listener_map(hook_type);
    map->erase(id);
#ifdef HOOK_DEBUG
    std::cout << "Unregistered listener\n";
#endif

    // If no hooks are in use
    if (map->empty())
    {
        detach_hook(hook_type);
    }
}

// Valid types are WH_KEYBOARD_LL and WH_MOUSE_LL
void HookManager::attach_hook(int hook_type)
{
    switch (hook_type) {
    case WH_KEYBOARD_LL:
        if (m_keyboard_hook != nullptr)
            return;

        m_keyboard_hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
        if (!m_keyboard_hook)
        {
            std::cerr << "Failed to install keyboard hook!" << std::endl;
            ::MessageBox(NULL, L"Failed to install keyboard hook!", L"Error", MB_ICONERROR | MB_OK);
        }
        return;

    case WH_MOUSE_LL:
        if (m_mouse_hook != nullptr)
            return;

        m_mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouse_proc, NULL, 0);
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
    case WH_KEYBOARD_LL:
        ::UnhookWindowsHookEx(m_keyboard_hook);
        m_keyboard_hook = nullptr;
        return;

    case WH_MOUSE_LL:
        ::UnhookWindowsHookEx(m_mouse_hook);
        m_mouse_hook = nullptr;
        return;
    }
}

void HookManager::attach_hooks()
{
    attach_hook(WH_KEYBOARD_LL);
    attach_hook(WH_MOUSE_LL);
}

void HookManager::detach_hooks()
{
    detach_hook(WH_KEYBOARD_LL);
    detach_hook(WH_MOUSE_LL);
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

std::unordered_map<int, HookManager::Listener> *HookManager::get_hook_listener_map(int hook_type)
{
    switch (hook_type) {
    case WH_KEYBOARD_LL:
        return &m_keyboard_listeners;
    case WH_MOUSE_LL:
        return &m_mouse_listeners;
    default:
        return nullptr;
    }
}