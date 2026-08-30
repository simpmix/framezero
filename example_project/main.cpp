#include <FrameZero.h>
#include <iostream>

using namespace FrameZero;

// A custom game component that we want rollback to track!
struct HealthComponent {
    int hp;
    int maxHp;
};

struct ProjectileComponent {
    Fixed speed;
    int lifetimeFrames;
};

int main() {
    std::cout << "--- FrameZero Engine: Rollback-Aware ECS Example ---\n";
    
    RollbackEngine engine;
    
    // 1. Setup the ECS Registry
    Registry ecs;
    ecs.registerComponent<HealthComponent>();
    ecs.registerComponent<ProjectileComponent>();
    
    // Tell the rollback engine to automatically snapshot and rewind our ECS
    engine.ecsRegistry = &ecs;
    
    // 2. Create entities and attach components
    Entity player1 = ecs.create();
    ecs.addComponent(player1, HealthComponent{100, 100});
    
    Entity player2 = ecs.create();
    ecs.addComponent(player2, HealthComponent{100, 100});
    
    // 3. Simulate forward 10 frames where Player 2 takes damage
    for (int i = 0; i < 10; ++i) {
        if (i == 5) {
            // Player 2 gets hit on frame 5
            ecs.getComponent<HealthComponent>(player2).hp -= 15;
            std::cout << "Frame 5: Player 2 took damage! HP is now " 
                      << ecs.getComponent<HealthComponent>(player2).hp << "\n";
        }
        engine.simulateFrame(Input(), Input());
    }
    
    std::cout << "Frame 10: Player 2 HP is " << ecs.getComponent<HealthComponent>(player2).hp << "\n";
    
    // 4. Oh no! A late packet arrived for Frame 3 from Player 2.
    // The engine needs to rollback 7 frames and resimulate!
    std::cout << "--- Triggering Rollback to Frame 3 ---\n";
    Input lateInput;
    lateInput.moveX = 127; // Player 2 moved out of the way!
    
    // Simulating how you would intercept the resimulation loop in a real game
    // For the sake of this standalone test, we'll just demonstrate the memory rollback directly
    engine.loadState(3); // Manually rewind memory to Frame 3
    
    std::cout << "Frame 3 (After Rewind): Player 2 HP is " 
              << ecs.getComponent<HealthComponent>(player2).hp << "\n";
              
    if (ecs.getComponent<HealthComponent>(player2).hp == 100) {
        std::cout << "SUCCESS: The ECS automatically restored the custom HealthComponent during rollback!\n";
    } else {
        std::cout << "FAILED: ECS rollback did not work.\n";
    }
    
    return 0;
}
