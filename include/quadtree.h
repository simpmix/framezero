#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include "physics_body.h"
#include <vector>
#include <cstring>

namespace FrameZero {

// A strictly deterministic, fixed-point QuadTree for hyper-fast Broadphase collision detection
class QuadTree {
private:
    static constexpr int MAX_OBJECTS = 4;
    static constexpr int MAX_LEVELS = 5;
    
    int level;
    Vector2 boundsMin;
    Vector2 boundsMax;
    
    std::vector<PhysicsBody*> objects;
    QuadTree* nodes[4];
    
public:
    QuadTree(int pLevel, Vector2 pMin, Vector2 pMax) : level(pLevel), boundsMin(pMin), boundsMax(pMax) {
        for (int i = 0; i < 4; i++) nodes[i] = nullptr;
    }
    
    ~QuadTree() {
        clear();
    }
    
    void clear() {
        objects.clear();
        for (int i = 0; i < 4; i++) {
            if (nodes[i] != nullptr) {
                nodes[i]->clear();
                delete nodes[i];
                nodes[i] = nullptr;
            }
        }
    }
    
    void split() {
        Fixed subWidth = (boundsMax.x - boundsMin.x) / Fixed(2);
        Fixed subHeight = (boundsMax.y - boundsMin.y) / Fixed(2);
        
        Vector2 center(boundsMin.x + subWidth, boundsMin.y + subHeight);
        
        nodes[0] = new QuadTree(level + 1, center, boundsMax); // Top Right
        nodes[1] = new QuadTree(level + 1, Vector2(boundsMin.x, center.y), Vector2(center.x, boundsMax.y)); // Top Left
        nodes[2] = new QuadTree(level + 1, boundsMin, center); // Bottom Left
        nodes[3] = new QuadTree(level + 1, Vector2(center.x, boundsMin.y), Vector2(boundsMax.x, center.y)); // Bottom Right
    }
    
    int getIndex(PhysicsBody* pRect) {
        int index = -1;
        Fixed verticalMidpoint = boundsMin.x + (boundsMax.x - boundsMin.x) / Fixed(2);
        Fixed horizontalMidpoint = boundsMin.y + (boundsMax.y - boundsMin.y) / Fixed(2);
        
        Vector2 min = pRect->getMin();
        Vector2 max = pRect->getMax();
        
        bool topQuadrant = (min.y > horizontalMidpoint);
        bool bottomQuadrant = (max.y < horizontalMidpoint);
        
        bool leftQuadrant = (max.x < verticalMidpoint);
        bool rightQuadrant = (min.x > verticalMidpoint);
        
        if (leftQuadrant) {
            if (topQuadrant) index = 1;
            else if (bottomQuadrant) index = 2;
        } else if (rightQuadrant) {
            if (topQuadrant) index = 0;
            else if (bottomQuadrant) index = 3;
        }
        
        return index;
    }
    
    void insert(PhysicsBody* pRect) {
        if (nodes[0] != nullptr) {
            int index = getIndex(pRect);
            if (index != -1) {
                nodes[index]->insert(pRect);
                return;
            }
        }
        
        objects.push_back(pRect);
        
        if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS) {
            if (nodes[0] == nullptr) split();
            
            int i = 0;
            while (i < objects.size()) {
                int index = getIndex(objects[i]);
                if (index != -1) {
                    nodes[index]->insert(objects[i]);
                    objects.erase(objects.begin() + i);
                } else {
                    i++;
                }
            }
        }
    }
    
    void retrieve(std::vector<PhysicsBody*>& returnObjects, PhysicsBody* pRect) {
        int index = getIndex(pRect);
        if (index != -1 && nodes[0] != nullptr) {
            nodes[index]->retrieve(returnObjects, pRect);
        }
        
        // Add all objects in this node
        returnObjects.insert(returnObjects.end(), objects.begin(), objects.end());
    }
};

} // namespace FrameZero
