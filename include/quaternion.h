#pragma once
#include "fixed_point.h"
#include "vector3.h"

namespace FrameZero {

struct Quaternion {
    Fixed x, y, z, w;

    Quaternion() : x(0), y(0), z(0), w(Fixed::fromInt(1)) {}
    Quaternion(Fixed x, Fixed y, Fixed z, Fixed w) : x(x), y(y), z(z), w(w) {}

    // Quaternion multiplication (combining rotations)
    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        );
    }

    void normalize() {
        Fixed magSq = (x * x) + (y * y) + (z * z) + (w * w);
        if (magSq.raw == 0) {
            w = Fixed::fromInt(1);
            return;
        }
        Fixed mag = Fixed::sqrt(magSq);
        x = x / mag;
        y = y / mag;
        z = z / mag;
        w = w / mag;
    }

    // Rotate a 3D vector by this quaternion
    Vector3 rotateVector(const Vector3& v) const {
        // v' = q * v * q^(-1)
        // Optimized standard implementation:
        Vector3 qvec(x, y, z);
        Vector3 uv = Vector3::cross(qvec, v);
        Vector3 uuv = Vector3::cross(qvec, uv);
        
        uv = uv * (w * Fixed(2));
        uuv = uuv * Fixed(2);
        
        return v + uv + uuv;
    }
    
    // Create quaternion from Axis-Angle
    static Quaternion fromAxisAngle(Vector3 axis, Fixed angleRadians) {
        axis.normalize();
        Fixed halfAngle = angleRadians / Fixed(2);
        Fixed s = Fixed::sin(halfAngle);
        
        return Quaternion(
            axis.x * s,
            axis.y * s,
            axis.z * s,
            Fixed::cos(halfAngle)
        );
    }
};

} // namespace FrameZero
