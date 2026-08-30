#ifndef FRAMEZERO_STATE_SERIALIZATION_H
#define FRAMEZERO_STATE_SERIALIZATION_H

#include "physics_body.h"
#include <cstring>
#include <chrono>

namespace FrameZero {

// Fast state serialization for rollback snapshots
class StateSerializer {
public:
    static constexpr int MAX_BODIES = 256;
    static constexpr int BODY_SIZE = PhysicsBody::getSize();
    
    uint8_t buffer[MAX_BODIES * BODY_SIZE];
    size_t dataSize;
    
    StateSerializer() : dataSize(0) {}
    
    // Serialize all bodies to buffer
    void serialize(PhysicsBody* bodies, int count) {
        count = (count > MAX_BODIES) ? MAX_BODIES : count;
        dataSize = count * BODY_SIZE;
        
        for (int i = 0; i < count; i++) {
            bodies[i].serialize(buffer + i * BODY_SIZE);
        }
    }
    
    // Deserialize back to bodies
    void deserialize(PhysicsBody* bodies, int count) {
        count = (count > MAX_BODIES) ? MAX_BODIES : count;
        
        for (int i = 0; i < count; i++) {
            bodies[i].deserialize(buffer + i * BODY_SIZE);
        }
    }
    
    // Get current serialized data size
    size_t getSize() const { return dataSize; }
    
    // Compute FNV-1a checksum (Optimized for 32-bit blocks to divide loop iterations by 4)
    uint32_t computeChecksum() const {
        uint32_t hash = 2166136261u;
        const uint32_t* data32 = reinterpret_cast<const uint32_t*>(buffer);
        size_t blocks = dataSize / 4;
        
        for (size_t i = 0; i < blocks; i++) {
            hash ^= data32[i];
            hash *= 16777619u;
        }
        
        // Handle remaining bytes
        for (size_t i = blocks * 4; i < dataSize; i++) {
            hash ^= buffer[i];
            hash *= 16777619u;
        }
        return hash;
    }
    
    // Performance test: measure serialization time
    double benchmarkSerialization(PhysicsBody* bodies, int count, int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            serialize(bodies, count);
            deserialize(bodies, count);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        
        return elapsed.count() / iterations;  // Average ms per operation
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_STATE_SERIALIZATION_H
