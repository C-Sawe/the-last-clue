// Crime scene — Alvin
// The study where the crime happened, built only from our own primitives:
// rectangles/polygons (scanline fill), circles, ellipses and Bresenham lines.
// Yellow outlines mark objects the player can interact with.
#include <cmath>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
void rect(Canvas& c, double x, double y, double w, double h, Color fill, Color edge) {
    const Polygon r = makeRect(x, y, w, h);
    scanlineFill(c, r, fill);
    drawPolygonOutline(c, r, edge);
}

void fillOnly(Canvas& c, double x, double y, double w, double h, Color fill) {
    scanlineFill(c, makeRect(x, y, w, h), fill);
}

// Two-pixel yellow outline around an interactive object
void highlight(Canvas& c, double x, double y, double w, double h) {
    drawPolygonOutline(c, makeRect(x - 4, y - 4, w + 8, h + 8), SuspectColour::Highlight);
    drawPolygonOutline(c, makeRect(x - 5, y - 5, w + 10, h + 10), SuspectColour::Highlight);
}

void wall(Canvas& c) {
    fillOnly(c, 0, 0, 800, 330, Color{70, 78, 96});
    for (int x = 20; x < 800; x += 40) drawLineBresenham(c, {x, 0}, {x, 320}, Color{78, 87, 106});  // wallpaper
    fillOnly(c, 0, 320, 800, 12, Color{60, 40, 25});                                                // skirting
}

void floor(Canvas& c) {
    fillOnly(c, 0, 332, 800, 168, Color{112, 74, 44});
    for (int row = 0, y = 332; y < 500; y += 28, ++row) {
        drawLineBresenham(c, {0, y}, {799, y}, Color{86, 56, 33});
        for (int x = (row % 2) * 70; x < 800; x += 140) drawLineBresenham(c, {x, y}, {x, y + 28}, Color{86, 56, 33});
    }
}

void window(Canvas& c) {
    rect(c, 70, 45, 180, 165, Color{58, 38, 24}, Palette::Black);      // frame
    fillOnly(c, 82, 57, 156, 141, Color{22, 30, 58});                  // night sky
    fillCircle(c, {200, 95}, 18, Color{235, 232, 200});                 // moon
    fillCircle(c, {208, 89}, 15, Color{22, 30, 58});                    // crescent cut-out
    const Point stars[] = {{100, 80}, {130, 120}, {160, 70}, {115, 160}, {220, 150}, {180, 175}};
    for (auto s : stars) { c.setPixel(s.x, s.y, Palette::White); c.setPixel(s.x + 1, s.y, Palette::White); }
    drawThickLine(c, {160, 57}, {160, 197}, 5, Color{58, 38, 24});      // cross bars
    drawThickLine(c, {82, 127}, {238, 127}, 5, Color{58, 38, 24});
    // curtains (polygons)
    const Polygon left{{{55, 35}, {95, 35}, {80, 120}, {92, 225}, {55, 225}}};
    const Polygon right{{{225, 35}, {265, 35}, {265, 225}, {228, 225}, {240, 120}}};
    scanlineFill(c, left, Color{120, 30, 40});
    scanlineFill(c, right, Color{120, 30, 40});
    drawPolygonOutline(c, left, Color{70, 15, 20});
    drawPolygonOutline(c, right, Color{70, 15, 20});
    fillOnly(c, 45, 30, 230, 8, Color{40, 30, 20});                    // curtain rod
}

void painting(Canvas& c) {
    rect(c, 300, 60, 100, 80, Color{190, 150, 60}, Palette::Black);
    fillOnly(c, 308, 68, 84, 64, Color{120, 160, 190});
    scanlineFill(c, Polygon{{{308, 132}, {340, 92}, {365, 132}}}, Color{80, 100, 80});
    scanlineFill(c, Polygon{{{345, 132}, {372, 100}, {392, 132}}}, Color{60, 85, 65});
    fillCircle(c, {378, 82}, 6, Palette::Yellow);
}

void clock(Canvas& c) {
    const Point ctr{470, 100};
    fillCircle(c, ctr, 32, Color{60, 40, 25});
    fillCircle(c, ctr, 26, Color{240, 235, 215});
    for (int i = 0; i < 12; ++i) {
        const double a = i * 3.14159265 / 6;
        drawLineBresenham(c, {ctr.x + static_cast<int>(20 * std::cos(a)), ctr.y + static_cast<int>(20 * std::sin(a))},
                             {ctr.x + static_cast<int>(24 * std::cos(a)), ctr.y + static_cast<int>(24 * std::sin(a))}, Palette::Black);
    }
    // Hands stopped at 9:00 — a clue (the note says "9PM")
    drawThickLine(c, ctr, {ctr.x, ctr.y - 18}, 2, Palette::Black);
    drawThickLine(c, ctr, {ctr.x - 13, ctr.y}, 3, Palette::Black);
    fillCircle(c, ctr, 3, Palette::Red);
}

void door(Canvas& c) {
    rect(c, 630, 110, 112, 222, Color{60, 40, 25}, Palette::Black);    // frame
    rect(c, 640, 120, 92, 212, Color{96, 62, 36}, Palette::Black);     // door leaf
    drawPolygonOutline(c, makeRect(652, 135, 68, 70), Color{70, 45, 26});
    drawPolygonOutline(c, makeRect(652, 225, 68, 90), Color{70, 45, 26});
    fillCircle(c, {720, 230}, 6, Color{200, 170, 70});
    highlight(c, 711, 221, 18, 18);
}

void cabinet(Canvas& c) {
    rect(c, 535, 175, 80, 157, Color{88, 98, 86}, Palette::Black);
    for (int i = 0; i < 3; ++i) {
        const double y = 185 + i * 48;
        rect(c, 543, y, 64, 40, Color{104, 116, 102}, Color{60, 66, 58});
        drawThickLine(c, {565, static_cast<int>(y) + 20}, {585, static_cast<int>(y) + 20}, 3, Color{200, 190, 150});
    }
}

void desk(Canvas& c) {
    const Polygon top{{{265, 248}, {525, 248}, {532, 262}, {258, 262}}};
    scanlineFill(c, top, Color{128, 84, 48});
    drawPolygonOutline(c, top, Palette::Black);
    rect(c, 268, 262, 14, 80, Color{100, 64, 36}, Palette::Black);      // legs
    rect(c, 508, 262, 14, 80, Color{100, 64, 36}, Palette::Black);
    rect(c, 400, 262, 108, 38, Color{112, 72, 40}, Palette::Black);     // drawer
    fillCircle(c, {454, 281}, 4, Color{200, 170, 70});
    highlight(c, 400, 262, 108, 38);

    // Lamp: ellipse base, pole, trapezoid shade, glow
    fillEllipse(c, {300, 246}, 18, 4, Color{50, 50, 50});
    drawThickLine(c, {300, 246}, {300, 200}, 3, Color{50, 50, 50});
    const Polygon shade{{{285, 180}, {315, 180}, {328, 205}, {272, 205}}};
    scanlineFill(c, shade, Color{40, 110, 70});
    drawPolygonOutline(c, shade, Palette::Black);
    fillEllipse(c, {300, 208}, 8, 4, Color{255, 240, 170});

    // Papers on the desk — rotated with our transform module
    const Polygon paper = makeRect(360, 236, 50, 12);
    const Polygon rotated = apply(rotateAbout(-0.12, {385, 242}), paper);
    scanlineFill(c, rotated, Palette::Paper);
    drawPolygonOutline(c, rotated, Color{150, 140, 110});
    drawPolygonOutline(c, apply(rotateAbout(-0.12, {385, 242}), makeRect(354, 230, 62, 24)), SuspectColour::Highlight);

    // Overturned cup
    fillEllipse(c, {480, 244}, 10, 4, Palette::White);
    fillEllipse(c, {495, 246}, 14, 2, Color{90, 50, 30});                // spilled coffee
}

void safe(Canvas& c) {
    rect(c, 35, 235, 115, 110, Color{55, 58, 64}, Palette::Black);
    rect(c, 45, 245, 95, 90, Color{72, 76, 84}, Color{30, 30, 30});
    fillCircle(c, {92, 288}, 18, Color{40, 40, 44});
    fillCircle(c, {92, 288}, 13, Color{150, 150, 155});
    for (int i = 0; i < 12; ++i) {
        const double a = i * 3.14159265 / 6;
        drawLineBresenham(c, {92 + static_cast<int>(10 * std::cos(a)), 288 + static_cast<int>(10 * std::sin(a))},
                             {92 + static_cast<int>(13 * std::cos(a)), 288 + static_cast<int>(13 * std::sin(a))}, Palette::Black);
    }
    drawThickLine(c, {120, 275}, {120, 302}, 4, Color{180, 180, 185});    // handle
    for (int cx : {44, 141}) for (int cy : {244, 336}) fillCircle(c, {cx, cy}, 2, Color{170, 170, 175});  // bolts
    highlight(c, 35, 235, 115, 110);
}

void rug(Canvas& c) {
    fillEllipse(c, {400, 415}, 190, 50, Color{120, 30, 42});
    fillEllipse(c, {400, 415}, 165, 38, Color{176, 128, 64});
    fillEllipse(c, {400, 415}, 135, 28, Color{120, 30, 42});
    drawEllipseMidpoint(c, {400, 415}, 100, 18, Color{176, 128, 64});
}

void chalkOutline(Canvas& c) {
    const Color chalk{235, 235, 235};
    drawCircleMidpoint(c, {580, 395}, 14, chalk);
    const Polygon body{{{592, 404}, {640, 390}, {700, 398}, {745, 380}, {752, 392}, {705, 412},
                        {742, 432}, {735, 444}, {690, 425}, {640, 430}, {600, 440}, {596, 428}, {625, 418}}};
    drawPolygonOutline(c, body, chalk);
}

void marker(Canvas& c, double x, double y, const char* label) {
    const Polygon tri{{{x, y - 18}, {x + 13, y + 6}, {x - 13, y + 6}}};
    scanlineFill(c, tri, Palette::Yellow);
    drawPolygonOutline(c, tri, Palette::Black);
    drawText(c, static_cast<int>(x) - 2, static_cast<int>(y) - 5, label, Palette::Black);
}
}  // namespace

void renderCrimeScene(Canvas& c) {
    wall(c);
    floor(c);
    window(c);
    painting(c);
    clock(c);
    door(c);
    cabinet(c);
    rug(c);
    desk(c);
    safe(c);
    chalkOutline(c);
    marker(c, 520, 470, "1");
    marker(c, 190, 380, "2");
    marker(c, 610, 360, "3");
    drawText(c, 10, 486, "YELLOW OUTLINE = INTERACTIVE OBJECT", Color{255, 230, 150});
}
