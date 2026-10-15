// Clipping and hit testing — Sudheysi
#include <vector>

#include "polygon.h"

// ---------------------------------------------------------------------------
// Cohen–Sutherland line clipping.
// Each endpoint gets a 4-bit outcode saying which side(s) of the window it lies:
//      TOP=8 | BOTTOM=4 | RIGHT=2 | LEFT=1   (0000 means inside)
//   - both codes 0          -> trivially ACCEPT (line fully inside)
//   - codeA & codeB != 0    -> trivially REJECT (both on the same outside side)
//   - otherwise pick an outside endpoint, intersect the line with the boundary
//     it violates, replace that endpoint, and repeat.
// ---------------------------------------------------------------------------
namespace {
enum : int { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };

int outcode(const PointF& p, const ClipRect& r) {
    int code = INSIDE;
    if (p.x < r.xmin) code |= LEFT;
    else if (p.x > r.xmax) code |= RIGHT;
    if (p.y < r.ymin) code |= TOP;        // screen y grows downward: smaller y is "top"
    else if (p.y > r.ymax) code |= BOTTOM;
    return code;
}
}  // namespace

bool cohenSutherlandClip(PointF& a, PointF& b, const ClipRect& r) {
    int codeA = outcode(a, r), codeB = outcode(b, r);
    while (true) {
        if ((codeA | codeB) == 0) return true;   // accept
        if ((codeA & codeB) != 0) return false;  // reject

        const int out = codeA ? codeA : codeB;
        PointF p;
        if (out & TOP) {
            p.x = a.x + (b.x - a.x) * (r.ymin - a.y) / (b.y - a.y);
            p.y = r.ymin;
        } else if (out & BOTTOM) {
            p.x = a.x + (b.x - a.x) * (r.ymax - a.y) / (b.y - a.y);
            p.y = r.ymax;
        } else if (out & RIGHT) {
            p.y = a.y + (b.y - a.y) * (r.xmax - a.x) / (b.x - a.x);
            p.x = r.xmax;
        } else {  // LEFT
            p.y = a.y + (b.y - a.y) * (r.xmin - a.x) / (b.x - a.x);
            p.x = r.xmin;
        }
        if (out == codeA) { a = p; codeA = outcode(a, r); }
        else              { b = p; codeB = outcode(b, r); }
    }
}

// ---------------------------------------------------------------------------
// Sutherland–Hodgman polygon clipping.
// Clip the polygon against ONE window edge at a time (left, right, top, bottom).
// For every polygon edge S -> E, relative to the clip edge:
//   in  -> in  : output E
//   in  -> out : output the intersection
//   out -> in  : output the intersection, then E
//   out -> out : output nothing
// The output of one pass is the input of the next.
// ---------------------------------------------------------------------------
namespace {
enum class Side { Left, Right, Top, Bottom };

bool inside(const PointF& p, Side s, const ClipRect& r) {
    switch (s) {
        case Side::Left:   return p.x >= r.xmin;
        case Side::Right:  return p.x <= r.xmax;
        case Side::Top:    return p.y >= r.ymin;
        case Side::Bottom: return p.y <= r.ymax;
    }
    return false;
}

PointF intersect(const PointF& s, const PointF& e, Side side, const ClipRect& r) {
    switch (side) {
        case Side::Left:   return {r.xmin, s.y + (e.y - s.y) * (r.xmin - s.x) / (e.x - s.x)};
        case Side::Right:  return {r.xmax, s.y + (e.y - s.y) * (r.xmax - s.x) / (e.x - s.x)};
        case Side::Top:    return {s.x + (e.x - s.x) * (r.ymin - s.y) / (e.y - s.y), r.ymin};
        case Side::Bottom: return {s.x + (e.x - s.x) * (r.ymax - s.y) / (e.y - s.y), r.ymax};
    }
    return s;
}
}  // namespace

Polygon sutherlandHodgmanClip(const Polygon& subject, const ClipRect& r) {
    std::vector<PointF> output = subject.vertices;
    for (Side side : {Side::Left, Side::Right, Side::Top, Side::Bottom}) {
        if (output.empty()) break;
        const std::vector<PointF> input = output;
        output.clear();
        PointF s = input.back();
        for (const PointF& e : input) {
            const bool eIn = inside(e, side, r), sIn = inside(s, side, r);
            if (eIn) {
                if (!sIn) output.push_back(intersect(s, e, side, r));
                output.push_back(e);
            } else if (sIn) {
                output.push_back(intersect(s, e, side, r));
            }
            s = e;
        }
    }
    return Polygon{output};
}

// ---------------------------------------------------------------------------
// Point in polygon — ray casting (even-odd rule).
// Shoot a horizontal ray to the right of p and count edge crossings:
// odd = inside, even = outside. Works for concave polygons too.
// This is what mouse picking / drag-and-drop will use in CP3.
// ---------------------------------------------------------------------------
bool pointInPolygon(const Polygon& poly, PointF p) {
    bool in = false;
    const size_t n = poly.vertices.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const PointF& a = poly.vertices[i];
        const PointF& b = poly.vertices[j];
        if ((a.y > p.y) != (b.y > p.y)) {
            const double xCross = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
            if (p.x < xCross) in = !in;
        }
    }
    return in;
}
