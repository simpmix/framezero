#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include <vector>

namespace FrameZero {

// A deterministic visual effect (e.g. Hit Spark, Dust Cloud)
struct VisualEffect {
    int id;
    Vector2 position;
    int animationId;
    int currentFrame;
    int totalFrames;
    bool active;
    
    // Size is exactly 24 bytes (4 + 8 + 4 + 4 + 4)
    void serialize(uint8_t* buffer) const {
        memcpy(buffer, &id, 4);
        memcpy(buffer + 4, &position.x.raw, 4);
        memcpy(buffer + 8, &position.y.raw, 4);
        memcpy(buffer + 12, &animationId, 4);
        memcpy(buffer + 16, &currentFrame, 4);
        memcpy(buffer + 20, &totalFrames, 4);
    }
    
    void deserialize(const uint8_t* buffer) {
        memcpy(&id, buffer, 4);
        memcpy(&position.x.raw, buffer + 4, 4);
        memcpy(&position.y.raw, buffer + 8, 4);
        memcpy(&animationId, buffer + 12, 4);
        memcpy(&currentFrame, buffer + 16, 4);
        memcpy(&totalFrames, buffer + 20, 4);
        active = true;
    }
    
    static constexpr size_t getSize() { return 24; }
};

// Manages a fixed pool of visual effects for deterministic rollback
class EffectManager {
public:
    static constexpr int MAX_EFFECTS = 32;
    VisualEffect effects[MAX_EFFECTS];
    int nextId = 1;
    
    EffectManager() {
        for (int i = 0; i < MAX_EFFECTS; i++) {
            effects[i].active = false;
        }
    }
    
    void spawnEffect(Vector2 pos, int animId, int frames) {
        for (int i = 0; i < MAX_EFFECTS; i++) {
            if (!effects[i].active) {
                effects[i].id = nextId++;
                effects[i].position = pos;
                effects[i].animationId = animId;
                effects[i].currentFrame = 0;
                effects[i].totalFrames = frames;
                effects[i].active = true;
                return;
            }
        }
    }
    
    void advanceFrame() {
        for (int i = 0; i < MAX_EFFECTS; i++) {
            if (effects[i].active) {
                effects[i].currentFrame++;
                if (effects[i].currentFrame >= effects[i].totalFrames) {
                    effects[i].active = false; // Despawn
                }
            }
        }
    }
    
    void serialize(uint8_t* buffer) const {
        memcpy(buffer, &nextId, 4);
        buffer += 4;
        
        int activeCount = 0;
        for (int i = 0; i < MAX_EFFECTS; i++) {
            if (effects[i].active) activeCount++;
        }
        
        memcpy(buffer, &activeCount, 4);
        buffer += 4;
        
        for (int i = 0; i < MAX_EFFECTS; i++) {
            if (effects[i].active) {
                effects[i].serialize(buffer);
                buffer += VisualEffect::getSize();
            }
        }
    }
    
    void deserialize(const uint8_t* buffer) {
        for (int i = 0; i < MAX_EFFECTS; i++) effects[i].active = false;
        
        memcpy(&nextId, buffer, 4);
        buffer += 4;
        
        int activeCount = 0;
        memcpy(&activeCount, buffer, 4);
        buffer += 4;
        
        for (int i = 0; i < activeCount; i++) {
            effects[i].deserialize(buffer);
            buffer += VisualEffect::getSize();
        }
    }
};

} // namespace FrameZero
