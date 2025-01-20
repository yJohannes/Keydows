#pragma once
#include <windows.h>
#include <iostream>
#include <functional>

#define KEYBOARD 0
#define MOUSE 1
// #define STATIC_WRAP(instance, func) \
    (HOOKPROC)([instance](int a, WPARAM b, LPARAM c) -> LRESULT { instance->func(a, b, c); })

// #define STATIC_WRAP(instance, func) \
    (HOOKPROC)([instance](int code, WPARAM b, LPARAM c) -> LRESULT { return instance->func(code, b, c); })


#define CREATE_LISTENER(listener) \
    [this](int a, WPARAM b, LPARAM c) { return listener(a, b, c); }

using HookListener = std::function<bool(int, WPARAM, LPARAM)>;

class HookManager
{
private:
    HHOOK m_keyboard_hook = nullptr;
    HHOOK m_mouse_hook = nullptr;

    std::unordered_map<int, HookListener> m_keyboard_listeners;
    std::unordered_map<int, HookListener> m_mouse_listeners;
public:
    void initialize();
    int register_listener(int hook_type, HookListener listener);
    void unregister_listener(int hook_type, int id);

    void attach_hook(int hook_type);
    void detach_hook(int hook_type);
    void attach_hooks();
    void detach_hooks();

private:
    LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
    LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);
    std::unordered_map<int, HookListener>* get_hook_listener_map(int hook_type);

    static LRESULT CALLBACK static_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (instance) {
            return instance->keyboard_proc(nCode, wParam, lParam);
        }
        return 0;
    }
};