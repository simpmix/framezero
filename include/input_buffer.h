#ifndef FRAMEZERO_INPUT_BUFFER_H
#define FRAMEZERO_INPUT_BUFFER_H

#include "input.h"
#include <vector>

namespace FrameZero {

// Represents a fighting game directional input on a standard numpad layout
enum NumpadDirection {
    DIR_NONE = 5,
    DIR_DOWN = 2,
    DIR_DOWN_FWD = 3,
    DIR_FWD = 6,
    DIR_UP_FWD = 9,
    DIR_UP = 8,
    DIR_UP_BACK = 7,
    DIR_BACK = 4,
    DIR_DOWN_BACK = 1
};

class MotionDetector {
public:
    static NumpadDirection getDirection(int8_t moveX, int8_t moveY, int facingDirection) {
        bool down = moveY < -64;
        bool up = moveY > 64;
        bool fwd = (moveX > 64 && facingDirection == 1) || (moveX < -64 && facingDirection == -1);
        bool back = (moveX < -64 && facingDirection == 1) || (moveX > 64 && facingDirection == -1);

        if (down && fwd) return DIR_DOWN_FWD;
        if (down && back) return DIR_DOWN_BACK;
        if (up && fwd) return DIR_UP_FWD;
        if (up && back) return DIR_UP_BACK;
        
        if (down) return DIR_DOWN;
        if (up) return DIR_UP;
        if (fwd) return DIR_FWD;
        if (back) return DIR_BACK;
        
        return DIR_NONE;
    }

    // Check if a sequence of motions was performed recently (e.g., 236 for fireball)
    // motionSequence is expected in reverse chronological order (newest input first)
    static bool checkMotion(const InputQueue& queue, const std::vector<NumpadDirection>& sequence, int facingDirection, int maxFrames = 20) {
        if (sequence.empty() || queue.size == 0) return false;
        
        int seqIdx = 0;
        int framesSearched = 0;
        
        // Traverse queue backwards (newest to oldest)
        for (int i = 0; i < queue.size && framesSearched < maxFrames; ++i) {
            int idx = (queue.tail - 1 - i + InputQueue::MAX_SIZE) % InputQueue::MAX_SIZE;
            const Input& inp = queue.inputs[idx];
            
            NumpadDirection dir = getDirection(inp.moveX, inp.moveY, facingDirection);
            
            if (dir == sequence[seqIdx]) {
                seqIdx++;
                if (seqIdx >= sequence.size()) return true; // Motion complete!
            }
            framesSearched++;
        }
        
        return false;
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_INPUT_BUFFER_H
