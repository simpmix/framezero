#pragma once
#include "fixed_point.h"
#include <cmath>

namespace FrameZero {

struct Vector3 {
    Fixed x;
    Fixed y;
    Fixed z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(Fixed x, Fixed y, Fixed z) : x(x), y(y), z(z) {}
    Vector3(double x, double y, double z) : x(Fixed(x)), y(Fixed(y)), z(Fixed(z)) {}

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(Fixed scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(Fixed scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }

    Vector3& operator+=(const Vector3& other) {
        x = x + other.x;
        y = y + other.y;
        z = z + other.z;
        return *this;
    }

    static Fixed dot(const Vector3& a, const Vector3& b) {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }
    
    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            (a.y * b.z) - (a.z * b.y),
            (a.z * b.x) - (a.x * b.z),
            (a.x * b.y) - (a.y * b.x)
        );
    }

    // Since we don't have a fixed-point sqrt yet, we use a basic approximation or leave squared
    // For a true AAA 3D engine, we need Fixed::sqrt()
    Fixed lengthSquared() const {
        return dot(*this, *this);
    }
    
    Fixed length() const {
        return Fixed::sqrt(lengthSquared());
    }
    
    void normalize() {
        Fixed len = length();
        if (len.raw != 0) {
            x = x / len;
            y = y / len;
            z = z / len;
        }
    }
};

} // namespace FrameZero
