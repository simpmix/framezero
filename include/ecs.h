#ifndef FRAMEZERO_ECS_H
#define FRAMEZERO_ECS_H

#include <cstdint>
#include <vector>
#include <cstring>
#include <typeinfo>
#include <unordered_map>

namespace FrameZero {

using Entity = uint32_t;
constexpr Entity MAX_ENTITIES = 256;
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
    
    // Memory footprint: sizeof(data) + sizeof(active)
    size_t getMemorySize() const override {
        return sizeof(data) + sizeof(active);
    }
    
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
    
    // Store component arrays
    std::unordered_map<const char*, IComponentArray*> componentArrays;

public:
    Registry() {
        std::memset(activeEntities, 0, sizeof(activeEntities));
    }
    
    ~Registry() {
        for (auto& pair : componentArrays) {
            delete pair.second;
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
            for (auto& pair : componentArrays) {
                pair.second->entityDestroyed(entity);
            }
        }
    }

    template<typename T>
    void registerComponent() {
        const char* typeName = typeid(T).name();
        if (componentArrays.find(typeName) == componentArrays.end()) {
            componentArrays[typeName] = new ComponentArray<T>();
        }
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>()->insert(entity, component);
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
        return getComponentArray<T>()->has(entity);
    }

    // --- Rollback Hooks ---
    
    size_t getSerializationSize() const {
        size_t total = sizeof(activeEntities);
        for (const auto& pair : componentArrays) {
            total += pair.second->getMemorySize();
        }
        return total;
    }

    void serialize(uint8_t* buffer, size_t& offset) const {
        std::memcpy(buffer + offset, activeEntities, sizeof(activeEntities));
        offset += sizeof(activeEntities);
        for (const auto& pair : componentArrays) {
            pair.second->serialize(buffer, offset);
        }
    }

    void deserialize(const uint8_t* buffer, size_t& offset) {
        std::memcpy(activeEntities, buffer + offset, sizeof(activeEntities));
        offset += sizeof(activeEntities);
        for (auto& pair : componentArrays) {
            pair.second->deserialize(buffer, offset);
        }
    }

private:
    template<typename T>
    ComponentArray<T>* getComponentArray() {
        const char* typeName = typeid(T).name();
        return static_cast<ComponentArray<T>*>(componentArrays[typeName]);
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_ECS_H
