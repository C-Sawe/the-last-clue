// Polygon representation — Alvin
//
// A polygon is an ordered list of vertices; edge i joins vertex i to vertex i+1,
// and the last vertex joins back to the first. Vertices are stored as doubles so
// transformed (rotated / scaled) polygons keep their precision; they are rounded
// to pixels only when drawn.
#include "polygon.h"

#include <cmath>

#include "../primitives/primitives.h"

Polygon makeRect(double x, double y, double w, double h) {
    return Polygon{{{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}}};
}

Polygon makeRegularPolygon(PointF centre, double radius, int sides, double startAngle) {
    Polygon p;
    const double PI = 3.14159265358979;
    for (int i = 0; i < sides; ++i) {
        const double a = startAngle + i * 2 * PI / sides;
        p.vertices.push_back({centre.x + radius * std::cos(a), centre.y + radius * std::sin(a)});
    }
    return p;
}

Polygon makeStar(PointF centre, double outerR, double innerR, int points) {
    Polygon p;
    const double PI = 3.14159265358979;
    for (int i = 0; i < points * 2; ++i) {
        const double r = (i % 2 == 0) ? outerR : innerR;
        const double a = -PI / 2 + i * PI / points;
        p.vertices.push_back({centre.x + r * std::cos(a), centre.y + r * std::sin(a)});
    }
    return p;
}

// Outline = Bresenham line for every edge (closing edge included).
void drawPolygonOutline(Canvas& c, const Polygon& poly, Color col) {
    const size_t n = poly.vertices.size();
    if (n < 2) return;
    for (size_t i = 0; i < n; ++i) {
        const PointF a = poly.vertices[i];
        const PointF b = poly.vertices[(i + 1) % n];
        drawLineBresenham(c, {static_cast<int>(std::lround(a.x)), static_cast<int>(std::lround(a.y))},
                             {static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y))}, col);
    }
}
