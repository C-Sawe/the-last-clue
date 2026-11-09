// Magnifying glass — Sudheysi (CP2, issue #37)
//
// The player moves the magnifier over the crime scene. Inside the lens the scene
// is shown scaled by `zoom` ABOUT THE FOCUS POINT:
//     Z = scaleAbout(zoom, zoom, focus)        (screen <- scene)
// Each lens pixel p shows the scene pixel inverse(Z) * p, so zooming never leaves holes.
// Clipping keeps everything tidy at the screen edges:
//   - the lens rectangle is clipped to the screen with Sutherland–Hodgman,
//     and only pixels inside the clipped window are zoomed;
//   - the handle is a long line clipped to the screen with Cohen–Sutherland.
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
}  // namespace

void renderMagnifierAt(Canvas& c, PointF focus, double zoom) {
    Canvas scene(c.width(), c.height());
    renderCrimeScene(scene);
    c.blit(scene, 0, 0);

    // Lens window around the focus point, clipped to the screen
    const double lw = 220, lh = 150;
    const Polygon lens = makeRect(focus.x - lw / 2, focus.y - lh / 2, lw, lh);
    const ClipRect screen{0, 0, c.width() - 1.0, c.height() - 1.0};
    const Polygon visible = sutherlandHodgmanClip(lens, screen);
    if (visible.vertices.empty()) return;
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& v : visible.vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }

    // Zoomed pixels, only inside the clipped lens window
    const Mat3 back = inverse(scaleAbout(zoom, zoom, focus));
    for (int y = static_cast<int>(std::ceil(minY)); y <= static_cast<int>(maxY); ++y)
        for (int x = static_cast<int>(std::ceil(minX)); x <= static_cast<int>(maxX); ++x) {
            const PointF s = apply(back, PointF{x + 0.5, y + 0.5});
            c.setPixel(x, y, scene.getPixel(static_cast<int>(s.x), static_cast<int>(s.y)));
        }

    // Frame and handle
    const Color brass{200, 160, 60}, wood{90, 58, 30};
    drawPolygonOutline(c, visible, Palette::Black);
    drawPolygonOutline(c, sutherlandHodgmanClip(makeRect(focus.x - lw / 2 - 3, focus.y - lh / 2 - 3, lw + 6, lh + 6), screen), brass);
    drawPolygonOutline(c, sutherlandHodgmanClip(makeRect(focus.x - lw / 2 - 4, focus.y - lh / 2 - 4, lw + 8, lh + 8), screen), brass);
    PointF h0{focus.x + lw / 2 + 4, focus.y + lh / 2 + 4}, h1{h0.x + 90, h0.y + 90};
    if (cohenSutherlandClip(h0, h1, screen)) drawThickLine(c, px(h0), px(h1), 9, wood);

    // Crosshair at the focus point + zoom label
    const Point f = px(focus);
    drawLineBresenham(c, {f.x - 6, f.y}, {f.x + 6, f.y}, Palette::Red);
    drawLineBresenham(c, {f.x, f.y - 6}, {f.x, f.y + 6}, Palette::Red);
    const int tenths = static_cast<int>(std::lround(zoom * 10));
    drawText(c, static_cast<int>(std::max(minX, 0.0)) + 6, static_cast<int>(std::max(minY, 0.0)) + 6,
             "X" + std::to_string(tenths / 10) + "." + std::to_string(tenths % 10), Palette::White);
}
