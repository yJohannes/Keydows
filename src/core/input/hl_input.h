#ifndef HLINPUT_H
#define HLINPUT_H

#include <windows.h>
#include <thread>
#include <vector>
#include <iostream>

class HLInput
{
public:
    static void move_cursor(int x, int y);
    static void click(int n, int x, int y, bool right_click);
    static void click_async(int n, int x, int y, bool right_click);
    static void scroll(double delta, bool horizontal);
    static void scroll(double dx, double dy);
    static void set_key(int vk_code, bool pressed);
    static void set_mouse(int mk_code, bool pressed);
    static bool keydown(int vk_code);
};

#endif // HLINPUT_H