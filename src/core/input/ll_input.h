#pragma once
#include <windows.h>
#include <iostream>
#include <functional>
#include <unordered_map>

// #define HOOK_DEBUG

#define CREATE_LISTENER(listener) \
    [this](WPARAM b, LPARAM c) -> bool { return listener(b, c); }

/// @brief A callback function must return a bool block key propagation.
typedef std::function<CALLBACK bool(WPARAM, LPARAM)> Listener;

struct ListenerKey
{
    int hook_type;
    int id;

    bool operator==(const ListenerKey& other) const {
        return hook_type == other.hook_type && id == other.id;
    }
};

// Specialize std::hash for ListenerKey
namespace std
{
    template <>
    struct hash<ListenerKey>
    {
        std::size_t operator()(const ListenerKey& key) const noexcept
        {
            return std::hash<int>{}(key.hook_type) ^ (std::hash<int>{}(key.id) << 1);
        }
    };
}


// Manages low-level keyboard and mouse hooks
class LLInput
{
public:
    static bool keys[256];
private:
    static HHOOK m_keyboard_hook;
    static HHOOK m_mouse_hook;

    static std::unordered_map<ListenerKey, Listener> m_registered_listeners;
public:

    /// @brief Register a listener to a hook.
    /// @param hook_type WH_KEYBOARD_LL or WH_MOUSE_LL 
    /// @param listener Callback function
    /// @return listener ID
    [[nodiscard]]
    static int register_listener(int hook_type, Listener listener);
    static void unregister_listener(int hook_type, int id);

    /// @brief HookLLInput 
    static void attach_hook(int hook_type);
    static void detach_hook(int hook_type);

    /// @brief LLInput uses all hooks
    static void attach_hooks();
    static void detach_hooks();
    static bool keydown(int vk_code) { return keys[vk_code]; }
private:
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);
};