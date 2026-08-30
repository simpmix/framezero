#ifndef FRAMEZERO_RANDOM_H
#define FRAMEZERO_RANDOM_H

#include "fixed_point.h"
#include <cstdint>

namespace FrameZero {

// A lightweight, deterministic, rollback-safe Random Number Generator.
// Games MUST use this instead of std::rand() to prevent desyncs.
// It uses a simple PCG (Permuted Congruential Generator) algorithm.
class Random {
private:
    uint64_t state;
    uint64_t inc;

public:
    // Initialize with a seed (should be agreed upon by both players at match start)
    Random(uint64_t seed = 0x853c49e6748fea9bULL, uint64_t seq = 0xda3e39cb94b95bdbULL) {
        state = 0U;
        inc = (seq << 1u) | 1u;
        next();
        state += seed;
        next();
    }

    // Get the next random 32-bit integer
    uint32_t next() {
        uint64_t oldstate = state;
        // Advance internal state
        state = oldstate * 6364136223846793005ULL + (inc | 1);
        // Calculate output function (XSH RR), uses old state for max ILP
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Get a random fixed-point number between 0.0 and 1.0
    Fixed nextFixed() {
        // We use the upper 16 bits of the 32-bit uint to create a 0.16 fixed point
        uint32_t val = next();
        uint16_t fraction = static_cast<uint16_t>(val >> 16);
        return Fixed::fromRaw(fraction); // raw value between 0 and 65535
    }

    // Get a random fixed-point number between min and max
    Fixed range(Fixed min, Fixed max) {
        if (min.raw >= max.raw) return min;
        Fixed r = nextFixed(); // 0.0 to ~1.0
        return min + r * (max - min);
    }
    
    // Get a random integer between min (inclusive) and max (exclusive)
    int32_t rangeInt(int32_t min, int32_t max) {
        if (min >= max) return min;
        uint32_t bound = max - min;
        uint32_t threshold = -bound % bound;
        
        // Uniform distribution modulo rejection
        for (;;) {
            uint32_t r = next();
            if (r >= threshold) {
                return min + (r % bound);
            }
        }
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_RANDOM_H
