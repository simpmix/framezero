#pragma once
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace FrameZero {

// Base interface for all game screens/states
class Scene {
public:
    virtual ~Scene() = default;
    
    virtual void onEnter() {}       // Called when scene is pushed
    virtual void onExit() {}        // Called when scene is popped
    virtual void onPause() {}       // Called when another scene is pushed on top
    virtual void onResume() {}      // Called when the scene on top is popped
    
    // Core game loop
    virtual void update(double dt) = 0;
    virtual void fixedUpdate(double dt) = 0;
    virtual void draw() = 0;
};

// A Stack-based State Machine for managing game screens (Menu, Gameplay, Pause)
// Cross-platform and deterministic-safe (doesn't interfere with fixed logic)
class SceneManager {
private:
    std::vector<std::unique_ptr<Scene>> scenes;
    
public:
    // Adds a new scene to the top of the stack (e.g., Opening a Pause Menu)
    void pushScene(std::unique_ptr<Scene> scene) {
        if (!scenes.empty()) {
            scenes.back()->onPause();
        }
        scenes.push_back(std::move(scene));
        scenes.back()->onEnter();
    }
    
    // Removes the current top scene (e.g., Closing the Pause Menu)
    void popScene() {
        if (scenes.empty()) return;
        
        scenes.back()->onExit();
        scenes.pop_back();
        
        if (!scenes.empty()) {
            scenes.back()->onResume();
        }
    }
    
    // Replaces the current scene entirely (e.g., Moving from Main Menu to Gameplay)
    void changeScene(std::unique_ptr<Scene> scene) {
        if (!scenes.empty()) {
            scenes.back()->onExit();
            scenes.pop_back();
        }
        scenes.push_back(std::move(scene));
        scenes.back()->onEnter();
    }
    
    void update(double dt) {
        if (!scenes.empty()) {
            scenes.back()->update(dt);
        }
    }
    
    void fixedUpdate(double dt) {
        if (!scenes.empty()) {
            scenes.back()->fixedUpdate(dt);
        }
    }
    
    void draw() {
        if (!scenes.empty()) {
            scenes.back()->draw();
        }
    }
    
    bool isEmpty() const {
        return scenes.empty();
    }
};

} // namespace FrameZero
