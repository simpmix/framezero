#include "fixed_point.h"

namespace FrameZero {

// 16.16 Fixed Point CORDIC implementation (100% Deterministic)
// Angles are mapped such that PI = 205887 (which is 3.14159265 * 65536)
static const int32_t CORDIC_ATAN[16] = {
    51471, 30385, 16054, 8149, 4090, 2047, 1023, 511, 255, 127, 63, 31, 15, 7, 3, 1
};
static const int32_t CORDIC_K = 39811; // 0.607252935 * 65536

static void cordic(int32_t theta, int32_t& out_cos, int32_t& out_sin) {
    // Wrap theta to -PI to PI (PI = 205887, 2PI = 411774)
    int32_t two_pi = 411774;
    theta = theta % two_pi;
    if (theta < -205887) theta += two_pi;
    else if (theta > 205887) theta -= two_pi;

    // Shift to -PI/2 to PI/2
    bool negate_cos = false;
    bool negate_sin = false;
    
    if (theta > 102943) { // > PI/2
        theta = 205887 - theta; // PI - theta
        negate_cos = true;
    } else if (theta < -102943) { // < -PI/2
        theta = -205887 - theta; // -PI - theta
        negate_cos = true;
    }

    int32_t x = CORDIC_K;
    int32_t y = 0;
    int32_t z = theta;

    for (int i = 0; i < 16; i++) {
        int32_t d = z < 0 ? -1 : 1;
        int32_t tx = x - d * (y >> i);
        int32_t ty = y + d * (x >> i);
        int32_t tz = z - d * CORDIC_ATAN[i];
        x = tx;
        y = ty;
        z = tz;
    }

    out_cos = negate_cos ? -x : x;
    out_sin = negate_sin ? -y : y;
}

Fixed Fixed::sin(Fixed angle) {
    if (angle.raw == 0 || angle.raw == 205887 || angle.raw == -205887) return Fixed(0);
    int32_t c, s;
    cordic(angle.raw, c, s);
    return Fixed::fromRaw(s);
}

Fixed Fixed::cos(Fixed angle) {
    if (angle.raw == 0) return Fixed::fromRaw(65536);
    if (angle.raw == 102943 || angle.raw == -102943) return Fixed(0); // PI/2 or -PI/2
    if (angle.raw == 205887 || angle.raw == -205887) return Fixed::fromRaw(-65536); // PI or -PI
    int32_t c, s;
    cordic(angle.raw, c, s);
    return Fixed::fromRaw(c);
}

Fixed Fixed::atan2(Fixed y, Fixed x) {
    if (x.raw == 0 && y.raw == 0) return Fixed(0);
    
    int32_t xv = x.raw;
    int32_t yv = y.raw;
    int32_t z = 0;
    
    // Shift to quadrants I and IV
    int32_t offset = 0;
    if (xv < 0) {
        xv = -xv;
        if (yv < 0) {
            yv = -yv;
            offset = -205887; // -PI
        } else {
            yv = -yv;
            offset = 205887; // PI
        }
    }

    for (int i = 0; i < 16; i++) {
        int32_t d = yv < 0 ? -1 : 1;
        int32_t tx = xv + d * (yv >> i);
        int32_t ty = yv - d * (xv >> i);
        int32_t tz = z + d * CORDIC_ATAN[i];
        xv = tx;
        yv = ty;
        z = tz;
    }

    return Fixed::fromRaw(z + offset);
}

} // namespace FrameZero
