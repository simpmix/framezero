#pragma once
#include "physics_body.h"
#include "input.h"
#include "raycast.h"

namespace FrameZero {

// A deterministic Top-Down shooter/RPG controller
class TopDownController {
public:
    PhysicsBody* body;
    Fixed moveSpeed;
    
    // Direction the player is facing (0 to 255 mapped to 360 degrees, or just X/Y)
    Vector2 facingDirection;

    TopDownController() : body(nullptr), moveSpeed(Fixed(6)), facingDirection(0, 1) {}

    void bind(PhysicsBody* b) {
        body = b;
    }

    void update(const Input& input) {
        if (!body) return;

        // 8-way movement
        Fixed dx = Fixed(0);
        Fixed dy = Fixed(0);

        if (input.moveX > 0) dx = moveSpeed;
        else if (input.moveX < 0) dx = moveSpeed * Fixed(-1);

        if (input.moveY > 0) dy = moveSpeed;
        else if (input.moveY < 0) dy = moveSpeed * Fixed(-1);

        // Normalize diagonal movement speed
        if (dx != Fixed(0) && dy != Fixed(0)) {
            // approx 0.707
            Fixed diag = Fixed(707) / Fixed(1000); 
            dx = dx * diag;
            dy = dy * diag;
        }

        body->velocity.x = dx;
        body->velocity.y = dy;

        // Update facing direction if moving
        if (dx != Fixed(0) || dy != Fixed(0)) {
            facingDirection = Vector2(dx, dy);
        }

        // Example: Shoot hitscan weapon
        if (input.buttons & 2) { // Shoot button
            // In a real game, you would pass the full bodies array here
            // RaycastHit hit = Raycaster::cast(body->position, facingDirection, Fixed(100), engine.bodies, engine.bodyCount);
            // if (hit.hit) applyDamage(hit.body);
        }
    }
};

} // namespace FrameZero
