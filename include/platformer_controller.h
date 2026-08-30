#pragma once
#include "physics_body.h"
#include "input.h"

namespace FrameZero {

// A highly tuned, deterministic platformer controller.
// Features: Variable Jump Height, Coyote Time, Jump Buffering
class PlatformerController {
public:
    PhysicsBody* body;
    
    Fixed moveSpeed;
    Fixed jumpForce;
    Fixed gravityMultiplier;
    
    int coyoteTimeFrames;
    int jumpBufferFrames;
    
    // Internal State
    int currentCoyoteTime;
    int currentJumpBuffer;
    bool isGrounded;
    bool isJumping;

    PlatformerController() : body(nullptr), moveSpeed(Fixed(5)), jumpForce(Fixed(12)), gravityMultiplier(Fixed(1)),
                             coyoteTimeFrames(6), jumpBufferFrames(6),
                             currentCoyoteTime(0), currentJumpBuffer(0),
                             isGrounded(false), isJumping(false) {}

    void bind(PhysicsBody* b) {
        body = b;
    }

    void update(const Input& input) {
        if (!body) return;

        // Grounded check (naive for now, assumes floor is Y=0)
        isGrounded = (body->position.y <= Fixed(0));
        
        if (isGrounded) {
            body->position.y = Fixed(0);
            body->velocity.y = Fixed(0);
            currentCoyoteTime = coyoteTimeFrames;
            isJumping = false;
        } else {
            if (currentCoyoteTime > 0) currentCoyoteTime--;
        }

        // Horizontal Movement
        if (input.moveX > 0) {
            body->velocity.x = moveSpeed;
        } else if (input.moveX < 0) {
            body->velocity.x = moveSpeed * Fixed(-1);
        } else {
            // Instant stop for tight platforming
            body->velocity.x = Fixed(0); 
        }

        // Jump Buffering
        if (input.buttons & 1) { // Jump button pressed this frame
            currentJumpBuffer = jumpBufferFrames;
        } else if (currentJumpBuffer > 0) {
            currentJumpBuffer--;
        }

        // Execute Jump
        if (currentJumpBuffer > 0 && currentCoyoteTime > 0) {
            body->velocity.y = jumpForce;
            currentJumpBuffer = 0;
            currentCoyoteTime = 0;
            isJumping = true;
        }

        // Variable Jump Height (release button early to cut jump short)
        if (isJumping && !(input.buttons & 1) && body->velocity.y > Fixed(0)) {
            body->velocity.y = body->velocity.y * Fixed(0.5); // Halve upward velocity
        }
    }
};

} // namespace FrameZero
