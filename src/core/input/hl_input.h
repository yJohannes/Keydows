#ifndef HLINPUT_H
#define HLINPUT_H

#include <windows.h>
#include <thread>
#include <iostream>

class HLInput
{
public:
    static void move_cursor(int x, int y);
    static void click(int n, int x, int y, bool right_click);
    static void click_async(int n, int x, int y, bool right_click);
    static void release_key(int vk_code);
    static bool is_key_down(int vk_code);
};

#endif // HLINPUT_H