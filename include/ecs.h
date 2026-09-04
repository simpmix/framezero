#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <utility>

namespace FrameZero {

using Entity = uint32_t;
constexpr Entity MAX_ENTITIES = 512; // Tuned for deterministic cache locality and rollback speed
constexpr Entity NULL_ENTITY = 0xFFFFFFFF;

// Base class for arrays to allow polymorphism during serialization
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
    virtual size_t getMemorySize() const = 0;
    virtual void serialize(uint8_t* buffer, size_t& offset) const = 0;
    virtual void deserialize(const uint8_t* buffer, size_t& offset) = 0;
};

// Contiguous data-oriented array for a specific component type
template<typename T>
class ComponentArray : public IComponentArray {
private:
    T data[MAX_ENTITIES];
    bool active[MAX_ENTITIES];

public:
    ComponentArray() {
        std::memset(active, 0, sizeof(active));
    }

    void insert(Entity entity, const T& component) {
        if (entity < MAX_ENTITIES) {
            data[entity] = component;
            active[entity] = true;
        }
    }

    void remove(Entity entity) {
        if (entity < MAX_ENTITIES) {
            active[entity] = false;
        }
    }

    T& get(Entity entity) {
        return data[entity];
    }
    
    bool has(Entity entity) const {
        return entity < MAX_ENTITIES && active[entity];
    }

    void entityDestroyed(Entity entity) override {
        remove(entity);
    }
    
    size_t getMemorySize() const override {
        return sizeof(data) + sizeof(active);
    }
    
    // O(1) contiguous serialization per component type!
    void serialize(uint8_t* buffer, size_t& offset) const override {
        std::memcpy(buffer + offset, data, sizeof(data));
        offset += sizeof(data);
        std::memcpy(buffer + offset, active, sizeof(active));
        offset += sizeof(active);
    }
    
    void deserialize(const uint8_t* buffer, size_t& offset) override {
        std::memcpy(data, buffer + offset, sizeof(data));
        offset += sizeof(data);
        std::memcpy(active, buffer + offset, sizeof(active));
        offset += sizeof(active);
    }
};

class Registry {
private:
    uint32_t nextEntity = 0;
    bool activeEntities[MAX_ENTITIES];
    
    static constexpr int MAX_COMPONENTS = 64;
    IComponentArray* componentArrays[MAX_COMPONENTS];
    int componentCount = 0;

    // Component Type ID Generator
    static int getNextComponentTypeId() {
        static int nextId = 0;
        return nextId++;
    }

    template<typename T>
    static int getComponentTypeId() {
        static int id = getNextComponentTypeId();
        return id;
    }

public:
    Registry() {
        std::memset(activeEntities, 0, sizeof(activeEntities));
        for (int i = 0; i < MAX_COMPONENTS; i++) {
            componentArrays[i] = nullptr;
        }
    }
    
    ~Registry() {
        for (int i = 0; i < MAX_COMPONENTS; i++) {
            if (componentArrays[i]) {
                delete componentArrays[i];
            }
        }
    }

    Entity create() {
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            if (!activeEntities[i]) {
                activeEntities[i] = true;
                return i;
            }
        }
        return NULL_ENTITY;
    }

    void destroy(Entity entity) {
        if (entity < MAX_ENTITIES) {
            activeEntities[entity] = false;
            for (int i = 0; i < MAX_COMPONENTS; i++) {
                if (componentArrays[i]) {
                    componentArrays[i]->entityDestroyed(entity);
                }
            }
        }
    }

    template<typename T>
    void registerComponent() {
        int typeId = getComponentTypeId<T>();
        if (typeId < MAX_COMPONENTS && !componentArrays[typeId]) {
            componentArrays[typeId] = new ComponentArray<T>();
            if (typeId >= componentCount) {
                componentCount = typeId + 1;
            }
        }
    }

    template<typename T>
    void addComponent(Entity entity, const T& component) {
        getComponentArray<T>()->insert(entity, component);
    }
    
    // Modern Emplace API
    template<typename T, typename... Args>
    T& emplaceComponent(Entity entity, Args&&... args) {
        T comp(std::forward<Args>(args)...);
        getComponentArray<T>()->insert(entity, comp);
        return getComponentArray<T>()->get(entity);
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->remove(entity);
    }

    template<typename T>
    T& getComponent(Entity entity) {
        return getComponentArray<T>()->get(entity);
    }
    
    template<typename T>
    bool hasComponent(Entity entity) {
        int typeId = getComponentTypeId<T>();
        if (typeId >= MAX_COMPONENTS || !componentArrays[typeId]) {
            return false;
        }
        return getComponentArray<T>()->has(entity);
    }

    // Modern View API (EnTT style) for lightning-fast system iteration!
    template<typename... Components>
    std::vector<Entity> view() {
        std::vector<Entity> result;
        result.reserve(64); // Prevent frequent reallocation
        for (Entity e = 0; e < MAX_ENTITIES; ++e) {
            if (activeEntities[e]) {
                // Fold expression checks if entity has ALL requested components
                if ((hasComponent<Components>(e) && ...)) {
                    result.push_back(e);
                }
            }
        }
        return result;
    }

    // --- Rollback Hooks ---
    
    size_t getSerializationSize() const {
        size_t total = sizeof(activeEntities);
        for (int i = 0; i < componentCount; i++) {
            if (componentArrays[i]) {
                total += componentArrays[i]->getMemorySize();
            }
        }
        return total;
    }

    void serialize(uint8_t* buffer, size_t& offset) const {
        std::memcpy(buffer + offset, activeEntities, sizeof(activeEntities));
        offset += sizeof(activeEntities);
        for (int i = 0; i < componentCount; i++) {
            if (componentArrays[i]) {
                componentArrays[i]->serialize(buffer, offset);
            }
        }
    }

    void deserialize(const uint8_t* buffer, size_t& offset) {
        std::memcpy(activeEntities, buffer + offset, sizeof(activeEntities));
        offset += sizeof(activeEntities);
        for (int i = 0; i < componentCount; i++) {
            if (componentArrays[i]) {
                componentArrays[i]->deserialize(buffer, offset);
            }
        }
    }

private:
    template<typename T>
    ComponentArray<T>* getComponentArray() {
        int typeId = getComponentTypeId<T>();
        if (typeId < MAX_COMPONENTS && !componentArrays[typeId]) {
            registerComponent<T>();
        }
        return static_cast<ComponentArray<T>*>(componentArrays[typeId]);
    }
};

} // namespace FrameZero
