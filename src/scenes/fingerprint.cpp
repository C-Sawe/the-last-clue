// Fingerprint art — Alvin (final build, issue #49)
#include "fingerprint.h"

#include <algorithm>
#include <cmath>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
Point px(double x, double y) { return {static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y))}; }

// One ridge = a midpoint ellipse drawn twice (1 px apart) so it reads at small sizes.
void ridge(Canvas& c, PointF centre, double rx, double ry, Color ink) {
    drawEllipseMidpoint(c, px(centre.x, centre.y), static_cast<int>(rx), static_cast<int>(ry), ink);
    drawEllipseMidpoint(c, px(centre.x, centre.y), static_cast<int>(rx) + 1, static_cast<int>(ry) + 1, ink);
}
}  // namespace

const PrintSpec& suspectPrint(int suspect) {
    static const PrintSpec prints[3] = {
        // The butler: a plain arch
        {PrintSpec::Pattern::Arch, 18, 7.5, 2.1, {0, 0}, 0, false, 0},
        // The heiress: a tight whorl, slightly off-centre
        {PrintSpec::Pattern::Whorl, 9, 6.5, 1.3, {-6, -4}, 0, false, 0},
        // The partner: a loop sliding to the right, with an old scar
        {PrintSpec::Pattern::Loop, 8, 7, 1.15, {4, -10}, 16, true, -0.55},
    };
    return prints[std::max(0, std::min(2, suspect))];
}

const PrintSpec& foundPrint() { return suspectPrint(2); }

const char* printPatternName(const PrintSpec& s) {
    switch (s.pattern) {
        case PrintSpec::Pattern::Whorl: return "WHORL";
        case PrintSpec::Pattern::Loop: return "LOOP";
        case PrintSpec::Pattern::Arch: return "ARCH";
    }
    return "?";
}

Canvas renderPrint(const PrintSpec& s, int size, Color ink, Color paper) {
    Canvas c(size, size, paper);
    const double k = size / 160.0;                         // specs are written for a 160 px print
    const PointF mid{size / 2.0, size / 2.0};
    const PointF core{mid.x + s.core.x * k, mid.y + s.core.y * k};

    switch (s.pattern) {
        case PrintSpec::Pattern::Whorl:
            for (int i = 0; i < s.ridges; ++i) {
                const double rx = (4 + i * s.gap) * k;
                ridge(c, core, rx, rx * s.stretch, ink);
            }
            break;
        case PrintSpec::Pattern::Loop: {
            // Top halves of the ellipses: draw them whole, then wipe everything
            // below the core and replace it with the two "legs" of each ridge.
            for (int i = 0; i < s.ridges; ++i) {
                const double rx = (4 + i * s.gap) * k;
                ridge(c, core, rx, rx * s.stretch, ink);
            }
            scanlineFill(c, makeRect(0, core.y + 1, size, size), paper);
            for (int i = 0; i < s.ridges; ++i) {
                const double rx = (4 + i * s.gap) * k;
                for (int side : {-1, 1}) {
                    const PointF top{core.x + side * rx, core.y};
                    const PointF bottom{top.x + s.slant * k * (1 + 0.15 * i), size + 0.0};
                    drawLineBresenham(c, px(top.x, top.y), px(bottom.x, bottom.y), ink);
                    drawLineBresenham(c, px(top.x + side, top.y), px(bottom.x + side, bottom.y), ink);
                }
            }
            break;
        }
        case PrintSpec::Pattern::Arch: {
            // Centre far below the finger: only the flat tops of the ellipses show.
            // Ridge i peaks at height top + i*gap, so the arches stack down the print.
            const double cy = core.y + 150 * k;
            for (int i = 0; i < s.ridges; ++i) {
                const double ry = cy - (core.y - 66 * k + i * s.gap * k);
                ridge(c, {core.x, cy}, ry / s.stretch, ry, ink);
            }
            break;
        }
    }
    if (s.scar) {                                          // a scar breaks the ridges
        const double len = 46 * k;
        const PointF a{mid.x - len * std::cos(s.scarAngle), mid.y + 18 * k - len * std::sin(s.scarAngle)};
        const PointF b{mid.x + len * std::cos(s.scarAngle), mid.y + 18 * k + len * std::sin(s.scarAngle)};
        drawThickLine(c, px(a.x, a.y), px(b.x, b.y), std::max(3, static_cast<int>(5 * k)), paper);
    }
    // The finger: erase ridges outside the outline ellipse (point-in-ellipse test).
    const double fx = size * 0.40, fy = size * 0.48;
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            const double dx = (x + 0.5 - mid.x) / fx, dy = (y + 0.5 - mid.y) / fy;
            if (dx * dx + dy * dy > 1) c.setPixel(x, y, paper);
        }
    return c;
}

void drawPrintRotated(Canvas& c, const Canvas& print, PointF centre, double angle, Color paper, bool transparent) {
    // Forward map: print pixel -> screen = T(centre) * R(angle) * T(-size/2).
    // We walk the SCREEN pixels of the bounding box and map each one BACK with the
    // inverse matrix, so every screen pixel gets exactly one source pixel (no holes).
    const double hw = print.width() / 2.0, hh = print.height() / 2.0;
    const Mat3 M = translate(centre.x, centre.y) * rotate(angle) * translate(-hw, -hh);
    const Mat3 back = inverse(M);
    const double r = std::hypot(hw, hh) + 1;
    for (int y = static_cast<int>(centre.y - r); y <= static_cast<int>(centre.y + r); ++y)
        for (int x = static_cast<int>(centre.x - r); x <= static_cast<int>(centre.x + r); ++x) {
            if (!c.inBounds(x, y)) continue;
            const PointF s = apply(back, PointF{x + 0.5, y + 0.5});
            if (s.x < 0 || s.y < 0 || s.x >= print.width() || s.y >= print.height()) continue;
            const Color col = print.getPixel(static_cast<int>(s.x), static_cast<int>(s.y));
            if (transparent && col == paper) continue;
            c.setPixel(x, y, col);
        }
}

void drawPrintCard(Canvas& c, double x, double y, int suspect, bool selected, bool hovered) {
    static const Color colours[3] = {SuspectColour::Butler, SuspectColour::Heiress, SuspectColour::Partner};
    static const char* names[3] = {"THE BUTLER", "THE HEIRESS", "THE PARTNER"};
    const double lift = selected ? -10 : (hovered ? -4 : 0);           // selected card is raised
    const double w = PrintCardW, h = PrintCardH, top = y + lift;
    scanlineFill(c, makeRect(x + 5, y + 6, w, h), Color{18, 18, 24});   // shadow stays on the table
    scanlineFill(c, makeRect(x, top, w, h), Color{248, 246, 238});
    drawPolygonOutline(c, makeRect(x, top, w, h), Color{120, 110, 95});
    scanlineFill(c, makeRect(x, top, w, 26), colours[suspect]);
    drawText(c, static_cast<int>(x + w / 2) - textWidth(names[suspect]) / 2, static_cast<int>(top) + 9, names[suspect], Palette::White);

    static Canvas prints[3] = {renderPrint(suspectPrint(0), 120, Color{50, 50, 70}, Color{248, 246, 238}),
                               renderPrint(suspectPrint(1), 120, Color{50, 50, 70}, Color{248, 246, 238}),
                               renderPrint(suspectPrint(2), 120, Color{50, 50, 70}, Color{248, 246, 238})};
    c.blit(prints[suspect], static_cast<int>(x + 15), static_cast<int>(top + 32));
    const char* pattern = printPatternName(suspectPrint(suspect));
    drawText(c, static_cast<int>(x + w / 2) - textWidth(pattern) / 2, static_cast<int>(top + h - 16), pattern, Color{90, 90, 100});
    if (selected || hovered) {
        const Color edge = selected ? SuspectColour::Highlight : Color{200, 190, 150};
        drawPolygonOutline(c, makeRect(x - 3, top - 3, w + 6, h + 6), edge);
        drawPolygonOutline(c, makeRect(x - 4, top - 4, w + 8, h + 8), edge);
    }
}
