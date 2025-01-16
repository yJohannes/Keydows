#pragma once

#include <windows.h>
#include <thread>

#include "defines.h"
#include "utils/smoothing_functions.h"

#define SCROLL_UP VK_UP
#define SCROLL_DOWN VK_DOWN

class Application;

// Accelerated scrolling with smooth scroll
// No need to worry about sideways scrolling
// Because the system will automatically see shift
class SmoothScroll
{
private:
    int m_scroll_interval_ms;
    double m_acceleration; // Time to reach max speed
    double m_max_speed;
    double m_speed;

    LARGE_INTEGER m_timer_start;
    LARGE_INTEGER m_timer_frequency;

    bool m_scrolling;
public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    void scroll(double delta) const;

    void set_acceleration(double a);
    void set_max_speed(double max_speed);

    void mark_time_start();
    double time_elapsed() const;

private:
    void start_scroll(int dir);
};