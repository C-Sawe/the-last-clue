// Demo: translate, rotate about a pivot, scale about a pivot, composite.
// Output: output/09_transforms.bmp
#include <cmath>

#include "demo_common.h"
#include "primitives/primitives.h"
#include "transform/transform2d.h"

namespace {
// Outline with our own Bresenham (keeps this demo independent of polygon fill).
void outline(Canvas& c, const Polygon& p, Color col) {
    const size_t n = p.vertices.size();
    for (size_t i = 0; i < n; ++i) {
        const PointF a = p.vertices[i], b = p.vertices[(i + 1) % n];
        drawThickLine(c, {static_cast<int>(std::lround(a.x)), static_cast<int>(std::lround(a.y))},
                         {static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y))}, 3, col);
    }
}

// A paper "clue" card with a folded corner
Polygon card(double x, double y) {
    return Polygon{{{x, y}, {x + 70, y}, {x + 90, y + 20}, {x + 90, y + 110}, {x, y + 110}}};
}

PointF centroid(const Polygon& p) {
    PointF s{0, 0};
    for (const auto& v : p.vertices) { s.x += v.x; s.y += v.y; }
    return {s.x / p.vertices.size(), s.y / p.vertices.size()};
}
}  // namespace

int main() {
    const double PI = 3.14159265358979;
    Canvas c(1000, 340, Palette::White);
    demo::title(c, "2D TRANSFORMATIONS", "3X3 HOMOGENEOUS MATRICES");

    const Polygon base = card(40, 110);
    outline(c, base, Palette::Blue);
    demo::caption(c, 40, 260, "ORIGINAL");

    // Translation: x' = x + tx
    outline(c, apply(translate(190, 0), base), Palette::Blue);
    demo::caption(c, 230, 260, "TRANSLATE(190,0)");

    // Rotation about the card's centre (pivot): T(p) R T(-p)
    const Polygon moved = apply(translate(420, 0), base);
    outline(c, moved, Color{200, 200, 210});
    outline(c, apply(rotateAbout(PI / 6, centroid(moved)), moved), Palette::Red);
    demo::caption(c, 410, 260, "ROTATE 30 ABOUT CENTRE");

    // Scale about pivot
    const Polygon moved2 = apply(translate(620, 30), base);
    outline(c, moved2, Color{200, 200, 210});
    outline(c, apply(scaleAbout(0.6, 0.6, centroid(moved2)), moved2), Palette::Green);
    demo::caption(c, 640, 260, "SCALE 0.6");

    // Composite: rotate, then scale, then translate — ONE matrix
    const Mat3 m = translate(860, 170) * scale(0.8, 0.8) * rotate(-PI / 8) * translate(-85, -165);
    outline(c, apply(m, base), Color{140, 60, 160});
    demo::caption(c, 800, 260, "COMPOSITE T*S*R");
    demo::caption(c, 12, 312, "GREY = BEFORE, COLOUR = AFTER.  USED NEXT FOR THE SAFE DIAL AND TORN DOCUMENT PIECES.", demo::Muted);
    return demo::save(c, "09_transforms");
}
