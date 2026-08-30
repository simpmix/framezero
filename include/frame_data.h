#ifndef FRAMEZERO_FRAME_DATA_H
#define FRAMEZERO_FRAME_DATA_H

#include "combat_system.h"

namespace FrameZero {

// Represents the frame data and hitboxes for a specific attack
struct AttackData {
    int startupFrames;    // Frames before the hitbox becomes active
    int activeFrames;     // Frames the hitbox stays active
    int recoveryFrames;   // Frames after active before the player can act again
    
    Hitbox hitbox;
    
    int getTotalDuration() const {
        return startupFrames + activeFrames + recoveryFrames;
    }
    
    // Check if the attack is currently in its active window
    bool isActiveFrame(int currentFrameOfAnimation) const {
        return currentFrameOfAnimation >= startupFrames && 
               currentFrameOfAnimation < (startupFrames + activeFrames);
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_FRAME_DATA_H
