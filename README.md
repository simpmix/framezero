# FrameZero Engine

**FrameZero** is a cross-platform, deterministic 2D C++ game framework built from the ground up for rollback netcode. Featuring a custom fixed-point physics engine, a rollback-aware ECS, spatial hashing, and native Behavior Trees for AI. FrameZero provides the ultimate, scalable foundation for fighting games, shooters, and fast-paced multiplayer.

## Features
- **Rollback-Aware ECS**: Data-oriented component system that takes sub-millisecond snapshots.
- **Deterministic Physics**: Fixed-point math (`Fixed`, `Vector2`), Raycasting, and Spatial Grid Hash broadphase.
- **Advanced Collision**: OBB (Oriented Bounding Boxes) with SAT (Separating Axis Theorem).
- **Rollback Audio**: Suppresses duplicate sound effects during fast-forward resimulations.
- **Rollback Graphics**: Deterministic Sprite Animator and Particle System.
- **Logic & Scripting**: Native C++ Behavior Tree Engine and Motion Input Parser (QCF/DP).
- **Cross-Platform Networking**: Built-in UDP abstractions for Windows and POSIX (Linux/macOS).
- **Replay & Tools**: Replay viewer, Desync finder, and Delta Compression.

## Building

### Visual Studio (Windows)
1. Open Visual Studio → "Open a local folder" → Select `framezero/`
2. Wait for CMake to configure automatically
3. Build → Build All (or Ctrl+Shift+B)
4. Debug → Start Without Debugging (or Ctrl+F5)

### Linux/macOS (Terminal)
```bash
cd framezero
mkdir -p build && cd build
cmake ..
make
./FrameZeroTests
```

## Test Results
Tests cover 61 highly rigorous deterministic validations including:
- Fixed-point arithmetic & trigonometry (sin/cos/atan2)
- OBB SAT Collision and Spatial Grid Hashing
- ECS Snapshot Serialization (< 0.02ms)
- Cross-simulation determinism and Replay integrity
- Full Rollback Netcode execution

## Usage Example (Motion Parser & OBB)

```cpp
#include "FrameZero.h"
using namespace FrameZero;

MotionParser p1InputParser;
OBB swordHitbox;

// Game loop
void gameLogicCallback(RollbackEngine* engine, Input local, Input remote) {
    p1InputParser.update(local);
    
    // Detect a Hadouken (Quarter-Circle Forward)
    if (p1InputParser.detectQCF(15) && local.buttons & BTN_PUNCH) {
        // ... Fire Projectile ...
    }

    // Rotated Hitbox Collision
    swordHitbox.center = Vector2(Fixed(10), Fixed(10));
    swordHitbox.extents = Vector2(Fixed(5), Fixed(20));
    swordHitbox.angle = Fixed(0.785); // 45 degrees

    if (checkOBBOverlap(swordHitbox, enemyHurtbox)) {
        // ... Apply Damage ...
    }
}
```

## License
MIT License - Use it for anything!
