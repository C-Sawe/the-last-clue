// Bézier curves — Latiffa
#include <algorithm>
#include <cmath>
#include <vector>

#include "../primitives/primitives.h"
#include "curves.h"

// de Casteljau's algorithm: repeated linear interpolation.
// With control points P0..Pn, interpolate each neighbouring pair at t,
// giving n points; repeat until one point is left — that point is B(t).
//   quadratic (3 points): string sagging between two pins
//   cubic     (4 points): smooth S-shapes such as handwriting
// It works for any degree and is numerically stable.
PointF bezierPoint(const std::vector<PointF>& control, double t) {
    if (control.empty()) return {0, 0};
    std::vector<PointF> pts = control;
    for (size_t level = pts.size() - 1; level > 0; --level)
        for (size_t i = 0; i < level; ++i)
            pts[i] = {(1 - t) * pts[i].x + t * pts[i + 1].x,
                      (1 - t) * pts[i].y + t * pts[i + 1].y};
    return pts[0];
}

namespace {
Point toPixel(PointF p) {
    return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))};
}
}  // namespace

// Sample the curve at `segments` + 1 values of t and join them with Bresenham lines.
void drawBezier(Canvas& c, const std::vector<PointF>& control, Color col, int segments) {
    if (control.size() < 2 || segments < 1) return;
    Point prev = toPixel(bezierPoint(control, 0.0));
    for (int i = 1; i <= segments; ++i) {
        const Point cur = toPixel(bezierPoint(control, static_cast<double>(i) / segments));
        drawLineBresenham(c, prev, cur, col);
        prev = cur;
    }
}

void drawControlPolygon(Canvas& c, const std::vector<PointF>& control, Color col) {
    for (size_t i = 0; i + 1 < control.size(); ++i) {
        // dashed control polygon: draw only every other 6-pixel chunk
        const PointF a = control[i], b = control[i + 1];
        const double len = std::hypot(b.x - a.x, b.y - a.y);
        const int dashes = std::max(1, static_cast<int>(len / 6));
        for (int d = 0; d < dashes; d += 2) {
            const double t0 = static_cast<double>(d) / dashes, t1 = static_cast<double>(d + 1) / dashes;
            drawLineBresenham(c, toPixel({a.x + (b.x - a.x) * t0, a.y + (b.y - a.y) * t0}),
                                 toPixel({a.x + (b.x - a.x) * t1, a.y + (b.y - a.y) * t1}), col);
        }
    }
    for (const auto& p : control) {
        fillCircle(c, toPixel(p), 4, col);
        drawCircleMidpoint(c, toPixel(p), 4, Palette::Black);
    }
}
