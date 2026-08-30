#ifndef FRAMEZERO_PHYSICS_BODY_H
#define FRAMEZERO_PHYSICS_BODY_H

#include "vector2.h"
#include "input.h"
#include <cstring>

namespace FrameZero {

enum BodyType { DYNAMIC, STATIC, KINEMATIC };

struct PhysicsBody {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 size;  // Half-extents for AABB
    Fixed mass;
    Fixed invMass;
    
    // Collision Filtering (Bitmasks)
    uint32_t collisionCategory = 0x00000001; // What am I? (Default: Layer 1)
    uint32_t collisionMask     = 0xFFFFFFFF; // What do I collide with? (Default: Everything)
    
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
    void serialize(uint8_t* buffer) const {
        memcpy(buffer, &id, 4);
        buffer[4] = active ? 1 : 0;
        memcpy(buffer + 5, &position.x.raw, 4);
        memcpy(buffer + 9, &position.y.raw, 4);
        memcpy(buffer + 13, &velocity.x.raw, 4);
        memcpy(buffer + 17, &velocity.y.raw, 4);
        memcpy(buffer + 21, &acceleration.x.raw, 4);
        memcpy(buffer + 25, &acceleration.y.raw, 4);
        memcpy(buffer + 29, &size.x.raw, 4);
        memcpy(buffer + 33, &size.y.raw, 4);
        memcpy(buffer + 37, &mass.raw, 4);
        memcpy(buffer + 41, &freezeFrames, 4);
        memcpy(buffer + 45, &restitution.raw, 4);
        memcpy(buffer + 49, &friction.raw, 4);
        buffer[53] = static_cast<uint8_t>(type);
        memcpy(buffer + 54, &collisionCategory, 4);
        memcpy(buffer + 58, &collisionMask, 4);
    }
    
    // Deserialize from bytes
    void deserialize(const uint8_t* buffer) {
        memcpy(&id, buffer, 4);
        active = buffer[4] != 0;
        memcpy(&position.x.raw, buffer + 5, 4);
        memcpy(&position.y.raw, buffer + 9, 4);
        memcpy(&velocity.x.raw, buffer + 13, 4);
        memcpy(&velocity.y.raw, buffer + 17, 4);
        memcpy(&acceleration.x.raw, buffer + 21, 4);
        memcpy(&acceleration.y.raw, buffer + 25, 4);
        memcpy(&size.x.raw, buffer + 29, 4);
        memcpy(&size.y.raw, buffer + 33, 4);
        memcpy(&mass.raw, buffer + 37, 4);
        invMass = (mass.raw == 0) ? Fixed(0) : Fixed(1) / mass;
        memcpy(&freezeFrames, buffer + 41, 4);
        memcpy(&restitution.raw, buffer + 45, 4);
        memcpy(&friction.raw, buffer + 49, 4);
        type = static_cast<BodyType>(buffer[53]);
        memcpy(&collisionCategory, buffer + 54, 4);
        memcpy(&collisionMask, buffer + 58, 4);
    }
    
    static constexpr size_t getSize() {
        return 62; // Total size
    }

    // Get AABB bounds
    Vector2 getMin() const { return Vector2(position.x - size.x, position.y - size.y); }
    Vector2 getMax() const { return Vector2(position.x + size.x, position.y + size.y); }
};

} // namespace FrameZero

#endif // FRAMEZERO_PHYSICS_BODY_H
