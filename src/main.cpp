#include <raylib.h>
#include "rollback_netcode.h"
#include "player_controller.h"
#include "interpolation_renderer.h"
#include "renderer.h"
#include "input.h"

using namespace FrameZero;

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "FrameZero Engine Demo");
    SetTargetFPS(60);

    RollbackEngine engine;
    
    // Initialize 2 bodies
    PhysicsBody initialBodies[2];
    initialBodies[0].position = FrameZero::Vector2(Fixed(200.0), Fixed(100.0));
    initialBodies[0].size = FrameZero::Vector2(Fixed(20.0), Fixed(40.0));
    initialBodies[0].id = 1;
    initialBodies[0].active = true;
    initialBodies[0].setMass(Fixed(1.0));
    
    initialBodies[1].position = FrameZero::Vector2(Fixed(600.0), Fixed(100.0));
    initialBodies[1].size = FrameZero::Vector2(Fixed(20.0), Fixed(40.0));
    initialBodies[1].id = 2;
    initialBodies[1].active = true;
    initialBodies[1].setMass(Fixed(1.0));

    engine.setBodies(initialBodies, 2);

    PlayerController pc1, pc2;
    pc1.bind(&engine.bodies[0]);
    pc2.bind(&engine.bodies[1]);

    InterpolationRenderer renderer;
    
    // Fixed timestep logic
    double accumulator = 0.0;
    const double dt = 1.0 / 60.0;

    while (!WindowShouldClose()) {
        double frameTime = GetFrameTime();
        if (frameTime > 0.25) frameTime = 0.25; // prevent spiral of death
        accumulator += frameTime;

        Input p1_input, p2_input;
        
        if (IsKeyDown(KEY_W)) p1_input.moveY = 127;
        else if (IsKeyDown(KEY_S)) p1_input.moveY = -127;
        if (IsKeyDown(KEY_D)) p1_input.moveX = 127;
        else if (IsKeyDown(KEY_A)) p1_input.moveX = -127;
        if (IsKeyDown(KEY_F)) p1_input.buttons |= 1;

        if (IsKeyDown(KEY_UP)) p2_input.moveY = 127;
        else if (IsKeyDown(KEY_DOWN)) p2_input.moveY = -127;
        if (IsKeyDown(KEY_RIGHT)) p2_input.moveX = 127;
        else if (IsKeyDown(KEY_LEFT)) p2_input.moveX = -127;
        if (IsKeyDown(KEY_ENTER)) p2_input.buttons |= 1;

        while (accumulator >= dt) {
            // Save previous state
            renderer.savePreviousState(engine.bodies, engine.bodyCount);

            // Update controllers
            pc1.update(p1_input);
            pc2.update(p2_input);

            // engine.simulateFrame has hardcoded gravity for body 0 only
            // Manually add gravity to body 1 to compensate
            if (engine.bodyCount > 1) {
                engine.bodies[1].applyForce(FrameZero::Vector2(0, -engine.gravity) * engine.bodies[1].mass);
            }

            // Advance frame (simulateFrame handles local/remote inputs)
            engine.simulateFrame(p1_input, p2_input);

            // Save current state
            renderer.saveCurrentState(engine.bodies, engine.bodyCount);

            accumulator -= dt;
        }

        // Interpolation
        Fixed alpha(accumulator / dt);
        auto renderStates = renderer.getInterpolatedState(alpha);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        PlayerController controllers[2] = { pc1, pc2 };
        DebugRenderer::Draw(renderStates, controllers, 2);
        
        DrawText("FrameZero Engine - WASD/F for P1 | Arrows/Enter for P2", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
