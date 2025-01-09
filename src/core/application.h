// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <vector>
#include <cwchar>

#include "hotkeys.h"

#define NULL_CHAR 0

/*
TODO:
- Key that removes all input characters 
- Don't do all repaint rendering on show, use old DC 
*/

class Application
{
private:
    static HWND m_hwnd;
    static HHOOK m_keyboard_hook;
    static HHOOK m_mouse_hook;

    static LONG m_display_width;
    static LONG m_display_height;

    static LONG m_block_width;
    static LONG m_block_height;
    static LONG m_horizontal_blocks;
    static LONG m_vertical_blocks;

    static bool m_overlay_active;
    static wchar_t m_input_char_1;
    static wchar_t m_input_char_2;
    static constexpr const wchar_t* m_chars = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";

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

    static void handle_keydown(WPARAM w_param, LPARAM l_param);
    static void handle_hotkey(WPARAM w_param);
    static void paint_event(HWND h_wnd);
    static void show_overlay(bool show);
    static void force_repaint(HWND h_wnd);
    static void click_at(int x, int y, bool right_click);
    static void release_key(int vk_code);

    static bool is_key_down(int vk_code);
    static wchar_t get_key_char(WPARAM w_param, LPARAM l_param);
    static int get_char_index(wchar_t c);
    static void char_id_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out);
    static void chars_to_coordinates(wchar_t c1, wchar_t c2, LONG* x_out, LONG* y_out);

    static bool is_valid_coordinate(LONG x, LONG y)
    { return x >= 0 && x < m_display_width && y >= 0 && y < m_display_height; }

    static bool is_valid_char(wchar_t c)
    { return (get_char_index(c) != -1); }
};