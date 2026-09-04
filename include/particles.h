#pragma once
#include "physics_body.h"
#include "random.h"
#include <cstdint>

namespace FrameZero {

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Fixed lifetime;     // Current age / frames
    Fixed maxLifetime;  // When it dies
    uint32_t colorID;   // Mapped to rendering colors
    bool active;

    Particle() : position(0, 0), velocity(0, 0), lifetime(0), maxLifetime(0), colorID(0), active(false) {}
};

// A highly optimized, deterministic particle emitter that safely rolls back.
// Since we don't want to overflow the ECS snapshot budget, we keep the particle 
// array small and use a fixed-size pool.
template <int MAX_PARTICLES>
class ParticleSystem {
public:
    Particle particles[MAX_PARTICLES];
    int activeCount = 0;
    
    // Step all particles forward by one fixed-tick.
    // If a rollback occurs, the ECS snapshot restores this entire struct automatically!
    void tick(Fixed dt) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) continue;
            
            // Integrate physics
            particles[i].position += particles[i].velocity * dt;
            particles[i].lifetime += dt;
            
            // Check death
            if (particles[i].lifetime >= particles[i].maxLifetime) {
                particles[i].active = false;
                activeCount--;
            }
        }
    }
    
    // Emit an explosion of deterministic particles
    void emitBurst(Vector2 origin, int count, Fixed speed, Fixed lifetime, Random& rng) {
        count = (count > MAX_PARTICLES) ? MAX_PARTICLES : count;
        
        int spawned = 0;
        for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
            if (!particles[i].active) {
                particles[i].active = true;
                particles[i].position = origin;
                
                // Random deterministic angle
                Fixed angle = rng.range(Fixed(0), Fixed(6.28318)); 
                particles[i].velocity = Vector2(Fixed::cos(angle), Fixed::sin(angle)) * speed;
                
                // Slight deterministic variance in lifetime (+- 20%)
                Fixed variance = rng.nextFixed() * Fixed(0.4) - Fixed(0.2); 
                particles[i].maxLifetime = lifetime * (Fixed(1) + variance);
                particles[i].lifetime = Fixed(0);
                
                spawned++;
                activeCount++;
            }
        }
    }
    
    void clear() {
        for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
        activeCount = 0;
    }
};

} // namespace FrameZero
