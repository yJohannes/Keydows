#include "smooth_scroll.h"
#include "application.h"

SmoothScroll::SmoothScroll()
    : m_frequency(144)
    , m_step_size(0.15)
    , m_modifier_factor(3)
    , m_ease_in_time(0.2)
    , m_ease_out_time(0.1)
    , m_scrolling(false)
{
    m_keys[Action::ACTIVATE] = 220;
    m_keys[Action::SCROLL_UP] = '1';
    m_keys[Action::SCROLL_DOWN] = '2';
    m_keys[Action::SLOW_SCROLL] = 'Z';
    m_keys[Action::FAST_SCROLL] = 'X';

    m_key_states[Action::ACTIVATE] = 0;
    m_key_states[Action::SCROLL_UP] = 0;
    m_key_states[Action::SCROLL_DOWN] = 0;
    m_key_states[Action::SLOW_SCROLL] = 0;
    m_key_states[Action::FAST_SCROLL] = 0;
}

SmoothScroll::~SmoothScroll()
{
}

void SmoothScroll::activate(bool on)
{
    static int keyboard_id;

    if (on)
    {
        keyboard_id = Application::register_listener(
            KEYBOARD,
            CREATE_LISTENER(keyboard_hook_listener)
        );
    }
    else
    {
        Application::unregister_listener(KEYBOARD, keyboard_id);
    }
}

bool CALLBACK SmoothScroll::keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param)
{
    if (n_code < 0)
        return false;

    KBDLLHOOKSTRUCT* keydata = reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);
    WPARAM key = keydata->vkCode;

    // Check is key is valid and save the enum
    Action action_id;
    bool valid_key = false;
    for (const auto& pair : m_keys)
    {
        if (pair.second == key)
        {
            action_id = pair.first;
            valid_key = true;
            break;
        }
    }

    if (!valid_key)
        return false;

    static int held_key = -1;
    if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
    {
        m_key_states[action_id] = true;

        // Block all keys in m_keys
        if (m_key_states[Action::ACTIVATE])
        {
            if (key == m_keys[Action::SCROLL_UP] || key == m_keys[Action::SCROLL_DOWN])
            {
                held_key = key;
                if (!m_scrolling)
                {
                    m_scrolling = true;
                    std::thread(&SmoothScroll::start_scroll, this).detach();
                }
            }
            return true;
        }
    }
    else if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP)
    {
        // Log key as released
        m_key_states[action_id] = false;

        if (key == held_key)
        {
            m_scrolling = false;
            return true;
        }
    }
    return false;
}

void SmoothScroll::start_scroll()
{
    LARGE_INTEGER t0;
    timer::get_tick(t0);

    double dt, p, mod;
    signed dir;
    while (m_scrolling)
    {
        dt = timer::time_elapsed(t0);
        p = std::clamp(dt / m_ease_in_time, 0.0, 1.0);
        mod = m_key_states[Action::FAST_SCROLL] ? m_modifier_factor : (m_key_states[Action::SLOW_SCROLL] ? 1.0 / m_modifier_factor : 1.0);
        dir = m_key_states[Action::SCROLL_UP] ? 1 : -1;
        
        scroll(m_step_size * easing::ease_in_out_sine(p) * mod * dir);
        if (!m_scrolling)
        {
            break;
        }
        ::Sleep(static_cast<int>(1.0 / m_frequency * 1000));
    }
    end_scroll(p, mod, dir);
}

/// Decelerate scroll
/// @brief 
/// @param p0: initial percentage of max scroll speed 
/// @param mod: modifier that the scroll is scaled by
/// @param dir: scroll direction: 1 = up, -1 = down 
void SmoothScroll::end_scroll(double p0, double mod, signed dir)
{
    LARGE_INTEGER t0;
    timer::get_tick(t0);

    double dt, p;
    while ((dt = timer::time_elapsed(t0)) <= m_ease_out_time)
    {
        p = p0 * (1.0 - dt / m_ease_out_time);  // p ∈ [0, p0]
        // p = p0 - dt / m_ease_out_time; // Scale dt / t_e by 1 / p0
        scroll(m_step_size * easing::ease_out_sine(p) * mod * dir);
        if (m_scrolling) // Halt if started scrolling again
        {
            break;
        }
        ::Sleep(static_cast<int>(1.0 / m_frequency * 1000));
    }
}

void SmoothScroll::scroll(double delta) const
{
    int d = static_cast<int>(delta * 120);  // Scale by the standard delta unit

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (d > 0) ? MOUSEEVENTF_WHEEL : MOUSEEVENTF_WHEEL;
    input.mi.mouseData = d;

    SendInput(1, &input, sizeof(INPUT));
    std::cout << "Scrolled:    " << delta << "\n";
}