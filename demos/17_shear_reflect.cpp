// Demo (CP2): shear and reflection, plus an animated card flip.
// Output: output/17_shear_reflect.bmp (static) and output/17_card_flip.gif/.bmp (animation)
#include <cmath>

#include "demo_common.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "transform/transform2d.h"

namespace {
const double PI = 3.14159265358979;

// An evidence card: polygon + a folded corner + a "photo" so orientation is visible
Polygon cardShape(double x, double y) {
    return Polygon{{{x, y}, {x + 70, y}, {x + 90, y + 20}, {x + 90, y + 110}, {x, y + 110}}};
}

void drawCard(Canvas& c, const Mat3& m, double x, double y, Color fill) {
    const Polygon card = apply(m, cardShape(x, y));
    scanlineFill(c, card, fill);
    drawPolygonOutline(c, card, Palette::Black);
    const Polygon photo = apply(m, makeRect(x + 10, y + 12, 40, 34));    // top-left photo
    scanlineFill(c, photo, Color{70, 90, 140});
    const Polygon line1 = apply(m, makeRect(x + 10, y + 60, 60, 4));
    const Polygon line2 = apply(m, makeRect(x + 10, y + 74, 45, 4));
    scanlineFill(c, line1, Palette::DarkGrey);
    scanlineFill(c, line2, Palette::DarkGrey);
}

PointF centre(double x, double y) { return {x + 45, y + 55}; }
}  // namespace

int main() {
    // ---------- Static sheet ----------
    Canvas c(1000, 340, Palette::White);
    demo::title(c, "SHEAR AND REFLECTION", "CP2 TRANSFORMS");

    drawCard(c, Mat3::identity(), 40, 110, Palette::Paper);
    demo::caption(c, 40, 260, "ORIGINAL");

    // Shear: x' = x + 0.4 y  (about the card's bottom edge so it leans right)
    drawCard(c, shearAbout(-0.4, 0, {230, 220}), 190, 110, Palette::Paper);
    demo::caption(c, 190, 260, "SHEAR X (-0.4)");

    // Reflect in a vertical line through the card centre (mirror image)
    drawCard(c, reflectAbout(centre(380, 110), PI / 2), 380, 110, Color{250, 220, 200});
    drawThickLine(c, {425, 95}, {425, 235}, 3, Palette::Red);
    demo::caption(c, 380, 260, "REFLECT (VERTICAL LINE)");

    // Reflect in the x-axis direction (upside down)
    drawCard(c, reflectAbout(centre(580, 110), 0), 580, 110, Color{210, 235, 210});
    drawThickLine(c, {565, 165}, {685, 165}, 3, Palette::Red);
    demo::caption(c, 580, 260, "REFLECT (HORIZONTAL)");

    // Reflect in a diagonal line through a point
    const PointF p{860, 165};
    drawCard(c, Mat3::identity(), 760, 110, Color{230, 230, 230});
    drawCard(c, reflectAbout(p, PI / 4), 760, 110, Color{200, 215, 245});
    drawThickLine(c, {800, 105}, {920, 225}, 3, Palette::Red);
    demo::caption(c, 780, 260, "REFLECT (45 DEG LINE)");
    demo::caption(c, 12, 312, "RED = MIRROR LINE.  REFLECTION = T(P) R(A) S(1,-1) R(-A) T(-P) AS ONE 3X3 MATRIX.", demo::Muted);
    if (demo::save(c, "17_shear_reflect") != EXIT_SUCCESS) return EXIT_FAILURE;

    // ---------- Animation: card flip (scale x = cos) then a shear wobble ----------
    return demo::animate("17_card_flip", 400, 260, 40, [](Canvas& f, double t) {
        f.clear(Palette::Cork);
        const PointF ctr{200, 130};
        const double a = t * 2 * PI;
        const double sx = std::cos(a);                                   // flip: 1 -> -1 -> 1
        const Mat3 m = translate(ctr.x, ctr.y) * shear(0.25 * std::sin(a), 0) * scale(sx, 1) *
                       translate(-145, -165);                            // card centre -> origin
        drawCard(f, m, 100, 110, sx >= 0 ? Palette::Paper : Color{200, 215, 245});
        drawText(f, 10, 240, "FLIP = SCALE(COS T, 1) + SHEAR", Palette::Black);
    });
}
