# FrameZero Framework Usage Guide

Welcome to **FrameZero**! This guide explains how to plug your own custom game logic into the engine, handle inputs, and use the Rollback-Aware Entity Component System (ECS).

## 1. The Game Logic Callback
In a rollback networking engine, you **cannot** update your game state in a standard `Update()` loop. When a late packet arrives from the opponent, the engine must rewind time and rapidly fast-forward (resimulate) the physics using the corrected inputs.

To achieve this, you must package your game's logic into a callback function and pass it to the `RollbackEngine`.

```cpp
#include <FrameZero.h>
using namespace FrameZero;

void MyGameLogic(RollbackEngine* engine, Input local, Input remote) {
    // 1. Process local input (Player 1)
    if (local.buttons & 1) { // Jump button pressed
        engine->bodies[0].applyForce(Vector2(0, 500));
    }
    
    // 2. Process remote input (Player 2)
    if (remote.buttons & 1) { 
        engine->bodies[1].applyForce(Vector2(0, 500));
    }
    
    // 3. The engine automatically handles gravity and physics integration after this callback!
}

int main() {
    RollbackEngine engine;
    
    // Bind your logic to the engine
    engine.gameLogicCallback = MyGameLogic;
    
    bool gameIsRunning = true;
    while (gameIsRunning) {
        Input local; // = GetLocalInput();
        Input remote; // = GetNetworkInput();
        
        // This will automatically call MyGameLogic(), and if a rollback is needed, 
        // it will rewind and call MyGameLogic() multiple times instantly!
        engine.simulateFrame(local, remote);
    }
    return 0;
}
```

## 2. Using the Rollback-Aware ECS
Standard variables (like `int playerHealth = 100;`) **will desync** during a rollback because the engine doesn't know how to rewind them.

To add custom state to your game (health, timers, ammo), use our ECS. The engine will automatically snapshot and rewind your components.

```cpp
// 1. Define your custom data as a struct
struct Health { int hp; };
struct Timer  { Fixed timeLeft; };

int main() {
    RollbackEngine engine;
    Registry ecs;
    
    // 2. Register components with the ECS
    ecs.registerComponent<Health>();
    ecs.registerComponent<Timer>();
    
    // 3. Tell the Rollback Engine to manage this ECS
    engine.ecsRegistry = &ecs;
    
    // 4. Create entities and attach data
    Entity boss = ecs.create();
    ecs.addComponent(boss, Health{1000});
    
    // Now you can modify this health inside `MyGameLogic`. 
    // If a rollback occurs, the boss's health will automatically be restored!
    return 0;
}
```

## 3. Developer Tools
We provide standalone tools in the `tools/` folder to help you debug your game:
* **FrameZeroReplayViewer**: Renders and visualizes recorded `.frz` replay files.
* **FrameZeroDesyncFinder**: Compares two replay files (e.g., from P1's PC and P2's PC) and pinpoints the exact frame where their simulations desynchronized.
