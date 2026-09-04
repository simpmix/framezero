#pragma once
#include "vector3.h"
#include "quaternion.h"
#include <cstring>

namespace FrameZero {

#pragma pack(push, 1)
// @FrameZeroComponent
struct PhysicsBody3D {
    uint32_t id;
    bool active;
    uint8_t _padding[3]; // Alignment padding for fast O(1) serialization
    
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    
    // Size is now 3D half-extents (Width, Height, Depth)
    Vector3 size; 
    
    Fixed mass;
    Fixed invMass;
    
    // 3D Rotational Physics
    Quaternion orientation;
    Vector3 angularVelocity;
    Vector3 torque;
    
    // Simple scalar inertia for a sphere (real 3D uses Inertia Tensors)
    Fixed inertia;
    Fixed invInertia;
    
    Fixed restitution;
    Fixed friction;
    
    uint32_t collisionCategory;
    uint32_t collisionMask;
    
    PhysicsBody3D() {
        std::memset(this, 0, sizeof(PhysicsBody3D));
        
        size = Vector3(Fixed(1), Fixed(1), Fixed(1));
        mass = Fixed(1);
        invMass = Fixed(1);
        
        orientation = Quaternion(); // Identity rotation
        
        inertia = Fixed(1);
        invInertia = Fixed(1);
        
        restitution = Fixed(50) / Fixed(100);
        friction = Fixed(30) / Fixed(100);
        active = true;
        
        collisionCategory = 0x00000001;
        collisionMask = 0xFFFFFFFF;
    }
    
    void setMass(Fixed m) {
        mass = m;
        invMass = (m.raw != 0) ? (Fixed(1) / m) : Fixed(0);
    }

    Vector3 getMin() const { return Vector3(position.x - size.x, position.y - size.y, position.z - size.z); }
    Vector3 getMax() const { return Vector3(position.x + size.x, position.y + size.y, position.z + size.z); }

    static bool checkAABB3D(const PhysicsBody3D& a, const PhysicsBody3D& b) {
        Vector3 minA = a.getMin(), maxA = a.getMax();
        Vector3 minB = b.getMin(), maxB = b.getMax();
        return (minA.x <= maxB.x && maxA.x >= minB.x) &&
               (minA.y <= maxB.y && maxA.y >= minB.y) &&
               (minA.z <= maxB.z && maxA.z >= minB.z);
    }

    void applyForce(const Vector3& force) {
        if (mass.raw == 0) return;
        acceleration += force * invMass;
    }
    
    void integrate(Fixed dt) {
        if (!active || mass.raw == 0) return;
        
        // Linear integration
        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = Vector3(0, 0, 0);
        
        // Angular integration (Quaternion derivative)
        // q_new = q_old + 0.5 * w * q_old * dt
        Quaternion q_w(angularVelocity.x, angularVelocity.y, angularVelocity.z, Fixed(0));
        Quaternion spin = q_w * orientation;
        
        orientation.x += spin.x * Fixed(50) / Fixed(100) * dt;
        orientation.y += spin.y * Fixed(50) / Fixed(100) * dt;
        orientation.z += spin.z * Fixed(50) / Fixed(100) * dt;
        orientation.w += spin.w * Fixed(50) / Fixed(100) * dt;
        
        orientation.normalize();
        
        angularVelocity += torque * invInertia * dt;
        torque = Vector3(0, 0, 0);
    }
    
    // O(1) Memory Block Serialization for Rollback
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(PhysicsBody3D));
    }
    
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(PhysicsBody3D));
    }
    
    static constexpr size_t getSize() {
        return sizeof(PhysicsBody3D);
    }
};
#pragma pack(pop)

} // namespace FrameZero
