#if defined(_WIN32)
#define NOGDI
#define NOUSER
#endif

#include <raylib.h>
#include "FrameZero.h"
#include <iostream>
#include <string>

using namespace FrameZero;

int main(int argc, char** argv) {
    std::string replayFile = "test_replay.frz";
    if (argc > 1) {
        replayFile = argv[1];
    }
    
    ReplaySystem replaySys;
    if (!replaySys.loadFromFile(replayFile.c_str())) {
        std::cerr << "Failed to load replay file: " << replayFile << "\n";
        return 1;
    }
    
    std::cout << "Loaded Replay: " << replayFile << " with " << replaySys.getFrameCount() << " frames.\n";
    
    // Initialize Raylib
    InitWindow(800, 600, "FrameZero - Replay Viewer");
    SetTargetFPS(60);
    
    // Set up engine purely for playback
    RollbackEngine engine;
    
    // Setup dummy bodies (state gets overwritten by simulation anyway)
    PhysicsBody bodies[2];
    bodies[0].id = 1; bodies[0].setMass(Fixed(1.0)); bodies[0].size = FrameZero::Vector2(Fixed(20.0), Fixed(40.0));
    bodies[1].id = 2; bodies[1].setMass(Fixed(1.0)); bodies[1].size = FrameZero::Vector2(Fixed(20.0), Fixed(40.0));
    engine.setBodies(bodies, 2);
    
    PlayerController p1, p2;
    p1.bind(&engine.bodies[0]);
    p2.bind(&engine.bodies[1]);
    
    InterpolationRenderer renderer;
    renderer.saveCurrentState(engine.bodies, engine.bodyCount);
    
    uint32_t currentReplayFrame = 0;
    bool isPlaying = true;
    
    while (!WindowShouldClose()) {
        // Playback Controls
        if (IsKeyPressed(KEY_SPACE)) isPlaying = !isPlaying;
        if (IsKeyPressed(KEY_RIGHT) && !isPlaying) {
            // Step one frame forward
            if (currentReplayFrame < replaySys.getFrameCount()) {
                engine.simulateFrame(replaySys.getFrameInput(currentReplayFrame, 0), replaySys.getFrameInput(currentReplayFrame, 1));
                currentReplayFrame++;
            }
        }
        
        // Continuous playback
        if (isPlaying && currentReplayFrame < replaySys.getFrameCount()) {
            renderer.savePreviousState(engine.bodies, engine.bodyCount);
            
            Input p1Inp = replaySys.getFrameInput(currentReplayFrame, 0);
            Input p2Inp = replaySys.getFrameInput(currentReplayFrame, 1);
            p1.update(p1Inp);
            p2.update(p2Inp);
            
            engine.simulateFrame(p1Inp, p2Inp);
            currentReplayFrame++;
            
            renderer.saveCurrentState(engine.bodies, engine.bodyCount);
        }
        
        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        PlayerController players[] = {p1, p2};
        DebugRenderer::Draw(renderer.getInterpolatedState(Fixed(1.0)), players, 2);
        
        DrawText(TextFormat("Replay Frame: %d / %d", currentReplayFrame, replaySys.getFrameCount()), 10, 10, 20, DARKGRAY);
        DrawText("SPACE to Pause/Play | RIGHT ARROW to Step Frame", 10, 40, 20, GRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
