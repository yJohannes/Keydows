#pragma once

#include <windows.h>
#include <thread>
#include <atomic>

#include "defines.h"
#include "utils/smoothing_functions.h"

class Application;

// Accelerated scrolling with smooth scroll
// No need to worry about sideways scrolling
// Because the system will automatically see shift
class SmoothScroll
{
private:
    double m_frequency;
    double m_step_size;
    double m_multiplier;
    double m_easing_time;

    int m_vk_up;
    int m_vk_down;
    int m_vk_multiplier;

    LARGE_INTEGER m_timer_start;
    LARGE_INTEGER m_timer_frequency;

    std::atomic<bool> m_scrolling;
public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    void scroll(double delta) const;

    void mark_time_start();
    double time_elapsed() const;

private:
    void start_scroll(int dir);
    void end_scroll();
};

// Test site
// https://infinite-scroll.com/options.html