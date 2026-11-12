// Mouse interaction helpers — Sudheysi (CP3, issue #41)
#include "input.h"

#include <algorithm>
#include <cmath>

namespace game {

const Hotspot* pick(const std::vector<Hotspot>& spots, PointF p) {
    for (size_t i = spots.size(); i-- > 0;)                // topmost first
        if (pointInPolygon(spots[i].area, p)) return &spots[i];
    return nullptr;
}

PointF clampTo(PointF p, const ClipRect& a) {
    return {std::min(std::max(p.x, a.xmin), a.xmax), std::min(std::max(p.y, a.ymin), a.ymax)};
}

double angleAt(PointF c, PointF p) { return std::atan2(p.y - c.y, p.x - c.x); }

double angleBetween(double from, double to) {
    const double PI = 3.14159265358979;
    double d = std::fmod(to - from, 2 * PI);
    if (d > PI) d -= 2 * PI;
    if (d <= -PI) d += 2 * PI;
    return d;
}

bool inCircle(PointF c, double r, PointF p) {
    const double dx = p.x - c.x, dy = p.y - c.y;
    return dx * dx + dy * dy <= r * r;
}

}  // namespace game
