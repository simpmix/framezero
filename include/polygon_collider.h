#pragma once
#include "fixed_point.h"
#include "vector2.h"
#include "collision.h"

namespace FrameZero {

struct Polygon {
    static constexpr int MAX_VERTICES = 8;
    Vector2 vertices[MAX_VERTICES];
    int vertexCount;

    Polygon() : vertexCount(0) {}

    void addVertex(const Vector2& v) {
        if (vertexCount < MAX_VERTICES) {
            vertices[vertexCount++] = v;
        }
    }

    // Rotates all vertices around a center point (useful for rotating hitboxes)
    void rotate(const Vector2& center, Fixed angleRadians) {
        Fixed cosA = Fixed::cos(angleRadians);
        Fixed sinA = Fixed::sin(angleRadians);

        for (int i = 0; i < vertexCount; i++) {
            Fixed px = vertices[i].x - center.x;
            Fixed py = vertices[i].y - center.y;

            vertices[i].x = center.x + (px * cosA - py * sinA);
            vertices[i].y = center.y + (px * sinA + py * cosA);
        }
    }
};

class PolygonCollider {
public:
    // Separating Axis Theorem (SAT) for convex polygons
    static bool checkOverlap(const Polygon& p1, const Polygon& p2, CollisionContact* outContact = nullptr) {
        Fixed minOverlap = Fixed(999999);
        Vector2 smallestAxis(0, 0);

        // Test edges of polygon 1
        for (int i = 0; i < p1.vertexCount; i++) {
            Vector2 p1_a = p1.vertices[i];
            Vector2 p1_b = p1.vertices[(i + 1) % p1.vertexCount];

            Vector2 edge = p1_b - p1_a;
            Vector2 axis(-edge.y, edge.x); // Normal
            
            Fixed length = axis.length();
            if (length == Fixed(0)) continue;
            axis.x = axis.x / length;
            axis.y = axis.y / length;

            if (!checkAxis(p1, p2, axis, minOverlap, smallestAxis)) return false;
        }

        // Test edges of polygon 2
        for (int i = 0; i < p2.vertexCount; i++) {
            Vector2 p2_a = p2.vertices[i];
            Vector2 p2_b = p2.vertices[(i + 1) % p2.vertexCount];

            Vector2 edge = p2_b - p2_a;
            Vector2 axis(-edge.y, edge.x);
            
            Fixed length = axis.length();
            if (length == Fixed(0)) continue;
            axis.x = axis.x / length;
            axis.y = axis.y / length;

            if (!checkAxis(p1, p2, axis, minOverlap, smallestAxis)) return false;
        }

        // Generate contact info if requested
        if (outContact) {
            outContact->normal = smallestAxis;
            outContact->penetration = minOverlap;
        }

        return true;
    }

private:
    static bool checkAxis(const Polygon& p1, const Polygon& p2, const Vector2& axis, Fixed& minOverlap, Vector2& smallestAxis) {
        Fixed min1 = Fixed(999999), max1 = Fixed(-999999);
        Fixed min2 = Fixed(999999), max2 = Fixed(-999999);

        // Project p1
        for (int i = 0; i < p1.vertexCount; i++) {
            Fixed proj = Vector2::dot(p1.vertices[i], axis);
            if (proj < min1) min1 = proj;
            if (proj > max1) max1 = proj;
        }

        // Project p2
        for (int i = 0; i < p2.vertexCount; i++) {
            Fixed proj = Vector2::dot(p2.vertices[i], axis);
            if (proj < min2) min2 = proj;
            if (proj > max2) max2 = proj;
        }

        if (max1 < min2 || max2 < min1) {
            return false; // Gap found, no collision!
        }

        // Find overlap depth
        Fixed overlap1 = max1 - min2;
        Fixed overlap2 = max2 - min1;
        Fixed overlap = (overlap1 < overlap2) ? overlap1 : overlap2;

        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }

        return true;
    }
};

} // namespace FrameZero
