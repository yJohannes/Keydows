#pragma once

#include <windows.h>
#include <thread>
#include <atomic>

#include "defines.h"
#include "utils/easing_functions.h"

class Application;

// Accelerated scrolling with smooth scroll
// No need to worry about sideways scrolling
// Because the system will automatically see shift
class SmoothScroll
{
private:
    double m_frequency;
    double m_step_size;
    double m_modifier_scale;
    double m_ease_in_time;
    double m_ease_out_time;

    int m_activation_key;
    int m_up_binding;
    int m_down_binding;
    int m_multiplier_binding;

    std::atomic<bool> m_scrolling;
    bool m_activation_key_down;
    signed m_scroll_direction;

public:
    enum class Direction
    {
        UP = 1,
        DOWN = -1,
        STOP = 0
    };

public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    void scroll(double delta) const;

private:
    void start_scroll();
    void end_scroll(double p);
};

// Test site
// https://infinite-scroll.com/options.html