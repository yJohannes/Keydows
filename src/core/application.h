// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cwchar>

#include "defines.h"
#include "overlay.h"
#include "hotkeys.h"

#include "json.hpp"
using json = nlohmann::json;
/*
TODO:
- Adjustable alpha for text
- GUI for custom bindings
- If hotkey mod key is down and hook recives the key, let it pass to close overlay  
- Make holding input key hold click and arrows move cursor
*/


/*
 * The Keydows overlay application. Uses low-level
 * mouse and keyboard hooks to catch keystrokes. The overlay never
 * obtains focus and therefore only relies on said hooks.
 */
class Application
{
public:
    static HWND h_wnd;
private:
    static WNDCLASSEXW m_wcex;
    static HHOOK m_keyboard_hook;
    static HHOOK m_mouse_hook;

    static Overlay m_overlay;
public:
    Application(HINSTANCE h_instance);
    ~Application();
    static int run();
    static void shutdown();
    
    static void attach_hooks();
    static void detach_hooks();
    static void repaint();
    static void show_window(bool show);
    static void click(int x, int y, bool right_click);
    static void click_async(int x, int y, bool right_click);

    static void release_key(int vk_code);
    static bool is_key_down(int vk_code);

private:
    static void load_config();
    static LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);

    static void handle_hotkey(WPARAM w_param);
    static void paint_event();
};