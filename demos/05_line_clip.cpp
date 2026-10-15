// Demo: Cohen–Sutherland line clipping, before vs after.
// Output: output/05_line_clip.bmp
#include <cmath>

#include "demo_common.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"

namespace {
Point px(PointF p) { return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}; }

void drawWindow(Canvas& c, const ClipRect& r) {
    drawPolygonOutline(c, makeRect(r.xmin, r.ymin, r.xmax - r.xmin, r.ymax - r.ymin), Palette::Black);
}

// Simple deterministic pseudo-random numbers (same image every run).
unsigned seed = 12345;
double rnd(double lo, double hi) {
    seed = seed * 1103515245u + 12345u;
    return lo + (hi - lo) * ((seed >> 8) & 0xFFFF) / 65535.0;
}
}  // namespace

int main() {
    Canvas c(940, 400, Palette::White);
    demo::title(c, "COHEN-SUTHERLAND LINE CLIPPING", "OUTCODES  TBRL");

    const int offset = 470;
    const ClipRect left{110, 110, 360, 300};
    const ClipRect right{left.xmin + offset, left.ymin, left.xmax + offset, left.ymax};

    for (int i = 0; i < 28; ++i) {
        PointF a{rnd(20, 450), rnd(50, 360)}, b{rnd(20, 450), rnd(50, 360)};
        drawLineBresenham(c, px(a), px(b), Palette::Blue);                         // before

        PointF a2{a.x + offset, a.y}, b2{b.x + offset, b.y};
        drawLineBresenham(c, px(a2), px(b2), Color{220, 220, 228});                // ghost
        if (cohenSutherlandClip(a2, b2, right))
            drawThickLine(c, px(a2), px(b2), 2, Palette::StringRed);               // kept part
    }
    drawWindow(c, left);
    drawWindow(c, right);
    demo::caption(c, 110, 375, "BEFORE: ALL SEGMENTS");
    demo::caption(c, 110 + offset, 375, "AFTER: ONLY THE PART INSIDE THE WINDOW");
    return demo::save(c, "05_line_clip");
}
