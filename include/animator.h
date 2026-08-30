#pragma once
#include "animation.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <utility>

namespace FrameZero {

struct AnimTransition {
    std::string targetState;
    std::function<bool()> condition; // Returns true if we should transition
};

class StateMachineNode {
public:
    int sequenceId;
    std::vector<AnimTransition> transitions;
    bool loop;

    StateMachineNode(int seqId, bool shouldLoop = true) 
        : sequenceId(seqId), loop(shouldLoop) {}

    void addTransition(const std::string& target, std::function<bool()> condition) {
        transitions.push_back(AnimTransition{target, std::move(condition)});
    }
};

class AnimatorStateMachine {
private:
    std::unordered_map<std::string, std::unique_ptr<StateMachineNode>> states;
    std::string currentStateName;
    StateMachineNode* currentNode;
    FrameZero::AnimatorState coreState; // From animation.h

public:
    AnimatorStateMachine() : currentNode(nullptr) {}

    void addState(const std::string& name, int sequenceId, bool loop = true) {
        states[name] = std::make_unique<StateMachineNode>(sequenceId, loop);
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
                currentNode = states[name].get();
                coreState.play(currentNode->sequenceId);
            }
        }
    }

    // Called every LOGIC FRAME
    void tick(const AnimationSequence* allSequences, int sequenceCount) {
        if (!currentNode) return;

        // 1. Check Transitions
        for (const auto& trans : currentNode->transitions) {
            if (trans.condition()) {
                play(trans.targetState);
                break;
            }
        }

        // 2. Advance Animation
        coreState.tick(allSequences, sequenceCount);
        if (coreState.finished && currentNode->loop) {
            coreState.play(currentNode->sequenceId); // loop back manually if animator overrides
        }
    }

    int getCurrentFrame() const {
        return coreState.currentFrameIndex;
    }

    std::string getCurrentState() const {
        return currentStateName;
    }
    
    // For rollback serialization
    const FrameZero::AnimatorState& getCoreState() const { return coreState; }
    void setCoreState(const FrameZero::AnimatorState& state) { coreState = state; }
};

} // namespace FrameZero
