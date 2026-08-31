# FrameZero: Advanced Features & Architecture

Now that you've mastered the basics of FrameZero's rollback engine, this guide will walk you through the advanced AAA features we've implemented to help you build a commercial-ready game.

## 1. Network Synchronization & Frame Advantage (`SyncManager`)
In a real peer-to-peer match, you cannot just immediately start simulating Frame 1. If Player 1 is in Tokyo and Player 2 is in New York, the latency (ping) means their packets will arrive out of sync, leading to severe rollback stutter.

Before a match begins, you must use the `SyncManager` to establish **Frame Advantage**.
```cpp
#include <sync_manager.h>

SyncManager sync;
sync.startSync();

// In your menu/lobby loop:
while (!sync.isReady()) {
    sync.update(udpSocket, opponentIP, opponentPort);
}

// Once ready, calculate the required input delay to mask the latency!
int inputDelayFrames = sync.getRecommendedFrameDelay();
```
By delaying your local input execution by `inputDelayFrames` (e.g. 2 frames), you give your packets 33ms of "head start" across the internet, completely eliminating rollbacks for any jitter under 33ms!

## 2. Advanced Collision: Convex Polygons (`PolygonCollider`)
Standard 2D engines use basic rectangles (AABBs). FrameZero provides the `PolygonCollider` which uses the **Separating Axis Theorem (SAT)** for mathematically flawless convex polygon detection.

```cpp
#include <polygon_collider.h>

Polygon spearHitbox;
spearHitbox.addVertex(Vector2(0, 0));
spearHitbox.addVertex(Vector2(10, 0));
spearHitbox.addVertex(Vector2(5, 20)); // Triangular Hitbox!

// Rotate the hitbox 45 degrees
spearHitbox.rotate(Vector2(5, 5), Fixed::pi() / Fixed(4));

CollisionContact contact;
if (PolygonCollider::checkOverlap(spearHitbox, enemyHurtbox, &contact)) {
    // contact.normal gives you the exact pushback vector!
    // contact.penetration gives you the exact depth of the overlap!
}
```

## 3. Decoupled Architecture (`EventBus`)
Your UI (health bars) and Audio systems should **never** be part of the Rollback ECS. If they are, a rolled-back hit will cause the UI to glitch and the audio to stutter.

Instead, your simulation should broadcast events to the `EventBus`. The UI/Audio layers simply listen for those events.

```cpp
#include <event_bus.h>

EventBus eventBus;

// 1. Audio System (Non-deterministic) subscribes to hits
eventBus.subscribe<HitEvent>([](const HitEvent& e) {
    if (e.damage >= 50) PlaySound(heavyHitSound);
    else PlaySound(lightHitSound);
});

// 2. Core Simulation (Deterministic) broadcasts the hit
if (attackConnects) {
    HitEvent e;
    e.attackerId = 1; e.damage = 60;
    eventBus.publish(e);
}
```

## 4. Rollback Visual Effects (`VFXSystem`)
Particles are a nightmare for rollback. If you spawn sparks on Frame 10, but then roll back to Frame 5 and realize the punch was actually blocked, those sparks will permanently exist on the screen.

The `VFXSystem` solves this by strictly simulating particles using Fixed-Point math and serializing their lifetimes into the Rollback Snapshots!

```cpp
#include <vfx_system.h>

VFXSystem vfx;

// Inside your simulation callback:
vfx.spawnExplosion(impactPosition, Fixed(10.0), 15, COLOR_RED);

// If the engine rolls back, the StateSerializer will seamlessly restore 
// the `vfx.particles` array, instantly erasing false explosions!
```
