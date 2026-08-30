#include <raylib.h>
#include "rollback_netcode.h"
#include "player_controller.h"
#include "interpolation_renderer.h"
#include "renderer.h"
#include "input.h"

#include "editor_gui.h"

using namespace FrameZero;

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "FrameZero Engine Demo");
    SetTargetFPS(60);
    
    rlImGuiSetup(true);

    RollbackEngine engine;
    EditorGUI editor;
    
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
        
        // Prevent game input when interacting with UI
        if (!ImGui::GetIO().WantCaptureKeyboard) {
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
        }

        while (accumulator >= dt) {
            renderer.savePreviousState(engine.bodies, engine.bodyCount);

            pc1.update(p1_input);
            pc2.update(p2_input);
            engine.simulateFrame(p1_input, p2_input);

            renderer.saveCurrentState(engine.bodies, engine.bodyCount);
            accumulator -= dt;
        }

        Fixed alpha(accumulator / dt);
        int renderCount = 0;
        const auto* renderStates = renderer.getInterpolatedState(alpha, renderCount);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        PlayerController players[2] = { pc1, pc2 };
        if (editor.showPhysicsBodies || editor.showHitboxes) {
            DebugRenderer::Draw(renderStates, renderCount, players, 2);
        }
        
        DrawText("FrameZero Engine - WASD/F for P1 | Arrows/Enter for P2", 10, 10, 20, DARKGRAY);

        // GUI Overlay
        rlImGuiBegin();
        editor.draw(&engine, players, 2);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
