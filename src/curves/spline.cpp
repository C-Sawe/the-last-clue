// Catmull-Rom spline — Latiffa
//
// A Bézier curve only passes through its first and last control points.
// A Catmull-Rom spline passes through EVERY point, which is what we want for
// handwriting or a path the detective walks along.
//
// Each span between P1 and P2 uses its neighbours P0 and P3:
//   C(t) = 0.5 * [ 2P1
//                + (-P0 + P2) t
//                + (2P0 - 5P1 + 4P2 - P3) t^2
//                + (-P0 + 3P1 - 3P2 + P3) t^3 ]
// The first and last points are duplicated so the curve reaches the ends.
#include <cmath>
#include <vector>

#include "../primitives/primitives.h"
#include "curves.h"

namespace {
PointF catmullRom(const PointF& p0, const PointF& p1, const PointF& p2, const PointF& p3, double t) {
    const double t2 = t * t, t3 = t2 * t;
    auto f = [&](double a, double b, double c, double d) {
        return 0.5 * (2 * b + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t2 + (-a + 3 * b - 3 * c + d) * t3);
    };
    return {f(p0.x, p1.x, p2.x, p3.x), f(p0.y, p1.y, p2.y, p3.y)};
}

Point toPixel(PointF p) {
    return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))};
}
}  // namespace

void drawCatmullRom(Canvas& c, const std::vector<PointF>& pts, Color col, int segmentsPerSpan) {
    if (pts.size() < 2) return;
    std::vector<PointF> p;
    p.push_back(pts.front());
    p.insert(p.end(), pts.begin(), pts.end());
    p.push_back(pts.back());

    Point prev = toPixel(pts.front());
    for (size_t i = 1; i + 2 < p.size(); ++i)
        for (int s = 1; s <= segmentsPerSpan; ++s) {
            const Point cur = toPixel(catmullRom(p[i - 1], p[i], p[i + 1], p[i + 2],
                                                 static_cast<double>(s) / segmentsPerSpan));
            drawLineBresenham(c, prev, cur, col);
            prev = cur;
        }
}
