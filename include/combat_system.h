#ifndef FRAMEZERO_COMBAT_SYSTEM_H
#define FRAMEZERO_COMBAT_SYSTEM_H

#include "physics_body.h"
#include <vector>

namespace FrameZero {

struct Hitbox {
    Vector2 offset;
    Vector2 size;
    bool active;
    Fixed damage;
    int hitstopDuration;
    
    Hitbox() : offset(0, 0), size(1, 1), active(false), damage(0), hitstopDuration(0) {}
    Hitbox(Vector2 off, Vector2 s, Fixed dmg, int hitstop)
        : offset(off), size(s), active(false), damage(dmg), hitstopDuration(hitstop) {}
};

struct Hurtbox {
    Vector2 offset;
    Vector2 size;
    
    Hurtbox() : offset(0, 0), size(1, 1) {}
    Hurtbox(Vector2 off, Vector2 s) : offset(off), size(s) {}
};

class CombatSystem {
public:
    static bool checkHit(const PhysicsBody& attacker, const Hitbox& hitbox,
                         const PhysicsBody& defender, const Hurtbox& hurtbox) {
        if (!hitbox.active || !attacker.active || !defender.active) return false;
        
        // Attacker's Hitbox AABB
        Vector2 hbMin = attacker.position + hitbox.offset - hitbox.size;
        Vector2 hbMax = attacker.position + hitbox.offset + hitbox.size;
        
        // Defender's Hurtbox AABB
        Vector2 hurtMin = defender.position + hurtbox.offset - hurtbox.size;
        Vector2 hurtMax = defender.position + hurtbox.offset + hurtbox.size;
        
        // AABB Overlap test
        return (hbMin.x < hurtMax.x && hbMax.x > hurtMin.x &&
                hbMin.y < hurtMax.y && hbMax.y > hurtMin.y);
    }
    
    static void processHit(PhysicsBody* attacker, Hitbox& hitbox,
                           PhysicsBody* defender, Fixed& defenderHealth) {
        if (!attacker || !defender) return;
        
        // Apply damage
        defenderHealth -= hitbox.damage;
        
        // Apply Hitstop (Freeze frames)
        attacker->freezeFrames = hitbox.hitstopDuration;
        defender->freezeFrames = hitbox.hitstopDuration;
        
        // Deactivate hitbox to prevent multi-hits in same animation
        hitbox.active = false;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_COMBAT_SYSTEM_H
