#pragma once
#include "ecs.h"
#include <vector>
#include <memory>
#include <functional>

namespace FrameZero {

// Return status for Behavior Tree nodes
enum class BTStatus {
    RUNNING,
    SUCCESS,
    FAILURE
};

// Base Node
class BTNode {
public:
    virtual ~BTNode() = default;
    virtual BTStatus tick(Registry& ecs, Entity entity) = 0;
};

// Sequence: runs children in order until one fails
class BTSequence : public BTNode {
private:
    std::vector<std::unique_ptr<BTNode>> children;
    int currentChild = 0;
public:
    void addChild(std::unique_ptr<BTNode> child) {
        children.push_back(std::move(child));
    }
    
    BTStatus tick(Registry& ecs, Entity entity) override {
        while (currentChild < static_cast<int>(children.size())) {
            BTStatus status = children[currentChild]->tick(ecs, entity);
            if (status != BTStatus::SUCCESS) {
                if (status == BTStatus::FAILURE) currentChild = 0; // Reset on failure
                return status;
            }
            currentChild++;
        }
        currentChild = 0;
        return BTStatus::SUCCESS;
    }
};

// Selector: runs children in order until one succeeds
class BTSelector : public BTNode {
private:
    std::vector<std::unique_ptr<BTNode>> children;
    int currentChild = 0;
public:
    void addChild(std::unique_ptr<BTNode> child) {
        children.push_back(std::move(child));
    }
    
    BTStatus tick(Registry& ecs, Entity entity) override {
        while (currentChild < static_cast<int>(children.size())) {
            BTStatus status = children[currentChild]->tick(ecs, entity);
            if (status != BTStatus::FAILURE) {
                if (status == BTStatus::SUCCESS) currentChild = 0; // Reset on success
                return status;
            }
            currentChild++;
        }
        currentChild = 0;
        return BTStatus::FAILURE;
    }
};

// Action Node: Executes a custom C++ lambda
class BTAction : public BTNode {
private:
    std::function<BTStatus(Registry&, Entity)> action;
public:
    BTAction(std::function<BTStatus(Registry&, Entity)> func) : action(func) {}
    
    BTStatus tick(Registry& ecs, Entity entity) override {
        return action(ecs, entity);
    }
};

} // namespace FrameZero
