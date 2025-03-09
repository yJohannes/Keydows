#include "smooth_navigate.h"

namespace smooth_navigate
{

SmoothNavigate::SmoothNavigate()
{
    m_scroll.config.frequency = 144;
    m_scroll.config.interval = static_cast<int>(1.0 / m_scroll.config.frequency * 1000);
    m_scroll.config.step = 0.1;
    m_scroll.config.mod_factor = 3;
    m_scroll.config.ease_in = 0.2;
    m_scroll.config.ease_out = 0.15;

    m_move.config.frequency = 60;
    m_move.config.interval = static_cast<int>(1.0 / m_move.config.frequency * 1000);
    m_move.config.step = 0.1;
    m_move.config.mod_factor = 2;
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
}

SmoothNavigate::~SmoothNavigate()
{
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

    static int counter = 0;

    // Check is key is valid and save the enum
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

    if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
    {
        if (LLInput::keydown(m_keys[ACTIVATE]))
        {
            if (key == m_keys[SCROLL_UP] || key == m_keys[SCROLL_DOWN])
            {
                if (!m_scroll.state.on && !m_thread_active)
                {
                    m_scroll.state.on = true;
                    std::cout << "Thread Started: " << counter++ << "\n";
                    std::thread(&SmoothNavigate::start_scroll, this).detach();
                }
            }

            // else if (key == m_keys[CLICK])
            // {
            //     POINT p;
            //     if (::GetCursorPos(&p))
            //     {
            //         HLInput::click_async(1, p.x, p.y, false);
            //     }
            // }
            // else // Move keys
            // {
                // while (
                //     LLInput::keys[m_keys[MOVE_LEFT]]  ||
                //     LLInput::keys[m_keys[MOVE_RIGHT]] ||
                //     LLInput::keys[m_keys[MOVE_UP]]    ||
                //     LLInput::keys[m_keys[MOVE_DOWN]]
                // )
                // {
                //     POINT p;

                //     if (::GetCursorPos(&p))
                //     {
                //         int dx = 0;
                //         int dy = 0;
                //         dx -= key == m_keys[MOVE_LEFT];
                //         dx += key == m_keys[MOVE_RIGHT];
                //         dy -= key == m_keys[MOVE_UP];
                //         dy += key == m_keys[MOVE_DOWN];
                //         HLInput::move_cursor(p.x + dx, p.y + dy);
                //     }
                // }
            // }

            return true;
        }
    }
    else if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP)
    {
        if (key == m_keys[SCROLL_UP] || key == m_keys[SCROLL_DOWN])
        {
            // If both keys were down we shouldn't stop scrolling
            if (LLInput::keydown(m_keys[SCROLL_UP]) || LLInput::keydown(m_keys[SCROLL_DOWN]))
            {
                return true;
            }

            m_scroll.state.on = false;
            return true;
        }

        // If any valid keys are released while pressing the activator key block the release
        if (LLInput::keydown(m_keys[ACTIVATE]))
        {
            return true;
        }
    }
    return false;
}

void SmoothNavigate::start_scroll()
{
    m_thread_active = true;

    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    double p0 = m_scroll.state.progress;
    double ease_time_left = m_scroll.config.ease_in - p0 * m_scroll.config.ease_in;

    double dt;
    while (m_scroll.state.on && LLInput::keydown(m_keys[ACTIVATE]))
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        // p in range [0, 1]

        m_scroll.state.progress = p0 + std::min(dt / ease_time_left, 1 - p0);

        std::cout << "Progess: " << m_scroll.state.progress << "\n";

        m_scroll.state.mod = LLInput::keydown(m_keys[FAST_SCROLL]) ? m_scroll.config.mod_factor
            : LLInput::keydown(m_keys[SLOW_SCROLL]) ? 1.0 / m_scroll.config.mod_factor
            : 1.0;
        m_scroll.state.dir = LLInput::keydown(m_keys[SCROLL_DOWN]) ? -1 : 1;
        
        double amount = m_scroll.config.step * easing::ease_in_out_sine(m_scroll.state.progress) * m_scroll.state.mod * m_scroll.state.dir;
        HLInput::scroll(amount);

        next_tick += std::chrono::milliseconds(m_scroll.config.interval);
        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_scroll_cv.wait_until(lock, next_tick, [&]() { return !m_scroll.state.on; });
    }
    
    end_scroll();
}

void SmoothNavigate::end_scroll()
{
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    double p0 = m_scroll.state.progress;
    double ease_time_left = p0 * m_scroll.config.ease_out;

    double dt, p;
    while (true)
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        if (dt > ease_time_left || m_scroll.state.on)
        {
            break;
        }

        m_scroll.state.progress = p0 - std::min(dt / ease_time_left, p0);

        std::cout << "Progess: " << m_scroll.state.progress << "\n";

        double amount = m_scroll.config.step * easing::ease_out_sine(m_scroll.state.progress) * m_scroll.state.mod * m_scroll.state.dir;
        HLInput::scroll(amount);
        next_tick += std::chrono::milliseconds(m_scroll.config.interval);

        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_scroll_cv.wait_until(lock, next_tick, [&]() { return m_scroll.state.on; });
    }

    m_thread_active = false;
    std::cout << "Thread Killed\n";
}

} // namespace smooth_navigate
