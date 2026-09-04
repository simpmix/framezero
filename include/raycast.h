#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include "collision.h"

namespace FrameZero {

struct RaycastHit {
    bool hit;
    Vector2 point;
    Vector2 normal;
    Fixed distance;
    PhysicsBody* body;

    RaycastHit() : hit(false), point(0,0), normal(0,0), distance(0), body(nullptr) {}
};

class Raycaster {
public:
    // Performs a deterministic Raycast against all active PhysicsBodies.
    // Returns the closest hit.
    static RaycastHit cast(Vector2 origin, Vector2 direction, Fixed maxDistance, PhysicsBody* bodies, int bodyCount) {
        RaycastHit closestHit;
        closestHit.distance = maxDistance;
        
        // Normalize direction
        Fixed dirLength = direction.length();
        if (dirLength == Fixed(0)) return closestHit;
        
        direction.x = direction.x / dirLength;
        direction.y = direction.y / dirLength;

        for (int i = 0; i < bodyCount; i++) {
            if (!bodies[i].active) continue;

            // Simple Ray vs AABB intersection using slabs method (deterministic)
            Vector2 minB = bodies[i].getMin();
            Vector2 maxB = bodies[i].getMax();

            Fixed tmin = Fixed(0);
            Fixed tmax = maxDistance;
            Vector2 normal(0, 0);

            // X Axis
            if (direction.x != Fixed(0)) {
                Fixed invDirX = Fixed(1) / direction.x;
                Fixed t1 = (minB.x - origin.x) * invDirX;
                Fixed t2 = (maxB.x - origin.x) * invDirX;

                Fixed signX = (invDirX < Fixed(0)) ? Fixed(-1) : Fixed(1);

                if (t1 > t2) { Fixed temp = t1; t1 = t2; t2 = temp; }

                if (t1 > tmin) {
                    tmin = t1;
                    normal = Vector2(Fixed(-1) * signX, Fixed(0));
                }
                if (t2 < tmax) tmax = t2;
            } else if (origin.x < minB.x || origin.x > maxB.x) {
                continue; // Ray is parallel and outside the box
            }

            // Y Axis
            if (direction.y != Fixed(0)) {
                Fixed invDirY = Fixed(1) / direction.y;
                Fixed t1 = (minB.y - origin.y) * invDirY;
                Fixed t2 = (maxB.y - origin.y) * invDirY;

                Fixed signY = (invDirY < Fixed(0)) ? Fixed(-1) : Fixed(1);

                if (t1 > t2) { Fixed temp = t1; t1 = t2; t2 = temp; }

                if (t1 > tmin) {
                    tmin = t1;
                    normal = Vector2(Fixed(0), Fixed(-1) * signY);
                }
                if (t2 < tmax) tmax = t2;
            } else if (origin.y < minB.y || origin.y > maxB.y) {
                continue;
            }

            if (tmax >= tmin && tmin < closestHit.distance) {
                closestHit.hit = true;
                closestHit.distance = tmin;
                closestHit.body = &bodies[i];
                closestHit.normal = normal;
                closestHit.point = origin + (direction * tmin);
            }
        }

        return closestHit;
    }
};

} // namespace FrameZero
