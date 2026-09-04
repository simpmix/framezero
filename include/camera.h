#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include <cstring>

namespace FrameZero {

// A strictly deterministic Camera system that supports Rollback screen-shake and visual smoothing
class RollbackCamera {
public:
    Vector2 position;            // Deterministic simulation position
    Vector2 logicPosition;       // Alias for backward compatibility
    Vector2 target;              // Target to track
    Vector2 renderPosition;      // Visual smoothed position
    Fixed zoom;
    Fixed smoothingSpeed;
    
    // Screenshake state
    Fixed shakeMagnitude;
    Fixed shakeDecay;
    int shakeFrames;
    Vector2 currentShakeOffset;
    
    RollbackCamera() 
        : position(0, 0), logicPosition(0, 0), target(0, 0), renderPosition(0, 0),
          zoom(Fixed(1)), smoothingSpeed(Fixed(15) / Fixed(100)),
          shakeMagnitude(0), shakeDecay(Fixed(90) / Fixed(100)), shakeFrames(0), currentShakeOffset(0, 0) {}

    void applyShake(Fixed magnitude, int frames) {
        shakeMagnitude = magnitude;
        shakeFrames = frames;
    }
    
    void updateLogic(Vector2 targetPosition) {
        target = targetPosition;
        position = targetPosition;
        logicPosition = targetPosition;
    }
    
    void snapToTarget(Vector2 targetPosition) {
        position = targetPosition;
        logicPosition = targetPosition;
        target = targetPosition;
        renderPosition = targetPosition;
        currentShakeOffset = Vector2(0, 0);
    }
    
    void update(Fixed dt) {
        // Smoothly follow target (Deterministic Lerp)
        Fixed lerpSpeed = Fixed(5);
        position = position + (target - position) * lerpSpeed * dt;
        logicPosition = position;
        
        // Process Screen Shake deterministically
        if (shakeFrames > 0) {
            int seed = (position.x.raw ^ position.y.raw ^ shakeFrames) * 214013 + 2531011;
            Fixed randX = Fixed::fromInt((seed >> 16) & 0x7FFF) / Fixed::fromInt(16384) - Fixed(1);
            Fixed randY = Fixed::fromInt((seed >> 8) & 0x7FFF) / Fixed::fromInt(16384) - Fixed(1);
            currentShakeOffset = Vector2(randX * shakeMagnitude, randY * shakeMagnitude);
            shakeMagnitude = shakeMagnitude * shakeDecay;
            shakeFrames--;
        } else {
            currentShakeOffset = Vector2(0, 0);
        }
    }

    void updateRender(float renderDeltaTime) {
        float lx = static_cast<float>(position.x.toDouble());
        float ly = static_cast<float>(position.y.toDouble());
        float rx = static_cast<float>(renderPosition.x.toDouble());
        float ry = static_cast<float>(renderPosition.y.toDouble());
        
        float speed = static_cast<float>(smoothingSpeed.toDouble()) * 60.0f * renderDeltaTime;
        if (speed > 1.0f) speed = 1.0f;
        
        rx += (lx - rx) * speed;
        ry += (ly - ry) * speed;
        
        renderPosition.x = Fixed(static_cast<double>(rx));
        renderPosition.y = Fixed(static_cast<double>(ry));
    }
    
    Vector2 getRenderPosition() const {
        return position + currentShakeOffset;
    }
    
    // O(1) contiguous block serialization for Rollback Snapshotting
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(RollbackCamera));
    }
    
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(RollbackCamera));
    }
    
    static constexpr size_t getSize() {
        return sizeof(RollbackCamera);
    }
};

} // namespace FrameZero
