#include <windows.h>

#include "defines.h"
#include "application.h"
#include "utils/smoothing_functions.h"

// Accelerated scrolling with smooth scroll
class SmoothScroll
{
private:
    double m_acceleration;
    double m_max_speed;
    double m_speed;

    LARGE_INTEGER m_timer_start;
    LARGE_INTEGER m_timer_frequency;

public:
    SmoothScroll();
    ~SmoothScroll();
    
    void activate(bool on);
    bool CALLBACK keyboard_hook_listener(int n_code, WPARAM w_param, LPARAM l_param);

    void set_acceleration(double a);
    void set_max_speed(double max_speed);

    void mark_time_start();
    double time_elapsed() const;

private:
};