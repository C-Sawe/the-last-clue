// Security panel and terminal art — Alvin (final build, issue #60)
#include "security_panel.h"

#include <cmath>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "scenes.h"

namespace {
const Color WireRed{210, 40, 40}, WireGreen{40, 170, 70}, WireBlue{50, 90, 220};   // = ClueScript colours

Point px(PointF p) { return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}; }
}  // namespace

// Terminals in the order red, green, blue from the top; the sockets are wired
// in a different order, so a straight blue wire would cross the other two.
const std::vector<WirePort>& wireTerminals() {
    static const std::vector<WirePort> t = {
        {{45, 75}, WireRed, "RED"}, {{45, 150}, WireGreen, "GREEN"}, {{45, 225}, WireBlue, "BLUE"}};
    return t;
}

const std::vector<WirePort>& wireSockets() {
    static const std::vector<WirePort> s = {
        {{255, 75}, WireBlue, "BLUE"}, {{255, 150}, WireRed, "RED"}, {{255, 225}, WireGreen, "GREEN"}};
    return s;
}

void drawWiringBoard(Canvas& c, const Mat3& M, bool powered) {
    const double k = std::hypot(M.m[0][0], M.m[1][0]);              // scale of the mapping
    // Back plate: brushed metal (horizontal lines), screws in the corners.
    const Polygon plate = apply(M, makeRect(0, 0, 300, 300));
    scanlineFill(c, plate, Color{62, 66, 74});
    for (int y = 4; y < 300; y += 6) {
        const PointF a = apply(M, PointF{0, static_cast<double>(y)}), b = apply(M, PointF{299, static_cast<double>(y)});
        drawLineBresenham(c, px(a), px(b), Color{68, 72, 80});
    }
    for (PointF s : {PointF{10, 10}, PointF{290, 10}, PointF{10, 290}, PointF{290, 290}}) {
        const Point p = px(apply(M, s));
        fillCircle(c, p, static_cast<int>(4 * k), Color{150, 152, 160});
        drawLineBresenham(c, {p.x - 2, p.y}, {p.x + 2, p.y}, Color{70, 70, 76});
    }
    // Terminal block (left) and socket block (right)
    scanlineFill(c, apply(M, makeRect(22, 45, 46, 210)), Color{40, 42, 48});
    scanlineFill(c, apply(M, makeRect(232, 45, 46, 210)), Color{40, 42, 48});
    for (const auto& t : wireTerminals()) {                         // screw terminals
        const Point p = px(apply(M, t.pos));
        fillCircle(c, p, static_cast<int>(10 * k), t.colour);
        fillCircle(c, p, static_cast<int>(4 * k), Color{210, 210, 215});
        drawText(c, p.x - static_cast<int>(24 * k), p.y + static_cast<int>(14 * k), t.name, Color{200, 200, 210});
    }
    for (const auto& s : wireSockets()) {                           // sockets: coloured ring, dark hole
        const Point p = px(apply(M, s.pos));
        fillCircle(c, p, static_cast<int>(PortRadius * k), s.colour);
        fillCircle(c, p, static_cast<int>((PortRadius - 4) * k), Color{14, 14, 18});
        drawText(c, p.x - static_cast<int>(18 * k), p.y + static_cast<int>(16 * k), s.name, Color{200, 200, 210});
    }
    // Status LED and label
    const Point led = px(apply(M, PointF{150, 22}));
    fillCircle(c, led, static_cast<int>(6 * k), powered ? Feedback::Correct : Feedback::Wrong);
    drawCircleMidpoint(c, led, static_cast<int>(6 * k), Palette::Black);
    const Point lbl = px(apply(M, PointF{150, 278}));
    const char* text = powered ? "DOOR LOCK: POWERED" : "DOOR LOCK: NO POWER";
    drawText(c, lbl.x - textWidth(text) / 2, lbl.y, text, powered ? Feedback::Correct : Color{200, 200, 210});
}

void drawMonitorBezel(Canvas& c, double x, double y, double w, double h, const char* label, bool on) {
    const double b = 10;                                            // bezel width
    scanlineFill(c, makeRect(x - b + 3, y - b + 4, w + 2 * b, h + 2 * b + 14), Color{14, 14, 18});   // shadow
    scanlineFill(c, makeRect(x - b, y - b, w + 2 * b, h + 2 * b + 14), Color{52, 54, 62});
    drawPolygonOutline(c, makeRect(x - b, y - b, w + 2 * b, h + 2 * b + 14), Color{24, 24, 30});
    drawPolygonOutline(c, makeRect(x - 1, y - 1, w + 2, h + 2), Color{24, 24, 30});
    drawText(c, static_cast<int>(x), static_cast<int>(y + h + 6), label, Color{150, 152, 166});
    const Point led{static_cast<int>(x + w - 6), static_cast<int>(y + h + 9)};
    fillCircle(c, led, 3, on ? Feedback::Correct : Color{90, 40, 40});
}
