// https://nicmulvaney.com/easing

#ifndef EASING_FUNCTIONS_H
#define EASING_FUNCTIONS_H

#include <cmath>

/// @brief A collection of easing functions.
/// @brief Parameter t should be in the range [0, 1].
namespace easing
{
    inline constexpr double pi = 3.14159265358979323846;

    inline double
    reverse(double (*f)(double), double t)
    { return 1.0 - f(t); }

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
    ease_in_sine(double t)
    { return 1.0 - std::cos(pi * t / 2); }

    inline double
    ease_out_sine(double t)
    { return std::sin((pi * t) / 2); }

    inline double
    ease_in_out_sine(double t)
    { return (1.0 - std::cos(pi * t)) / 2; }

} // namespace easing

#endif // EASING_FUNCTIONS_H