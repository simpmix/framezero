#ifndef FRAMEZERO_INTERPOLATION_RENDERER_H
#define FRAMEZERO_INTERPOLATION_RENDERER_H

#include "physics_body.h"
#include <vector>

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
class InterpolationRenderer {
private:
    std::vector<RenderState> previousState;
    std::vector<RenderState> currentState;
    
public:
    InterpolationRenderer() {}
    
    // Save current simulation state (called right before physics integration)
    void savePreviousState(PhysicsBody* bodies, int count) {
        previousState.clear();
        for (int i = 0; i < count; i++) {
            previousState.push_back(RenderState(bodies[i]));
        }
    }
    
    // Save new simulation state (called right after physics integration)
    void saveCurrentState(PhysicsBody* bodies, int count) {
        currentState.clear();
        for (int i = 0; i < count; i++) {
            currentState.push_back(RenderState(bodies[i]));
        }
    }
    
    // Get interpolated state for rendering
    // alpha is usually calculated as: accumulator / dt (0.0 to 1.0)
    std::vector<RenderState> getInterpolatedState(Fixed alpha) const {
        std::vector<RenderState> result;
        
        // Find matching bodies by ID and interpolate
        for (const auto& curr : currentState) {
            RenderState interpolated = curr;
            
            if (curr.active) {
                // Find matching body in previous state
                bool found = false;
                for (const auto& prev : previousState) {
                    if (prev.id == curr.id && prev.active) {
                        // Interpolate position
                        interpolated.position = Vector2::lerp(prev.position, curr.position, alpha);
                        // Can also interpolate rotation, color, etc. here
                        found = true;
                        break;
                    }
                }
                
                // If not found in previous state (e.g. just spawned), use current position directly
            }
            
            result.push_back(interpolated);
        }
        
        return result;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_INTERPOLATION_RENDERER_H
