#pragma once
#include <windows.h>
#include <string>
#include <iostream>

#undef DOUBLE_CLICK

#define QUIT_PROGRAM    VK_F4
#define HIDE_OVERLAY    VK_ESCAPE
#define REMOVE_INPUT    VK_BACK
#define CLEAR_INPUTS    VK_RETURN

#define M_MOVE_MOUSE      L'C'
#define M_DOUBLE_CLICK    L'V'
#define M_TRIPLE_CLICK    L'N'
#define M_QUAD_CLICK      L'M'

class Application;

class Overlay
{
public:
    enum InputType
    {
        NO_INPUT,
        FIRST_INPUT,
        SECOND_INPUT,
        MOVE_MOUSE,
        CLICK,
        DOUBLE_CLICK,
        TRIPLE_CLICK,
        QUAD_CLICK
    };

private:
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

public:
    Overlay();
    ~Overlay();
    void activate(bool on);
    void render(HWND h_wnd);

    // Key Events
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    bool CALLBACK mouse_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    int enter_input(wchar_t input_char);
    void undo_input();
    void clear_input();

    // Setters & Getters
    void set_size(int x, int y);
    void set_resolution(int x, int y);
    void set_charset(const wchar_t* charset);
    void set_click_direction_charset(const wchar_t* charset);

    wchar_t input_1() const { return m_input_char_1; } 
    wchar_t input_2() const { return m_input_char_2; } 

    // Helpers
    int get_char_index(wchar_t c) const;
    void char_ids_to_coordinates(int char_id1, int char_id2, int* x_out, int* y_out) const;
    void chars_to_coordinates(wchar_t c1, wchar_t c2, int* x_out, int* y_out) const;
    bool is_valid_coordinate(int x, int y) const { return x >= 0 && x < m_size.cx && y >= 0 && y < m_size.cy; }
    bool is_valid_char(wchar_t c) const { return (get_char_index(c) != -1); }

private:
    // Render
    void render_overlay_bitmap(HDC h_dc);
    void delete_cached_default_overlay();

    // Key Events
    void process_keydown(WPARAM key, LPARAM details);
};