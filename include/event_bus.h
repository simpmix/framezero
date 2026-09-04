#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <typeindex>

namespace FrameZero {

// Base interface for all events
struct Event {
    virtual ~Event() = default;
};

// Example specific events
struct HitEvent : public Event {
    int attackerId = 0;
    int defenderId = 0;
    int damage = 0;
};

struct RollbackTriggeredEvent : public Event {
    int frameRewindedTo;
};

// A completely decoupled Event Bus for broadcasting simulation events to UI and Audio
class EventBus {
private:
    using EventHandler = std::function<void(const Event&)>;
    std::unordered_map<std::type_index, std::vector<EventHandler>> listeners;

public:
    // Subscribe to a specific event type
    template <typename TEvent>
    void subscribe(std::function<void(const TEvent&)> callback) {
        auto wrapper = [callback](const Event& e) {
            callback(static_cast<const TEvent&>(e));
        };
        listeners[typeid(TEvent)].push_back(wrapper);
    }

    // Publish an event to all subscribers instantly
    template <typename TEvent>
    void publish(const TEvent& event) {
        auto it = listeners.find(typeid(TEvent));
        if (it != listeners.end()) {
            for (const auto& handler : it->second) {
                handler(event);
            }
        }
    }
};

} // namespace FrameZero
