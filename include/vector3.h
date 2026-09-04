#pragma once
#include "fixed_point.h"

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

    bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3& other) const { return !(*this == other); }

    Vector3& operator+=(const Vector3& other) {
        x = x + other.x;
        y = y + other.y;
        z = z + other.z;
        return *this;
    }

    Fixed dot(const Vector3& other) const {
        return (x * other.x) + (y * other.y) + (z * other.z);
    }

    Vector3 cross(const Vector3& other) const {
        return cross(*this, other);
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
