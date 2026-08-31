#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include <cstring>

namespace FrameZero {

// A strictly deterministic Camera system that supports Rollback screen-shake
class RollbackCamera {
public:
    Vector2 position;
    Vector2 target;
    Fixed zoom;
    
    // Screenshake state
    Fixed shakeMagnitude;
    Fixed shakeDecay;
    int shakeFrames;
    Vector2 currentShakeOffset;
    
    RollbackCamera() : position(0,0), target(0,0), zoom(Fixed(1)), 
                       shakeMagnitude(0), shakeDecay(Fixed(90)/Fixed(100)), shakeFrames(0), currentShakeOffset(0,0) {}
                       
    void applyShake(Fixed magnitude, int frames) {
        shakeMagnitude = magnitude;
        shakeFrames = frames;
    }
    
    void update(Fixed dt) {
        // Smoothly follow target (Deterministic Lerp)
        Fixed lerpSpeed = Fixed(5);
        position = position + (target - position) * lerpSpeed * dt;
        
        // Process Screen Shake deterministically
        if (shakeFrames > 0) {
            // Use deterministic pseudo-RNG based on position and frames
            // This guarantees the shake pattern is identical on both clients during rollback
            int seed = (position.x.raw ^ position.y.raw ^ shakeFrames) * 214013 + 2531011;
            
            // Map seed to [-1.0, 1.0]
            Fixed randX = Fixed::fromInt((seed >> 16) & 0x7FFF) / Fixed::fromInt(16384) - Fixed(1);
            Fixed randY = Fixed::fromInt((seed >> 8) & 0x7FFF) / Fixed::fromInt(16384) - Fixed(1);
            
            currentShakeOffset = Vector2(randX * shakeMagnitude, randY * shakeMagnitude);
            
            shakeMagnitude = shakeMagnitude * shakeDecay;
            shakeFrames--;
        } else {
            currentShakeOffset = Vector2(0, 0);
        }
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
