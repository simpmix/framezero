# 3D Deterministic Physics Guide

FrameZero v1.0.0 officially introduces true 3D rigid body physics simulation. Building a deterministic 3D game (like *Rocket League* or *Smash Bros*) requires careful attention to fixed-point math and rotational mechanics.

## 1. Vector3 & Fixed-Point Math
In standard 3D engines, you use `float` vectors. In FrameZero, you must use `FrameZero::Vector3`, which relies on `Fixed`.

```cpp
#include <vector3.h>

FrameZero::Vector3 velocity(Fixed(10), Fixed(5), Fixed(0));
FrameZero::Vector3 gravity(Fixed(0), Fixed(-9), Fixed(0));

// Math operations are 100% deterministic
velocity += gravity * deltaTime;

// Getting the length uses a bitwise deterministic integer square root
Fixed speed = velocity.length();
```

## 2. Quaternions (Gimbal Lock Prevention)
Euler angles (X, Y, Z degrees) are universally terrible for 3D physics due to Gimbal Lock (losing a degree of freedom when rotating past 90 degrees).

FrameZero provides a flawless `Quaternion` engine.

```cpp
#include <quaternion.h>

// Create a quaternion that represents a 45-degree rotation around the Y-axis
Fixed angle = Fixed::pi() / Fixed(4);
FrameZero::Quaternion yRotation = FrameZero::Quaternion::fromAxisAngle(
    FrameZero::Vector3(Fixed(0), Fixed(1), Fixed(0)), 
    angle
);

// Apply this rotation to a vector
FrameZero::Vector3 forward(Fixed(0), Fixed(0), Fixed(1));
FrameZero::Vector3 newDirection = yRotation.rotateVector(forward);
```

## 3. PhysicsBody3D ECS Component
Instead of using the 2D `PhysicsBody`, assign `PhysicsBody3D` to your entities!

```cpp
#include <physics_body_3d.h>

FrameZero::PhysicsBody3D carBody;
carBody.mass = Fixed(1500); // 1500 kg
carBody.size = FrameZero::Vector3(Fixed(2), Fixed(1), Fixed(4)); // Width, Height, Length

// Apply torque to make the car spin in mid-air
carBody.torque = FrameZero::Vector3(Fixed(0), Fixed(500), Fixed(0));

// The engine automatically integrates the torque into the Quaternion orientation!
carBody.integrate(Fixed(0.016)); 
```

## 4. O(1) Rollback Serialization
The `PhysicsBody3D` struct is meticulously packed using `#pragma pack(push, 1)`. 

This means it has exactly 0 bytes of padding. When the rollback engine takes a snapshot of your 3D world, it uses an `O(1)` memory block copy (`memcpy`), allowing you to snapshot 500+ tumbling 3D crates in less than a millisecond!
