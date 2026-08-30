#ifndef FRAMEZERO_FIXED_POINT_H
#define FRAMEZERO_FIXED_POINT_H

#include <cstdint>

namespace FrameZero {

// 16.16 Fixed-Point Implementation for Deterministic Math
class Fixed {
public:
    int32_t raw;

    static constexpr int32_t SHIFT = 16;
    static constexpr int32_t ONE = 1 << SHIFT;

    Fixed() : raw(0) {}
    explicit Fixed(int32_t i) : raw(i * ONE) {}
    Fixed(double d) : raw(static_cast<int32_t>(d * ONE + (d >= 0 ? 0.5 : -0.5))) {}
    
    static Fixed fromInt(int i) { Fixed f; f.raw = i * ONE; return f; }

    double toDouble() const { return static_cast<double>(raw) / ONE; }
    int toInt() const { return raw >> SHIFT; }

    static Fixed fromRaw(int32_t r) { Fixed f; f.raw = r; return f; }

    Fixed operator+(const Fixed& o) const { return Fixed::fromRaw(raw + o.raw); }
    Fixed operator-(const Fixed& o) const { return Fixed::fromRaw(raw - o.raw); }
    Fixed operator*(const Fixed& o) const { return Fixed::fromRaw(static_cast<int32_t>((static_cast<int64_t>(raw) * o.raw) >> SHIFT)); }
    Fixed operator/(const Fixed& o) const { 
        if (o.raw == 0) return Fixed::fromRaw(0);
        return Fixed::fromRaw(static_cast<int32_t>((static_cast<int64_t>(raw) << SHIFT) / o.raw)); 
    }

    bool operator==(const Fixed& o) const { return raw == o.raw; }
    bool operator!=(const Fixed& o) const { return raw != o.raw; }
    bool operator<(const Fixed& o) const { return raw < o.raw; }
    bool operator>(const Fixed& o) const { return raw > o.raw; }
    bool operator<=(const Fixed& o) const { return raw <= o.raw; }
    bool operator>=(const Fixed& o) const { return raw >= o.raw; }

    Fixed& operator+=(const Fixed& o) { raw += o.raw; return *this; }
    Fixed& operator-=(const Fixed& o) { raw -= o.raw; return *this; }
    Fixed& operator*=(const Fixed& o) { *this = *this * o; return *this; }
    Fixed& operator/=(const Fixed& o) { *this = *this / o; return *this; }

    Fixed operator-() const { return Fixed::fromRaw(-raw); }

    // Absolute value
    static Fixed abs(const Fixed& x) {
        return x.raw < 0 ? Fixed::fromRaw(-x.raw) : x;
    }
    
    // Clamp value between min and max
    static Fixed clamp(const Fixed& v, const Fixed& min, const Fixed& max) {
        if (v < min) return min;
        if (v > max) return max;
        return v;
    }

    // Fast integer sqrt (fully deterministic, zero floating point)
    static Fixed sqrt(const Fixed& x) {
        if (x.raw <= 0) return Fixed::fromRaw(0);
        
        uint64_t num = static_cast<uint64_t>(x.raw) << SHIFT;
        uint64_t res = 0;
        uint64_t bit = 1ULL << 62; // The second-to-top bit is set
        
        // "bit" starts at the highest power of four <= the argument.
        while (bit > num) bit >>= 2;
        
        while (bit != 0) {
            if (num >= res + bit) {
                num -= res + bit;
                res = (res >> 1) + bit;
            } else {
                res >>= 1;
            }
            bit >>= 2;
        }
        return Fixed::fromRaw(static_cast<int32_t>(res));
    }

    // Lookup table based sin/cos for determinism
    static Fixed sin(Fixed angle);
    static Fixed cos(Fixed angle);
    
    // Deterministic atan2 approximation
    static Fixed atan2(Fixed y, Fixed x);
    
    static Fixed pi() { return Fixed(3.14159265); }
};

} // namespace FrameZero

#endif // FRAMEZERO_FIXED_POINT_H
