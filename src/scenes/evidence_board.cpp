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
#include "../animation/animation.h"
#include "../transform/transform2d.h"
#include "board_anim.h"
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
// sag proportional to the distance between the pins. `amount` < 1 draws only
// the first part of the curve (CP2: the string being pulled across).
void string(Canvas& c, PointF a, PointF b, Color col, double amount = 1.0) {
    const double dist = std::hypot(b.x - a.x, b.y - a.y);
    const PointF ctrl{(a.x + b.x) / 2, (a.y + b.y) / 2 + dist * 0.18};
    drawBezierPartial(c, {a, ctrl, b}, col, amount, 60);
    drawBezierPartial(c, {{a.x, a.y + 1}, {ctrl.x, ctrl.y + 1}, {b.x, b.y + 1}}, col, amount, 60);
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

void renderEvidenceBoard(Canvas& c) { renderEvidenceBoardAt(c, 1.0); }

// CP2 (issue #31): the same board, animated.
// Each item is a small drawing function taking an offset (dx, dy). While an item
// is "dropping in" the offset comes from an eased translation; at rest it is (0, 0).
void renderEvidenceBoardAt(Canvas& c, double progress) {
    using anim::Ease;
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

    struct Item {
        PointF pin;
        Color pinColour;
        void (*draw)(Canvas&, double, double);
    };
    const Item items[] = {
        {{450, 206}, Palette::Black, [](Canvas& k, double dx, double dy) {          // victim
             card(k, 395 + dx, 200 + dy, 110, 130, Color{250, 250, 246});
             scanlineFill(k, makeRect(403 + dx, 208 + dy, 94, 90), Color{70, 70, 80});
             fillEllipse(k, {450 + int(dx), 298 + int(dy)}, 36, 22, Color{30, 30, 34});
             scanlineFill(k, makeRect(396 + dx, 298 + dy, 108, 31), Color{250, 250, 246});
             fillEllipse(k, {450 + int(dx), 250 + int(dy)}, 18, 23, Color{120, 120, 125});
             drawText(k, 450 - textWidth("VICTIM") / 2 + int(dx), 310 + int(dy), "VICTIM", Color{150, 30, 30});
         }},
        {{115, 116}, SuspectColour::Butler, [](Canvas& k, double dx, double dy) {
             photo(k, 60 + dx, 110 + dy, SuspectColour::Butler, "THE BUTLER"); }},
        {{785, 116}, SuspectColour::Heiress, [](Canvas& k, double dx, double dy) {
             photo(k, 730 + dx, 110 + dy, SuspectColour::Heiress, "THE HEIRESS"); }},
        {{450, 406}, SuspectColour::Partner, [](Canvas& k, double dx, double dy) {
             photo(k, 395 + dx, 400 + dy, SuspectColour::Partner, "THE PARTNER"); }},
        {{145, 346}, Palette::Red, [](Canvas& k, double dx, double dy) {             // torn letter
             card(k, 70 + dx, 340 + dy, 150, 110, Palette::Paper);
             for (int y = 374; y < 440; y += 14)
                 drawLineBresenham(k, {80 + int(dx), y + int(dy)}, {205 + int(dx), y + int(dy)}, Color{110, 110, 150});
             drawText(k, 80 + int(dx), 358 + int(dy), "TORN LETTER", Color{150, 30, 30});
             std::vector<PointF> sig{{150, 425}, {162, 410}, {170, 430}, {182, 412}, {200, 428}};
             for (auto& p : sig) { p.x += dx; p.y += dy; }
             drawCatmullRom(k, sig, Color{30, 40, 110}, 10);
         }},
        {{760, 336}, Palette::Red, [](Canvas& k, double dx, double dy) {             // fingerprint
             card(k, 690 + dx, 330 + dy, 140, 130, Palette::White);
             for (int i = 0; i < 7; ++i)
                 drawEllipseMidpoint(k, {760 + int(dx), 410 + int(dy)}, 6 + i * 5, 10 + i * 6, Color{60, 60, 90});
             drawText(k, 700 + int(dx), 348 + int(dy), "FINGERPRINT", Color{40, 40, 40});
         }},
        {{260, 190}, Palette::Red, [](Canvas& k, double dx, double dy) {
             stickyNote(k, {260 + dx, 230 + dy}, -0.15, Color{250, 235, 120},
                        {{-40, -15}, {-25, -30}, {-10, -10}, {5, -28}, {25, -12}, {40, -25}});
             drawText(k, 225 + int(dx), 255 + int(dy), "9PM?", Color{30, 40, 110}, 2);
         }},
        {{640, 200}, Palette::Red, [](Canvas& k, double dx, double dy) {
             stickyNote(k, {640 + dx, 240 + dy}, 0.18, Color{170, 220, 240},
                        {{-40, 10}, {-25, -20}, {-5, 15}, {10, -15}, {30, 12}, {42, -18}});
         }},
    };
    const int n = sizeof(items) / sizeof(items[0]);

    // Title card is always there
    card(c, 300, 32, 300, 40, Palette::Paper);
    drawText(c, 318, 46, "CASE 47: THE LAST CLUE", Color{150, 30, 30}, 2);

    // Phase 1: items drop in from above, one after another (staggered), and
    // land with a small overshoot (OutBack easing applied to a translation).
    std::vector<double> landed(n);
    for (int i = 0; i < n; ++i) {
        const double start = 0.05 * i, end = start + 0.18;
        landed[i] = anim::phase(progress, start, end);
    }
    for (int i = 0; i < n; ++i) {
        if (landed[i] <= 0) continue;
        const double dy = anim::lerp(-600, 0, anim::ease(Ease::OutBack, landed[i]));
        items[i].draw(c, 0, dy);
    }

    // Phase 2: strings are pulled from pin to pin, one after another.
    struct Link { int a, b; Color col; };
    const Link links[] = {
        {0, 1, Palette::StringRed}, {0, 2, Palette::StringRed}, {0, 3, Palette::StringRed},
        {4, 1, Color{30, 60, 160}}, {6, 4, Color{30, 60, 160}},
        {5, 2, Color{30, 120, 60}}, {7, 5, Color{30, 120, 60}},
    };
    const int m = sizeof(links) / sizeof(links[0]);
    for (int j = 0; j < m; ++j) {
        const double start = 0.55 + 0.055 * j, end = start + 0.09;
        const double amount = anim::ease(Ease::InOutCubic, anim::phase(progress, start, end));
        if (amount > 0) string(c, items[links[j].a].pin, items[links[j].b].pin, links[j].col, amount);
    }

    // Pins go in last so they sit on top of the strings
    for (int i = 0; i < n; ++i) {
        if (landed[i] < 1) {
            if (landed[i] <= 0) continue;
            const double dy = anim::lerp(-600, 0, anim::ease(Ease::OutBack, landed[i]));
            pin(c, {items[i].pin.x, items[i].pin.y + dy}, items[i].pinColour);
        } else {
            pin(c, items[i].pin, items[i].pinColour);
        }
    }
}
