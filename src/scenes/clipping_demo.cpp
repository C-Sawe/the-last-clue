// Clipping in the game — Sudheysi
// Left : furniture and evidence strings that stick out of the Crime Scene
//        viewport are clipped to it (Sutherland–Hodgman for polygons,
//        Cohen–Sutherland for lines). Grey = the part that was thrown away.
// Right: the magnifying glass. A zoomed copy of the scene is shown only inside
//        the magnifier window — every pixel outside the window is clipped.
#include <cmath>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "../transform/transform2d.h"
#include "scenes.h"

namespace {
Point px(PointF p) { return {static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y))}; }

void clippedPolygon(Canvas& c, const Polygon& p, const ClipRect& view, Color fill) {
    drawPolygonOutline(c, p, Color{150, 150, 158});                      // discarded outline
    const Polygon kept = sutherlandHodgmanClip(p, view);
    scanlineFill(c, kept, fill);
    drawPolygonOutline(c, kept, Palette::Black);
}

void clippedString(Canvas& c, PointF a, PointF b, const ClipRect& view) {
    drawLineBresenham(c, px(a), px(b), Color{150, 150, 158});
    if (cohenSutherlandClip(a, b, view)) drawThickLine(c, px(a), px(b), 3, Palette::StringRed);
}
}  // namespace

void renderClippingDemo(Canvas& c) {
    c.clear(Color{236, 236, 240});

    // ---------- Left: viewport clipping ----------
    const ClipRect view{60, 90, 440, 400};
    scanlineFill(c, makeRect(view.xmin, view.ymin, view.xmax - view.xmin, view.ymax - view.ymin), Color{70, 78, 96});
    scanlineFill(c, makeRect(view.xmin, 320, view.xmax - view.xmin, view.ymax - 320), Color{112, 74, 44});

    clippedPolygon(c, makeRect(20, 230, 120, 110), view, Color{55, 58, 64});                       // safe, sticking out left
    clippedPolygon(c, Polygon{{{200, 260}, {500, 260}, {508, 278}, {192, 278}}}, view, Color{128, 84, 48});  // desk top past the right edge
    clippedPolygon(c, apply(rotateAbout(0.5, {420, 120}), makeRect(370, 60, 110, 90)), view, Color{190, 150, 60});  // tilted painting
    clippedPolygon(c, makeStar({250, 395}, 45, 19, 5), view, Color{200, 200, 70});                // evidence marker off the bottom

    clippedString(c, {10, 140}, {300, 200}, view);
    clippedString(c, {150, 50}, {380, 380}, view);
    clippedString(c, {300, 330}, {500, 120}, view);
    drawPolygonOutline(c, makeRect(view.xmin, view.ymin, view.xmax - view.xmin, view.ymax - view.ymin), Palette::Black);
    drawText(c, 60, 60, "VIEWPORT CLIPPING", Palette::Black, 2);
    drawText(c, 60, 420, "GREY = CLIPPED AWAY", Color{110, 110, 120});

    // ---------- Right: magnifier ----------
    Canvas scene(800, 500, Palette::Black);
    renderCrimeScene(scene);

    // Small overview of the room (scaled 0.5 by sampling every 2nd pixel)
    const int sx = 520, sy = 90;
    for (int y = 0; y < 250; ++y)
        for (int x = 0; x < 400; ++x) c.setPixel(sx + x, sy + y, scene.getPixel(x * 2, y * 2));
    drawText(c, 520, 60, "MAGNIFIER (ZOOM X3)", Palette::Black, 2);

    // The magnifier looks at the safe dial: region centred on scene point (92, 288)
    const PointF focus{92, 288};
    const double zoom = 3.0;
    const ClipRect lens{700, 250, 900, 400};                              // magnifier window on screen
    const PointF lensCentre{(lens.xmin + lens.xmax) / 2, (lens.ymin + lens.ymax) / 2};
    // screen -> scene: undo translate-to-lens, undo zoom, move to focus
    const Mat3 toScene = translate(focus.x, focus.y) * scale(1 / zoom, 1 / zoom) * translate(-lensCentre.x, -lensCentre.y);
    for (int y = static_cast<int>(lens.ymin); y <= static_cast<int>(lens.ymax); ++y)
        for (int x = static_cast<int>(lens.xmin); x <= static_cast<int>(lens.xmax); ++x) {
            const PointF s = apply(toScene, PointF{static_cast<double>(x), static_cast<double>(y)});
            c.setPixel(x, y, scene.getPixel(static_cast<int>(s.x), static_cast<int>(s.y)));
        }
    // Frame + the region it is looking at on the overview
    const Polygon lensFrame = makeRect(lens.xmin, lens.ymin, lens.xmax - lens.xmin, lens.ymax - lens.ymin);
    drawPolygonOutline(c, lensFrame, Palette::Black);
    drawPolygonOutline(c, makeRect(lens.xmin - 2, lens.ymin - 2, lens.xmax - lens.xmin + 4, lens.ymax - lens.ymin + 4), Palette::Black);
    const double rw = (lens.xmax - lens.xmin) / zoom / 2, rh = (lens.ymax - lens.ymin) / zoom / 2;
    const Polygon region = makeRect(sx + (focus.x - rw) / 2, sy + (focus.y - rh) / 2, rw, rh);
    drawPolygonOutline(c, region, SuspectColour::Highlight);
    drawLineBresenham(c, px(region.vertices[1]), {static_cast<int>(lens.xmin), static_cast<int>(lens.ymin)}, SuspectColour::Highlight);
    drawLineBresenham(c, px(region.vertices[2]), {static_cast<int>(lens.xmin), static_cast<int>(lens.ymax)}, SuspectColour::Highlight);
    drawText(c, 520, 420, "ZOOMED PIXELS ARE CLIPPED TO THE LENS WINDOW", Color{110, 110, 120});
}
