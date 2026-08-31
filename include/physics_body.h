#pragma once
#include "vector2.h"
#include "input.h"
#include <cstring>

namespace FrameZero {

enum BodyType : uint8_t { DYNAMIC, STATIC, KINEMATIC };

#pragma pack(push, 1)
struct PhysicsBody {
    uint32_t id;
    bool active;
    uint8_t _padding[3]; // Explicit padding for 4-byte alignment
    
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 size;  // Half-extents for AABB
    
    Fixed mass;
    Fixed invMass;
    
    // Angular Physics
    Fixed rotation;        // Radians
    Fixed angularVelocity;
    Fixed torque;
    Fixed inertia;
    Fixed invInertia;
    
    // Collision Filtering
    uint32_t collisionCategory; 
    uint32_t collisionMask;     
    
    Fixed restitution;
    Fixed friction;
    BodyType type;
    uint8_t _padding2[3]; // Alignment
    
    int freezeFrames;
    uint32_t stateVersion;
    
    PhysicsBody() {
        // Zero out EVERYTHING including padding to ensure deterministic FNV-1a hashing
        std::memset(this, 0, sizeof(PhysicsBody));
        
        size = Vector2(Fixed::fromInt(1), Fixed::fromInt(1));
        mass = Fixed::fromInt(1);
        invMass = Fixed::fromInt(1);
        
        inertia = Fixed::fromInt(1);
        invInertia = Fixed::fromInt(1);
        
        restitution = Fixed(0.5);
        friction = Fixed(0.3);
        type = DYNAMIC;
        active = true;
        
        collisionCategory = 0x00000001;
        collisionMask = 0xFFFFFFFF;
    }
        
    void setStatic() {
        type = STATIC;
        mass = Fixed(0);
        invMass = Fixed(0);
        inertia = Fixed(0);
        invInertia = Fixed(0);
        velocity = Vector2(0, 0);
        angularVelocity = Fixed(0);
    }
    
    void setMass(Fixed newMass) {
        mass = newMass;
        if (mass.raw > 0) invMass = Fixed::fromInt(1) / mass;
        else invMass = Fixed(0);
        
        // Very basic inertia tensor for a 2D box: (m * (w^2 + h^2)) / 12
        if (mass.raw > 0) {
            Fixed w = size.x * Fixed(2);
            Fixed h = size.y * Fixed(2);
            inertia = (mass * ((w * w) + (h * h))) / Fixed(12);
            invInertia = Fixed(1) / inertia;
        } else {
            inertia = Fixed(0);
            invInertia = Fixed(0);
        }
    }
    
    void applyForce(const Vector2& force) {
        if (type == STATIC || mass.raw == 0) return;
        acceleration += force * invMass;
    }
    
    void applyForceAtPoint(const Vector2& force, const Vector2& point) {
        if (type == STATIC || mass.raw == 0) return;
        acceleration += force * invMass;
        
        Vector2 r = point - position;
        // 2D Cross product: r.x * f.y - r.y * f.x
        Fixed cross = (r.x * force.y) - (r.y * force.x);
        torque += cross;
    }
    
    void applyImpulse(const Vector2& impulse) {
        if (type == STATIC || mass.raw == 0) return;
        velocity += impulse * invMass;
    }
    
    void applyAngularImpulse(Fixed impulse) {
        if (type == STATIC || mass.raw == 0) return;
        angularVelocity += impulse * invInertia;
    }
    
    void integrate(Fixed dt) {
        if (type == STATIC || !active) return;
        
        if (freezeFrames > 0) {
            freezeFrames--;
            acceleration = Vector2(0, 0);
            torque = Fixed(0);
            return;
        }
        
        // Linear integration
        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = Vector2(0, 0);
        
        // Angular integration
        angularVelocity += torque * invInertia * dt;
        rotation += angularVelocity * dt;
        torque = Fixed(0);
        
        stateVersion++;
    }
    
    // Insanely fast O(1) serialization!
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(PhysicsBody));
    }
    
    // Insanely fast O(1) deserialization!
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(PhysicsBody));
    }
    
    static constexpr size_t getSize() {
        return sizeof(PhysicsBody);
    }

    Vector2 getMin() const { return Vector2(position.x - size.x, position.y - size.y); }
    Vector2 getMax() const { return Vector2(position.x + size.x, position.y + size.y); }
};
#pragma pack(pop)

} // namespace FrameZero
