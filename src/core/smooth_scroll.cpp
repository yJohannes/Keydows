#include "smooth_scroll.h"
#include "application.h"

SmoothScroll::SmoothScroll()
    : m_frequency(144)
    , m_step_size(0.3)
    , m_modifier_scale(2)
    , m_ease_in_time(0.2)
    , m_ease_out_time(0.1)
    , m_activation_key(220)
    , m_up_binding(VK_UP)
    , m_down_binding(VK_DOWN)
    , m_multiplier_binding(VK_RIGHT)
    , m_scrolling(false)
{
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
    static int held_key = -1;


    if (n_code < 0)
        return false;

    KBDLLHOOKSTRUCT* keydata = (KBDLLHOOKSTRUCT*)l_param;
    WPARAM key = keydata->vkCode;
    LPARAM press_type = w_param;

    if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
    {
        if (key == m_activation_key)
        {
            m_activation_key_down = true;
            return true;
        }

        if (m_activation_key_down && (key == m_up_binding || key == m_down_binding))
        {
            held_key = key;
            m_scroll_direction = (key == m_up_binding) ? 1 : -1;
            if (!m_scrolling)
            {
                std::thread(&SmoothScroll::start_scroll, this).detach();
            }
            return true;
        }
    }
    else if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP)
    {
        if (key == m_activation_key)
        {
            m_activation_key_down = false;
            return true;
        }

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
    m_scrolling = true;

    LARGE_INTEGER t0;
    Application::get_tick(t0);

    double dt, p, mul;
    while (m_scrolling)
    {
        dt = Application::time_elapsed(t0);
        p = std::clamp(dt / m_ease_in_time, 0.0, 1.0);
        mul = Application::is_key_down(m_multiplier_binding) ? m_modifier_scale : (Application::is_key_down(VK_LEFT) ? 1.0 / m_modifier_scale : 1.0);

        scroll(m_step_size * easing::ease_in_out_sine(p) * mul * m_scroll_direction);

        if (!m_scrolling)
        {
            break;
        }
        ::Sleep(static_cast<int>(1.0 / m_frequency * 1000));
    }
    m_scrolling = false;
    end_scroll(p);
}

// Decelerate scroll
void SmoothScroll::end_scroll(double p0)
{
    LARGE_INTEGER t0;
    Application::get_tick(t0);

    double dt, p;
    while ((dt = Application::time_elapsed(t0)) <= m_ease_out_time)
    {
        p = p0 * (1.0 - dt / m_ease_out_time);  // p ∈ [0, p0]
        scroll(m_step_size * easing::ease_out_sine(p) * m_scroll_direction);

        if (m_scrolling)
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