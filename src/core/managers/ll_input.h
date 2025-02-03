#pragma once
#include <windows.h>
#include <iostream>
#include <functional>

// #define HOOK_DEBUG

#define CREATE_LISTENER(listener) \
    [this](WPARAM b, LPARAM c) -> bool { return listener(b, c); }

// Manages low-level keyboard and mouse hooks.
class LLInput
{
public:
    using Listener =  std::function<CALLBACK bool(WPARAM, LPARAM)>;
    static bool keys[256];
private:
    static HHOOK m_keyboard_hook;
    static HHOOK m_mouse_hook;

    static std::unordered_map<int, Listener> m_keyboard_listeners;
    static std::unordered_map<int, Listener> m_mouse_listeners;
public:
    static void initialize();
    static int register_listener(int hook_type, Listener listener);
    static void unregister_listener(int hook_type, int id);
    static void attach_hook(int hook_type);
    static void detach_hook(int hook_type);
    static void attach_hooks();
    static void detach_hooks();

    static bool keydown(int vk_code) { return keys[vk_code]; }
private:
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static std::unordered_map<int, Listener>* get_hook_listener_map(int hook_type);
};