// Test site
// https://infinite-scroll.com/options.html

#ifndef SMOOTH_NAVIGATE_H
#define SMOOTH_NAVIGATE_H

#include <windows.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "defines.h"
#include "core/tool_interface.h"
#include "core/input/ll_input.h"
#include "core/input/hl_input.h"
#include "utils/easing_functions.h"

namespace smooth_navigate
{

struct SmoothInput
{
    struct
    {
        double frequency;
        int    interval;
        double step;
        double mod_factor;
        double ease_in;
        double ease_out;
    } config;

    struct
    {
        bool on;
        double progress;    // range [0, 1]
        signed dir;         // 1 = up, -1 = down
        double mod;         // modifier in use
    } state;

    
};

class SmoothNavigate : public ITool
{
private:
    SmoothInput m_scroll;
    SmoothInput m_move; 

    std::atomic<bool> m_thread_active = false;
    std::mutex m_scroll_mutex;
    std::condition_variable m_scroll_cv;

    enum Action
    {
        ACTIVATE,
        CLICK,
        MOVE_UP,
        MOVE_DOWN,
        MOVE_LEFT,
        MOVE_RIGHT,
        SCROLL_UP,
        SCROLL_DOWN,
        SIDEWAYS_SCROLL,
        SLOW_SCROLL,
        FAST_SCROLL
    };

    std::unordered_map<Action, int> m_keys;
public:
    SmoothNavigate();
    ~SmoothNavigate();
    
    int run() override;
    void toggle(bool b) override;
    bool CALLBACK keyboard_hook_listener(WPARAM w_param, LPARAM l_param);

private:
    void start_scroll();
    void end_scroll();
};

// Expose functions for the DLL
extern "C" EXPORT_API
ITool* create_tool()
{
    return new smooth_navigate::SmoothNavigate();
}

extern "C" EXPORT_API
void destroy_tool(ITool* tool)
{
    delete tool;
}

} // namespace smooth_navigate

#endif // SMOOTH_NAVIGATE_H
