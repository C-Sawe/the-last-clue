// Safe dial — Sudheysi
// A combination dial with 100 ticks. Every tick, number and the grip handle is
// placed by ROTATING a point about the dial centre with our transform module,
// so turning the dial is just a change of one angle.
#include <cmath>
#include <string>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
Point px(PointF p) { return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}; }
const double PI = 3.14159265358979;
}  // namespace

void renderSafeDial(Canvas& c, double angle) {
    const PointF centre{c.width() / 2.0, c.height() / 2.0 + 10};
    const Point C = px(centre);

    // Safe door: steel plate with bolts in the corners
    c.clear(Color{40, 42, 48});
    const Polygon door = makeRect(20, 30, c.width() - 40, c.height() - 50);
    scanlineFill(c, door, Color{70, 74, 82});
    drawPolygonOutline(c, door, Palette::Black);
    for (double bx : {38.0, c.width() - 38.0})
        for (double by : {48.0, c.height() - 38.0}) {
            fillCircle(c, px({bx, by}), 7, Color{150, 152, 158});
            drawCircleMidpoint(c, px({bx, by}), 7, Palette::Black);
        }

    // Dial body: concentric filled circles
    fillCircle(c, C, 150, Color{30, 30, 34});
    fillCircle(c, C, 142, Color{185, 188, 194});
    drawCircleMidpoint(c, C, 142, Palette::Black);
    fillCircle(c, C, 92, Color{150, 153, 160});
    drawCircleMidpoint(c, C, 92, Palette::Black);

    // 100 ticks; every 5th is longer, every 10th is labelled.
    for (int i = 0; i < 100; ++i) {
        const double len = (i % 10 == 0) ? 22 : (i % 5 == 0) ? 15 : 9;
        const Mat3 r = rotateAbout(angle + i * 2 * PI / 100, centre);
        const PointF outer = apply(r, PointF{centre.x, centre.y - 140});
        const PointF inner = apply(r, PointF{centre.x, centre.y - 140 + len});
        if (i % 10 == 0) drawThickLine(c, px(inner), px(outer), 2, Palette::Black);
        else             drawLineBresenham(c, px(inner), px(outer), Palette::Black);
        if (i % 10 == 0) {
            const PointF label = apply(r, PointF{centre.x, centre.y - 104});
            const std::string s = std::to_string(i);
            drawText(c, static_cast<int>(label.x) - textWidth(s, 2) / 2, static_cast<int>(label.y) - 7, s, Palette::Black, 2);
        }
    }

    // Grip handle: a bar polygon rotated with the dial, plus a knurled hub
    const Polygon bar = makeRect(centre.x - 12, centre.y - 70, 24, 140);
    const Polygon barRot = apply(rotateAbout(angle, centre), bar);
    scanlineFill(c, barRot, Color{95, 98, 106});
    drawPolygonOutline(c, barRot, Palette::Black);
    fillCircle(c, C, 30, Color{110, 113, 120});
    for (int k = 0; k < 24; ++k) {
        const Mat3 r = rotateAbout(angle + k * 2 * PI / 24, centre);
        drawLineBresenham(c, px(apply(r, PointF{centre.x, centre.y - 30})),
                             px(apply(r, PointF{centre.x, centre.y - 22})), Color{60, 62, 68});
    }
    drawCircleMidpoint(c, C, 30, Palette::Black);
    fillCircle(c, C, 6, Color{60, 62, 68});

    // Fixed red pointer at 12 o'clock (does NOT rotate)
    const Polygon pointer{{{centre.x, centre.y - 152}, {centre.x - 11, centre.y - 172}, {centre.x + 11, centre.y - 172}}};
    scanlineFill(c, pointer, Palette::Red);
    drawPolygonOutline(c, pointer, Palette::Black);

    // Current number under the pointer
    int number = static_cast<int>(std::lround(-angle / (2 * PI) * 100)) % 100;
    if (number < 0) number += 100;
    const std::string label = "DIAL AT " + std::to_string(number);
    drawText(c, 34, c.height() - 34, label, Palette::White, 2);
}
