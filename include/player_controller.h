#ifndef FRAMEZERO_PLAYER_CONTROLLER_H
#define FRAMEZERO_PLAYER_CONTROLLER_H

#include "physics_body.h"
#include "input.h"
#include "combat_system.h"

namespace FrameZero {

// Basic state machine and physics controller for a fighting game character
class PlayerController {
public:
    PhysicsBody* body;
    
    // Configurable parameters
    Fixed walkSpeed;
    Fixed jumpForce;
    Fixed groundFriction;
    Fixed airFriction;
    
    // Combat
    Hitbox attackHitbox;
    Hurtbox bodyHurtbox;
    Fixed health;
    
    // State
    bool isGrounded;
    bool isCrouching;
    bool isAttacking;
    int attackFrameTimer;
    int facingDirection; // 1 for right, -1 for left

    PlayerController() 
        : body(nullptr),
          walkSpeed(Fixed(5.0)),
          jumpForce(Fixed(10.0)),
          groundFriction(Fixed(0.8)),
          airFriction(Fixed(0.98)),
          attackHitbox(Vector2(Fixed(2.0), Fixed(0)), Vector2(Fixed(1.0), Fixed(1.0)), Fixed(10.0), 5), // Hitstop of 5 frames
          bodyHurtbox(Vector2(0, 0), Vector2(Fixed(1.0), Fixed(2.0))),
          health(Fixed(100.0)),
          isGrounded(false),
          isCrouching(false),
          isAttacking(false),
          attackFrameTimer(0),
          facingDirection(1) {}

    void bind(PhysicsBody* targetBody) {
        body = targetBody;
    }

    void update(const Input& input) {
        if (!body) return;

        // Simple ground check based on velocity (in a real game, use collision normals)
        // Here we assume a flat floor at Y=0 for demonstration
        if (body->position.y <= body->size.y) {
            body->position.y = body->size.y; // Snap to floor
            if (body->velocity.y < Fixed(0)) {
                body->velocity.y = Fixed(0);
            }
            isGrounded = true;
        } else {
            isGrounded = false;
        }

        // Manage attack state
        if (isAttacking) {
            attackFrameTimer--;
            
            // Hitbox is active on frames 10 to 5 (active frames)
            if (attackFrameTimer <= 10 && attackFrameTimer > 5) {
                attackHitbox.active = true;
                // Orient hitbox offset based on facing direction
                attackHitbox.offset.x = Fixed(2.0) * Fixed(facingDirection);
            } else {
                attackHitbox.active = false;
            }
            
            if (attackFrameTimer <= 0) {
                isAttacking = false;
            }
        } else {
            // Check for new attack
            if (input.buttons & BTN_PUNCH) {
                isAttacking = true;
                attackFrameTimer = 15; // Attack lasts 15 frames
                attackHitbox.active = false;
            }
        }

        // Apply movement forces if not attacking
        if (!isAttacking) {
            // Horizontal movement
            Fixed moveX = Fixed(input.moveX) / Fixed(127);
            
            // Crouching (assuming down is negative Y)
            if (input.moveY < -64 && isGrounded) {
                isCrouching = true;
            } else {
                isCrouching = false;
            }

            if (!isCrouching) {
                // Apply horizontal movement force
                body->applyForce(Vector2(moveX * walkSpeed, Fixed(0)) * body->mass);
                
                // Update facing direction
                if (input.moveX > 0) facingDirection = 1;
                else if (input.moveX < 0) facingDirection = -1;
            }

            // Jumping (assuming up is positive Y)
            if (input.moveY > 64 && isGrounded) {
                body->applyImpulse(Vector2(Fixed(0), jumpForce));
                isGrounded = false;
            }
        }

        // Apply custom friction based on state
        if (isGrounded) {
            body->velocity.x *= groundFriction;
        } else {
            body->velocity.x *= airFriction;
        }
    }
    
    // Serialization for rollback
    void serialize(uint8_t* out) const {
        out[0] = isGrounded ? 1 : 0;
        out[1] = isCrouching ? 1 : 0;
        out[2] = isAttacking ? 1 : 0;
        out[3] = static_cast<uint8_t>(attackFrameTimer);
        out[4] = facingDirection == 1 ? 1 : 0;
    }
    
    void deserialize(const uint8_t* in) {
        isGrounded = in[0] != 0;
        isCrouching = in[1] != 0;
        isAttacking = in[2] != 0;
        attackFrameTimer = static_cast<int>(in[3]);
        facingDirection = in[4] != 0 ? 1 : -1;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_PLAYER_CONTROLLER_H
