#ifndef FRAMEZERO_AUDIO_SYSTEM_H
#define FRAMEZERO_AUDIO_SYSTEM_H

#include <raylib.h>
#include "rollback_netcode.h"

namespace FrameZero {

class RollbackAudio {
private:
    static constexpr int MAX_SOUNDS_PER_FRAME = 16;
    RollbackEngine* engine;
    int lastFrame;
    void* playedThisFrame[MAX_SOUNDS_PER_FRAME];
    int playedCount;

public:
    RollbackAudio() : engine(nullptr), lastFrame(-1), playedCount(0) {}

    void initialize(RollbackEngine* eng) {
        engine = eng;
        lastFrame = -1;
        playedCount = 0;
    }

    void playSound(Sound sound) {
        if (!engine) return;

        // Prevent ear-rape during resimulations (rollback frames)
        if (engine->isResimulating()) {
            return;
        }

        int currentFrame = engine->getCurrentFrame();
        
        // Clear tracker if we moved to a new logic frame
        if (currentFrame != lastFrame) {
            playedCount = 0;
            lastFrame = currentFrame;
        }

        void* soundId = sound.stream.buffer;
        
        // Prevent stacking the exact same sound 5 times on the exact same frame
        for (int i = 0; i < playedCount; i++) {
            if (playedThisFrame[i] == soundId) {
                return; // Already played this frame
            }
        }

        // Add to tracker and play if we have space
        if (playedCount < MAX_SOUNDS_PER_FRAME) {
            playedThisFrame[playedCount++] = soundId;
            PlaySound(sound);
        }
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_AUDIO_SYSTEM_H
