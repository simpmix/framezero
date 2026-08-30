#ifndef FRAMEZERO_AUDIO_SYSTEM_H
#define FRAMEZERO_AUDIO_SYSTEM_H

#include <raylib.h>
#include "rollback_netcode.h"
#include <vector>

namespace FrameZero {

class RollbackAudio {
private:
    RollbackEngine* engine;
    int lastFrame;
    std::vector<void*> playedThisFrame;

public:
    RollbackAudio() : engine(nullptr), lastFrame(-1) {}

    void initialize(RollbackEngine* eng) {
        engine = eng;
        lastFrame = -1;
        playedThisFrame.clear();
    }

    void playSound(Sound sound) {
        if (!engine) return;

        // Prevent ear-rape during resimulations
        if (engine->isResimulating()) {
            return;
        }

        int currentFrame = engine->getCurrentFrame();
        
        // Clear tracker if we moved to a new frame
        if (currentFrame != lastFrame) {
            playedThisFrame.clear();
            lastFrame = currentFrame;
        }

        // Check if the exact same sound played this frame
        void* soundId = sound.stream.buffer;
        for (void* played : playedThisFrame) {
            if (played == soundId) {
                return; // Already played this frame
            }
        }

        // Add to tracker and play
        playedThisFrame.push_back(soundId);
        PlaySound(sound);
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_AUDIO_SYSTEM_H
