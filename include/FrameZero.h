#ifndef FRAMEZERO_H
#define FRAMEZERO_H

// Win32 Macro Sandboxing
#if defined(_WIN32)
#define NOMINMAX
#define NOGDI
#define NOUSER
#endif

// Core Math & Physics (2D & 3D)
#include "fixed_point.h"
#include "vector2.h"
#include "vector3.h"
#include "quaternion.h"
#include "physics_body.h"
#include "physics_body_3d.h"
#include "collision.h"
#include "polygon_collider.h"
#include "quadtree.h"
#include "raycast.h"

// Entity Component System & Serialization
#include "ecs.h"
#include "state_serialization.h"

// Deterministic Random & Gameplay Systems
#include "random.h"
#include "rollback_rng.h"
#include "state_machine.h"
#include "behavior_tree.h"
#include "pathfinding.h"
#include "flow_field.h"
#include "animation.h"
#include "animator.h"
#include "particles.h"
#include "vfx_system.h"

// Camera & Visual Systems
#include "camera.h"
#include "camera_controller.h"
#include "camera_system.h"
#include "predictive_smoothing.h"
#include "interpolation_renderer.h"

// Engine Architecture & Utilities
#include "thread_pool.h"
#include "scene_manager.h"
#include "event_system.h"
#include "event_bus.h"
#include "config_parser.h"

// Rollback Netcode & Networking
#include "input.h"
#include "input_parser.h"
#include "input_buffer.h"
#include "rollback_netcode.h"
#include "replay_system.h"
#include "delta_compression.h"
#include "network_socket.h"
#include "network_simulator.h"
#include "sync_manager.h"

// Combat & Controllers
#include "frame_data.h"
#include "combat_math.h"
#include "combat_system.h"
#include "player_controller.h"
#include "platformer_controller.h"
#include "topdown_controller.h"
#include "effect_manager.h"

// Optional Raylib Hardware Integration (Renderer, Audio, Gamepad/Keyboard Mapper)
#if defined(FRAMEZERO_ENABLE_RAYLIB)
#include "audio_system.h"
#include "asset_manager.h"
#include "input_mapper.h"
#include "renderer.h"
#endif

#endif // FRAMEZERO_H
