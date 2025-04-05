#ifndef VEC_H
#define VEC_H

#include <cmath>

template <typename T>
struct Vec2
{
    T x;
    T y;

    constexpr Vec2() : x(0), y(0) {}
    constexpr Vec2(T val) : x(val), y(val) {}
    constexpr Vec2(T _x, T _y) : x(_x), y(_y) {}

    constexpr void operator=(T scalar) { x = scalar; y = scalar; };

    constexpr Vec2 operator-() const { return Vec2(-x, -y); }

    constexpr Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }

    constexpr Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }

    constexpr Vec2 operator*(const Vec2& v) const { return Vec2(x * v.x, y * v.y); }
    constexpr Vec2 operator*(T scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }


    constexpr Vec2 operator/(T scalar) const { return Vec2(x / scalar, y / scalar); }
    Vec2& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    constexpr bool operator==(const Vec2& v) const { return x == v.x && y == v.y; }
    constexpr bool operator!=(const Vec2& v) const { return !(*this == v); }
    
    constexpr bool is_zero() const { return x == T{} && y == T{}; }

    constexpr T length() const { return std::sqrt(x * x + y * y); }
    constexpr T length_squared() const { return x * x + y * y; }

    Vec2 unit_vector() const
    {
        T len = length();
        return len > 0 ? *this / len : Vec2(0, 0);
    }

    constexpr T dot(const Vec2& v) const { return x * v.x + y * v.y; }

    static constexpr T distance(const Vec2& a, const Vec2& b) {
        return (b - a).length();
    }

    static constexpr Vec2 lerp(const Vec2& a, const Vec2& b, T t) {
        return a + (b - a) * t;
    }

    constexpr Vec2 reflect(const Vec2& normal) const {
        return *this - normal * (2 * dot(normal));
    }

    constexpr Vec2 project(const Vec2& v) const {
        return v * (dot(v) / v.length_squared());
    }

    constexpr Vec2 perpendicular() const {
        return Vec2(-y, x);
    }
};

template <typename T>
constexpr Vec2<T> operator*(T scalar, const Vec2<T>& v) {
    return v * scalar;
}

#endif // VEC_H
