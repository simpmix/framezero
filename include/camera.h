#pragma once
#include "vector2.h"

namespace FrameZero {

// A rollback-aware camera system that prevents nausea/screen-jerking during network resimulations.
class RollbackCamera {
public:
    Vector2 logicPosition;  // The deterministic position (snaps during rollback)
    Vector2 renderPosition; // The visual position (smooths out rollback snaps)
    
    Fixed zoom;
    Fixed smoothingSpeed;
    
    RollbackCamera() : logicPosition(0, 0), renderPosition(0, 0), zoom(1), smoothingSpeed(Fixed(0.15)) {}

    // Attach the camera to a target (like a player or the midpoint between two players)
    // This is called during the deterministic fixed-update (gameLogicCallback)
    void updateLogic(Vector2 targetPosition) {
        // Deterministically track the target
        logicPosition = targetPosition;
    }
    
    // Snap the camera instantly (useful for teleporting or round start)
    void snapToTarget(Vector2 targetPosition) {
        logicPosition = targetPosition;
        renderPosition = targetPosition;
    }

    // Called every rendering frame (not fixed tick!) to smoothly glide the camera
    // towards the logic position, hiding rollback jitter from the player.
    void updateRender(float renderDeltaTime) {
        // Convert Fixed to float for rendering
        float lx = static_cast<float>(logicPosition.x.toDouble());
        float ly = static_cast<float>(logicPosition.y.toDouble());
        float rx = static_cast<float>(renderPosition.x.toDouble());
        float ry = static_cast<float>(renderPosition.y.toDouble());
        
        // Framerate-independent spring smoothing
        float speed = static_cast<float>(smoothingSpeed.toDouble()) * 60.0f * renderDeltaTime;
        if (speed > 1.0f) speed = 1.0f;
        
        rx += (lx - rx) * speed;
        ry += (ly - ry) * speed;
        
        renderPosition.x = Fixed(static_cast<double>(rx));
        renderPosition.y = Fixed(static_cast<double>(ry));
    }
};

} // namespace FrameZero
