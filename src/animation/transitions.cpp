// Scene transitions — Latiffa (CP2, issue #32)
#include "transitions.h"

#include <cmath>

#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "animation.h"

namespace anim {

void crossFade(Canvas& out, const Canvas& from, const Canvas& to, double t) {
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x)
            out.setPixel(x, y, lerp(from.getPixel(x, y), to.getPixel(x, y), t));
}

void fadeThroughBlack(Canvas& out, const Canvas& from, const Canvas& to, double t) {
    const bool first = t < 0.5;
    const Canvas& src = first ? from : to;
    const double k = first ? (1 - t * 2) : (t * 2 - 1);      // brightness 1 -> 0 -> 1
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x)
            out.setPixel(x, y, lerp(Palette::Black, src.getPixel(x, y), k));
}

void wipe(Canvas& out, const Canvas& from, const Canvas& to, double t) {
    out.blit(from, 0, 0);
    const double edge = ease(Ease::InOutCubic, t) * out.width();
    if (edge <= 0) return;

    // The revealed region is the screen rectangle clipped to x <= edge.
    const ClipRect window{0, 0, edge, static_cast<double>(out.height())};
    const Polygon revealed = sutherlandHodgmanClip(makeRect(0, 0, out.width(), out.height()), window);
    double maxX = 0;
    for (const auto& v : revealed.vertices) maxX = std::max(maxX, v.x);
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < static_cast<int>(maxX) && x < out.width(); ++x)
            out.setPixel(x, y, to.getPixel(x, y));

    // Glowing edge: a long line clipped to the screen with Cohen–Sutherland.
    PointF a{edge, -50}, b{edge, out.height() + 50.0};
    const ClipRect screen{0, 0, out.width() - 1.0, out.height() - 1.0};
    if (t < 1 && cohenSutherlandClip(a, b, screen))
        drawThickLine(out, {static_cast<int>(a.x), static_cast<int>(a.y)},
                           {static_cast<int>(b.x), static_cast<int>(b.y)}, 3, Color{255, 230, 150});
}

void iris(Canvas& out, const Canvas& from, const Canvas& to, double t, PointF centre) {
    // Radius large enough to cover the farthest corner when t = 1
    double maxR = 0;
    for (double cx : {0.0, double(out.width())})
        for (double cy : {0.0, double(out.height())})
            maxR = std::max(maxR, std::hypot(cx - centre.x, cy - centre.y));
    const double r = ease(Ease::InQuad, t) * maxR;
    for (int y = 0; y < out.height(); ++y)
        for (int x = 0; x < out.width(); ++x) {
            const double dx = x - centre.x, dy = y - centre.y;
            out.setPixel(x, y, dx * dx + dy * dy <= r * r ? to.getPixel(x, y) : lerp(from.getPixel(x, y), Palette::Black, 0.5));
        }
    if (t > 0 && t < 1)
        drawCircleMidpoint(out, {static_cast<int>(centre.x), static_cast<int>(centre.y)}, static_cast<int>(r),
                           Color{255, 230, 150});
}

}  // namespace anim
