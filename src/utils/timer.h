#pragma once
#include <windows.h>

namespace timer
{
    inline LARGE_INTEGER m_timer_frequency;

    inline void
    initialize()
    { ::QueryPerformanceFrequency(&m_timer_frequency); }

    inline void
    get_tick(LARGE_INTEGER& t)
    { ::QueryPerformanceCounter(&t); }

    inline double
    time_elapsed(const LARGE_INTEGER& since_tick)
    {
        LARGE_INTEGER t;
        ::QueryPerformanceCounter(&t);
        return static_cast<double>(t.QuadPart - since_tick.QuadPart) / m_timer_frequency.QuadPart;
    }
} // namespace timer
