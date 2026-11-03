// Demo: Catmull-Rom spline vs Bézier on the same points; handwriting signature.
// Output: output/08_spline.bmp
#include "curves/curves.h"
#include "demo_common.h"
#include "primitives/primitives.h"

int main() {
    Canvas c(1000, 400, Palette::White);
    demo::title(c, "CATMULL-ROM SPLINES", "INTERPOLATING CURVE");

    std::vector<PointF> pts{{50, 260}, {110, 110}, {190, 230}, {260, 90}, {340, 250}, {420, 140}};

    // Same points, two curves: Bézier only touches the ends, Catmull-Rom hits them all.
    drawBezier(c, pts, Color{150, 190, 240}, 120);
    drawCatmullRom(c, pts, Palette::Red, 24);
    for (const auto& p : pts) fillCircle(c, {static_cast<int>(p.x), static_cast<int>(p.y)}, 5, Palette::Black);
    demo::caption(c, 50, 310, "RED: CATMULL-ROM PASSES THROUGH EVERY POINT");
    demo::caption(c, 50, 325, "BLUE: BEZIER ON SAME POINTS (ENDS ONLY)", Color{90, 130, 200});

    // Project use: a suspect's signature on the torn document
    for (int y = 120; y <= 300; y += 30) drawLineBresenham(c, {520, y}, {960, y}, Color{225, 220, 200});
    std::vector<PointF> sig{{540, 250}, {565, 170}, {585, 255}, {610, 175}, {640, 250}, {655, 215},
                            {690, 230}, {720, 180}, {735, 250}, {770, 205}, {805, 240}, {840, 190},
                            {870, 245}, {930, 215}};
    drawCatmullRom(c, sig, Color{20, 40, 120}, 16);
    drawCatmullRom(c, {{560, 275}, {700, 268}, {900, 280}}, Color{20, 40, 120}, 20);  // underline
    demo::caption(c, 600, 325, "SUSPECT SIGNATURE (14 POINTS)");
    return demo::save(c, "08_spline");
}
