// Polygon filling — Alvin
#include <algorithm>
#include <cmath>
#include <vector>

#include "polygon.h"

// ---------------------------------------------------------------------------
// Scanline fill with an Edge Table (ET) and Active Edge Table (AET).
//
// 1. Build the ET: for every non-horizontal edge store
//      yStart (first scanline it crosses), yEnd (scanline where it stops, exclusive),
//      x at yStart, and 1/m (how much x moves per scanline).
// 2. Walk scanlines from top to bottom:
//      - move edges whose yStart == y from the ET into the AET
//      - drop edges whose yEnd == y
//      - sort the AET by x and fill between pairs (1st-2nd, 3rd-4th, ...)
//        -> this even-odd pairing is what makes concave shapes work
//      - add 1/m to every active edge's x (incremental, no re-intersecting)
// Scanlines sample pixel rows at integer y, and spans cover x in [ceil(xa), ceil(xb)),
// so two polygons sharing an edge never paint the same pixel twice.
// ---------------------------------------------------------------------------
namespace {
struct Edge {
    int yStart, yEnd;   // [yStart, yEnd)
    double x;           // x at the current scanline
    double invSlope;    // dx/dy
};
}  // namespace

void scanlineFill(Canvas& c, const Polygon& poly, Color fill) {
    const size_t n = poly.vertices.size();
    if (n < 3) return;

    std::vector<Edge> edgeTable;
    int yMin = 1 << 30, yMax = -(1 << 30);
    for (size_t i = 0; i < n; ++i) {
        PointF a = poly.vertices[i], b = poly.vertices[(i + 1) % n];
        if (a.y == b.y) continue;            // horizontal edges add no crossings
        if (a.y > b.y) std::swap(a, b);      // make a the upper endpoint
        const int ys = static_cast<int>(std::ceil(a.y));
        const int ye = static_cast<int>(std::ceil(b.y));
        if (ys >= ye) continue;              // edge falls between two scanlines
        const double inv = (b.x - a.x) / (b.y - a.y);
        edgeTable.push_back({ys, ye, a.x + (ys - a.y) * inv, inv});
        yMin = std::min(yMin, ys);
        yMax = std::max(yMax, ye);
    }
    std::sort(edgeTable.begin(), edgeTable.end(),
              [](const Edge& l, const Edge& r) { return l.yStart < r.yStart; });

    yMin = std::max(yMin, 0);
    yMax = std::min(yMax, c.height());
    std::vector<Edge> active;
    size_t next = 0;
    // Edges that start above the canvas: advance them to yMin.
    for (; next < edgeTable.size() && edgeTable[next].yStart < yMin; ++next) {
        Edge e = edgeTable[next];
        if (e.yEnd > yMin) { e.x += (yMin - e.yStart) * e.invSlope; active.push_back(e); }
    }

    for (int y = yMin; y < yMax; ++y) {
        while (next < edgeTable.size() && edgeTable[next].yStart == y) active.push_back(edgeTable[next++]);
        active.erase(std::remove_if(active.begin(), active.end(),
                                    [y](const Edge& e) { return e.yEnd <= y; }),
                     active.end());
        std::sort(active.begin(), active.end(), [](const Edge& l, const Edge& r) { return l.x < r.x; });

        for (size_t i = 0; i + 1 < active.size(); i += 2) {
            const int xa = static_cast<int>(std::ceil(active[i].x));
            const int xb = static_cast<int>(std::ceil(active[i + 1].x));
            for (int x = std::max(xa, 0); x < std::min(xb, c.width()); ++x) c.setPixel(x, y, fill);
        }
        for (auto& e : active) e.x += e.invSlope;
    }
}

// ---------------------------------------------------------------------------
// Flood fill (4-connected): recolour every pixel connected to `seed` that has
// the SAME colour as the seed. Uses an explicit stack instead of recursion, so
// large areas cannot overflow the call stack.
// ---------------------------------------------------------------------------
void floodFill(Canvas& c, Point seed, Color fill) {
    if (!c.inBounds(seed.x, seed.y)) return;
    const Color target = c.getPixel(seed.x, seed.y);
    if (target == fill) return;

    std::vector<Point> stack{seed};
    while (!stack.empty()) {
        const Point p = stack.back();
        stack.pop_back();
        if (!c.inBounds(p.x, p.y) || c.getPixel(p.x, p.y) != target) continue;
        c.setPixel(p.x, p.y, fill);
        stack.push_back({p.x + 1, p.y});
        stack.push_back({p.x - 1, p.y});
        stack.push_back({p.x, p.y + 1});
        stack.push_back({p.x, p.y - 1});
    }
}

// ---------------------------------------------------------------------------
// Boundary fill (4-connected): spread from `seed` until reaching pixels of the
// `boundary` colour. Unlike flood fill, the inside may contain several colours.
// ---------------------------------------------------------------------------
void boundaryFill(Canvas& c, Point seed, Color fill, Color boundary) {
    std::vector<Point> stack{seed};
    while (!stack.empty()) {
        const Point p = stack.back();
        stack.pop_back();
        if (!c.inBounds(p.x, p.y)) continue;
        const Color cur = c.getPixel(p.x, p.y);
        if (cur == boundary || cur == fill) continue;
        c.setPixel(p.x, p.y, fill);
        stack.push_back({p.x + 1, p.y});
        stack.push_back({p.x - 1, p.y});
        stack.push_back({p.x, p.y + 1});
        stack.push_back({p.x, p.y - 1});
    }
}
