# FrameZero Engine

**FrameZero** is a deterministic 2D physics and rollback netcode engine designed specifically for fast-paced, highly competitive action games and fighting games.

## Features
- **Replay System**: Record and save gameplay sessions (~12 bytes/frame)
- **Delta Compression**: Reduce network traffic by 26-89% by sending only changes
- **Input Queue Management**: Buffered input handling with prediction support

## Project Structure

```
framezero/
├── include/
│   ├── fixed_point.h         # 16.16 fixed-point arithmetic
│   ├── vector2.h             # 2D vector operations
│   ├── physics_body.h        # Physics bodies with serialization
│   ├── collision.h           # AABB detection & impulse resolution
│   ├── input.h               # Compact 3-byte input system
│   ├── state_serialization.h # Sub-millisecond state snapshots
│   ├── replay_system.h       # Recording and playback
│   ├── delta_compression.h   # Network optimization
│   └── rollback_netcode.h    # GGPO-style rollback engine
├── src/
│   └── fixed_point.cpp       # Trig lookup tables
├── tests/
│   └── test_engine.cpp       # Comprehensive test suite (43 tests)
├── build/
│   └── FrameZeroTests        # Compiled test binary
├── CMakeLists.txt            # Build configuration
└── README.md                 # This file
```

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

```
========================================
Test Results:
  Passed: 43
  Failed: 0
========================================
ALL TESTS PASSED!
```

Tests cover:
- Fixed-point arithmetic correctness
- Vector2 operations
- Physics body simulation & serialization
- Collision detection (AABB)
- Input queue management
- State serialization performance (<1ms target: ✅ 0.002ms)
- Cross-simulation determinism
- Replay system save/load/verify
- Delta compression efficiency
- Rollback netcode execution

## Key Performance Metrics

| Metric | Target | Actual |
|--------|--------|--------|
| State Serialization (100 bodies) | <1.0ms | 0.002ms |
| Replay File Size (100 frames) | Minimal | ~1.2KB |
| Delta Compression Savings | >0% | 26-89% |
| Determinism Verification | Perfect | ✅ Pass |

## How Rollback Works

1. **Predict**: Guess remote player's input while waiting for network packet
2. **Simulate**: Run game forward with predicted inputs
3. **Receive**: Get actual remote input from network
4. **Compare**: Check if prediction matched reality
5. **Rollback** (if wrong):
   - Rewind to frame where input differed
   - Apply correct input
   - Resimulate all frames to present (<1ms)
   - Snap visuals to corrected state

## Usage Example

```cpp
#include "rollback_netcode.h"
using namespace FrameZero;

RollbackEngine engine;

// Setup physics bodies
PhysicsBody bodies[2];
bodies[0].position = Vector2(0.0, 5.0);
engine.setBodies(bodies, 2);

// Game loop
while (running) {
    Input localInput = GetLocalInput();
    Input predictedRemote = engine.predictRemoteInput();
    
    // Simulate frame
    engine.simulateFrame(localInput, predictedRemote);
    
    // When network packet arrives
    Input actualRemote = ReceiveNetworkInput();
    int frameNumber = GetPacketFrameNumber();
    engine.receiveRemoteInput(actualRemote, frameNumber);
    
    // Render interpolated state
    Render(engine.getCurrentState());
}
```

## Why This Matters

Most multiplayer games use delay-based netcode (feels laggy). FrameZero uses rollback netcode like modern fighting games (Street Fighter 6, Guilty Gear Strive), providing:
- Zero perceived lag on good connections
- Playable experience even with 100-200ms latency
- Smooth gameplay despite packet loss
- Competitive-grade responsiveness

## Next Steps

To build a game on FrameZero:
1. Add rendering (Raylib, SDL, or custom OpenGL)
2. Create game-specific logic (player controller, attacks, etc.)
3. Integrate networking (UDP sockets, Steam Networking)
4. Add interpolation renderer for visual smoothness
5. Tune prediction heuristics for your game mechanics

## License

Public domain / MIT - Use it for anything.
