// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <vector>
#include <cwchar>

#include "defines.h"
#include "overlay.h"
#include "hotkeys.h"

/*
TODO:
- Adjustable alpha for text
- Don't do all repaint rendering on show, use old DC 
- GUI for custom bindings
- If hotkey mod key is down and hook recives the key, let it pass to close overlay  
- Make holding space hold click and arrows move cursor
*/
class Application
{
private:
    static WNDCLASSEXW m_wcex;
    static HWND m_hwnd;
    static HHOOK m_keyboard_hook;
    static HHOOK m_mouse_hook;

    static Overlay m_overlay;
public:
    Application(HINSTANCE h_instance);
    ~Application();
    int run();

private:
    static LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static void destroy_proc();
    static void attach_hooks();
    static void detach_hooks();

    static void handle_keydown(WPARAM key, LPARAM details);
    static void handle_hotkey(WPARAM w_param);
    static void show_overlay(bool show);
    static void paint_event();
    static void force_repaint();
    static void click_at(int x, int y, bool right_click);
    static void release_key(int vk_code);
    static bool is_key_down(int vk_code);
};