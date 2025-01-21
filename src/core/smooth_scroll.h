#pragma once

#include <windows.h>
#include <algorithm>
#include <thread>
#include <atomic>
#include <unordered_map>

#include "defines.h"
#include "managers/hook_manager.h"
#include "utils/easing_functions.h"
#include "utils/timer.h"

class SmoothScroll
{
private:
    double m_frequency;
    double m_step_size;
    double m_modifier_factor;
    double m_ease_in_time;
    double m_ease_out_time;

    std::atomic<bool> m_scrolling;

    enum class Action
    {
        ACTIVATE,
        SCROLL_UP,
        SCROLL_DOWN,
        SLOW_SCROLL,
        FAST_SCROLL
    };

    std::unordered_map<Action, int> m_keys;
    std::unordered_map<Action, bool> m_key_states;
public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);
    void scroll(double delta) const;

private:
    void start_scroll();
    void end_scroll(double p0, double mod, signed dir);
};

// Test site
// https://infinite-scroll.com/options.html