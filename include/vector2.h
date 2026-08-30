#ifndef FRAMEZERO_VECTOR2_H
#define FRAMEZERO_VECTOR2_H

#include "fixed_point.h"

namespace FrameZero {

struct Vector2 {
    Fixed x, y;

    Vector2() : x(0), y(0) {}
    Vector2(Fixed xx, Fixed yy) : x(xx), y(yy) {}
    Vector2(double dx, double dy) : x(dx), y(dy) {}

    Vector2 operator+(const Vector2& o) const { return Vector2(x + o.x, y + o.y); }
    Vector2 operator-(const Vector2& o) const { return Vector2(x - o.x, y - o.y); }
    Vector2 operator*(const Fixed& scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
    
    // Returns angle of vector in radians (-PI to PI)
    Fixed angle() const {
        return Fixed::atan2(y, x);
    }
    
    // Rotates the vector by a given angle (in radians)
    Vector2 rotate(Fixed theta) const {
        Fixed s = Fixed::sin(theta);
        Fixed c = Fixed::cos(theta);
        return Vector2(
            x * c - y * s,
            x * s + y * c
        );
    }
    Vector2 operator*(double s) const { return Vector2(x * Fixed(s), y * Fixed(s)); }
    Vector2 operator/(Fixed s) const { return Vector2(x / s, y / s); }
    
    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
    Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }

    bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vector2& o) const { return !(*this == o); }

    Fixed dot(const Vector2& o) const { return x * o.x + y * o.y; }
    
    Fixed lengthSquared() const { return x * x + y * y; }
    Fixed length() const { return Fixed::sqrt(lengthSquared()); }
    
    Vector2 normalized() const {
        Fixed len = length();
        if (len.raw == 0) return Vector2(0, 0);
        return *this / len;
    }

    Vector2 operator-() const { return Vector2(-x, -y); }
    
    // Linear interpolation for rendering smoothness
    static Vector2 lerp(const Vector2& a, const Vector2& b, Fixed t) {
        return a + (b - a) * t;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_VECTOR2_H
