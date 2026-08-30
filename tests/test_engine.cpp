#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <chrono>
#include <fstream>
#include <string>

#include "fixed_point.h"
#include "vector2.h"
#include "input.h"
#include "physics_body.h"
#include "collision.h"
#include "state_serialization.h"
#include "replay_system.h"
#include "delta_compression.h"
#include "rollback_netcode.h"
#include "player_controller.h"
#include "interpolation_renderer.h"

using namespace FrameZero;

// Test counters
int testsPassed = 0;
int testsFailed = 0;

#define TEST_ASSERT(condition, testName) \
    if (condition) { \
        std::cout << "[PASS] " << testName << std::endl; \
        testsPassed++; \
    } else { \
        std::cout << "[FAIL] " << testName << std::endl; \
        testsFailed++; \
    }

void test_fixed_point() {
    std::cout << "\n=== Testing Fixed-Point Math ===" << std::endl;
    
    Fixed a(5.5);
    Fixed b(2.0);
    
    Fixed sum = a + b;
    std::cout << "5.5 + 2.0 = " << sum.toDouble() << " (expected: 7.5)" << std::endl;
    TEST_ASSERT(std::abs(sum.toDouble() - 7.5) < 0.01, "Addition");
    
    Fixed diff = a - b;
    std::cout << "5.5 - 2.0 = " << diff.toDouble() << " (expected: 3.5)" << std::endl;
    TEST_ASSERT(std::abs(diff.toDouble() - 3.5) < 0.01, "Subtraction");
    
    Fixed prod = a * b;
    std::cout << "5.5 * 2.0 = " << prod.toDouble() << " (expected: 11.0)" << std::endl;
    TEST_ASSERT(std::abs(prod.toDouble() - 11.0) < 0.01, "Multiplication");
    
    Fixed quot = a / b;
    std::cout << "5.5 / 2.0 = " << quot.toDouble() << " (expected: 2.75)" << std::endl;
    TEST_ASSERT(std::abs(quot.toDouble() - 2.75) < 0.01, "Division");
    
    // Determinism test
    Fixed x1(3.14159);
    Fixed x2(3.14159);
    TEST_ASSERT(x1.raw == x2.raw, "Determinism test");
}

void test_vector2() {
    std::cout << "\n=== Testing Vector2 ===" << std::endl;
    
    Vector2 v1(3.0, 4.0);
    Vector2 v2(1.0, 0.0);
    
    Vector2 sum = v1 + v2;
    std::cout << "(3,4) + (1,0) = (" << sum.x.toDouble() << "," << sum.y.toDouble() << ")" << std::endl;
    TEST_ASSERT(sum.x.toInt() == 4 && sum.y.toInt() == 4, "Vector addition");
    
    Fixed dot = v1.dot(v2);
    std::cout << "Dot product: " << dot.toDouble() << " (expected: 3.0)" << std::endl;
    TEST_ASSERT(std::abs(dot.toDouble() - 3.0) < 0.01, "Dot product");
    
    Fixed len = v1.length();
    std::cout << "Length of (3,4): " << len.toDouble() << " (expected: 5.0)" << std::endl;
    TEST_ASSERT(std::abs(len.toDouble() - 5.0) < 0.01, "Vector length");
}

void test_physics_body() {
    std::cout << "\n=== Testing Physics Body ===" << std::endl;
    
    PhysicsBody body;
    body.position = Vector2(0.0, 0.0);
    body.velocity = Vector2(1.0, 0.0);
    body.setMass(Fixed(1.0));
    
    // Apply force
    body.applyForce(Vector2(0.0, -9.8));
    
    // Integrate
    Fixed dt(0.1);
    body.integrate(dt);
    
    std::cout << "Position after integration: (" 
              << body.position.x.toDouble() << ", " 
              << body.position.y.toDouble() << ")" << std::endl;
    
    TEST_ASSERT(body.position.x.toDouble() > 0.0, "X position changed");
    TEST_ASSERT(body.position.y.toDouble() < 0.0, "Y position affected by gravity");
    
    // Serialization test
    uint8_t buffer[64];
    body.serialize(buffer);
    
    PhysicsBody body2;
    body2.deserialize(buffer);
    
    TEST_ASSERT(body.position.x.raw == body2.position.x.raw, "Serialize/Deserialize X");
    TEST_ASSERT(body.position.y.raw == body2.position.y.raw, "Serialize/Deserialize Y");
}

void test_collision_detection() {
    std::cout << "\n=== Testing Collision Detection ===" << std::endl;
    
    PhysicsBody bodyA, bodyB;
    bodyA.position = Vector2(0.0, 0.0);
    bodyA.size = Vector2(1.0, 1.0);
    bodyB.position = Vector2(1.5, 0.0);
    bodyB.size = Vector2(1.0, 1.0);
    
    bool overlap = CollisionSystem::checkOverlap(bodyA, bodyB);
    std::cout << "Bodies overlapping: " << (overlap ? "yes" : "no") << " (expected: yes)" << std::endl;
    TEST_ASSERT(overlap, "Overlapping bodies detected");
    
    // Non-overlapping
    bodyB.position = Vector2(5.0, 0.0);
    overlap = CollisionSystem::checkOverlap(bodyA, bodyB);
    std::cout << "Distant bodies overlapping: " << (overlap ? "yes" : "no") << " (expected: no)" << std::endl;
    TEST_ASSERT(!overlap, "Non-overlapping bodies correct");
}

void test_input_queue() {
    std::cout << "\n=== Testing Input Queue ===" << std::endl;
    
    InputQueue queue;
    
    Input inp1;
    inp1.moveX = 100;
    inp1.buttons = 1;
    
    Input inp2;
    inp2.moveX = -50;
    inp2.buttons = 2;
    
    queue.push(inp1);
    queue.push(inp2);
    
    TEST_ASSERT(queue.getSize() == 2, "Queue size after push");
    
    Input popped = queue.pop();
    TEST_ASSERT(popped.moveX == 100, "First input popped correctly");
    TEST_ASSERT(queue.getSize() == 1, "Queue size after pop");
}

void test_state_serialization_performance() {
    std::cout << "\n=== Testing State Serialization Performance ===" << std::endl;
    
    const int BODY_COUNT = 100;
    PhysicsBody* bodies = new PhysicsBody[BODY_COUNT];
    
    for (int i = 0; i < BODY_COUNT; i++) {
        bodies[i].position = Vector2(i * 0.1, i * 0.1);
        bodies[i].velocity = Vector2(0.5, 0.5);
    }
    
    StateSerializer serializer;
    
    double avgTime = serializer.benchmarkSerialization(bodies, BODY_COUNT, 1000);
    
    std::cout << "Average serialization time for " << BODY_COUNT 
              << " bodies: " << avgTime << " ms" << std::endl;
    std::cout << "Target: < 1.0 ms" << std::endl;
    
    TEST_ASSERT(avgTime < 1.0, "Serialization under 1ms target");
    
    delete[] bodies;
}

void test_determinism() {
    std::cout << "\n=== Testing Cross-Simulation Determinism ===" << std::endl;
    
    // Create two identical simulations
    PhysicsBody sim1[2], sim2[2];
    
    sim1[0].position = Vector2(0.0, 10.0);
    sim1[0].velocity = Vector2(2.0, 0.0);
    sim1[0].setMass(Fixed(1.0));
    sim1[1] = sim1[0];
    
    memcpy(sim2, sim1, sizeof(sim1));
    
    Fixed dt(0.016);
    
    // Run 100 frames on both
    for (int i = 0; i < 100; i++) {
        sim1[0].applyForce(Vector2(0.0, -Fixed(9.8) * sim1[0].mass));
        sim1[0].integrate(dt);
        
        sim2[0].applyForce(Vector2(0.0, -Fixed(9.8) * sim2[0].mass));
        sim2[0].integrate(dt);
    }
    
    std::cout << "Sim1 position: (" << sim1[0].position.x.toDouble() 
              << ", " << sim1[0].position.y.toDouble() << ")" << std::endl;
    std::cout << "Sim2 position: (" << sim2[0].position.x.toDouble() 
              << ", " << sim2[0].position.y.toDouble() << ")" << std::endl;
    
    TEST_ASSERT(sim1[0].position.x.raw == sim2[0].position.x.raw, "X position deterministic");
    TEST_ASSERT(sim1[0].position.y.raw == sim2[0].position.y.raw, "Y position deterministic");
}

void test_replay_system() {
    std::cout << "\n=== Testing Replay System ===" << std::endl;
    
    ReplaySystem replay;
    replay.startRecording(2);
    
    // Record 100 frames
    for (int i = 0; i < 100; i++) {
        Input inputs[2];
        inputs[0].moveX = static_cast<int8_t>(i % 127);
        inputs[0].buttons = (i % 8);
        inputs[1].moveX = static_cast<int8_t>((i * 2) % 127);
        inputs[1].buttons = ((i * 3) % 8);
        
        replay.recordFrame(inputs, 2);
    }
    
    TEST_ASSERT(replay.getFrameCount() == 100, "Recorded 100 frames");
    
    // Save to file
    bool saved = replay.saveToFile("test_replay.frz");
    TEST_ASSERT(saved, "Replay saved to file");
    
    // Load from file
    ReplaySystem loadedReplay;
    bool loaded = loadedReplay.loadFromFile("test_replay.frz");
    TEST_ASSERT(loaded, "Replay loaded from file");
    
    // Verify determinism
    bool deterministic = replay.verifyDeterminism(loadedReplay);
    TEST_ASSERT(deterministic, "Replay determinism verified");
    
    std::cout << "Replay file size: ~" << (100 * 12 + 20) << " bytes for 100 frames" << std::endl;
}

void test_delta_compression() {
    std::cout << "\n=== Testing Delta Compression ===" << std::endl;
    
    DeltaCompressor compressor;
    
    // Simulate sequence of inputs
    Input inputs[5];
    inputs[0].moveX = 100;
    inputs[0].moveY = 0;
    inputs[0].buttons = 1;
    
    inputs[1].moveX = 100;  // Same as before
    inputs[1].moveY = 0;
    inputs[1].buttons = 1;
    
    inputs[2].moveX = 100;
    inputs[2].moveY = 50;   // Changed
    inputs[2].buttons = 1;
    
    inputs[3].moveX = -50;  // Changed
    inputs[3].moveY = 50;
    inputs[3].buttons = 3;  // Changed
    
    inputs[4].moveX = -50;
    inputs[4].moveY = 50;
    inputs[4].buttons = 3;
    
    size_t totalCompressed = 0;
    size_t totalUncompressed = 0;
    
    for (int i = 0; i < 5; i++) {
        DeltaCompressor::DeltaInput delta = compressor.compress(inputs[i]);
        size_t deltaSize = DeltaCompressor::getDeltaSize(delta);
        totalCompressed += deltaSize;
        totalUncompressed += DeltaCompressor::getFullInputSize();
        
        // Decompress and verify
        Input decompressed = compressor.decompress(delta);
        TEST_ASSERT(decompressed.moveX == inputs[i].moveX, 
                   ("Decompress moveX frame " + std::to_string(i)).c_str());
        TEST_ASSERT(decompressed.moveY == inputs[i].moveY, 
                   ("Decompress moveY frame " + std::to_string(i)).c_str());
        TEST_ASSERT(decompressed.buttons == inputs[i].buttons, 
                   ("Decompress buttons frame " + std::to_string(i)).c_str());
    }
    
    size_t savings = totalUncompressed - totalCompressed;
    double percent = (savings * 100.0) / totalUncompressed;
    
    std::cout << "Original: " << totalUncompressed << " bytes" << std::endl;
    std::cout << "Compressed: " << totalCompressed << " bytes" << std::endl;
    std::cout << "Savings: " << savings << " bytes (" << percent << "%)" << std::endl;
    
    TEST_ASSERT(percent > 0, "Delta compression achieves some savings");
}

void test_rollback_netcode() {
    std::cout << "\n=== Testing Rollback Netcode ===" << std::endl;
    
    RollbackEngine engine;
    
    // Setup physics bodies
    PhysicsBody bodies[2];
    bodies[0].position = Vector2(0.0, 5.0);
    bodies[0].setMass(Fixed(1.0));
    bodies[0].size = Vector2(0.5, 0.5);
    
    engine.setBodies(bodies, 2);
    
    // Simulate 10 frames with predicted inputs
    for (int i = 0; i < 10; i++) {
        Input localInp;
        localInp.moveX = 50;
        localInp.buttons = 1;
        
        Input predictedRemote;
        predictedRemote.moveX = -30;  // Predicted
        predictedRemote.buttons = 0;
        
        engine.simulateFrame(localInp, predictedRemote);
    }
    
    std::cout << "Simulated to frame: " << engine.getCurrentFrame() << std::endl;
    std::cout << "Confirmed frame: " << engine.getConfirmedFrame() << std::endl;
    
    TEST_ASSERT(engine.getCurrentFrame() == 10, "Advanced 10 frames");
    
    // Now receive actual remote input for frame 5 (triggering rollback)
    Input actualRemote;
    actualRemote.moveX = 80;  // Different from prediction!
    actualRemote.buttons = 2;
    
    engine.receiveRemoteInput(actualRemote, 5);
    
    std::cout << "Received late input for frame 5, triggered rollback" << std::endl;
    std::cout << "Current frame after rollback: " << engine.getCurrentFrame() << std::endl;
    
    TEST_ASSERT(engine.getCurrentFrame() == 10, "Still at frame 10 after rollback");
    TEST_ASSERT(engine.getConfirmedFrame() == 5, "Confirmed frame updated");
    
    std::cout << "Rollback netcode test completed successfully!" << std::endl;
}

void test_player_controller() {
    std::cout << "\n=== Testing Player Controller ===" << std::endl;
    
    PhysicsBody body;
    body.position = Vector2(0.0, 10.0);
    body.size = Vector2(1.0, 1.0);
    body.setMass(Fixed(1.0));
    
    PlayerController player;
    player.bind(&body);
    
    Input input;
    input.moveX = 127; // Max right
    input.moveY = 0;
    input.buttons = BTN_PUNCH; // Attack
    
    player.update(input);
    
    TEST_ASSERT(player.isAttacking, "Player attacks on button press");
    TEST_ASSERT(player.attackFrameTimer > 0, "Attack timer set");
    TEST_ASSERT(player.facingDirection == 1, "Player facing right");
}

void test_interpolation_renderer() {
    std::cout << "\n=== Testing Interpolation Renderer ===" << std::endl;
    
    PhysicsBody bodies[1];
    bodies[0].id = 1;
    bodies[0].active = true;
    bodies[0].position = Vector2(0.0, 0.0);
    
    InterpolationRenderer renderer;
    renderer.savePreviousState(bodies, 1);
    
    // Simulate frame (body moves to x=10)
    bodies[0].position = Vector2(10.0, 0.0);
    renderer.saveCurrentState(bodies, 1);
    
    // Interpolate at 50%
    int interpCount = 0;
    const auto* interpState = renderer.getInterpolatedState(Fixed(0.5), interpCount);
    
    TEST_ASSERT(interpCount == 1, "Got 1 interpolated state");
    
    double interpX = interpState[0].position.x.toDouble();
    std::cout << "Interpolated X at alpha 0.5: " << interpX << " (expected: 5.0)" << std::endl;
    TEST_ASSERT(std::abs(interpX - 5.0) < 0.01, "Correct interpolation");
}

void test_combat_hitstop() {
    std::cout << "\n=== Testing Combat System & Hitstop ===" << std::endl;
    
    PhysicsBody p1_body, p2_body;
    p1_body.setMass(Fixed(1.0));
    p2_body.setMass(Fixed(1.0));
    p1_body.position = Vector2(0.0, 0.0);
    p2_body.position = Vector2(3.0, 0.0);
    
    PlayerController p1, p2;
    p1.bind(&p1_body);
    p2.bind(&p2_body);
    
    // Simulate attack
    Input attackInput;
    attackInput.buttons = BTN_PUNCH; // trigger attack
    attackInput.moveX = 0; attackInput.moveY = 0;
    
    Input idleInput; idleInput.buttons = 0; idleInput.moveX = 0; idleInput.moveY = 0;
    
    // P1 attacks
    p1.update(attackInput);
    
    // Step until hitbox active (attack lasts 15 frames, active at <=10)
    for (int i = 0; i < 5; i++) {
        p1.update(idleInput);
    }
    
    TEST_ASSERT(p1.attackHitbox.active, "P1 hitbox became active");
    
    // Check hit
    bool hit = CombatSystem::checkHit(*p1.body, p1.attackHitbox, *p2.body, p2.bodyHurtbox);
    TEST_ASSERT(hit, "P1 attack connects with P2");
    
    if (hit) {
        CombatSystem::processHit(p1.body, p1.attackHitbox, p2.body, p2.health);
    }
    
    TEST_ASSERT(p2.health < Fixed(100.0), "P2 took damage");
    TEST_ASSERT(p1.body->freezeFrames == 5, "P1 entered hitstop");
    TEST_ASSERT(p2.body->freezeFrames == 5, "P2 entered hitstop");
    TEST_ASSERT(!p1.attackHitbox.active, "Hitbox deactivated after hit");
    
    // Test physics integration during hitstop
    p1.body->velocity = Vector2(10.0, 0.0); // Attempt to move
    p1.body->integrate(Fixed(0.016));
    TEST_ASSERT(p1.body->position.x.raw == 0, "Position didn't change during hitstop");
    TEST_ASSERT(p1.body->freezeFrames == 4, "Freeze frames decremented");
}

void test_deterministic_trig() {
    // Check sin and cos properties
    Fixed pi = Fixed::pi();
    
    TEST_ASSERT(Fixed::sin(Fixed(0)).raw == 0, "sin(0) == 0");
    TEST_ASSERT(Fixed::cos(Fixed(0)).raw == 65536, "cos(0) == 1.0"); // 1.0 in 16.16
    
    // Atan2 properties
    Fixed y(1.0);
    Fixed x(1.0);
    Fixed angle = Fixed::atan2(y, x);
    // Atan2(1,1) should be PI/4 (0.785398)
    Fixed pi4 = pi / Fixed(4);
    TEST_ASSERT(std::abs(angle.toDouble() - pi4.toDouble()) < 0.01, "atan2(1,1) == PI/4");
}

struct TestECSComponent {
    int value;
    Fixed timer;
};

void test_ecs_serialization() {
    RollbackEngine engine;
    Registry ecs;
    ecs.registerComponent<TestECSComponent>();
    engine.ecsRegistry = &ecs;
    
    Entity e1 = ecs.create();
    ecs.addComponent(e1, TestECSComponent{100, Fixed(5.0)});
    
    // Save state at frame 0
    engine.saveState(0);
    
    // Modify component
    ecs.getComponent<TestECSComponent>(e1).value = 500;
    ecs.getComponent<TestECSComponent>(e1).timer = Fixed(1.0);
    
    // Load state back to frame 0
    engine.loadState(0);
    
    // Check if values were restored
    TEST_ASSERT(ecs.getComponent<TestECSComponent>(e1).value == 100, "ECS Rollback integer");
    TEST_ASSERT(ecs.getComponent<TestECSComponent>(e1).timer == Fixed(5.0), "ECS Rollback Fixed point");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   FrameZero Engine - Complete Test Suite" << std::endl;
    std::cout << "   With Replay & Delta Compression" << std::endl;
    std::cout << "========================================" << std::endl;
    
    test_fixed_point();
    test_vector2();
    test_physics_body();
    test_collision_detection();
    test_input_queue();
    test_state_serialization_performance();
    test_determinism();
    test_replay_system();
    test_delta_compression();
    test_rollback_netcode();
    test_player_controller();
    test_interpolation_renderer();
    test_combat_hitstop();
    test_deterministic_trig();
    test_ecs_serialization();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "  Passed: " << testsPassed << std::endl;
    std::cout << "  Failed: " << testsFailed << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (testsFailed == 0) {
        std::cout << "\nALL TESTS PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "\nSOME TESTS FAILED!" << std::endl;
        return 1;
    }
}
