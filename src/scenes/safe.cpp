// Safe dial — Sudheysi
// A combination dial with 100 ticks. Every tick, number and the grip handle is
// placed by ROTATING a point about the dial centre with our transform module,
// so turning the dial is just a change of one angle.
#include <algorithm>
#include <cmath>
#include <string>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "puzzle_anim.h"
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

// ---------------------------------------------------------------------------
// CP2 (issue #35): the safe door swings open after the combination.
//
// 1. Draw the interior (what the player wins: the key and a fingerprint card).
// 2. Render the closed door (the CP1 dial view) onto its own canvas.
// 3. Swing it about the left hinge with ONE matrix
//        M = shearAbout(0, 0.3 * open, hinge) * scaleAbout(1 - 0.85 * open, 1, hinge)
//    and draw it by mapping each screen pixel back with inverse(M), keeping only
//    pixels that land inside the door rectangle. doorOpen = 0 is exactly the CP1 dial.
// ---------------------------------------------------------------------------
void renderSafeOpening(Canvas& c, double dialAngle, double doorOpen) {
    Canvas plate(c.width(), c.height());
    renderSafeDial(plate, dialAngle);
    if (doorOpen <= 0) { c.blit(plate, 0, 0); return; }

    const double dx = 20, dy = 30, dw = c.width() - 40, dh = c.height() - 50;   // door rectangle
    // ---- interior ----
    c.clear(Color{40, 42, 48});
    scanlineFill(c, makeRect(dx, dy, dw, dh), Color{22, 22, 28});
    scanlineFill(c, makeRect(dx + 12, dy + 12, dw - 24, dh - 24), Color{32, 33, 40});
    drawThickLine(c, px({dx + 12, dy + dh * 0.55}), px({dx + dw - 12, dy + dh * 0.55}), 4, Color{70, 72, 80});  // shelf
    // key on the shelf: bow (circle) + shaft + teeth
    const Point bow = px({dx + dw * 0.32, dy + dh * 0.55 - 20});
    drawCircleMidpoint(c, bow, 12, Color{220, 185, 70});
    drawCircleMidpoint(c, bow, 11, Color{220, 185, 70});
    drawThickLine(c, {bow.x + 12, bow.y}, {bow.x + 70, bow.y}, 4, Color{220, 185, 70});
    drawThickLine(c, {bow.x + 58, bow.y}, {bow.x + 58, bow.y + 10}, 3, Color{220, 185, 70});
    drawThickLine(c, {bow.x + 68, bow.y}, {bow.x + 68, bow.y + 8}, 3, Color{220, 185, 70});
    // fingerprint card below the shelf
    const Polygon card = makeRect(dx + dw * 0.5, dy + dh * 0.62, 110, 90);
    scanlineFill(c, card, Palette::White);
    drawPolygonOutline(c, card, Palette::Grey);
    const Point fp = px({dx + dw * 0.5 + 55, dy + dh * 0.62 + 45});
    for (int k = 0; k < 6; ++k) drawEllipseMidpoint(c, fp, 5 + k * 5, 8 + k * 6, Color{60, 60, 90});
    drawText(c, static_cast<int>(dx + dw) - textWidth("CODE ACCEPTED", 2) - 20, static_cast<int>(dy) + 20, "CODE ACCEPTED", Color{120, 230, 140}, 2);

    // ---- the door, swung about its hinge ----
    const PointF hinge{dx, dy + dh / 2};
    const Mat3 M = shearAbout(0, 0.3 * doorOpen, hinge) * scaleAbout(1 - 0.85 * doorOpen, 1, hinge);
    const Mat3 back = inverse(M);
    const Polygon doorShape = apply(M, makeRect(dx, dy, dw, dh));
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& v : doorShape.vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }
    for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); ++y)
        for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); ++x) {
            const PointF src = apply(back, PointF{x + 0.5, y + 0.5});
            if (src.x >= dx && src.x < dx + dw && src.y >= dy && src.y < dy + dh)
                c.setPixel(x, y, plate.getPixel(static_cast<int>(src.x), static_cast<int>(src.y)));
        }
    drawPolygonOutline(c, doorShape, Palette::Black);
    // door thickness: a strip along the free edge (a polygon between the edge and an offset copy)
    const PointF a = doorShape.vertices[1], b = doorShape.vertices[2];
    const double thick = 14 * doorOpen;
    const Polygon edge{{a, {a.x + thick, a.y + 4}, {b.x + thick, b.y - 4}, b}};
    scanlineFill(c, edge, Color{55, 58, 64});
    drawPolygonOutline(c, edge, Palette::Black);
}
