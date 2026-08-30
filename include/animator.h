#pragma once
#include "animation.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace FrameZero {

// A Node-Based Animator State Machine.
// Transitions between Animations deterministically based on conditions.
// Equivalent to Unity's Animator controller but strictly tied to Rollback logic frames.

struct AnimTransition {
    std::string targetState;
    std::function<bool()> condition; // Returns true if we should transition
};

class AnimatorState {
public:
    Animation animation;
    std::vector<AnimTransition> transitions;
    bool loop;

    AnimatorState(const Animation& anim, bool shouldLoop = true) 
        : animation(anim), loop(shouldLoop) {}

    void addTransition(const std::string& target, std::function<bool()> condition) {
        transitions.push_back({target, std::move(condition)});
    }
};

class Animator {
private:
    std::unordered_map<std::string, std::unique_ptr<AnimatorState>> states;
    std::string currentStateName;
    AnimatorState* currentState;

public:
    Animator() : currentState(nullptr) {}

    void addState(const std::string& name, const Animation& anim, bool loop = true) {
        states[name] = std::make_unique<AnimatorState>(anim, loop);
    }

    void addTransition(const std::string& from, const std::string& to, std::function<bool()> condition) {
        if (states.find(from) != states.end()) {
            states[from]->addTransition(to, std::move(condition));
        }
    }

    void play(const std::string& name) {
        if (states.find(name) != states.end()) {
            if (currentStateName != name) {
                currentStateName = name;
                currentState = states[name].get();
                currentState->animation.reset();
            }
        }
    }

    // Called every LOGIC FRAME (not render frame)
    void tick() {
        if (!currentState) return;

        // 1. Check Transitions
        for (const auto& trans : currentState->transitions) {
            if (trans.condition()) {
                play(trans.targetState);
                break;
            }
        }

        // 2. Advance Animation
        currentState->animation.tick();
        if (currentState->animation.isFinished() && currentState->loop) {
            currentState->animation.reset();
        }
    }

    // Retrieves the current frame index to be used by the Renderer
    int getCurrentFrame() const {
        if (!currentState) return 0;
        return currentState->animation.getCurrentFrame();
    }

    std::string getCurrentState() const {
        return currentStateName;
    }
};

} // namespace FrameZero
