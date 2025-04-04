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

    // Virtual key codes
    struct
    {
        int up    = 0;
        int down  = 0;
        int left  = 0;
        int right = 0;
        int fast  = 0;
        int slow  = 0;
    } keys;

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
        double dt = 0;
        Vec2<double> position = 0;
        Vec2<double> dir      = 0;
        Vec2<double> progress = 0;  // in range [0, 1]
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

    void update(bool active)
    {
        state.dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - state.last_tick).count();
        double dp = state.dt / config.ease_in;

        if (active)
        {
            state.dir.x = LLInput::keydown(keys.right) - LLInput::keydown(keys.left);
            state.dir.y = LLInput::keydown(keys.up) - LLInput::keydown(keys.down);

            state.mod = std::pow(
                config.mod_factor,
                LLInput::keydown(keys.fast) - LLInput::keydown(keys.slow)
            );

            state.progress.x = std::min(state.progress.x + dp * std::abs(state.dir.x), 1.0);
            state.progress.y = std::min(state.progress.y + dp * std::abs(state.dir.y), 1.0);

            // HLInput::scroll(
            //     easing::ease_in_out_sine(state.progress.x) * state.step *
            //     state.mod * state.dir.x, true
            // );

            // HLInput::scroll(
            //     easing::ease_in_out_sine(state.progress.y) * state.step *
            //     state.mod * state.dir.y, false
            // );

        }
        else
        {
            state.progress.x = std::max(state.progress.x - dp, 0.0);
            state.progress.y = std::max(state.progress.y - dp, 0.0);

            // HLInput::scroll(
            //     easing::ease_out_sine(state.progress.x) * state.step *
            //     state.mod * state.dir.x, true
            // );

            // HLInput::scroll(
            //     easing::ease_out_sine(state.progress.y) * state.step *
            //     state.mod * state.dir.y, false
            // );
        }

    }

    void next_tick(std::function<bool()> predicate)
    {
        int interval_ms = static_cast<int>(1.0 / config.frequency * 1000);
        state.last_tick = state.next_tick;
        state.next_tick += std::chrono::milliseconds(interval_ms);

        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait_until(lock, state.next_tick, [&]() { return predicate(); });
        }
    }

    void stop()
    {
        state.progress = 0.0;
    }

    bool moving()
    {
        return LLInput::keys[keys.up]   ||
               LLInput::keys[keys.down] ||
               LLInput::keys[keys.left] ||
               LLInput::keys[keys.right];
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

    // bool scrolling();
    // bool moving();
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