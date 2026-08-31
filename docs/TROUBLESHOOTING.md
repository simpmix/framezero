# FrameZero Troubleshooting Guide

This guide covers the most common issues developers face when building games with FrameZero, specifically focusing on Rollback Netcode Desynchronizations (Desyncs) and C++ determinism.

## 1. What is a Desync?
A desync occurs when Player 1's computer and Player 2's computer calculate different physics states for the same frame. Because rollback relies on deterministic lockstep, even a difference of `0.0001` in a character's X position will eventually snowball into entirely different game states, causing the players to essentially play two different matches.

## 2. Common Causes of Desyncs

### Floating-Point Math (`float` or `double`)
**NEVER** use standard floating-point math in your game logic callback or ECS components. Different CPUs (Intel vs. AMD) or compilation flags (e.g., `-O3` vs `-O0`) will round floats differently.
* **Bad**: `float speed = 1.5f; body.position.x += speed;`
* **Good**: `Fixed speed = Fixed(1.5); body.position.x += speed;`
Always use the `FrameZero::Fixed` struct for all game-affecting logic.

### UI and Audio Logic in the Physics Loop
If you play a sound effect or trigger a UI health bar animation directly inside the physics loop, a rollback will cause that code to execute multiple times, leading to explosive "ear-rape" audio or stuttering UI.
* **Fix**: Use the `EventBus` (`event_bus.h`). Fire a `HitEvent` from the physics loop, and let the non-deterministic UI layer listen to the bus safely!

### Using Global or Static State
The Rollback Engine rewinds the `PhysicsBody` array and the `Registry` (ECS). If you store game state in a global variable (e.g., `static int comboCount = 0;`), the engine **cannot** rewind it.
* **Fix**: Move `comboCount` into an ECS Component.

### Standard C++ RNG (Random Number Generators)
Standard `rand()` or `std::mt19937` will desync instantly if you call them during a rollback resimulation (because they will advance their internal state incorrectly).
* **Fix**: You must use the built-in `FrameZero::RollbackRNG` (`rollback_rng.h`). Its internal Permuted Congruential Generator (PCG) seed is directly tied into the rollback snapshot buffer, guaranteeing that random critical hits or procedural generation rewinds safely.

## 3. Dealing with Initial Match Stutter (High Ping)
If a match stutters horribly as soon as it begins, it means Player 1 and Player 2 started executing Frame 0 at different real-world times. 
* **Fix**: You must use `SyncManager` (`sync_manager.h`) before the match starts to exchange UDP ping/pongs. It will automatically calculate the required **Input Delay Frame Advantage** so both clients execute Frame 0 in perfect lockstep.

## 4. How to Debug a Desync
If your game desyncs, follow these steps:

1. **Enable Replay Recording**: Ensure both clients are saving their inputs and physics snapshots to a `.frz` replay file.
2. **Use FrameZeroDesyncFinder**: Run the included desync finder tool in the `tools/` directory.
   ```bash
   ./FrameZeroDesyncFinder p1_replay.frz p2_replay.frz
   ```
3. The tool will compare the FNV-1a checksums for every frame and print exactly which frame first diverged, and which variables caused it.

## 5. Testing Network Conditions (NetworkSimulator)
Do not wait until your game is finished to test it online. Network latency hides bugs. 

Use the built-in `NetworkSimulator` class to artificially inject latency, jitter, and packet loss into your local development builds. 

```cpp
#include <network_simulator.h>

// Inject 80ms latency, 20ms jitter, and 5% packet loss
FrameZero::NetworkSimulator netSim(80, 20, 0.05f);

netSim.bind(8080);
netSim.send("127.0.0.1", 8081, data, size); // Will artificially delay the send!
```
If your game runs smoothly locally under `NetworkSimulator`, it will run smoothly online!
