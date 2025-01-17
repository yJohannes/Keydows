#include "smooth_scroll.h"
#include "application.h"

SmoothScroll::SmoothScroll()
    : m_frequency(144)
    , m_step_size(0.35)
    , m_multiplier(2)
    , m_easing_time(0.25)
    , m_vk_up(VK_UP)
    , m_vk_down(VK_DOWN)
    , m_vk_multiplier(VK_RIGHT)
    , m_scrolling(false)
{
    ::QueryPerformanceFrequency(&m_timer_frequency);
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

    KBDLLHOOKSTRUCT* keydata = (KBDLLHOOKSTRUCT*)l_param;
    WPARAM key = keydata->vkCode;
    LPARAM press_type = w_param;

    if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN)
    {
        if (key == m_vk_up || key == m_vk_down)
        {
            if (!m_scrolling)
            {
                std::thread(&SmoothScroll::start_scroll, this, key).detach();
            }
            return true;
        }
    }

    else if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP)
    {
        m_scrolling = false;
    }

    return false;
}

void SmoothScroll::start_scroll(int vk_direction)
{
    signed dir = (vk_direction == m_vk_up) ? 1 : -1;
    m_scrolling = true;
    mark_time_start();
    while (m_scrolling)
    {
        double dt = time_elapsed();
        double p = std::clamp(dt / m_easing_time, 0.0, 1.0);
        double mul = Application::is_key_down(m_vk_multiplier) ? m_multiplier : (Application::is_key_down(VK_LEFT) ? 1.0 / m_multiplier : 1.0);
        scroll(dir * m_step_size * smoothing::ease_in_out_sine(p) * mul);
        if (!m_scrolling)
        {
            break;
        }
        ::Sleep(static_cast<int>(1.0 / m_frequency * 1000));
    }
    m_scrolling = false;
}

// Decelerate scroll
void SmoothScroll::end_scroll()
{}

void SmoothScroll::scroll(double delta) const
{
    int d = static_cast<int>(delta * 120); // Scale by the standard delta unit

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (d > 0) ? MOUSEEVENTF_WHEEL : MOUSEEVENTF_WHEEL;
    input.mi.mouseData = d;

    // Send the input event
    SendInput(1, &input, sizeof(INPUT));

    std::cout << "Scrolled " << delta << "\n";
}

void SmoothScroll::mark_time_start()
{
    ::QueryPerformanceCounter(&m_timer_start);
}

double SmoothScroll::time_elapsed() const
{
    LARGE_INTEGER time;
    ::QueryPerformanceCounter(&time);
    return static_cast<double>(time.QuadPart - m_timer_start.QuadPart) / m_timer_frequency.QuadPart;
}