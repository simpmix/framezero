#pragma once
#include "fixed_point.h"
#include "physics_body.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace FrameZero {

// Opcodes for the Deterministic Character State Virtual Machine
enum class Opcode : uint8_t {
    PLAY_ANIMATION,     // Arg1: AnimID
    SET_VELOCITY_X,     // Arg1: Speed (Fixed)
    SET_VELOCITY_Y,     // Arg1: Speed (Fixed)
    APPLY_FORCE_X,      // Arg1: Force (Fixed)
    SPAWN_HITBOX,       // Arg1: Width, Arg2: Height, Arg3: Damage, Arg4: ActiveFrames
    WAIT_FRAMES,        // Arg1: Frames to wait
    GOTO_STATE,         // Arg1: StateID
    END                 // End of script
};

struct Instruction {
    Opcode op;
    int arg1 = 0;
    int arg2 = 0;
    int arg3 = 0;
    int arg4 = 0;
};

// Represents a single state (e.g., "Standing", "Walking", "LightPunch")
struct CharacterState {
    int id;
    std::vector<Instruction> bytecode;
};

// A Deterministic Virtual Machine that executes character logic scripts
// Safely rolls back since its internal PC and WaitTimers are serialized
class StateMachineVM {
public:
    int currentStateId;
    int programCounter;
    int waitTimer;
    
    // Engine references
    PhysicsBody* body;
    
    StateMachineVM() : currentStateId(0), programCounter(0), waitTimer(0), body(nullptr) {}
    
    void bind(PhysicsBody* b) {
        body = b;
    }
    
    // Changes state and resets the instruction pointer
    void changeState(int stateId) {
        currentStateId = stateId;
        programCounter = 0;
        waitTimer = 0;
    }
    
    // Steps the virtual machine forward by one frame
    // 'states' is a pointer to the character's definition data (read-only, does not need rollback)
    void execute(const std::unordered_map<int, CharacterState>& stateMachineDef) {
        if (!body) return;
        
        if (waitTimer > 0) {
            waitTimer--;
            return; // Still waiting on a WAIT_FRAMES instruction
        }
        
        auto it = stateMachineDef.find(currentStateId);
        if (it == stateMachineDef.end()) return;
        
        const auto& bytecode = it->second.bytecode;
        
        // Execute instructions until we hit a WAIT or END
        while (programCounter < bytecode.size()) {
            const Instruction& inst = bytecode[programCounter++];
            
            switch (inst.op) {
                case Opcode::PLAY_ANIMATION:
                    // In a real engine, dispatch to the AnimationSystem
                    break;
                    
                case Opcode::SET_VELOCITY_X:
                    body->velocity.x = Fixed::fromRaw(inst.arg1); // Assuming arg1 is the raw fixed-point value
                    break;
                    
                case Opcode::SET_VELOCITY_Y:
                    body->velocity.y = Fixed::fromRaw(inst.arg1);
                    break;
                    
                case Opcode::APPLY_FORCE_X:
                    body->applyForce(Vector2(Fixed::fromRaw(inst.arg1), Fixed(0)));
                    break;
                    
                case Opcode::SPAWN_HITBOX:
                    // Dispatch to CombatSystem
                    break;
                    
                case Opcode::WAIT_FRAMES:
                    waitTimer = inst.arg1;
                    return; // Yield execution for this frame
                    
                case Opcode::GOTO_STATE:
                    changeState(inst.arg1);
                    return; // Jump to new state and yield
                    
                case Opcode::END:
                    return;
            }
        }
    }
    
    // O(1) Contiguous Block Serialization
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(StateMachineVM));
    }
    
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(StateMachineVM));
    }
    
    static constexpr size_t getSize() {
        return sizeof(StateMachineVM);
    }
};

} // namespace FrameZero
