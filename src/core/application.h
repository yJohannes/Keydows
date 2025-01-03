// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5

#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <cwchar>

#include "hotkeys.h"

/*
TODO:
- Key that removes all input characters 
- Don't do all repaint rendering on show, use old DC 
- Start on run instead of what line 42 says
*/

class Application
{
private:
    static HWND m_hwnd;
    static HHOOK m_keyboard_hook;
    static bool m_keyboard_state[256];

    LONG m_display_w;
    LONG m_display_h;

    LONG m_cell_w;
    LONG m_cell_h;
    LONG m_num_cells_x = 20;
    LONG m_num_cells_y = 16;

    wchar_t m_inchar1 = 0;
    wchar_t m_inchar2 = 0;
    
    static constexpr const wchar_t* m_chars = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";

    bool m_listening = true;  // Opens as listening

public:
    Application(HINSTANCE h_instance, int n_cmd_show);
    int run();
    void destroy_proc() const;

private:
    void handle_keydown(WPARAM w_param, LPARAM l_param);
    void handle_hotkey(WPARAM w_param);
    void paint_event(HWND h_wnd);
    void show_window(bool show);
    void force_window_repaint(HWND h_wnd) const;
    void click_at(int x, int y, bool right_click=false) const;

    bool is_key_down(int vk_code);
    wchar_t get_key_char(WPARAM w_param, LPARAM l_param);
    int get_char_index(wchar_t c) const;
    void char_id_to_coordinates(int char_id1, int char_id2, LONG* x_out, LONG* y_out);
    void chars_to_coordinates(wchar_t c1, wchar_t c2, LONG* x_out, LONG* y_out);

    bool is_valid_coordinate(LONG x, LONG y) { return x >= 0 && x < m_display_w && y >= 0 && y < m_display_h; }
    bool is_valid_char(wchar_t c) { return (get_char_index(c) != -1); }
    bool is_null_char(wchar_t c) { return c == L'\0'; }

    static LRESULT CALLBACK wnd_proc_wrapper(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
    LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
    static LRESULT CALLBACK keyboard_proc(int n_code, WPARAM w_param, LPARAM l_param);
};