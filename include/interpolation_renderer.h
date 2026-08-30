#ifndef FRAMEZERO_INTERPOLATION_RENDERER_H
#define FRAMEZERO_INTERPOLATION_RENDERER_H

#include "physics_body.h"

namespace FrameZero {

// Stores rendering state for interpolation
struct RenderState {
    Vector2 position;
    Vector2 size;
    uint32_t id;
    bool active;
    
    RenderState() : position(0, 0), size(0, 0), id(0), active(false) {}
    
    RenderState(const PhysicsBody& body) 
        : position(body.position), size(body.size), id(body.id), active(body.active) {}
};

// Manages rendering interpolation between simulation frames
// Fully optimized for O(1) performance with Zero heap allocations
class InterpolationRenderer {
private:
    static constexpr int MAX_BODIES = 1024;
    
    RenderState previousState[MAX_BODIES];
    RenderState currentState[MAX_BODIES];
    RenderState interpolatedState[MAX_BODIES];
    int bodyCount;
    
public:
    InterpolationRenderer() : bodyCount(0) {}
    
    // Save current simulation state (called right before physics integration)
    void savePreviousState(PhysicsBody* bodies, int count) {
        bodyCount = (count > MAX_BODIES) ? MAX_BODIES : count;
        for (int i = 0; i < bodyCount; i++) {
            previousState[i] = RenderState(bodies[i]);
        }
    }
    
    // Save new simulation state (called right after physics integration)
    void saveCurrentState(PhysicsBody* bodies, int count) {
        int targetCount = (count > MAX_BODIES) ? MAX_BODIES : count;
        for (int i = 0; i < targetCount; i++) {
            currentState[i] = RenderState(bodies[i]);
        }
    }
    
    // Get interpolated state for rendering
    // alpha is usually calculated as: accumulator / dt (0.0 to 1.0)
    // Returns a pointer to the internal array and writes the size to outCount
    const RenderState* getInterpolatedState(Fixed alpha, int& outCount) {
        outCount = bodyCount;
        
        for (int i = 0; i < bodyCount; i++) {
            interpolatedState[i] = currentState[i];
            
            // O(1) lookup since the arrays perfectly mirror the Physics engine's body array
            if (currentState[i].active && previousState[i].active && currentState[i].id == previousState[i].id) {
                interpolatedState[i].position = Vector2::lerp(previousState[i].position, currentState[i].position, alpha);
            }
        }
        
        return interpolatedState;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_INTERPOLATION_RENDERER_H
