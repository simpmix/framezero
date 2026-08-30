#ifndef FRAMEZERO_H
#define FRAMEZERO_H

// Win32 Macro Sandboxing
#if defined(_WIN32)
#define NOMINMAX
#define NOGDI
#define NOUSER
#endif

// Core Math & Physics
#include "fixed_point.h"
#include "vector2.h"
#include "physics_body.h"
#include "collision.h"
#include "ecs.h"
#include "random.h"
#include "audio_system.h"
#include "behavior_tree.h"
#include "animation.h"
#include "particles.h"
#include "pathfinding.h"
#include "camera.h"
#include "thread_pool.h"
#include "asset_manager.h"
#include "scene_manager.h"

// Rollback Netcode & Logic
#include "input.h"
#include "input_parser.h"
#include "state_serialization.h"
#include "rollback_netcode.h"
#include "replay_system.h"
#include "delta_compression.h"
#include "network_socket.h"
#include "input_buffer.h"

// Game & Combat
#include "frame_data.h"
#include "combat_system.h"
#include "player_controller.h"

// Rendering
#include "interpolation_renderer.h"
#include "renderer.h"

#endif // FRAMEZERO_H
