#ifndef FRAMEZERO_COLLISION_H
#define FRAMEZERO_COLLISION_H

#include "physics_body.h"
#include <algorithm>

namespace FrameZero {

struct CollisionContact {
    PhysicsBody* bodyA;
    PhysicsBody* bodyB;
    Vector2 normal;
    Fixed penetration;
    Vector2 contactPoint;
    
    CollisionContact() : bodyA(nullptr), bodyB(nullptr), normal(0, 0), penetration(0), contactPoint(0, 0) {}
};

struct OBB {
    Vector2 center;
    Vector2 extents; // Half-widths
    Fixed angle;     // Rotation in radians
    
    // Get the 4 corners of the OBB
    void getVertices(Vector2* outVertices) const {
        Fixed cosA = Fixed::cos(angle);
        Fixed sinA = Fixed::sin(angle);
        
        Vector2 xDir = Vector2(cosA, sinA) * extents.x;
        Vector2 yDir = Vector2(-sinA, cosA) * extents.y;
        
        outVertices[0] = center + xDir + yDir;
        outVertices[1] = center - xDir + yDir;
        outVertices[2] = center - xDir - yDir;
        outVertices[3] = center + xDir - yDir;
    }
};

inline bool checkOBBOverlap(const OBB& a, const OBB& b) {
    Vector2 aVerts[4];
    Vector2 bVerts[4];
    a.getVertices(aVerts);
    b.getVertices(bVerts);
    
    // 4 axes from OBB A, 4 from OBB B (reduced to 4 unique axes because rectangles are parallel)
    Vector2 axes[4] = {
        Vector2(Fixed::cos(a.angle), Fixed::sin(a.angle)),
        Vector2(-Fixed::sin(a.angle), Fixed::cos(a.angle)),
        Vector2(Fixed::cos(b.angle), Fixed::sin(b.angle)),
        Vector2(-Fixed::sin(b.angle), Fixed::cos(b.angle))
    };
    
    for (int i = 0; i < 4; i++) {
        Vector2 axis = axes[i];
        
        Fixed minA = Fixed(99999), maxA = Fixed(-99999);
        for (int j = 0; j < 4; j++) {
            Fixed proj = aVerts[j].dot(axis);
            if (proj < minA) minA = proj;
            if (proj > maxA) maxA = proj;
        }
        
        Fixed minB = Fixed(99999), maxB = Fixed(-99999);
        for (int j = 0; j < 4; j++) {
            Fixed proj = bVerts[j].dot(axis);
            if (proj < minB) minB = proj;
            if (proj > maxB) maxB = proj;
        }
        
        // If there's NO overlap on this axis, the OBBs don't intersect
        if (maxA < minB || maxB < minA) {
            return false; 
        }
    }
    return true; // Overlap on all axes!
}

// Spatial Grid Broadphase
class SpatialGrid {
public:
    static constexpr int MAX_CELLS = 1024;
    Fixed cellSize;

    struct Node {
        PhysicsBody* body;
        Node* next;
    };

    Node* cells[MAX_CELLS];
    
    // Memory pool for nodes to avoid allocations
    static constexpr int MAX_NODES = 8192;
    Node nodePool[MAX_NODES];
    int nodeCount;

    SpatialGrid(Fixed cellSz = Fixed(2)) : cellSize(cellSz), nodeCount(0) {
        clear();
    }

    void clear() {
        for (int i = 0; i < MAX_CELLS; ++i) {
            cells[i] = nullptr;
        }
        nodeCount = 0;
    }

    // Check if a cell contains any bodies (useful for pathfinding obstacles)
    bool hasBodiesInCell(int x, int y) const {
        int cellIndex = hash(x, y);
        return cells[cellIndex] != nullptr;
    }

    // Deterministic hash for spatial coordinates
    int hash(int x, int y) const {
        unsigned int h = static_cast<unsigned int>((x * 73856093) ^ (y * 19349663));
        return h % MAX_CELLS;
    }

    void insert(PhysicsBody* body) {
        Vector2 min = body->getMin();
        Vector2 max = body->getMax();

        // Convert spatial coordinates to grid cell coordinates
        int minX = (min.x / cellSize).toInt();
        int minY = (min.y / cellSize).toInt();
        int maxX = (max.x / cellSize).toInt();
        int maxY = (max.y / cellSize).toInt();

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                int cellIndex = hash(x, y);

                if (nodeCount < MAX_NODES) {
                    Node* node = &nodePool[nodeCount++];
                    node->body = body;
                    node->next = cells[cellIndex];
                    cells[cellIndex] = node;
                }
            }
        }
    }
};

// AABB Collision Detection
class CollisionSystem {
public:
    static constexpr int MAX_CONTACTS = 64;
    
    CollisionContact contacts[MAX_CONTACTS];
    int contactCount;
    SpatialGrid grid;
    
    CollisionSystem() : contactCount(0), grid(Fixed(2)) {}
    
    struct RaycastResult {
        bool hit;
        PhysicsBody* body;
        Fixed distance;
        Vector2 point;
        Vector2 normal;
    };
    
    // Deterministic Raycast against a single AABB
    static bool raycastAABB(Vector2 origin, Vector2 dir, Fixed maxDist, const PhysicsBody& target, RaycastResult& outResult) {
        // Slab method for AABB ray intersection
        Vector2 min = target.getMin();
        Vector2 max = target.getMax();
        
        Fixed tmin = Fixed(0);
        Fixed tmax = maxDist;
        Vector2 normal(0, 0);
        
        // X-axis
        if (dir.x.raw != 0) {
            Fixed invDirX = Fixed(1) / dir.x;
            Fixed tx1 = (min.x - origin.x) * invDirX;
            Fixed tx2 = (max.x - origin.x) * invDirX;
            
            Fixed nX = Fixed(-1);
            if (tx1 > tx2) { std::swap(tx1, tx2); nX = Fixed(1); }
            
            if (tx1 > tmin) { tmin = tx1; normal = Vector2(nX, Fixed(0)); }
            tmax = std::min(tmax, tx2);
            if (tmin > tmax) return false;
        } else if (origin.x < min.x || origin.x > max.x) {
            return false;
        }
        
        // Y-axis
        if (dir.y.raw != 0) {
            Fixed invDirY = Fixed(1) / dir.y;
            Fixed ty1 = (min.y - origin.y) * invDirY;
            Fixed ty2 = (max.y - origin.y) * invDirY;
            
            Fixed nY = Fixed(-1);
            if (ty1 > ty2) { std::swap(ty1, ty2); nY = Fixed(1); }
            
            if (ty1 > tmin) { tmin = ty1; normal = Vector2(Fixed(0), nY); }
            tmax = std::min(tmax, ty2);
            if (tmin > tmax) return false;
        } else if (origin.y < min.y || origin.y > max.y) {
            return false;
        }
        
        outResult.hit = true;
        outResult.distance = tmin;
        outResult.point = origin + dir * tmin;
        outResult.normal = normal;
        return true;
    }
    
    // Deterministic Raycast against all active bodies
    RaycastResult raycast(Vector2 origin, Vector2 dir, Fixed maxDist, PhysicsBody* bodies, int bodyCount) {
        RaycastResult closestHit;
        closestHit.hit = false;
        closestHit.distance = maxDist;
        closestHit.body = nullptr;
        
        dir = dir.normalized(); // Ensure direction is a unit vector
        
        for (int i = 0; i < bodyCount; i++) {
            if (!bodies[i].active) continue;
            
            RaycastResult tempHit;
            if (raycastAABB(origin, dir, closestHit.distance, bodies[i], tempHit)) {
                closestHit = tempHit;
                closestHit.body = &bodies[i];
            }
        }
        
        return closestHit;
    }
    
    // Check if two bodies overlap (AABB test)
    static bool checkOverlap(const PhysicsBody& a, const PhysicsBody& b) {
        // Broadphase filter using Bitmasks!
        if (!(a.collisionMask & b.collisionCategory) || !(b.collisionMask & a.collisionCategory)) {
            return false;
        }
        Vector2 minA = a.getMin();
        Vector2 maxA = a.getMax();
        Vector2 minB = b.getMin();
        Vector2 maxB = b.getMax();
        
        return (minA.x < maxB.x && maxA.x > minB.x &&
                minA.y < maxB.y && maxA.y > minB.y);
    }
    
    // Detect collisions using 2D Spatial Grid (Grid Hash) Broadphase
    void detectCollisions(PhysicsBody* bodies, int bodyCount) {
        contactCount = 0;
        
        if (bodyCount < 2) return;
        
        grid.clear();
        
        // Insert active bodies into the grid based on their AABB
        for (int i = 0; i < bodyCount; ++i) {
            if (bodies[i].active) {
                grid.insert(&bodies[i]);
            }
        }
        
        // Check for collisions within grid cells
        for (int i = 0; i < SpatialGrid::MAX_CELLS; ++i) {
            SpatialGrid::Node* nodeA = grid.cells[i];
            while (nodeA) {
                PhysicsBody* a = nodeA->body;
                SpatialGrid::Node* nodeB = nodeA->next;
                
                while (nodeB) {
                    PhysicsBody* b = nodeB->body;
                    
                    // Enforce strict pointer ordering to prevent duplicate and self checks
                    if (a < b) {
                        if (a->type == STATIC && b->type == STATIC) {
                            nodeB = nodeB->next;
                            continue;
                        }
                        
                        if (checkOverlap(*a, *b)) {
                            // Check if this pair has already been resolved
                            bool alreadyResolved = false;
                            for (int c = 0; c < contactCount; ++c) {
                                if ((contacts[c].bodyA == a && contacts[c].bodyB == b) ||
                                    (contacts[c].bodyA == b && contacts[c].bodyB == a)) {
                                    alreadyResolved = true;
                                    break;
                                }
                            }
                            
                            if (!alreadyResolved && contactCount < MAX_CONTACTS) {
                                resolveCollision(*a, *b, contacts[contactCount]);
                                contactCount++;
                            }
                        }
                    }
                    
                    nodeB = nodeB->next;
                }
                nodeA = nodeA->next;
            }
        }
    }
    
    // Resolve collision and generate contact
    void resolveCollision(PhysicsBody& a, PhysicsBody& b, CollisionContact& contact) {
        contact.bodyA = &a;
        contact.bodyB = &b;
        
        // Find overlap on each axis
        Vector2 minA = a.getMin(), maxA = a.getMax();
        Vector2 minB = b.getMin(), maxB = b.getMax();
        
        Fixed overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
        Fixed overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
        
        // Determine collision normal (smallest overlap)
        if (overlapX < overlapY) {
            contact.penetration = overlapX;
            // Determine direction
            Fixed centerA = (minA.x + maxA.x) / Fixed(2);
            Fixed centerB = (minB.x + maxB.x) / Fixed(2);
            contact.normal = (centerA < centerB) ? Vector2(Fixed(-1), Fixed(0)) : Vector2(Fixed(1), Fixed(0));
        } else {
            contact.penetration = overlapY;
            Fixed centerA = (minA.y + maxA.y) / Fixed(2);
            Fixed centerB = (minB.y + maxB.y) / Fixed(2);
            contact.normal = (centerA < centerB) ? Vector2(Fixed(0), Fixed(-1)) : Vector2(Fixed(0), Fixed(1));
        }
        
        // Contact point (approximate midpoint)
        contact.contactPoint = (a.position + b.position) / Fixed(2);
        
        // Apply positional correction
        Fixed totalInvMass = a.invMass + b.invMass;
        if (totalInvMass.raw == 0) return;
        
        Fixed correctionPercent = Fixed(0.8);
        Fixed slop = Fixed(0.01);
        Fixed correctionMag = std::max(contact.penetration - slop, Fixed(0)) / totalInvMass * correctionPercent;
        Vector2 correction = contact.normal * correctionMag;
        
        if (a.type != STATIC) a.position -= correction * a.invMass;
        if (b.type != STATIC) b.position += correction * b.invMass;
        
        // Apply impulse
        Vector2 relVel = b.velocity - a.velocity;
        Fixed velAlongNormal = relVel.dot(contact.normal);
        
        if (velAlongNormal.raw > 0) return; // Moving apart
        
        Fixed e = std::min(a.restitution, b.restitution);
        Fixed j = -(Fixed(1) + e) * velAlongNormal;
        j = j / totalInvMass;
        
        Vector2 impulse = contact.normal * j;
        
        if (a.type != STATIC) a.velocity -= impulse * a.invMass;
        if (b.type != STATIC) b.velocity += impulse * b.invMass;
        
        // Friction
        Vector2 tangent = relVel - (contact.normal * velAlongNormal);
        Fixed tanLen = tangent.length();
        if (tanLen.raw > 0) {
            tangent = tangent / tanLen;
            Fixed mu = Fixed::sqrt(a.friction * b.friction);
            Fixed jt = -relVel.dot(tangent) / totalInvMass;
            
            Fixed frictionImpulseMag;
            if (jt < Fixed(0)) {
                frictionImpulseMag = -mu * j;
                jt = std::max(jt, frictionImpulseMag);
            } else {
                jt = std::min(jt, mu * j);
            }
            
            Vector2 frictionImpulse = tangent * jt;
            if (a.type != STATIC) a.velocity -= frictionImpulse * a.invMass;
            if (b.type != STATIC) b.velocity += frictionImpulse * b.invMass;
        }
    }
    
    void clearContacts() { contactCount = 0; }
};

} // namespace FrameZero

#endif // FRAMEZERO_COLLISION_H
