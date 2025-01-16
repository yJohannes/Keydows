// https://nicmulvaney.com/easing

#include <cmath>

namespace smf
{
    constexpr double pi = 3.14159265358979323846;

    inline double
    linear(double t)
    { return t; }

    inline double
    ease_out_quad(double t)
    { return 1.0 - std::pow(1.0 - t, 2); }

    inline double
    ease_out_cubic(double t)
    { return 1.0 - std::pow(1.0 - t, 3); }

    inline double
    ease_in_out_sine(double t)
    { return 1.0 - std::cos(pi * t) / 2; }
    
} // namespace smf