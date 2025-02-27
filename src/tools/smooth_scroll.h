#pragma once

#include <windows.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "defines.h"
// #include "core/application.h"
#include "core/input/ll_input.h"
#include "core/input/hl_input.h"
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

    bool m_thread_active = false;
    std::atomic<bool> m_scrolling;
    std::mutex m_scroll_mutex;
    std::condition_variable m_scroll_cv;


    enum Action
    {
        ACTIVATE,
        SCROLL_UP,
        SCROLL_DOWN,
        SLOW_SCROLL,
        FAST_SCROLL
    };

    std::unordered_map<Action, int> m_keys;
public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(WPARAM w_param, LPARAM l_param);
    void scroll(double delta) const;

private:
    void start_scroll();
    void end_scroll(double p0, double mod, signed dir);
};

// Test site
// https://infinite-scroll.com/options.html