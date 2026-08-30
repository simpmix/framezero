#ifndef FRAMEZERO_RENDERER_H
#define FRAMEZERO_RENDERER_H

#include "interpolation_renderer.h"
#include "player_controller.h"
#include <raylib.h>
#include <vector>

namespace FrameZero {

class DebugRenderer {
public:
    static void Draw(const std::vector<RenderState>& states, const PlayerController* players, int playerCount) {
        float screenHeight = static_cast<float>(GetScreenHeight());
        
        // Draw bodies
        for (const auto& state : states) {
            if (!state.active) continue;
            
            float px = static_cast<float>(state.position.x.toDouble());
            float py = static_cast<float>(state.position.y.toDouble());
            float sx = static_cast<float>(state.size.x.toDouble());
            float sy = static_cast<float>(state.size.y.toDouble());
            
            // Map Y so 0 is at bottom
            float drawX = px - sx;
            float drawY = screenHeight - (py + sy);
            float drawW = sx * 2.0f;
            float drawH = sy * 2.0f;
            
            DrawRectangleRec({drawX, drawY, drawW, drawH}, BLUE);
        }
        
        // Draw hitboxes and hurtboxes from players
        for (int i = 0; i < playerCount; ++i) {
            const PlayerController& pc = players[i];
            
            if (!pc.body || !pc.body->active) continue;
            
            float px = static_cast<float>(pc.body->position.x.toDouble());
            float py = static_cast<float>(pc.body->position.y.toDouble());
            
            // Draw hurtbox (Green)
            float bx = static_cast<float>(pc.bodyHurtbox.offset.x.toDouble());
            float by = static_cast<float>(pc.bodyHurtbox.offset.y.toDouble());
            float hsx = static_cast<float>(pc.bodyHurtbox.size.x.toDouble());
            float hsy = static_cast<float>(pc.bodyHurtbox.size.y.toDouble());
            
            float hurtDrawX = px + bx - hsx;
            float hurtDrawY = screenHeight - (py + by + hsy);
            float hurtDrawW = hsx * 2.0f;
            float hurtDrawH = hsy * 2.0f;
            
            DrawRectangleLinesEx({hurtDrawX, hurtDrawY, hurtDrawW, hurtDrawH}, 2.0f, GREEN);
            
            // Draw hitbox (Red)
            if (pc.attackHitbox.active) {
                float ax = static_cast<float>(pc.attackHitbox.offset.x.toDouble());
                float ay = static_cast<float>(pc.attackHitbox.offset.y.toDouble());
                float asx = static_cast<float>(pc.attackHitbox.size.x.toDouble());
                float asy = static_cast<float>(pc.attackHitbox.size.y.toDouble());
                
                float hitDrawX = px + ax - asx;
                float hitDrawY = screenHeight - (py + ay + asy);
                float hitDrawW = asx * 2.0f;
                float hitDrawH = asy * 2.0f;
                
                DrawRectangleLinesEx({hitDrawX, hitDrawY, hitDrawW, hitDrawH}, 2.0f, RED);
            }
        }
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_RENDERER_H
