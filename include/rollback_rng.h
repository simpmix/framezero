#pragma once
#include <cstdint>
#include <cstring>
#include "fixed_point.h"

namespace FrameZero {

// A strictly deterministic, rollback-aware Random Number Generator.
// Uses the PCG (Permuted Congruential Generator) algorithm for excellent statistical quality and extreme speed.
class RollbackRNG {
private:
    uint64_t state;
    uint64_t inc;

public:
    RollbackRNG() : state(0x853c49e6748fea9bULL), inc(0xda3e39cb94b95bdbULL) {}

    // Seed the RNG (Should be called once at the start of a match with an agreed-upon network seed)
    void seed(uint64_t initState, uint64_t initSeq) {
        state = 0U;
        inc = (initSeq << 1u) | 1u;
        next();
        state += initState;
        next();
    }

    // Generate a raw 32-bit random integer
    uint32_t next() {
        uint64_t oldstate = state;
        // Advance internal state
        state = oldstate * 6364136223846793005ULL + (inc | 1);
        // Calculate output function (XSH RR)
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Generate a random Fixed-Point number between 0.0 and 1.0
    Fixed nextFixed() {
        // Use top 15 bits for a clean 0 to 1 range mapping
        uint32_t raw = next() & 0x7FFF;
        return Fixed::fromInt(raw) / Fixed::fromInt(0x7FFF);
    }
    
    // Generate a random integer between min and max (inclusive)
    int nextInt(int min, int max) {
        if (max <= min) return min;
        uint32_t range = (max - min) + 1;
        return min + (next() % range);
    }

    // --- Rollback Snapshot Hooks ---
    // The RNG state MUST be serialized along with physics. 
    // If a frame is rolled back, the RNG seed rewinds so critical hits/loot drops don't change!
    
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(RollbackRNG));
    }
    
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(RollbackRNG));
    }
    
    static constexpr size_t getSize() {
        return sizeof(RollbackRNG);
    }
};

} // namespace FrameZero
