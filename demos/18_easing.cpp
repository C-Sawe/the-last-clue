// Demo (CP2): interpolation, easing curves and keyframe tracks.
// Output: output/18_easing.bmp (curve plots), output/18_easing_race.gif/.bmp (animation)
#include <cmath>
#include <string>

#include "animation/animation.h"
#include "demo_common.h"
#include "primitives/primitives.h"

namespace {
using anim::Ease;
const Ease EASES[] = {Ease::Linear, Ease::InQuad, Ease::OutQuad, Ease::InOutCubic, Ease::OutBack, Ease::OutBounce};
const char* NAMES[] = {"LINEAR", "IN QUAD", "OUT QUAD", "IN-OUT CUBIC", "OUT BACK", "OUT BOUNCE"};
const Color COLS[] = {{90, 90, 100}, {40, 80, 200}, {40, 160, 70}, {200, 40, 40}, {140, 60, 160}, {220, 140, 30}};
}  // namespace

int main() {
    // ---------- Static: plot each easing function y = ease(t) ----------
    Canvas c(1000, 400, Palette::White);
    demo::title(c, "EASING FUNCTIONS", "Y = EASE(T),  T: 0 -> 1");
    for (int i = 0; i < 6; ++i) {
        const int ox = 30 + i * 162, oy = 300, w = 130, h = 180;
        drawLineBresenham(c, {ox, oy}, {ox + w, oy}, Color{200, 200, 205});         // axes
        drawLineBresenham(c, {ox, oy}, {ox, oy - h}, Color{200, 200, 205});
        drawLineBresenham(c, {ox, oy - h}, {ox + w, oy - h}, Color{235, 235, 240}); // y = 1
        Point prev{ox, oy};
        for (int s = 1; s <= 100; ++s) {                                            // sample the curve
            const double t = s / 100.0;
            const Point p{ox + static_cast<int>(t * w), oy - static_cast<int>(std::lround(anim::ease(EASES[i], t) * h))};
            drawThickLine(c, prev, p, 2, COLS[i]);
            prev = p;
        }
        demo::caption(c, ox, oy + 14, NAMES[i], COLS[i]);
    }
    demo::caption(c, 12, 370, "OUT BACK GOES ABOVE 1 (OVERSHOOT).  OUT BOUNCE TOUCHES 1 SEVERAL TIMES.", demo::Muted);
    if (demo::save(c, "18_easing") != EXIT_SUCCESS) return EXIT_FAILURE;

    // ---------- Animation: six evidence pins race with different easing,
    //            plus one pin driven by a keyframe Track (there and back) ----------
    anim::Track track;
    track.key(0.0, 0).key(0.45, 1, Ease::OutBounce).key(0.6, 1).key(1.0, 0, Ease::InOutCubic);
    return demo::animate("18_easing_race", 640, 360, 50, [&](Canvas& f, double t) {
        f.clear(Palette::Cork);
        const int x0 = 150, x1 = 600;
        for (int i = 0; i < 7; ++i) {
            const int y = 30 + i * 45;
            drawLineBresenham(f, {x0, y}, {x1, y}, Color{160, 115, 70});
            const double v = (i < 6) ? anim::ease(EASES[i], anim::phase(t, 0.0, 0.8)) : track.at(t);
            const int x = x0 + static_cast<int>(std::lround(anim::lerp(0, x1 - x0, v)));
            const Color col = (i < 6) ? COLS[i] : Palette::Black;
            fillCircle(f, {x, y}, 10, col);
            fillCircle(f, {x - 3, y - 3}, 3, Palette::White);
            drawText(f, 10, y - 3, i < 6 ? NAMES[i] : "KEYFRAMES", Palette::Black);
        }
    });
}
