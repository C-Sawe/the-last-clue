// Drawing a ClueScript program's output — Caleb (CP3, issue #58)
// Every command is drawn with OUR algorithms and clipped to a window:
//   lines    -> transformed, clipped with Cohen–Sutherland, drawn with Bresenham (thick)
//   polygons -> transformed, clipped with Sutherland–Hodgman, scanline-filled
//   circles / ellipses -> midpoint algorithms on a scratch layer; only pixels
//                         inside the window are copied (pixel-level clipping)
#include <cmath>

#include "primitives/primitives.h"
#include "script.h"

namespace script {

void renderDrawing(Canvas& c, const std::vector<DrawCmd>& drawing, const Mat3& toScreen,
                   const ClipRect& clip, int thickness) {
    const double scaleFactor = std::hypot(toScreen.m[0][0], toScreen.m[1][0]);   // uniform scale of the matrix
    const Color key{255, 0, 255};
    Canvas layer(c.width(), c.height(), key);
    bool layerUsed = false;

    for (const auto& d : drawing) {
        switch (d.kind) {
            case DrawCmd::Kind::Line: {
                PointF a = apply(toScreen, d.pts[0]), b = apply(toScreen, d.pts[1]);
                if (cohenSutherlandClip(a, b, clip))
                    drawThickLine(c, {static_cast<int>(std::lround(a.x)), static_cast<int>(std::lround(a.y))},
                                     {static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y))}, thickness, d.color);
                break;
            }
            case DrawCmd::Kind::FillPoly: {
                Polygon p;
                for (const auto& v : d.pts) p.vertices.push_back(apply(toScreen, v));
                scanlineFill(c, sutherlandHodgmanClip(p, clip), d.color);
                break;
            }
            case DrawCmd::Kind::Circle:
            case DrawCmd::Kind::Ellipse: {
                const PointF ctr = apply(toScreen, d.pts[0]);
                const Point pc{static_cast<int>(std::lround(ctr.x)), static_cast<int>(std::lround(ctr.y))};
                const int rx = static_cast<int>(std::lround(d.rx * scaleFactor));
                const int ry = static_cast<int>(std::lround((d.kind == DrawCmd::Kind::Circle ? d.rx : d.ry) * scaleFactor));
                for (int k = 0; k < thickness; ++k) {
                    if (d.kind == DrawCmd::Kind::Circle) drawCircleMidpoint(layer, pc, rx - k, d.color);
                    else drawEllipseMidpoint(layer, pc, rx - k, ry - k, d.color);
                }
                layerUsed = true;
                break;
            }
        }
    }
    if (!layerUsed) return;
    for (int y = static_cast<int>(std::ceil(clip.ymin)); y <= static_cast<int>(clip.ymax); ++y)
        for (int x = static_cast<int>(std::ceil(clip.xmin)); x <= static_cast<int>(clip.xmax); ++x) {
            const Color p = layer.getPixel(x, y);
            if (p != key && layer.inBounds(x, y)) c.setPixel(x, y, p);
        }
}

}  // namespace script
