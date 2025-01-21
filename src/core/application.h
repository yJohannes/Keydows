// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5
#pragma once
#include "defines.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <cwchar>

#include "json.hpp"
using json = nlohmann::json;

#include "utils/hotkeys.h"
#include "utils/timer.h"
#include "managers/ll_input.h"
#include "overlay.h"
#include "smooth_scroll.h"

// #define OVERLAY_DEBUG

/*
 * The Keydows overlay application. Uses low-level
 * mouse and keyboard hooks to catch keystrokes.
 */
class Application
{
private:
    static HWND h_wnd;
    static WNDCLASSEXW m_wcex;

    static std::unordered_map<int, int> m_hotkey_ids;

    enum Hotkeys
    {
        NO_USE,
        QUIT,
        OVERLAY
    };

    static Overlay m_overlay;
    static SmoothScroll m_smooth_scroll;

public:
    Application(HINSTANCE h_instance);
    ~Application();
    static int run();
    static void shutdown();

    static void repaint();
    static void show_window(bool show);

    static void move_cursor(int x, int y);
    static void click(int n, int x, int y, bool right_click);
    static void click_async(int n, int x, int y, bool right_click);
    static void release_key(int vk_code);
    static bool is_key_down(int vk_code);

private:
    static void load_config();
    static LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);

    static void handle_hotkey(WPARAM w_param);
    static void paint_event();
};