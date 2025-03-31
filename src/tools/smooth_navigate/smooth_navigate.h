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
#include "core/event_types.h"
#include "core/tool_interface.h"
#include "core/input/ll_input.h"
#include "core/input/hl_input.h"
#include "utils/easing_functions.h"
#include "utils/vec.h"

namespace smooth_navigate
{

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

struct SmoothInput
{
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;

    struct
    {
        double frequency;
        double base_step;
        double mod_factor;
        double ease_in;
        double ease_out;
    } config;

    struct
    {
        TimePoint next_tick;
        TimePoint last_tick;
        double dt;
        Vec2<double> position;
        Vec2<double> dir;
        Vec2<double> progress;    // in range [0, 1]
        double step;
        double mod;
    } state;

    void reset_time()
    {
        const auto t = std::chrono::steady_clock::now();
        state.last_tick = t;
        state.next_tick = t;
        state.dt = 0.0;
    }

    void update()
    {
        state.dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - state.last_tick).count();
    }

    void next_tick(std::function<bool()> predicate)
    {
        int interval = static_cast<int>(1.0 / config.frequency * 1000);
        state.last_tick = state.next_tick;
        state.next_tick += std::chrono::milliseconds(interval);

        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait_until(lock, state.next_tick, [&]() { return predicate(); });
        }
    }

    void stop()
    {
        state.progress = 0.0;
    }
};

class SmoothNavigate : public ITool
{
private:
    SmoothInput m_scroll;
    SmoothInput m_move; 

    bool m_toggled_active = false;
    bool m_kill_threads = false;

    std::unordered_map<Event, int> m_keys;

public:
    SmoothNavigate();
    ~SmoothNavigate();

    int run() override;
    void toggle(bool b) override;
    bool CALLBACK keyboard_hook_listener(WPARAM w_param, LPARAM l_param);

private:
    void scroll_thread();
    void move_thread();

    void start_scroll();
    void start_move();

    bool scrolling();
    bool moving();
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