#pragma once

#include <windows.h>
#include <thread>
#include <atomic>
#include <unordered_map>

#include "defines.h"
#include "utils/easing_functions.h"
#include "utils/timer.h"

class Application;

// Accelerated scrolling with smooth scroll.
class SmoothScroll
{
private:
    double m_frequency;
    double m_step_size;
    double m_modifier_scale;
    double m_ease_in_time;
    double m_ease_out_time;

    std::atomic<bool> m_scrolling;
    bool m_activation_key_down;
    signed m_scroll_direction;

    enum class Action
    {
        ACTIVATE,
        SCROLL_UP,
        SCROLL_DOWN,
        SLOW_SCROLL,
        FAST_SCROLL
    };

    std::unordered_map<Action, int> m_keybinds;
    std::unordered_map<Action, bool> m_key_states;
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