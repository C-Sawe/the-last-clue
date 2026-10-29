// Demo: Bézier curves via de Casteljau — quadratic, cubic, higher degree, sagging strings.
// Output: output/07_bezier.bmp
#include <cmath>

#include "curves/curves.h"
#include "demo_common.h"
#include "primitives/primitives.h"

int main() {
    Canvas c(1000, 400, Palette::White);
    demo::title(c, "BEZIER CURVES", "DE CASTELJAU");

    // Quadratic
    std::vector<PointF> q{{40, 300}, {160, 70}, {280, 300}};
    drawControlPolygon(c, q, Color{160, 160, 175});
    drawBezier(c, q, Palette::Blue, 80);
    demo::caption(c, 90, 340, "QUADRATIC (3 PTS)");

    // Cubic S-curve
    std::vector<PointF> cu{{320, 300}, {360, 60}, {520, 330}, {560, 90}};
    drawControlPolygon(c, cu, Color{160, 160, 175});
    drawBezier(c, cu, Palette::Red, 100);
    demo::caption(c, 380, 340, "CUBIC (4 PTS)");

    // Degree 5 — de Casteljau handles any number of control points
    std::vector<PointF> d5{{600, 280}, {620, 80}, {700, 300}, {760, 70}, {800, 290}, {640, 200}};
    drawControlPolygon(c, d5, Color{160, 160, 175});
    drawBezier(c, d5, Palette::Green, 120);
    demo::caption(c, 640, 340, "DEGREE 5 (6 PTS)");

    // Project use: evidence-board strings sag between two pins.
    // Control point = midpoint pushed down by a sag proportional to the distance.
    for (int k = 0; k < 3; ++k) {
        const PointF a{850, 90.0 + k * 80}, b{975, 70.0 + k * 80};
        const double sag = 12 + k * 14;
        const PointF ctrl{(a.x + b.x) / 2, (a.y + b.y) / 2 + sag};
        drawBezier(c, {a, ctrl, b}, Palette::StringRed, 40);
        drawBezier(c, {{a.x, a.y + 1}, {ctrl.x, ctrl.y + 1}, {b.x, b.y + 1}}, Palette::StringRed, 40);
        fillCircle(c, {850, 90 + k * 80}, 6, Palette::Red);
        fillCircle(c, {975, 70 + k * 80}, 6, Palette::Red);
    }
    demo::caption(c, 845, 340, "SAGGING STRINGS");
    return demo::save(c, "07_bezier");
}
