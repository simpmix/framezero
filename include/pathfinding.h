#pragma once
#include "collision.h"
#include <vector>
#include <queue>

namespace FrameZero {

struct PathNode {
    int gridX, gridY;
    Fixed gCost;
    Fixed hCost;
    PathNode* parent;
    
    Fixed fCost() const { return gCost + hCost; }
    
    bool operator>(const PathNode& other) const {
        return fCost() > other.fCost();
    }
};

// Deterministic A* Pathfinding for Rollback
class Pathfinding {
private:
    static constexpr int MAX_NODES = 256;
    PathNode nodePool[MAX_NODES];
    int poolCount = 0;
    
    PathNode* allocNode(int x, int y, Fixed g, Fixed h, PathNode* parent) {
        if (poolCount >= MAX_NODES) return nullptr;
        PathNode* node = &nodePool[poolCount++];
        node->gridX = x;
        node->gridY = y;
        node->gCost = g;
        node->hCost = h;
        node->parent = parent;
        return node;
    }

    Fixed heuristic(int x1, int y1, int x2, int y2) {
        // Manhattan distance in fixed point
        int dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);
        int dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
        return Fixed(dx + dy);
    }

public:
    // Finds a deterministic path across the Spatial Grid. 
    // Uses no heap allocations at runtime (pool based).
    bool findPath(const SpatialGrid& grid, Vector2 startWorld, Vector2 targetWorld, std::vector<Vector2>& outPath) {
        poolCount = 0;
        outPath.clear();
        
        int startX = (startWorld.x / grid.cellSize).toInt();
        int startY = (startWorld.y / grid.cellSize).toInt();
        int targetX = (targetWorld.x / grid.cellSize).toInt();
        int targetY = (targetWorld.y / grid.cellSize).toInt();
        
        // Custom simple priority queue using vector to stay perfectly deterministic
        std::vector<PathNode*> openList;
        bool closedList[64][64] = {false}; // Assume a localized 64x64 search area for safety
        
        PathNode* startNode = allocNode(startX, startY, Fixed(0), heuristic(startX, startY, targetX, targetY), nullptr);
        if(!startNode) return false;
        
        openList.push_back(startNode);
        
        while (!openList.empty()) {
            // Find lowest fCost deterministically (no std::priority_queue which might swap identically-weighted items differently)
            int lowestIdx = 0;
            for (size_t i = 1; i < openList.size(); i++) {
                if (openList[i]->fCost() < openList[lowestIdx]->fCost()) {
                    lowestIdx = i;
                }
            }
            
            PathNode* current = openList[lowestIdx];
            openList.erase(openList.begin() + lowestIdx);
            
            // Reached target
            if (current->gridX == targetX && current->gridY == targetY) {
                PathNode* trace = current;
                while (trace != nullptr) {
                    outPath.push_back(Vector2(Fixed(trace->gridX) * grid.cellSize, Fixed(trace->gridY) * grid.cellSize));
                    trace = trace->parent;
                }
                // Reverse to start -> end
                for (size_t i = 0; i < outPath.size() / 2; i++) {
                    Vector2 temp = outPath[i];
                    outPath[i] = outPath[outPath.size() - 1 - i];
                    outPath[outPath.size() - 1 - i] = temp;
                }
                return true;
            }
            
            // Map grid coordinates to safe bounds for closed list
            int localX = current->gridX % 64;
            int localY = current->gridY % 64;
            if (localX < 0) localX += 64;
            if (localY < 0) localY += 64;
            
            closedList[localX][localY] = true;
            
            // Check 4 neighbors
            int neighbors[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
            for (int i = 0; i < 4; i++) {
                int nx = current->gridX + neighbors[i][0];
                int ny = current->gridY + neighbors[i][1];
                
                int nlX = nx % 64; int nlY = ny % 64;
                if (nlX < 0) nlX += 64; if (nlY < 0) nlY += 64;
                
                if (closedList[nlX][nlY]) continue;
                
                // If a body exists in this cell, treat it as a wall!
                if (grid.hasBodiesInCell(nx, ny)) continue;
                
                Fixed gCost = current->gCost + Fixed(1);
                Fixed hCost = heuristic(nx, ny, targetX, targetY);
                
                // Check if already in open list with better cost
                bool inOpenList = false;
                for (auto* node : openList) {
                    if (node->gridX == nx && node->gridY == ny) {
                        inOpenList = true;
                        if (gCost < node->gCost) {
                            node->gCost = gCost;
                            node->parent = current;
                        }
                        break;
                    }
                }
                
                if (!inOpenList) {
                    PathNode* neighborNode = allocNode(nx, ny, gCost, hCost, current);
                    if (neighborNode) openList.push_back(neighborNode);
                }
            }
        }
        return false;
    }
};

} // namespace FrameZero
