#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include "collision.h"

namespace FrameZero {

// Fixed-size deterministic A* Pathfinding system (Zero allocations)
class Pathfinder {
public:
    static constexpr int MAX_NODES = 1024;
    static constexpr int MAX_PATH_LENGTH = 128;

    struct Node {
        Vector2 position;
        int gridX;
        int gridY;
        Fixed gCost; // Distance from start
        Fixed hCost; // Distance to target
        Fixed fCost; // Total cost
        int parentIndex;
        bool isObstacle;
        bool closed;
        bool open;
        
        Fixed getFCost() const { return gCost + hCost; }
    };

private:
    Node nodes[MAX_NODES];
    int gridWidth;
    int gridHeight;
    Fixed cellSize;

    int getIndex(int x, int y) const {
        return y * gridWidth + x;
    }

    Fixed calculateDistance(const Node& a, const Node& b) const {
        Fixed dx = a.gridX - b.gridX;
        Fixed dy = a.gridY - b.gridY;
        // Manhattan distance for extreme speed and determinism
        return Fixed::abs(dx) + Fixed::abs(dy);
    }

public:
    Pathfinder(int width, int height, Fixed cellSz) : gridWidth(width), gridHeight(height), cellSize(cellSz) {
        // Initialize grid
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                int idx = getIndex(x, y);
                nodes[idx].gridX = x;
                nodes[idx].gridY = y;
                nodes[idx].position = Vector2(Fixed(x) * cellSize, Fixed(y) * cellSize);
                nodes[idx].isObstacle = false;
            }
        }
    }

    void setObstacle(int x, int y, bool isObstacle) {
        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
            nodes[getIndex(x, y)].isObstacle = isObstacle;
        }
    }

    // Returns the number of points in the path. Populates outPath array.
    int findPath(int startX, int startY, int targetX, int targetY, Vector2* outPath, int maxPathSize) {
        if (startX < 0 || startX >= gridWidth || startY < 0 || startY >= gridHeight ||
            targetX < 0 || targetX >= gridWidth || targetY < 0 || targetY >= gridHeight) {
            return 0; // Out of bounds
        }

        // Reset state
        for (int i = 0; i < gridWidth * gridHeight; i++) {
            nodes[i].gCost = Fixed(0);
            nodes[i].hCost = Fixed(0);
            nodes[i].fCost = Fixed(0);
            nodes[i].parentIndex = -1;
            nodes[i].closed = false;
            nodes[i].open = false;
        }

        int startIdx = getIndex(startX, startY);
        int targetIdx = getIndex(targetX, targetY);
        
        nodes[startIdx].open = true;

        int openCount = 1;

        while (openCount > 0) {
            // Find node with lowest fCost
            int currentIdx = -1;
            Fixed lowestFCost = Fixed(999999);
            
            for (int i = 0; i < gridWidth * gridHeight; i++) {
                if (nodes[i].open && nodes[i].getFCost() < lowestFCost) {
                    lowestFCost = nodes[i].getFCost();
                    currentIdx = i;
                }
            }

            if (currentIdx == -1) break; // Should never happen
            if (currentIdx == targetIdx) {
                // Path found!
                int pathSize = 0;
                int traceIdx = targetIdx;
                
                // Backtrace
                int tempPath[MAX_PATH_LENGTH];
                while (traceIdx != startIdx && pathSize < MAX_PATH_LENGTH) {
                    tempPath[pathSize++] = traceIdx;
                    traceIdx = nodes[traceIdx].parentIndex;
                }
                
                // Reverse and output
                int outSize = (pathSize < maxPathSize) ? pathSize : maxPathSize;
                for (int i = 0; i < outSize; i++) {
                    outPath[i] = nodes[tempPath[pathSize - 1 - i]].position;
                }
                return outSize;
            }

            nodes[currentIdx].open = false;
            nodes[currentIdx].closed = true;
            openCount--;

            // Check neighbors
            int cx = nodes[currentIdx].gridX;
            int cy = nodes[currentIdx].gridY;
            
            int neighbors[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
            
            for (int i = 0; i < 4; i++) {
                int nx = cx + neighbors[i][0];
                int ny = cy + neighbors[i][1];
                
                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    int nIdx = getIndex(nx, ny);
                    
                    if (nodes[nIdx].isObstacle || nodes[nIdx].closed) continue;
                    
                    Fixed moveCost = nodes[currentIdx].gCost + Fixed(1); // 1 grid unit
                    
                    if (moveCost < nodes[nIdx].gCost || !nodes[nIdx].open) {
                        nodes[nIdx].gCost = moveCost;
                        nodes[nIdx].hCost = calculateDistance(nodes[nIdx], nodes[targetIdx]);
                        nodes[nIdx].parentIndex = currentIdx;
                        
                        if (!nodes[nIdx].open) {
                            nodes[nIdx].open = true;
                            openCount++;
                        }
                    }
                }
            }
        }
        
        return 0; // No path found
    }
};

} // namespace FrameZero
