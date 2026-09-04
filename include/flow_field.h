#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include <cstring>

namespace FrameZero {

// Deterministic Flow Field Pathfinding for massive RTS-style unit swarms (O(1) pathing for 1000s of units)
class FlowField {
public:
    static constexpr int MAX_GRID_SIZE = 64;
    
    // Grid values: 0 = Target, 255 = Obstacle, 254 = Unreachable, else = Cost
    uint8_t costField[MAX_GRID_SIZE][MAX_GRID_SIZE];
    uint8_t integrationField[MAX_GRID_SIZE][MAX_GRID_SIZE];
    Vector2 vectorField[MAX_GRID_SIZE][MAX_GRID_SIZE]; // Direction vectors to target
    
    int gridWidth;
    int gridHeight;
    Fixed cellSize;
    
    FlowField(int width, int height, Fixed cellSz) : gridWidth(width), gridHeight(height), cellSize(cellSz) {
        if (gridWidth > MAX_GRID_SIZE) gridWidth = MAX_GRID_SIZE;
        if (gridHeight > MAX_GRID_SIZE) gridHeight = MAX_GRID_SIZE;
        reset();
    }
    
    void reset() {
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                costField[y][x] = 1; // Default walk cost
                integrationField[y][x] = 254; // Unreachable
                vectorField[y][x] = Vector2(0, 0);
            }
        }
    }
    
    void setObstacle(int x, int y, bool isObstacle) {
        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
            costField[y][x] = isObstacle ? 255 : 1;
        }
    }
    
    // Generates the entire flow field directed towards a single target
    void generate(int targetX, int targetY) {
        if (targetX < 0 || targetX >= gridWidth || targetY < 0 || targetY >= gridHeight) return;
        
        // 1. Reset Integration Field
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                integrationField[y][x] = 254;
            }
        }
        
        // 2. Dijkstra's Algorithm (Wavefront)
        integrationField[targetY][targetX] = 0;
        
        struct IntVec2 { int x; int y; };
        IntVec2 queue[MAX_GRID_SIZE * MAX_GRID_SIZE];
        int head = 0;
        int tail = 0;
        
        queue[tail++] = {targetX, targetY};
        
        int neighbors[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
        
        while (head < tail) {
            IntVec2 curr = queue[head++];
            
            for (int i = 0; i < 4; i++) {
                int nx = curr.x + neighbors[i][0];
                int ny = curr.y + neighbors[i][1];
                
                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    if (costField[ny][nx] == 255) continue; // Obstacle
                    
                    uint8_t cost = integrationField[curr.y][curr.x] + costField[ny][nx];
                    
                    if (cost < integrationField[ny][nx]) {
                        integrationField[ny][nx] = cost;
                        queue[tail++] = {nx, ny};
                    }
                }
            }
        }
        
        // 3. Generate Vector Field
        int diagNeighbors[8][2] = {
            {0, -1}, {1, -1}, {1, 0}, {1, 1},
            {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
        };
        
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (costField[y][x] == 255 || integrationField[y][x] == 254) {
                    vectorField[y][x] = Vector2(0, 0);
                    continue;
                }
                
                int minCost = integrationField[y][x];
                Vector2 minDir(0, 0);
                
                for (int i = 0; i < 8; i++) {
                    int nx = x + diagNeighbors[i][0];
                    int ny = y + diagNeighbors[i][1];
                    
                    if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                        if (integrationField[ny][nx] < minCost) {
                            minCost = integrationField[ny][nx];
                            // Approximate normalized direction vector for speed
                            Fixed dirX = Fixed::fromInt(diagNeighbors[i][0]);
                            Fixed dirY = Fixed::fromInt(diagNeighbors[i][1]);
                            
                            // Diagonal normalization (1 / sqrt(2) approx 0.707)
                            if (dirX.raw != 0 && dirY.raw != 0) {
                                dirX = dirX * Fixed(707) / Fixed(1000);
                                dirY = dirY * Fixed(707) / Fixed(1000);
                            }
                            minDir = Vector2(dirX, dirY);
                        }
                    }
                }
                vectorField[y][x] = minDir;
            }
        }
    }
    
    // O(1) query for any unit to instantly get its optimal direction vector
    Vector2 getDirection(Vector2 worldPos) const {
        if (worldPos.x < Fixed(0) || worldPos.y < Fixed(0)) return Vector2(0, 0);
        int x = (worldPos.x / cellSize).toInt();
        int y = (worldPos.y / cellSize).toInt();
        
        if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
            return vectorField[y][x];
        }
        return Vector2(0, 0);
    }
    
    // Serialization for rollback state
    void serialize(uint8_t* buffer) const {
        std::memcpy(buffer, this, sizeof(FlowField));
    }
    void deserialize(const uint8_t* buffer) {
        std::memcpy(this, buffer, sizeof(FlowField));
    }
    static constexpr size_t getSize() { return sizeof(FlowField); }
};

} // namespace FrameZero
