#include "smooth_scroll.h"

SmoothScroll::SmoothScroll()
    : m_frequency(144)
    , m_step_size(0.1)
    , m_modifier_factor(3)
    , m_ease_in_time(0.2)
    , m_ease_out_time(0.15)
    , m_scrolling(false)
{
    m_keys[ACTIVATE] = 220;
    m_keys[SCROLL_UP] = '1';
    m_keys[SCROLL_DOWN] = '2';
    m_keys[SLOW_SCROLL] = 'Z';
    m_keys[FAST_SCROLL] = 'X';
}

SmoothScroll::~SmoothScroll()
{
}

void SmoothScroll::activate(bool on)
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

bool CALLBACK SmoothScroll::keyboard_hook_listener(WPARAM w_param, LPARAM l_param)
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
                if (!m_scrolling || !m_thread_active)
                {
                    m_scrolling = true;
                    std::cout << "Thread Started: " << counter++ << "\n";
                    std::thread(&SmoothScroll::start_scroll, this).detach();
                }
            }
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

            m_scrolling = false;
            return true;
        }
    }
    return false;
}

void SmoothScroll::start_scroll()
{
    m_thread_active = true;

    const auto scroll_interval = std::chrono::milliseconds(static_cast<int>(1.0 / m_frequency * 1000));
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    double dt, p, mod;
    signed dir;
    while (m_scrolling && LLInput::keydown(m_keys[ACTIVATE]))
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        // p in range [0, 1]
        p = std::min(dt / m_ease_in_time, 1.0);
        mod = LLInput::keydown(m_keys[FAST_SCROLL]) ? m_modifier_factor : (LLInput::keydown(m_keys[SLOW_SCROLL]) ? 1.0 / m_modifier_factor : 1.0);
        dir = LLInput::keydown(m_keys[SCROLL_DOWN]) ? -1 : 1;
        
        scroll(m_step_size * easing::ease_in_out_sine(p) * mod * dir);
        next_tick += scroll_interval;

        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_scroll_cv.wait_until(lock, next_tick, [&]() { return !m_scrolling; });
    }
    
    end_scroll(p, mod, dir);
}

/// @brief 
/// @param p0: initial percentage of max scroll speed 
/// @param mod: modifier that the scroll is scaled by
/// @param dir: scroll direction: 1 = up, -1 = down 
void SmoothScroll::end_scroll(double p0, double mod, signed dir)
{
    const auto scroll_interval = std::chrono::milliseconds(static_cast<int>(1.0 / m_frequency * 1000));
    const auto t0 = std::chrono::steady_clock::now();
    auto next_tick = t0;

    double dt, p;
    while (true)
    {
        dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        if (dt > m_ease_out_time || m_scrolling)
        {
            break;
        }

        p = p0 * (1.0 - dt / m_ease_out_time);  // p ∈ [0, p0]
        scroll(m_step_size * easing::ease_out_sine(p) * mod * dir);  // Perform the scroll
        next_tick += scroll_interval;

        std::unique_lock<std::mutex> lock(m_scroll_mutex);
        m_scroll_cv.wait_until(lock, next_tick, [&]() { return m_scrolling.load(); });
    }

    m_thread_active = false;
    std::cout << "Thread Killed\n";
}

void SmoothScroll::scroll(double delta) const
{
    int d = static_cast<int>(delta * 120);  // Scale by the standard delta unit

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (d > 0) ? MOUSEEVENTF_WHEEL : MOUSEEVENTF_WHEEL;
    input.mi.mouseData = d;

    ::SendInput(1, &input, sizeof(INPUT));
}