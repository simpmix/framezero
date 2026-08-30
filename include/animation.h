#pragma once
#include "fixed_point.h"
#include <cstdint>

namespace FrameZero {

// A single frame of a 2D spritesheet animation
struct AnimationFrame {
    int sourceX, sourceY;     // Top-left pixel coordinate on spritesheet
    int width, height;        // Size of the frame
    int durationTicks;        // How many engine ticks (fixed frames) this image should display
    
    // Optional data for gameplay syncing (e.g. hitboxes become active on frame 4)
    uint32_t eventFlags = 0;  
};

// Represents a complete sequence (e.g., "Walk", "Punch", "Jump")
struct AnimationSequence {
    int id;
    AnimationFrame* frames;
    int frameCount;
    bool looping;
};

// Rollback-safe Animator state to attach to the ECS
// This contains NO pointers to textures, only deterministic state indices!
struct AnimatorState {
    int currentSequenceId = -1;
    int currentFrameIndex = 0;
    int ticksInCurrentFrame = 0;
    bool finished = false;
    
    // Call this inside your gameLogicCallback
    void tick(const AnimationSequence* allSequences, int sequenceCount) {
        if (finished || currentSequenceId < 0) return;
        
        // Find current sequence
        const AnimationSequence* seq = nullptr;
        for(int i = 0; i < sequenceCount; i++) {
            if (allSequences[i].id == currentSequenceId) {
                seq = &allSequences[i];
                break;
            }
        }
        if (!seq || seq->frameCount == 0) return;
        
        ticksInCurrentFrame++;
        
        // Check if it's time to advance to the next frame
        if (ticksInCurrentFrame >= seq->frames[currentFrameIndex].durationTicks) {
            ticksInCurrentFrame = 0;
            currentFrameIndex++;
            
            if (currentFrameIndex >= seq->frameCount) {
                if (seq->looping) {
                    currentFrameIndex = 0; // Loop back
                } else {
                    currentFrameIndex = seq->frameCount - 1; // Hold on last frame
                    finished = true;
                }
            }
        }
    }
    
    void play(int sequenceId) {
        if (currentSequenceId == sequenceId) return; // Already playing
        currentSequenceId = sequenceId;
        currentFrameIndex = 0;
        ticksInCurrentFrame = 0;
        finished = false;
    }
};

} // namespace FrameZero
