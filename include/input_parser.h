#pragma once
#include "input.h"
#include <vector>

namespace FrameZero {

// Define common fighting game motion commands
enum class MotionCommand {
    NONE,
    QUARTER_CIRCLE_FORWARD, // 236
    QUARTER_CIRCLE_BACK,    // 214
    DRAGON_PUNCH,           // 623
    HALF_CIRCLE_FORWARD,    // 41236
    CHARGE_BACK_FORWARD     // Hold 4, then 6
};

// A system that tracks input history and detects complex motion commands (like Hadoukens).
// Deterministic and safely rolls back.
class MotionParser {
private:
    static constexpr int MAX_HISTORY = 60; // Track 1 second of inputs at 60fps
    Input history[MAX_HISTORY];
    int head = 0;
    
public:
    void update(Input currentFrameInput) {
        history[head] = currentFrameInput;
        head = (head + 1) % MAX_HISTORY;
    }
    
    // Helper to get input from N frames ago (0 = current)
    Input getInputFramesAgo(int framesAgo) const {
        if (framesAgo < 0 || framesAgo >= MAX_HISTORY) return {};
        int index = (head - 1 - framesAgo);
        if (index < 0) index += MAX_HISTORY;
        return history[index];
    }
    
    // Detects a Quarter Circle Forward (Down -> DownForward -> Forward) within a window
    bool detectQCF(int windowFrames = 15) const {
        bool foundForward = false;
        bool foundDownForward = false;
        
        for (int i = 0; i < windowFrames; i++) {
            Input in = getInputFramesAgo(i);
            
            // Step 1: Find Forward (6)
            if (!foundForward && in.moveX > 0 && in.moveY == 0) {
                foundForward = true;
            }
            // Step 2: Find Down-Forward (3) before Forward
            else if (foundForward && !foundDownForward && in.moveX > 0 && in.moveY > 0) {
                foundDownForward = true;
            }
            // Step 3: Find Down (2) before Down-Forward
            else if (foundDownForward && in.moveX == 0 && in.moveY > 0) {
                return true; // Motion complete!
            }
        }
        return false;
    }

    // Detects a Dragon Punch / Shoryuken (Forward -> Down -> DownForward)
    bool detectDP(int windowFrames = 20) const {
        bool foundDownForward = false;
        bool foundDown = false;
        
        for (int i = 0; i < windowFrames; i++) {
            Input in = getInputFramesAgo(i);
            
            if (!foundDownForward && in.moveX > 0 && in.moveY > 0) {
                foundDownForward = true;
            }
            else if (foundDownForward && !foundDown && in.moveX == 0 && in.moveY > 0) {
                foundDown = true;
            }
            else if (foundDown && in.moveX > 0 && in.moveY == 0) {
                return true;
            }
        }
        return false;
    }
};

} // namespace FrameZero
