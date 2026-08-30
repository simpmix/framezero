#pragma once
#include "fixed_point.h"
#include <algorithm>

namespace FrameZero {

// A deterministic math utility for fighting game combat mechanics
class CombatMath {
public:
    // Calculates damage decay (proration) for long combos.
    // E.g. The first hit does 100% damage, 2nd hit does 80%, 3rd hit does 60%, but never drops below a minimum scaling (e.g. 20%).
    static Fixed calculateComboDamage(Fixed baseDamage, int comboCount, Fixed decayRate = Fixed(0.2), Fixed minScaling = Fixed(0.2)) {
        if (comboCount <= 1) return baseDamage;
        
        // scaling = 1.0 - (decayRate * (comboCount - 1))
        Fixed scaling = Fixed(1.0) - (decayRate * Fixed(comboCount - 1));
        
        if (scaling < minScaling) {
            scaling = minScaling;
        }
        
        return baseDamage * scaling;
    }

    // Calculates hitstun (how many frames the victim is frozen/recovering) 
    // based on the attack's base impact and the victim's current state.
    // If the victim is crouching, hitstun usually increases by a few frames.
    static int calculateHitstun(int baseHitstun, bool victimIsCrouching, int crouchBonus = 2) {
        if (victimIsCrouching) {
            return baseHitstun + crouchBonus;
        }
        return baseHitstun;
    }

    // Calculates pushback force applied to both characters when a hit connects.
    // In the corner, the attacker gets pushed back further to prevent infinite corner combos.
    static Fixed calculatePushback(Fixed basePushback, bool isVictimInCorner, Fixed cornerMultiplier = Fixed(1.5)) {
        if (isVictimInCorner) {
            return basePushback * cornerMultiplier;
        }
        return basePushback;
    }
};

} // namespace FrameZero
