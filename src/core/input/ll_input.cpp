#include "ll_input.h"

bool LLInput::keys[256] = {false};

HHOOK LLInput::m_keyboard_hook = nullptr;
HHOOK LLInput::m_mouse_hook = nullptr;

std::unordered_map<ListenerKey, Listener> LLInput::m_registered_listeners;

int LLInput::register_listener(int hook_type, Listener listener)
{
    int id = 0;
    for (auto& [key, _] : m_registered_listeners)
    {
        if (key.id == id)
            id++;
        else
            break;
    }

    m_registered_listeners[{hook_type, id}] = listener;
    attach_hook(hook_type);
#ifdef HOOK_DEBUG
    std::cout << "Registered listener with ID " << id << "\n";
#endif
    return id;
}

void LLInput::unregister_listener(int hook_type, int id)
{
    auto* map = &m_registered_listeners;
    map->erase({hook_type, id});

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
void LLInput::attach_hook(int hook_type)
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

void LLInput::detach_hook(int hook_type)
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

void LLInput::attach_hooks()
{
    attach_hook(WH_KEYBOARD_LL);
    attach_hook(WH_MOUSE_LL);
}

void LLInput::detach_hooks()
{
    detach_hook(WH_KEYBOARD_LL);
    detach_hook(WH_MOUSE_LL);
}

LRESULT CALLBACK LLInput::keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code < 0)
    {
        return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
    }

    KBDLLHOOKSTRUCT* keydata = (KBDLLHOOKSTRUCT*)l_param;
    WPARAM key = keydata->vkCode;

    switch (w_param) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        keys[key] = 1;
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        keys[key] = 0;
        break;
    }

    for (auto& [key, listener] : m_registered_listeners)
    {
        if (key.hook_type == WH_KEYBOARD_LL)
        {
            if (listener(w_param, l_param))
            {
                return 1; // Block input from propagating to further receivers
            }
        }
    }
    // Pass the input to further receivers
    return CallNextHookEx(m_keyboard_hook, n_code, w_param, l_param);
}

LRESULT CALLBACK LLInput::mouse_proc(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code < 0)
        return CallNextHookEx(m_mouse_hook, n_code, w_param, l_param);

    for (auto& [key, listener] : m_registered_listeners)
    {
        if (key.hook_type == WH_MOUSE_LL)
        {
            if (listener(w_param, l_param))
            {
                return 1;
            }
        }
    }

    // Pass the input to further receivers
    return CallNextHookEx(m_mouse_hook, n_code, w_param, l_param);
}
