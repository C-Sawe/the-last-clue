// Demo: Sutherland–Hodgman polygon clipping + point-in-polygon (ray casting).
// Output: output/06_poly_clip.bmp
#include "demo_common.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "transform/transform2d.h"

namespace {
void window(Canvas& c, const ClipRect& r) {
    drawPolygonOutline(c, makeRect(r.xmin, r.ymin, r.xmax - r.xmin, r.ymax - r.ymin), Palette::Black);
}
}  // namespace

int main() {
    Canvas c(1050, 400, Palette::White);
    demo::title(c, "SUTHERLAND-HODGMAN + POINT IN POLYGON", "");

    // Shapes overlapping the window (relative to the first panel)
    const Polygon shapes[] = {
        makeStar({120, 140}, 85, 35, 5),
        makeRegularPolygon({300, 260}, 80, 6),
        Polygon{{{180, 300}, {260, 180}, {330, 340}}},
    };
    const Color colours[] = {Palette::Yellow, Palette::Wood, Palette::Green};
    const ClipRect win{90, 110, 290, 300};

    // Panel 1: before
    for (int i = 0; i < 3; ++i) { scanlineFill(c, shapes[i], colours[i]); drawPolygonOutline(c, shapes[i], Palette::Black); }
    window(c, win);
    demo::caption(c, 60, 370, "BEFORE");

    // Panel 2: after — each polygon clipped against the window, then filled
    const double off = 350;
    const ClipRect win2{win.xmin + off, win.ymin, win.xmax + off, win.ymax};
    for (int i = 0; i < 3; ++i) {
        const Polygon moved = apply(translate(off, 0), shapes[i]);
        drawPolygonOutline(c, moved, Color{215, 215, 222});
        const Polygon clipped = sutherlandHodgmanClip(moved, win2);
        scanlineFill(c, clipped, colours[i]);
        drawPolygonOutline(c, clipped, Palette::Black);
    }
    window(c, win2);
    demo::caption(c, 60 + off, 370, "AFTER CLIPPING (NEW VERTICES ON EDGES)");

    // Panel 3: point-in-polygon test on a concave star
    const Polygon star = makeStar({880, 205}, 130, 55, 5);
    drawPolygonOutline(c, star, Palette::Black);
    for (int y = 70; y <= 340; y += 14)
        for (int x = 750; x <= 1010; x += 14) {
            const bool in = pointInPolygon(star, {static_cast<double>(x), static_cast<double>(y)});
            fillCircle(c, {x, y}, 3, in ? Palette::Green : Color{220, 90, 90});
        }
    demo::caption(c, 740, 370, "RAY CASTING: GREEN = INSIDE");
    return demo::save(c, "06_poly_clip");
}
