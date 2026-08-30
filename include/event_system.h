#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>

namespace FrameZero {

// A decoupled Event System (Message Bus) for cross-system communication.
// E.g., The Combat System can broadcast a "PlayerHitEvent". 
// The Audio System and UI System can listen to it without knowing about each other.

class Event {
public:
    virtual ~Event() = default;
};

// Base class for event listeners
class IEventListener {
public:
    virtual ~IEventListener() = default;
};

template <typename TEvent>
class EventListener : public IEventListener {
public:
    using Callback = std::function<void(const TEvent&)>;
    Callback callback;

    EventListener(Callback cb) : callback(std::move(cb)) {}
};

class EventBus {
private:
    std::unordered_map<std::type_index, std::vector<IEventListener*>> listeners;
    
public:
    ~EventBus() {
        for (auto& pair : listeners) {
            for (auto* listener : pair.second) {
                delete listener;
            }
        }
    }

    // Subscribe to a specific event type
    template <typename TEvent>
    void subscribe(std::function<void(const TEvent&)> callback) {
        auto* listener = new EventListener<TEvent>(std::move(callback));
        listeners[std::type_index(typeid(TEvent))].push_back(listener);
    }

    // Broadcast an event to all subscribers instantly
    template <typename TEvent>
    void publish(const TEvent& event) {
        auto it = listeners.find(std::type_index(typeid(TEvent)));
        if (it != listeners.end()) {
            for (auto* baseListener : it->second) {
                auto* listener = static_cast<EventListener<TEvent>*>(baseListener);
                listener->callback(event);
            }
        }
    }
};

} // namespace FrameZero
