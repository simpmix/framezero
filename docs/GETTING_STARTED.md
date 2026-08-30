# Getting Started with FrameZero

Welcome to **FrameZero**! This guide will walk you through setting up your first deterministic 2D physics simulation with rollback netcode.

## 1. Project Setup (CMake)
FrameZero is heavily optimized for C++17 and CMake. In your own project's `CMakeLists.txt`, simply add:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyFightingGame)

# Find the installed FrameZero library
find_package(FrameZero REQUIRED)

add_executable(MyGame src/main.cpp)
target_link_libraries(MyGame PRIVATE framezero)
```

## 2. Setting up the Simulation
All of FrameZero's classes are housed in the `FrameZero` namespace. 

```cpp
#include <FrameZero.h>

using namespace FrameZero;

int main() {
    // 1. Initialize the rollback engine
    RollbackEngine engine;
    
    // 2. Setup your physics bodies
    PhysicsBody bodies[2];
    
    // Player 1
    bodies[0].id = 1;
    bodies[0].position = Vector2(Fixed(100.0), Fixed(0));
    bodies[0].size = Vector2(Fixed(20.0), Fixed(40.0));
    bodies[0].setMass(Fixed(1.0));
    
    // Player 2
    bodies[1].id = 2;
    bodies[1].position = Vector2(Fixed(500.0), Fixed(0));
    bodies[1].size = Vector2(Fixed(20.0), Fixed(40.0));
    bodies[1].setMass(Fixed(1.0));
    
    // Load bodies into the engine
    engine.setBodies(bodies, 2);
    
    // 3. Step the simulation
    Input p1Input, p2Input;
    p1Input.moveX = 127; // Move P1 right
    
    // simulateFrame automatically handles snapshotting for rollbacks!
    engine.simulateFrame(p1Input, p2Input);
    
    return 0;
}
```

## 3. Handling Rollback Networking
When playing online, you will receive opponent inputs via UDP (see `UDPSocket`). If those inputs arrive late, FrameZero automatically rewinds time.

```cpp
// When you receive a packet from the network:
Input remoteInput = parseNetworkPacket(packet);
int frameOfInput = packet.frame;

// Feed it to the engine. If 'frameOfInput' is in the past, 
// the engine will automatically rollback and resimulate back to the current frame!
engine.receiveRemoteInput(remoteInput, frameOfInput);
```

## 4. Advanced: Frame Data and Hitboxes
You can easily define frame data for attacks using the `AttackData` struct:

```cpp
AttackData heavyPunch;
heavyPunch.startupFrames = 5;
heavyPunch.activeFrames = 3;
heavyPunch.recoveryFrames = 12;
heavyPunch.hitbox = Hitbox(Vector2(Fixed(2.0), Fixed(0)), Vector2(Fixed(1.0), Fixed(1.0)), Fixed(15.0), 10);
```

Check out the `PlayerController` source code to see how to bind this data to actual character states!
