#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include <cstring>

namespace FrameZero {

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Fixed lifetime;     // In frames
    Fixed maxLifetime;  // In frames
    uint32_t colorID;   // Mapped to rendering colors
    bool active;
    
    Particle() : position(0,0), velocity(0,0), lifetime(0), maxLifetime(0), colorID(0), active(false) {}
};

// Deterministic Visual Effects system that safely rolls back during network resimulations
class VFXSystem {
public:
    static constexpr int MAX_PARTICLES = 1024;
    Particle particles[MAX_PARTICLES];
    
    VFXSystem() {
        std::memset(particles, 0, sizeof(particles));
    }
    
    void spawnExplosion(Vector2 origin, Fixed power, int count, uint32_t colorID) {
        // Deterministic pseudo-randomness based on current frame state to ensure consistency across network
        Fixed angleStep = Fixed::pi() * Fixed(2) / Fixed::fromInt(count);
        
        int spawned = 0;
        for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
            if (!particles[i].active) {
                particles[i].active = true;
                particles[i].position = origin;
                
                Fixed angle = angleStep * Fixed::fromInt(spawned);
                
                // Very rudimentary sin/cos approximation for particle spread (deterministic)
                Fixed dirX = Fixed::cos(angle);
                Fixed dirY = Fixed::sin(angle);
                
                particles[i].velocity = Vector2(dirX * power, dirY * power);
                particles[i].maxLifetime = Fixed(30); // 30 frames
                particles[i].lifetime = particles[i].maxLifetime;
                particles[i].colorID = colorID;
                
                spawned++;
            }
        }
    }
    
    void update(Fixed dt) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                particles[i].position += particles[i].velocity * dt;
                
                // Simple drag
                particles[i].velocity = particles[i].velocity * Fixed(95) / Fixed(100);
                
                particles[i].lifetime = particles[i].lifetime - Fixed(1);
                if (particles[i].lifetime <= Fixed(0)) {
                    particles[i].active = false;
                }
            }
        }
    }
    
    // O(1) Contiguous Block Serialization
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, particles, sizeof(particles));
    }
    
    void deserialize(const uint8_t* buffer) {
        std::memcpy(particles, buffer, sizeof(particles));
    }
    
    static constexpr size_t getSize() {
        return sizeof(Particle) * MAX_PARTICLES;
    }
};

} // namespace FrameZero
