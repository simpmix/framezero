#ifndef FRAMEZERO_ROLLBACK_NETCODE_H
#define FRAMEZERO_ROLLBACK_NETCODE_H

#include "physics_body.h"
#include "input.h"
#include "state_serialization.h"
#include "collision.h"
#include "ecs.h"
#include <cstdint>
#include <vector>

namespace FrameZero {

// Rollback netcode engine core
class RollbackEngine {
public:
    static constexpr int MAX_FRAME_HISTORY = 128;
    static constexpr int MAX_BODIES = 64;
    
    struct FrameState {
        Input localInput;
        Input remoteInput;
        Input predictedRemote;
        bool hasRemoteInput;
        uint8_t stateSnapshot[StateSerializer::MAX_BODIES * StateSerializer::BODY_SIZE];
        uint8_t ecsSnapshot[8192]; // Up to 8KB of ECS data per frame
        
        FrameState() : hasRemoteInput(false) {}
    };
    
    PhysicsBody bodies[MAX_BODIES];
    int bodyCount;
    
    std::vector<FrameState> frameHistory;
    int currentFrame;
    int confirmedFrame;  // Last frame with confirmed remote input
    bool resimulating = false;
    
    CollisionSystem collision;
    StateSerializer serializer;
    
    // Optional ECS Registry for advanced game state rollback
    class Registry* ecsRegistry;
    
    Fixed gravity;
    Fixed dt;  // Fixed timestep
    
    // User-defined callback for game logic (runs BEFORE physics integration)
    void (*gameLogicCallback)(RollbackEngine* engine, Input localInp, Input remoteInp) = nullptr;
    
    RollbackEngine() 
        : bodyCount(0), currentFrame(0), confirmedFrame(0), ecsRegistry(nullptr)
        , gravity(Fixed(9.8)), dt(Fixed(0.01666)) {
        frameHistory.resize(MAX_FRAME_HISTORY);
    }
    
    void setBodies(PhysicsBody* newBodies, int count) {
        count = (count > MAX_BODIES) ? MAX_BODIES : count;
        for (int i = 0; i < count; i++) {
            bodies[i] = newBodies[i];
        }
        bodyCount = count;
    }
    
    // Save current state to history
    void saveState(int frame) {
        int idx = frame % MAX_FRAME_HISTORY;
        serializer.serialize(bodies, bodyCount);
        memcpy(frameHistory[idx].stateSnapshot, serializer.buffer, serializer.getSize());
        
        if (ecsRegistry) {
            size_t offset = 0;
            ecsRegistry->serialize(frameHistory[idx].ecsSnapshot, offset);
        }
    }
    
    void loadState(int frame) {
        int idx = frame % MAX_FRAME_HISTORY;
        memcpy(serializer.buffer, frameHistory[idx].stateSnapshot, serializer.getSize());
        serializer.deserialize(bodies, bodyCount);
        
        if (ecsRegistry) {
            size_t offset = 0;
            ecsRegistry->deserialize(frameHistory[idx].ecsSnapshot, offset);
        }
    }
    
    // Simulate one frame with given inputs
    void simulateFrame(Input localInp, Input remoteInp) {
        frameHistory[currentFrame % MAX_FRAME_HISTORY].localInput = localInp;
        frameHistory[currentFrame % MAX_FRAME_HISTORY].remoteInput = remoteInp;
        
        // Execute user game logic (e.g., applying player inputs)
            if (gameLogicCallback) {
                gameLogicCallback(this, localInp, remoteInp);
            }
        
        // Universal Gravity
        for (int i = 0; i < bodyCount; i++) {
            if (bodies[i].active && bodies[i].mass.raw != 0) {
                bodies[i].applyForce(Vector2(0, -gravity) * bodies[i].mass);
            }
        }
        
        // Integrate physics
        for (int i = 0; i < bodyCount; i++) {
            bodies[i].integrate(dt);
        }
        
        // Detect and resolve collisions
        collision.detectCollisions(bodies, bodyCount);
        
        saveState(currentFrame);
        currentFrame++;
    }
    
    // Receive remote input (may trigger rollback)
    void receiveRemoteInput(Input actualInput, int frameNumber) {
        if (frameNumber <= confirmedFrame) return;
        
        int framesToRollback = currentFrame - frameNumber;
        
        if (framesToRollback > 0) {
            Input predicted = frameHistory[frameNumber % MAX_FRAME_HISTORY].remoteInput;
            bool mispredicted = (predicted.moveX != actualInput.moveX || 
                                 predicted.moveY != actualInput.moveY || 
                                 predicted.buttons != actualInput.buttons);
            
            if (mispredicted) {
                // Misprediction - need to rollback
                rollbackAndResim(frameNumber, actualInput);
            } else {
                // Correct prediction
                frameHistory[frameNumber % MAX_FRAME_HISTORY].remoteInput = actualInput;
                frameHistory[frameNumber % MAX_FRAME_HISTORY].hasRemoteInput = true;
            }
        } else {
            // No rollback needed
            frameHistory[frameNumber % MAX_FRAME_HISTORY].remoteInput = actualInput;
            frameHistory[frameNumber % MAX_FRAME_HISTORY].hasRemoteInput = true;
        }
        
        confirmedFrame = frameNumber;
    }
    
    // Perform rollback and resimulation
    void rollbackAndResim(int startFrame, Input correctRemoteInput) {
        resimulating = true;
        // Save pre-rollback state for comparison
        uint8_t beforeState[MAX_BODIES * StateSerializer::BODY_SIZE];
        serializer.serialize(bodies, bodyCount);
        memcpy(beforeState, serializer.buffer, serializer.getSize());
        
        // Load state at startFrame
        loadState(startFrame);
        
        // Update the incorrect input with correct one
        frameHistory[startFrame % MAX_FRAME_HISTORY].remoteInput = correctRemoteInput;
        frameHistory[startFrame % MAX_FRAME_HISTORY].hasRemoteInput = true;
        
        // Resimulate all frames from startFrame to currentFrame
        for (int f = startFrame; f < currentFrame; f++) {
            Input localInp = frameHistory[f % MAX_FRAME_HISTORY].localInput;
            Input remoteInp = frameHistory[f % MAX_FRAME_HISTORY].remoteInput;
            
            // Clear forces
            for (int i = 0; i < bodyCount; i++) {
                bodies[i].acceleration = Vector2(0, 0);
            }
            
            // Execute user game logic (e.g., applying player inputs)
            if (gameLogicCallback) {
                gameLogicCallback(this, localInp, remoteInp);
            }
            
            // Universal Gravity
            for (int i = 0; i < bodyCount; i++) {
                if (bodies[i].active && bodies[i].mass.raw != 0) {
                    bodies[i].applyForce(Vector2(0, -gravity) * bodies[i].mass);
                }
            }
            
            for (int i = 0; i < bodyCount; i++) {
                bodies[i].integrate(dt);
            }
            
            collision.detectCollisions(bodies, bodyCount);
            saveState(f);
        }
        
        // Verify state changed (rollback was necessary)
        serializer.serialize(bodies, bodyCount);
        bool stateChanged = (memcmp(beforeState, serializer.buffer, serializer.getSize()) != 0);
        
        // In production: log stats, update render interpolation targets
        (void)stateChanged;  // Suppress unused warning
        resimulating = false;
    }
    
    // Predict remote input when packet hasn't arrived
    Input predictRemoteInput() {
        Input predicted;
        if (confirmedFrame >= currentFrame - 1) {
            // Have recent input, use last known
            predicted = frameHistory[confirmedFrame % MAX_FRAME_HISTORY].remoteInput;
        } else {
            // Simple prediction: assume same as last confirmed
            predicted = frameHistory[confirmedFrame % MAX_FRAME_HISTORY].remoteInput;
        }
        
        // Smart Prediction: Sustain directional movement, but clear action buttons 
        // to prevent false-positive button mashing during network lag.
        predicted.buttons = 0;
        
        return predicted;
    }
    
    int getCurrentFrame() const { return currentFrame; }
    int getConfirmedFrame() const { return confirmedFrame; }
    bool isResimulating() const { return resimulating; }
    
    // Get checksum of the last confirmed frame to broadcast for desync detection
    uint32_t getConfirmedChecksum() {
        int idx = confirmedFrame % MAX_FRAME_HISTORY;
        // Temporary load to compute hash
        serializer.dataSize = bodyCount * StateSerializer::BODY_SIZE;
        memcpy(serializer.buffer, frameHistory[idx].stateSnapshot, serializer.dataSize);
        return serializer.computeChecksum();
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_ROLLBACK_NETCODE_H
