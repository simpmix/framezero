#ifndef FRAMEZERO_PHYSICS_BODY_H
#define FRAMEZERO_PHYSICS_BODY_H

#include "vector2.h"
#include "input.h"

namespace FrameZero {

enum BodyType { DYNAMIC, STATIC, KINEMATIC };

struct PhysicsBody {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 size;  // Half-extents for AABB
    Fixed mass;
    Fixed invMass;
    Fixed restitution;
    Fixed friction;
    BodyType type;
    uint32_t id;
    bool active;
    int freezeFrames;
    
    // For serialization state
    uint32_t stateVersion;
    
    PhysicsBody() 
        : position(0, 0), velocity(0, 0), acceleration(0, 0)
        , size(Fixed::fromInt(1), Fixed::fromInt(1)), mass(Fixed::fromInt(1)), invMass(Fixed::fromInt(1))
        , restitution(Fixed(0.5)), friction(Fixed(0.3))
        , type(DYNAMIC), id(0), active(true), freezeFrames(0), stateVersion(0) {}
        
    void setStatic() {
        type = STATIC;
        mass = Fixed(0);
        invMass = Fixed(0);
        velocity = Vector2(0, 0);
    }
    
    void setMass(Fixed newMass) {
        mass = newMass;
        if (mass.raw > 0) invMass = Fixed::fromInt(1) / mass;
        else invMass = Fixed(0);
    }
    
    void applyForce(const Vector2& force) {
        if (type == STATIC || mass.raw == 0) return;
        acceleration += force / mass;
    }
    
    void applyImpulse(const Vector2& impulse) {
        if (type == STATIC || mass.raw == 0) return;
        velocity += impulse / mass;
    }
    
    void integrate(Fixed dt) {
        if (type == STATIC || !active) return;
        
        if (freezeFrames > 0) {
            freezeFrames--;
            // Still clear acceleration so accumulated forces during hitstop don't carry over massively
            acceleration = Vector2(0, 0);
            return;
        }
        
        // Semi-implicit Euler
        velocity += acceleration * dt;
        position += velocity * dt;
        
        // Reset acceleration
        acceleration = Vector2(0, 0);
        
        stateVersion++;
    }
    
    // Serialize body state to bytes (for rollback snapshots)
    void serialize(uint8_t* out) const {
        // Position (8 bytes)
        int32_t px = position.x.raw;
        int32_t py = position.y.raw;
        memcpy(out, &px, 4);
        memcpy(out + 4, &py, 4);
        
        // Velocity (8 bytes)
        int32_t vx = velocity.x.raw;
        int32_t vy = velocity.y.raw;
        memcpy(out + 8, &vx, 4);
        memcpy(out + 12, &vy, 4);
        
        // State version (4 bytes)
        memcpy(out + 16, &stateVersion, 4);
        
        // Freeze frames (4 bytes)
        memcpy(out + 20, &freezeFrames, 4);
        
        // Active flag (1 byte)
        out[24] = active ? 1 : 0;
    }
    
    // Deserialize from bytes
    void deserialize(const uint8_t* in) {
        int32_t px, py, vx, vy;
        memcpy(&px, in, 4);
        memcpy(&py, in + 4, 4);
        position.x.raw = px;
        position.y.raw = py;
        
        memcpy(&vx, in + 8, 4);
        memcpy(&vy, in + 12, 4);
        velocity.x.raw = vx;
        velocity.y.raw = vy;
        
        memcpy(&stateVersion, in + 16, 4);
        memcpy(&freezeFrames, in + 20, 4);
        active = (in[24] != 0);
    }
    
    // Get AABB bounds
    Vector2 getMin() const { return Vector2(position.x - size.x, position.y - size.y); }
    Vector2 getMax() const { return Vector2(position.x + size.x, position.y + size.y); }
};

} // namespace FrameZero

#endif // FRAMEZERO_PHYSICS_BODY_H
