<div align="center">
  <img src="https://raw.githubusercontent.com/simpmix/framezero/main/README_LOGO.png?v=2" alt="FrameZero Logo" width="600" />
  <h1>FrameZero Engine v1.0.0</h1>
  <p><b>The Ultimate C++ 2D & 3D Framework for Deterministic Rollback Netcode</b></p>
  
  [![Build Status](https://github.com/simpmix/framezero/actions/workflows/release.yml/badge.svg)](https://github.com/simpmix/framezero/actions)
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
</div>

---

**FrameZero** is a cross-platform (Windows, Linux, macOS) C++17 game engine built completely from scratch to solve the hardest problem in multiplayer game development: **Deterministic Rollback Netcode.**

FrameZero is designed from the metal up to be 100% deterministic and memory-safe. Every math operation, every physics step, and every particle effect guarantees identical results across all CPU architectures.

## 🚀 Next-Gen Features (v1.0.0)

### ⚡ Hyper-Optimized Rollback Architecture
* **O(1) Snapshot ECS**: A custom Entity Component System (ECS) designed with strict C++ memory alignment (`#pragma pack`) and contiguous EnTT-style tuple views. It serializes and deserializes the entire game state in a single, lightning-fast `memcpy` instruction.
* **Zero-Allocation Data Structures**: Custom object pools, pathfinding queues, and broadphase grids guarantee absolutely zero `new`/`malloc` heap allocations during gameplay to prevent garbage collection stutters.
* **Delta Compression**: Network inputs are aggressively delta-compressed, minimizing UDP bandwidth overhead by up to 70%.

### 🧊 2D & 3D Deterministic Physics
* **100% Floating-Point Free Math**: 16.16 Fixed-Point vectors (2D and 3D). We completely ripped out `<cmath>`. Sine, Cosine, and Atan2 are powered by a meticulously tuned **integer CORDIC algorithm**, and Square Root uses a bitwise integer method.
* **Rigid Body Angular Momentum**: Full support for deterministic 2D Torque and true **3D Quaternions**, completely preventing Gimbal Lock.
* **Arbitrary Polygon Physics (SAT)**: Full support for arbitrary convex hitboxes (Triangles, Hexagons) using the Separating Axis Theorem.
* **O(log N) Spatial QuadTree**: Hyper-fast broadphase collision detection for resolving thousands of entities efficiently without O(N²) slowdowns.

### 🧠 Advanced Mechanics & AI
* **Deterministic Character Virtual Machine**: A built-in bytecode interpreter (`StateMachineVM`) that allows developers to script complex character logic and attack data safely.
* **Flow Field Swarm Pathfinding**: O(1) Vector Field routing designed to seamlessly pathfind 1,000+ RTS units simultaneously without choking the CPU during a rollback frame.
* **Deterministic Rollback RNG (PCG)**: A Permuted Congruential Generator whose seed is deeply tied to the rollback state, allowing for true Procedurally Generated Multiplayer Roguelikes.

### 🎮 Multi-Genre Support
Out of the box, FrameZero ships with highly-tuned, deterministic physics controllers for multiple genres:
* **`PlatformerController`**: Modern game-feel mechanics like Coyote Time, Jump Buffering, and Variable Jump Heights.
* **`TopDownController`**: 8-way directional movement with strict diagonal speed normalization.
* **`Raycaster`**: Deterministic DDA line-of-sight checks for hitscan weapons and stealth mechanics.

### 🛠️ Developer & Debugging Suite
* **`SyncManager`**: Automates the pre-match UDP Ping/Pong handshake to calculate exact frame advantage and input delay.
* **`NetworkSimulator`**: Artificially inject latency, jitter, and packet loss directly into your local socket.
* **`FrameZeroDesyncFinder`**: Pass in two replay files and instantly pinpoint the exact frame and variable that caused a butterfly-effect desync via FNV-1a checksums.

---

## 📦 How to Use in Your Project

FrameZero is distributed as a headless C++ static library. You do not need to download packages or installers. Simply integrate it directly into your project's CMake configuration using `FetchContent`:

```cmake
include(FetchContent)

# Fetch the FrameZero Engine source code directly from GitHub
FetchContent_Declare(
    framezero
    URL https://github.com/simpmix/framezero/archive/refs/tags/v1.0.0.zip
)
FetchContent_MakeAvailable(framezero)

# Link it to your game executable
add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE framezero)
```

---

## 📖 Code Examples

### 1. Modern EnTT-Style ECS View API
```cpp
#include <ecs.h>
using namespace FrameZero;

Registry ecs;

// Instantly grab all entities that have BOTH a PhysicsBody and a PlayerController!
for (auto entity : ecs.view<PhysicsBody, PlayerController>()) {
    
    // Retrieve the actual component data
    auto& body = ecs.getComponent<PhysicsBody>(entity);
    auto& player = ecs.getComponent<PlayerController>(entity);
    
    // Process deterministic game logic...
}
```

### 2. Network Synchronization & Frame Advantage
```cpp
#include <sync_manager.h>
using namespace FrameZero;

SyncManager sync;
sync.startSync();

// In your menu/lobby loop:
while (!sync.isReady()) {
    sync.update(udpSocket, opponentIP, opponentPort);
}

// Once ready, calculate the required input delay to mask the latency!
int inputDelayFrames = sync.getRecommendedFrameDelay();
```

### 3. Decoupled Event Bus (Audio & UI)
```cpp
#include <event_bus.h>
using namespace FrameZero;

EventBus eventBus;

// 1. Audio System (Non-deterministic) subscribes to hits
eventBus.subscribe<HitEvent>([](const HitEvent& e) {
    if (e.damage >= 50) PlaySound(heavyHitSound);
});

// 2. Core Simulation (Deterministic) broadcasts the hit
if (attackConnects) {
    HitEvent e;
    e.damage = 60;
    eventBus.publish(e);
}
```

## 📚 Documentation
Check the `docs/` folder for comprehensive setup, architecture, and advanced feature guides!

## 📜 License
FrameZero is released under the **MIT License**. Use it for game jams, commercial projects, or modifications without restrictions.
