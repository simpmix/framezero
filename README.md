<div align="center">
  <img src="https://raw.githubusercontent.com/simpmix/framezero/main/README_LOGO.png?v=2" alt="FrameZero Logo" width="600" />
  <h1>FrameZero Engine</h1>
  <p><b>The Ultimate C++ 2D Framework for Deterministic Rollback Netcode</b></p>
  
  [![Build Status](https://github.com/simpmix/framezero/actions/workflows/ci.yml/badge.svg)](https://github.com/simpmix/framezero/actions)
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
</div>

---

**FrameZero** is a cross-platform (Windows, Linux, macOS) C++17 game engine built completely from scratch to solve the hardest problem in multiplayer game development: **Deterministic Rollback Netcode.**

Instead of retrofitting rollback into an existing engine (like Unity or Unreal) which often leads to desyncs and memory leaks, FrameZero is designed from the metal up to be 100% deterministic and memory-safe. Every math operation, every physics step, and every particle effect guarantees identical results across all CPU architectures.

## 🌟 Next-Gen Features

### ⏱️ Rollback-Native Architecture
* **State Snapshot ECS**: A custom Entity Component System (ECS) designed specifically to serialize and deserialize the entire game state (thousands of entities) in less than `0.02ms` per frame.
* **Zero-Allocation Data Structures**: Custom object pools, pathfinding queues, and broadphase grids guarantee absolutely zero `new`/`malloc` heap allocations during gameplay to prevent garbage collection stutters and state fragmentation.
* **Delta Compression**: Network inputs are aggressively delta-compressed, minimizing UDP bandwidth overhead by up to 70%.

### ⚡ True Deterministic Rollback Physics
* **100% Floating-Point Free Math Engine**: 16.16 Fixed-Point vectors and AABBs. We completely ripped out `<cmath>`. Sine, Cosine, and Atan2 are powered by a meticulously tuned **integer CORDIC algorithm**, and Square Root uses a binary search integer method, guaranteeing 100% bit-exact cross-platform determinism (Windows, Linux, macOS) down to the silicon.
* **Grid Hash Broadphase**: O(1) Spatial Grid collision detection for resolving hundreds of entities efficiently without O(N²) slowdowns.
* **Collision Layer Filtering**: Bitmask-based collision categories and masks. Perfect for ensuring projectiles only hit enemies and not the player who fired them.
* **Effect & Particle Manager**: A deterministic memory-pooled manager for spawning transient hit sparks and dust clouds that flawlessly rollback and auto-despawn.
* **OBB + SAT Collision**: Full support for rotated hitboxes (Oriented Bounding Boxes) using the Separating Axis Theorem (SAT).
* **Deterministic A* Pathfinding**: Navigates the Spatial Grid deterministically, avoiding floating-point heuristic drift.

### 🎮 Fighting Game & Action Tools
* **Motion Input Parser**: Track 60 frames of input history and instantly detect advanced joystick motions like Quarter-Circle Forward (QCF) or Dragon Punch (DP).
* **Input Config Mapper**: Dynamically map Keyboard and XInput Gamepad bindings directly into the deterministic rollback struct. Supports overriding D-Pad with analog sticks seamlessly.
* **Combat Math Engine**: Robust fighting game mathematics utility handling Combo Scaling (Proration), Crouching Hitstun bonuses, and dynamic Corner Pushback.
* **Animator State Machine**: A Unity-style node-based animator component that manages complex transitions between states (Idle, Walk, Attack) based on conditional lambdas.
* **Dynamic Camera Controller**: A fighting-game specific camera that automatically tracks the midpoint between players, dynamically zooms based on distance, and enforces hard stage boundaries deterministically.
* **Behavior Tree AI**: Native C++ behavior trees (`BTSequence`, `BTSelector`) for deterministic enemy AI without the overhead of Lua scripting.
* **Rollback-Safe Audio & Particles**: Prevents explosive "ear-rape" audio duplication and visual ghosting during network resimulations.

### 🚀 Cross-Platform Mastery
* **Automated CI/CD**: Fully tested via GitHub Actions on Ubuntu, macOS, and Windows.
* **Cross-OS Thread Pool**: Offload heavy AI or Audio tasks to a C++11 `<thread>` pool without stalling the 60FPS main loop.
* **Smart Asset Manager**: Automatically normalizes file paths (`\` vs `/`) and lowercase rules to prevent "works on my machine" crashes between Windows and Linux.
* **Stack-Based Scene Manager**: Push and Pop game states seamlessly (Menu -> Game -> Pause).
* **Decoupled Event Bus**: A robust Message Bus for broadcasting game events (e.g. `PlayerHitEvent`) to the Audio and UI systems without spaghetti dependencies.
* **Config/INI Parser**: Native cross-platform loading and saving of user configuration files (resolution, keybinds, volume).

---

## 🛠️ Getting Started

### Prerequisites
* **Windows**: Visual Studio 2022 (Desktop Development with C++)
* **Linux (Ubuntu/Debian)**: 
  ```bash
  sudo apt-get install cmake gcc g++ libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libxkbcommon-dev
  ```
* **macOS**: `brew install cmake`

### Building

**Visual Studio (Windows)**
1. Clone the repository and open the folder in Visual Studio.
2. Wait for the automatic CMake configuration to complete.
3. Build all and run `FrameZeroTests.exe` to verify the core engine math.
4. *Note: FrameZero is a headless engine library. Include `framezero` in your own project's CMake to build a game.*

**Terminal (Linux / macOS)**
```bash
git clone https://github.com/simpmix/framezero.git
cd framezero
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./FrameZeroTests
```

---

## 💻 Code Examples

### 1. Fighting Game Motions & Rotated Hitboxes
```cpp
#include <FrameZero.h>
using namespace FrameZero;

MotionParser p1InputParser;
OBB swordHitbox;

void gameLogicCallback(RollbackEngine* engine, Input local, Input remote) {
    p1InputParser.update(local);
    
    // Detect a Hadouken (Quarter-Circle Forward)
    if (p1InputParser.detectQCF(15) && (local.buttons & BTN_PUNCH)) {
        engine->spawnProjectile(ProjectileType::FIREBALL);
    }

    // Rotated Hitbox Collision (SAT)
    swordHitbox.center = Vector2(Fixed(10), Fixed(10));
    swordHitbox.extents = Vector2(Fixed(5), Fixed(20));
    swordHitbox.angle = Fixed::pi() / Fixed(4); // 45 degrees

    if (checkOBBOverlap(swordHitbox, enemyHurtbox)) {
        // Apply Damage and 5-Frame Hitstop!
    }
}
```

### 2. Scene Management
```cpp
SceneManager scenes;

// Transition to Gameplay
scenes.changeScene(std::make_unique<GameplayScene>());

// Player pauses the game
scenes.pushScene(std::make_unique<PauseMenuScene>());

// Update active scene
scenes.update(dt);
scenes.draw();
```

### 3. Thread Pool Task Offloading
```cpp
ThreadPool pool(4); // 4 Background Threads

// Offload heavy A* Pathfinding so the 60fps network loop never drops a frame!
pool.enqueue([&]() {
    std::vector<Vector2> path;
    pathfinder.findPath(spatialGrid, startPos, targetPos, path);
    applyPathToEntity(path);
});
```

---

## 🛠️ Built-in Debug Tools
FrameZero ships with developer tools to maintain your sanity:
* **Replay Viewer** (`FrameZeroReplayViewer`): Load `.frz` replay files and scrub through them frame-by-frame.
* **Desync Finder** (`FrameZeroDesyncFinder`): Pass in Player 1 and Player 2's replay files. The tool instantly scrubs the FNV-1a checksums and prints the exact frame, input state, and variables that caused the butterfly effect desync!

## 📜 License
FrameZero is released under the **MIT License**. Use it for game jams, commercial projects, or modifications without restrictions.
