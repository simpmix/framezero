#pragma once
#include "fixed_point.h"
#include <cstdint>
#include <cstddef>

namespace FrameZero {

// A perfectly deterministic Random Number Generator (RNG) for Rollback Netcode.
// Standard std::rand() or std::mt19937 are dangerous in rollback because their internal 
// states must be saved and restored exactly upon a resimulation, which is difficult.
// This uses a lightweight, high-quality XorShift32 algorithm that serializes cleanly.

class RollbackRNG {
private:
    uint32_t state;

public:
    RollbackRNG(uint32_t seed = 1337) : state(seed) {
        if (state == 0) state = 1; // XorShift cannot have a 0 state
    }

    // Set or reset the deterministic seed
    void setSeed(uint32_t seed) {
        state = seed;
        if (state == 0) state = 1;
    }

    // Generates a random 32-bit integer
    uint32_t nextInt() {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    // Generates a random integer in range [min, max]
    int nextRange(int min, int max) {
        if (max <= min) return min;
        return min + (nextInt() % (max - min + 1));
    }

    // Generates a random deterministic Fixed-point number between 0.0 and 1.0
    Fixed nextFixed() {
        // Use top 16 bits for high quality distribution
        uint32_t val = (nextInt() >> 16); 
        // 65535 is 0xFFFF. Divide by 65535.0 using Fixed math
        return Fixed(static_cast<int>(val)) / Fixed(65535);
    }
    
    // Generates a random Fixed-point number between min and max
    Fixed nextFixedRange(Fixed min, Fixed max) {
        if (max <= min) return min;
        Fixed t = nextFixed();
        return min + (max - min) * t;
    }

    // --- Rollback Serialization Hooks ---
    // The RNG state MUST be saved alongside the physics state so random events 
    // (like critical hits or loot drops) happen exactly the same during resimulation.
    
    void serialize(uint8_t* buffer) const {
        // Save 4 bytes
        buffer[0] = static_cast<uint8_t>(state & 0xFF);
        buffer[1] = static_cast<uint8_t>((state >> 8) & 0xFF);
        buffer[2] = static_cast<uint8_t>((state >> 16) & 0xFF);
        buffer[3] = static_cast<uint8_t>((state >> 24) & 0xFF);
    }

    void deserialize(const uint8_t* buffer) {
        // Restore 4 bytes
        state = static_cast<uint32_t>(buffer[0]) |
               (static_cast<uint32_t>(buffer[1]) << 8) |
               (static_cast<uint32_t>(buffer[2]) << 16) |
               (static_cast<uint32_t>(buffer[3]) << 24);
    }
    
    static constexpr size_t getSize() {
        return sizeof(uint32_t);
    }
};

} // namespace FrameZero
