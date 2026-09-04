#pragma once
#include "vector3.h"

namespace FrameZero {

// Hides the visual "snapping" caused by aggressive rollbacks by smoothly interpolating
// the visual position of a character toward their true deterministic physics position.
class PredictiveSmoother {
public:
    Vector3 visualPosition;
    Vector3 targetPosition;
    Fixed smoothSpeed;
    Fixed snapThresholdSq;
    
    PredictiveSmoother() : visualPosition(0,0,0), targetPosition(0,0,0), smoothSpeed(Fixed(15)), snapThresholdSq(Fixed(100)) {}
    
    void snapTo(const Vector3& pos) {
        visualPosition = pos;
        targetPosition = pos;
    }
    
    void setTarget(const Vector3& pos) {
        targetPosition = pos;
    }
    
    // Call this in the rendering loop (NOT the physics loop)
    void updateVisuals(Fixed renderDeltaTime) {
        // Exponential decay interpolation for buttery smooth visuals
        Vector3 diff = targetPosition - visualPosition;
        
        // If the error is too massive (e.g. teleporting), just snap
        if (diff.lengthSquared() > snapThresholdSq) {
            visualPosition = targetPosition;
            return;
        }
        
        visualPosition += diff * smoothSpeed * renderDeltaTime;
    }
    
    const Vector3& getRenderPosition() const {
        return visualPosition;
    }
};

} // namespace FrameZero
