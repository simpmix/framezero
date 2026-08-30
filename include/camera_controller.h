#pragma once
#include "camera.h"
#include "physics_body.h"

namespace FrameZero {

// A dynamic Camera Controller specifically designed for Fighting Games and Brawlers.
// Automatically calculates the midpoint between multiple players, dynamically adjusts 
// zoom based on distance, and enforces hard stage boundaries deterministically.

class FightingCameraController {
private:
    RollbackCamera* camera;
    Fixed minZoom;
    Fixed maxZoom;
    Fixed zoomPadding; // Extra space around players
    
    // Stage boundaries (Deterministic)
    Fixed stageLeft;
    Fixed stageRight;
    Fixed stageBottom;
    Fixed stageTop;

public:
    FightingCameraController(RollbackCamera* cam) 
        : camera(cam), 
          minZoom(Fixed(0.8)), 
          maxZoom(Fixed(2.0)), 
          zoomPadding(Fixed(150.0)),
          stageLeft(Fixed(-1000.0)),
          stageRight(Fixed(1000.0)),
          stageBottom(Fixed(0.0)),
          stageTop(Fixed(1000.0)) {}

    void setZoomLimits(Fixed min, Fixed max) {
        minZoom = min;
        maxZoom = max;
    }

    void setStageBounds(Fixed left, Fixed right, Fixed bottom, Fixed top) {
        stageLeft = left;
        stageRight = right;
        stageBottom = bottom;
        stageTop = top;
    }

    // Updates the camera logic position deterministically based on targets
    void update(PhysicsBody** targets, int count) {
        if (count == 0 || !camera) return;

        // 1. Find bounding box of all target players
        Fixed minX = targets[0]->position.x;
        Fixed maxX = targets[0]->position.x;
        Fixed minY = targets[0]->position.y;
        Fixed maxY = targets[0]->position.y;

        for (int i = 1; i < count; i++) {
            if (targets[i]->position.x < minX) minX = targets[i]->position.x;
            if (targets[i]->position.x > maxX) maxX = targets[i]->position.x;
            if (targets[i]->position.y < minY) minY = targets[i]->position.y;
            if (targets[i]->position.y > maxY) maxY = targets[i]->position.y;
        }

        // 2. Calculate Midpoint (Center of Action)
        Vector2 midpoint((minX + maxX) / Fixed(2.0), (minY + maxY) / Fixed(2.0));

        // 3. Calculate dynamic zoom based on horizontal distance between players
        Fixed distX = maxX - minX;
        Fixed screenWidth = Fixed(800.0); // Assume default 800px width for fixed logic
        
        // Desired zoom = ScreenWidth / (PlayerDistance + Padding)
        Fixed desiredZoom = screenWidth / (distX + zoomPadding);
        
        // Clamp Zoom
        desiredZoom = Fixed::clamp(desiredZoom, minZoom, maxZoom);
        
        // 4. Enforce Stage Boundaries on the Camera
        // Calculate the camera's view width at the current zoom
        Fixed viewWidth = screenWidth / desiredZoom;
        Fixed viewWidthHalf = viewWidth / Fixed(2.0);
        
        // Clamp X position so the view doesn't bleed past the stage edge
        if (midpoint.x - viewWidthHalf < stageLeft) midpoint.x = stageLeft + viewWidthHalf;
        if (midpoint.x + viewWidthHalf > stageRight) midpoint.x = stageRight - viewWidthHalf;

        // Apply to camera
        camera->logicPosition = midpoint;
        camera->zoom = desiredZoom;
    }
};

} // namespace FrameZero
