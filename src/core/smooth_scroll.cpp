#include "smooth_scroll.h"
#include "application.h"

SmoothScroll::SmoothScroll()
    : m_scroll_interval_ms(10)
    , m_acceleration(10.0)
    , m_max_speed(20.0)
    , m_speed(0.0)
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

    switch (key) {
    case SCROLL_UP:
    case SCROLL_DOWN:
        if (!m_scrolling)
        {
            std::thread(&SmoothScroll::start_scroll, this, key).detach();
        }
        // return true; FIX KEYS BEING SENT
    }
    return false;
}

void SmoothScroll::start_scroll(int vk_direction)
{
    m_scrolling = true;
    signed dir = (vk_direction == SCROLL_UP) ? 1 : -1;
    while (Application::is_key_down(vk_direction))
    {
        scroll(dir * 0.1);
        if (!Application::is_key_down(vk_direction))
        {
            break;
        }
        ::Sleep(m_scroll_interval_ms);
    }
    m_scrolling = false;
}

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