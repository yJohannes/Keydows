#include "smoothscroll.h"

SmoothScroll::SmoothScroll()
    : m_acceleration(10.0)
    , m_max_speed(20.0)
    , m_speed(0.0)
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

        Application::show_window(true);
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

    switch (key) {
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        double p = m_speed / m_max_speed;
        
    }

}

void SmoothScroll::set_acceleration(double a)
{
    m_acceleration = a;
}

void SmoothScroll::set_max_speed(double max_speed)
{
    m_max_speed = max_speed;
}

void SmoothScroll::mark_time_start()
{
    ::QueryPerformanceCounter(&m_timer_start);
}

double SmoothScroll::time_elapsed() const
{
    LARGE_INTEGER time;
    ::QueryPerformanceCounter(&time);
    return static_cast<double>(time.QuadPart - m_timer_start.QuadPart);
}