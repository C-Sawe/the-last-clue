// Evidence board — Latiffa
// Cork board where the player connects clues to suspects.
//   cards        -> rectangles / rotated polygons (scanline fill)
//   pins         -> filled midpoint circles with a highlight
//   strings      -> quadratic Bézier curves that sag under "gravity"
//   handwriting  -> Catmull-Rom splines, rotated with the note they are on
//   fingerprint  -> concentric midpoint ellipses
#include <cmath>
#include <string>
#include <vector>

#include "../core/font.h"
#include "../curves/curves.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
Point px(PointF p) { return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}; }

void card(Canvas& c, double x, double y, double w, double h, Color fill) {
    scanlineFill(c, makeRect(x + 4, y + 5, w, h), Color{110, 78, 45});    // shadow
    scanlineFill(c, makeRect(x, y, w, h), fill);
    drawPolygonOutline(c, makeRect(x, y, w, h), Color{120, 110, 95});
}

void pin(Canvas& c, PointF p, Color col) {
    fillCircle(c, px(p), 7, col);
    drawCircleMidpoint(c, px(p), 7, Color{40, 20, 20});
    fillCircle(c, {px(p).x - 2, px(p).y - 2}, 2, Palette::White);
}

// A string hangs between two pins: control point = midpoint pushed down by a
// sag proportional to the distance between the pins.
void string(Canvas& c, PointF a, PointF b, Color col) {
    const double dist = std::hypot(b.x - a.x, b.y - a.y);
    const PointF ctrl{(a.x + b.x) / 2, (a.y + b.y) / 2 + dist * 0.18};
    drawBezier(c, {a, ctrl, b}, col, 60);
    drawBezier(c, {{a.x, a.y + 1}, {ctrl.x, ctrl.y + 1}, {b.x, b.y + 1}}, col, 60);
}

// Mug-shot polaroid showing a suspect in their colour
void photo(Canvas& c, double x, double y, Color coat, const std::string& name) {
    card(c, x, y, 110, 130, Color{250, 250, 246});
    scanlineFill(c, makeRect(x + 8, y + 8, 94, 90), Color{190, 196, 206});
    fillEllipse(c, {static_cast<int>(x) + 55, static_cast<int>(y) + 98}, 36, 22, coat);   // shoulders
    scanlineFill(c, makeRect(x + 1, y + 98, 108, 31), Color{250, 250, 246});     // crop shoulders to the photo
    fillEllipse(c, {static_cast<int>(x) + 55, static_cast<int>(y) + 50}, 18, 23, Color{205, 160, 120});
    drawText(c, static_cast<int>(x) + 55 - textWidth(name) / 2, static_cast<int>(y) + 110, name, Color{40, 40, 40});
}

// Sticky note rotated by `angle`; its scribble is a Catmull-Rom spline whose
// points are rotated with the same matrix, so the writing turns with the paper.
void stickyNote(Canvas& c, PointF centre, double angle, Color col, const std::vector<PointF>& scribble) {
    const Mat3 M = rotateAbout(angle, centre);
    const Polygon note = apply(M, makeRect(centre.x - 55, centre.y - 45, 110, 90));
    scanlineFill(c, apply(translate(4, 5), note), Color{110, 78, 45});
    scanlineFill(c, note, col);
    drawPolygonOutline(c, note, Color{150, 140, 80});
    std::vector<PointF> pts;
    for (const auto& p : scribble) pts.push_back(apply(M, PointF{centre.x + p.x, centre.y + p.y}));
    drawCatmullRom(c, pts, Color{30, 40, 110}, 12);
}
}  // namespace

void renderEvidenceBoard(Canvas& c) {
    // Frame + cork texture (deterministic speckles)
    c.clear(Color{92, 58, 32});
    scanlineFill(c, makeRect(18, 18, c.width() - 36, c.height() - 36), Palette::Cork);
    unsigned s = 7;
    for (int i = 0; i < 6000; ++i) {
        s = s * 1103515245u + 12345u;
        const int x = 18 + static_cast<int>((s >> 8) % (c.width() - 36));
        s = s * 1103515245u + 12345u;
        const int y = 18 + static_cast<int>((s >> 8) % (c.height() - 36));
        c.setPixel(x, y, (i % 3 == 0) ? Color{160, 115, 70} : Color{210, 170, 120});
    }
    drawThickLine(c, {18, 18}, {c.width() - 19, 18}, 3, Color{60, 36, 18});
    drawThickLine(c, {18, c.height() - 19}, {c.width() - 19, c.height() - 19}, 3, Color{60, 36, 18});

    // Title card
    card(c, 300, 32, 300, 40, Palette::Paper);
    drawText(c, 318, 46, "CASE 47: THE LAST CLUE", Color{150, 30, 30}, 2);

    // Victim in the centre
    card(c, 395, 200, 110, 130, Color{250, 250, 246});
    scanlineFill(c, makeRect(403, 208, 94, 90), Color{70, 70, 80});
    fillEllipse(c, {450, 298}, 36, 22, Color{30, 30, 34});
    scanlineFill(c, makeRect(396, 298, 108, 31), Color{250, 250, 246});
    fillEllipse(c, {450, 250}, 18, 23, Color{120, 120, 125});
    drawText(c, 450 - textWidth("VICTIM") / 2, 310, "VICTIM", Color{150, 30, 30});

    // Suspects across the top/sides, each pinned in their colour
    photo(c, 60, 110, SuspectColour::Butler, "THE BUTLER");
    photo(c, 730, 110, SuspectColour::Heiress, "THE HEIRESS");
    photo(c, 395, 400, SuspectColour::Partner, "THE PARTNER");

    // Evidence items
    card(c, 70, 340, 150, 110, Palette::Paper);                            // torn letter
    for (int y = 374; y < 440; y += 14) drawLineBresenham(c, {80, y}, {205, y}, Color{110, 110, 150});
    drawText(c, 80, 358, "TORN LETTER", Color{150, 30, 30});
    drawCatmullRom(c, {{150, 425}, {162, 410}, {170, 430}, {182, 412}, {200, 428}}, Color{30, 40, 110}, 10);

    card(c, 690, 330, 140, 130, Palette::White);                           // fingerprint card
    for (int k = 0; k < 7; ++k) drawEllipseMidpoint(c, {760, 410}, 6 + k * 5, 10 + k * 6, Color{60, 60, 90});
    drawText(c, 700, 348, "FINGERPRINT", Color{40, 40, 40});

    stickyNote(c, {260, 230}, -0.15, Color{250, 235, 120},
               {{-40, -15}, {-25, -30}, {-10, -10}, {5, -28}, {25, -12}, {40, -25}});
    drawText(c, 225, 255, "9PM?", Color{30, 40, 110}, 2);
    stickyNote(c, {640, 240}, 0.18, Color{170, 220, 240},
               {{-40, 10}, {-25, -20}, {-5, 15}, {10, -15}, {30, 12}, {42, -18}});

    // Pins (top-centre of each item)
    const PointF pVictim{450, 206}, pButler{115, 116}, pHeiress{785, 116}, pPartner{450, 406},
                 pLetter{145, 346}, pPrint{760, 336}, pNote1{260, 190}, pNote2{640, 200};

    // Connections drawn by the player
    string(c, pVictim, pButler, Palette::StringRed);
    string(c, pVictim, pHeiress, Palette::StringRed);
    string(c, pVictim, pPartner, Palette::StringRed);
    string(c, pLetter, pButler, Color{30, 60, 160});
    string(c, pNote1, pLetter, Color{30, 60, 160});
    string(c, pPrint, pHeiress, Color{30, 120, 60});
    string(c, pNote2, pPrint, Color{30, 120, 60});

    pin(c, pVictim, Palette::Black);
    pin(c, pButler, SuspectColour::Butler);
    pin(c, pHeiress, SuspectColour::Heiress);
    pin(c, pPartner, SuspectColour::Partner);
    pin(c, pLetter, Palette::Red);
    pin(c, pPrint, Palette::Red);
    pin(c, pNote1, Palette::Red);
    pin(c, pNote2, Palette::Red);
}
