#pragma once
#include <windows.h>
#include <string>

class Overlay
{
public:
    enum InputState
    {
        NO_INPUT,
        FIRST_INPUT,
        SECOND_INPUT,
        TRIGGERED,
        TRIGGERED_X2,
        TRIGGERED_X3
    };

private:
    struct InputData
    {
        // int input state?
        int x;
        int y;
        int count;
    };
    InputData m_input_data;

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
    void render(HWND h_wnd);
    int enter_input(wchar_t input_char);
    int undo_input();
    void clear_input();

    void set_size(int x, int y);
    void set_resolution(int x, int y);
    void set_charset(const wchar_t* charset);
    void set_click_direction_charset(const wchar_t* charset);

    InputData* input_data() { return &m_input_data; }
    wchar_t input_1() const { return m_input_char_1; } 
    wchar_t input_2() const { return m_input_char_2; } 

    int get_char_index(wchar_t c) const;
    void char_ids_to_coordinates(int char_id1, int char_id2, int* x_out, int* y_out) const;
    void chars_to_coordinates(wchar_t c1, wchar_t c2, int* x_out, int* y_out) const;

    bool is_valid_coordinate(LONG x, LONG y) const
    { return x >= 0 && x < m_size.cx && y >= 0 && y < m_size.cy; }

    bool is_valid_char(wchar_t c) const
    { return (get_char_index(c) != -1); }
private:
    void render_overlay_bitmap(HDC h_dc);
    void make_default_overlay_bitmap(HDC h_dc);
};