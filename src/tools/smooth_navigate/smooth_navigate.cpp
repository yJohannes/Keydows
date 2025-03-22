#include "smooth_navigate.h"

namespace smooth_navigate
{

SmoothNavigate::SmoothNavigate()
{
    m_scroll.config.frequency = 240;
    m_scroll.config.interval = static_cast<int>(1.0 / m_scroll.config.frequency * 1000);
    m_scroll.config.step = 0.05;
    m_scroll.config.mod_factor = 2;
    m_scroll.config.ease_in = 0.25;
    m_scroll.config.ease_out = 0.12;

    m_move.config.frequency = 144;
    m_move.config.interval = static_cast<int>(1.0 / m_move.config.frequency * 1000);
    m_move.config.step = 3;
    m_move.config.mod_factor = 2.5;
    m_move.config.ease_in = 0.2;
    m_move.config.ease_out = 0; // No easing out

    m_scroll.state.on = false;
    m_scroll.state.progress = 0.0;

    m_move.state.on = false;
    m_move.state.progress = 0.0;


    m_keys = {
        {ACTIVATE,    220},  // §
        {CLICK,        13},  // Enter
        {MOVE_UP,    VK_UP},
        {MOVE_DOWN,  VK_DOWN},
        {MOVE_LEFT,  VK_LEFT},
        {MOVE_RIGHT, VK_RIGHT},
        {SCROLL_UP,   '1'},
        {SCROLL_DOWN, '2'},
        {SIDEWAYS_SCROLL, '3'},
        {SLOW_SCROLL, 'Z'},
        {FAST_SCROLL, 'X'}
    };

    m_scroll_thread = std::thread(&SmoothNavigate::scroll_thread, this);
    m_move_thread = std::thread(&SmoothNavigate::move_thread, this);
}

SmoothNavigate::~SmoothNavigate()
{
    m_kill_threads = true;
    m_cv.notify_all(); // Wake up scroll thread

    if (m_scroll_thread.joinable())
    {
        m_scroll_thread.join();
    }

    if (m_move_thread.joinable())
    {
        m_move_thread.join();
    }
}

int SmoothNavigate::run()
{
    return 0;
}

void SmoothNavigate::toggle(bool on)
{
    static int keyboard_id;

    if (on)
    {
        keyboard_id = LLInput::register_listener(
            WH_KEYBOARD_LL,
            CREATE_LISTENER(keyboard_hook_listener)
        );
    }
    else
    {
        LLInput::unregister_listener(WH_KEYBOARD_LL, keyboard_id);
    }
}

bool CALLBACK SmoothNavigate::keyboard_hook_listener(WPARAM w_param, LPARAM l_param)
{
    KBDLLHOOKSTRUCT* keydata = reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);
    WPARAM key = keydata->vkCode;

    // Check if key is in use
    bool valid_key = false;
    for (const auto& pair : m_keys)
    {
        if (pair.second == key)
        {
            valid_key = true;
            break;
        }
    }

    if (!valid_key)
        return false;

    // Process listed keys
    switch (w_param)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (key == m_keys[ACTIVATE])
        {
            POINT p;
            if (::GetCursorPos(&p))
            {
                m_move.state.v_position.x = p.x;
                m_move.state.v_position.y = p.y;
            }

            return true;
        }

        if (key == m_keys[SCROLL_UP] || key == m_keys[SCROLL_DOWN])
        {
            if (LLInput::keydown(m_keys[ACTIVATE]))
            {
                m_cv.notify_all(); // Wake up scroll thread
                return true;
            }
        }

        if (key == m_keys[MOVE_UP] ||
            key == m_keys[MOVE_DOWN] ||
            key == m_keys[MOVE_LEFT] ||
            key == m_keys[MOVE_RIGHT]
        )
        {
            if (LLInput::keydown(m_keys[ACTIVATE]))
            {
                m_cv.notify_all(); // Wake up scroll thread
                return true;
            }
        }

        if (LLInput::keydown(m_keys[ACTIVATE])) // Move these out?
        {
            return true;
        }

        return false;

    // If any valid keys are released while pressing the activator key block the release
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (LLInput::keydown(m_keys[ACTIVATE]))
        {
            return true;
        }

    // No blocking
    default:
        return false;
    }
}


void SmoothNavigate::scroll_thread()
{
    while (!m_kill_threads)
    {
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        start_scroll();
        end_scroll();
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_cv.wait(lock, [&]() { return scrolling() || m_kill_threads; });
    }
}

void SmoothNavigate::move_thread()
{
    while (!m_kill_threads)
    {
        start_move();

        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_cv.wait(lock, [&]() { return moving() || m_kill_threads; });
    }
}

void SmoothNavigate::start_scroll()
{
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    const double p0 = m_scroll.state.progress;
    double t_ease_in = m_scroll.config.ease_in - p0 * m_scroll.config.ease_in;

    double dt = 0;
    while (scrolling() && LLInput::keydown(m_keys[ACTIVATE]))
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        m_scroll.state.progress = p0 + std::min(dt / t_ease_in, 1 - p0);
        m_scroll.state.mod = LLInput::keydown(m_keys[FAST_SCROLL]) ? m_scroll.config.mod_factor
            : LLInput::keydown(m_keys[SLOW_SCROLL]) ? 1.0 / m_scroll.config.mod_factor
            : 1.0;
        m_scroll.state.dir[0] = LLInput::keydown(m_keys[SCROLL_DOWN]) ? -1 : 1;

        HLInput::scroll(
            easing::ease_in_out_sine(m_scroll.state.progress) * m_scroll.config.step *
            m_scroll.state.mod * m_scroll.state.dir[0]
        );

        next_tick += std::chrono::milliseconds(m_scroll.config.interval);
        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_cv.wait_until(lock, next_tick, [&]() { return !scrolling(); });
    }
}

void SmoothNavigate::end_scroll()
{
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    const double p0 = m_scroll.state.progress;
    double t_ease_out = p0 * m_scroll.config.ease_out;

    double dt = 0;
    while (!scrolling() && dt < t_ease_out)
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        m_scroll.state.progress = p0 - std::min(dt / t_ease_out, p0);

        HLInput::scroll(
            easing::ease_out_sine(m_scroll.state.progress) * m_scroll.config.step *
            m_scroll.state.mod * m_scroll.state.dir[0]
        );

        next_tick += std::chrono::milliseconds(m_scroll.config.interval);
        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_cv.wait_until(lock, next_tick, [&]() { return scrolling(); });
    }
}

void SmoothNavigate::start_move()
{
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    const double p0 = m_move.state.progress;
    double t_ease_in = m_move.config.ease_in - p0 * m_move.config.ease_in;

    double dt = 0;
    while (moving() && LLInput::keydown(m_keys[ACTIVATE]))
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        m_move.state.progress = p0 + std::min(dt / t_ease_in, 1 - p0);
        m_move.state.mod = LLInput::keydown(m_keys[FAST_SCROLL]) ? m_move.config.mod_factor
            : LLInput::keydown(m_keys[SLOW_SCROLL]) ? 1.0 / m_move.config.mod_factor
            : 1.0;

        POINT p;
        if (::GetCursorPos(&p))
        {
            int dirx = LLInput::keydown(m_keys[MOVE_RIGHT]) - LLInput::keydown(m_keys[MOVE_LEFT]);
            int diry = LLInput::keydown(m_keys[MOVE_DOWN]) - LLInput::keydown(m_keys[MOVE_UP]);

            m_move.state.dir[0] = dirx;
            m_move.state.dir[1] = dirx;

            auto& pos = m_move.state.v_position;

            double normalize = std::abs(dirx) && std::abs(diry) ? 1 / 1.41421 : 1;

            pos.x += dirx * m_move.config.step * m_move.state.mod * normalize;
            pos.y += diry * m_move.config.step * m_move.state.mod * normalize;

            HLInput::move_cursor(pos.x, pos.y);
        }

        next_tick += std::chrono::milliseconds(m_move.config.interval);
        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_cv.wait_until(lock, next_tick, [&]() { return !moving(); });

    }
}

bool SmoothNavigate::scrolling()
{
    return LLInput::keys[m_keys[SCROLL_UP]] || LLInput::keys[m_keys[SCROLL_DOWN]];
}

bool SmoothNavigate::moving()
{
    return LLInput::keys[m_keys[MOVE_UP]]   || LLInput::keys[m_keys[MOVE_DOWN]] ||
           LLInput::keys[m_keys[MOVE_LEFT]] || LLInput::keys[m_keys[MOVE_RIGHT]];
}

} // namespace smooth_navigate