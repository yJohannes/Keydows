#ifndef OVERLAY_H
#define OVERLAY_H

#include <windows.h>
#undef DOUBLE_CLICK

#include <unordered_map>
#include <string>
#include <iostream>
#include "core/tool_interface.h"
#include "core/hotkeys/hotkey_manager.h"
#include "core/input/hl_input.h"
#include "core/input/ll_input.h"
#define OVERLAY_DEBUG

class Overlay : public ITool
{
private:
    WNDCLASSEX m_wcex;
    HWND m_hwnd;

    POINT m_click_pos;
    SIZE m_size;
    SIZE m_resolution;

    int m_block_width;
    int m_block_height;
    
    wchar_t m_input_char_1;
    wchar_t m_input_char_2;
    std::wstring m_charset = L"ABCDEFGHIJKLMNOPQRTSUVWXYZ1234567890,.-";
    std::wstring m_click_direction_charset = L"WSADQEOP"; // U, D, L, R, TL, TR, BL, BR

    HDC m_default_mem_dc;
    HBITMAP m_default_mem_bitmap;

    enum Action
    {
        // Keybinds
        HIDE,
        REMOVE_INPUT,
        CLEAR_INPUTS,
        MOVE_MOUSE,
        DOUBLE_CLICK,
        TRIPLE_CLICK,
        QUAD_CLICK,

        // Hotkeys
        ACTIVATE
    };

    std::unordered_map<Action, int> m_keybinds;
    std::unordered_map<Action, int> m_hotkeys;

public:
    Overlay();
    ~Overlay();
    int run();
    void shutdown();
    void activate(bool on);
    void render(HWND h_wnd);
    void repaint();
    void show_overlay(bool show);

    // Key Events
    bool CALLBACK keyboard_hook_listener(WPARAM w_param, LPARAM l_param);
    bool CALLBACK mouse_hook_listener(WPARAM w_param, LPARAM l_param);
    int enter_input(wchar_t input_char);
    void undo_input();
    void clear_input();

    // Setters & Getters
    void set_size(int x, int y);
    void set_resolution(int x, int y);
    void set_charset(const wchar_t* charset);
    void set_click_direction_charset(const wchar_t* charset);

private:
    static LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param);
    LRESULT handle_message(UINT msg, WPARAM w_param, LPARAM l_param);

    // Helpers
    int get_char_index(wchar_t c) const;
    void char_ids_to_coordinates(int char_id1, int char_id2, int* x_out, int* y_out) const;
    void chars_to_coordinates(wchar_t c1, wchar_t c2, int* x_out, int* y_out) const;
    void apply_direction(wchar_t c, int* x, int* y) const;
    bool is_valid_coordinate(int x, int y) const { return x >= 0 && x < m_size.cx && y >= 0 && y < m_size.cy; }
    bool is_valid_char(wchar_t c) const { return (get_char_index(c) != -1); }

    // Render
    void render_overlay_bitmap(HDC h_dc);
    void delete_cached_default_overlay();

    // Key Events
    void process_key(WPARAM key, LPARAM details);
    // void process_hotkey(HotkeyID id);
};

// Expose functions for the DLL
extern "C" EXPORT_API
ITool* create_tool()
{
    return new Overlay();
}

extern "C" EXPORT_API
void destroy_tool(ITool* tool)
{
    delete tool;
}

#endif // OVERLAY_H