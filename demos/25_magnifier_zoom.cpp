// Demo (CP2): magnifying glass glides between clues and zooms (scale about the
// focus point), clipped to its lens and to the screen.
// Output: output/25_magnifier_zoom.gif / .bmp
#include <cmath>

#include "animation/animation.h"
#include "demo_common.h"
#include "scenes/puzzle_anim.h"

int main() {
    using anim::Ease;
    // Stops: the painting (code hidden here), the clock (stopped at 9), the safe dial, the drawer
    const PointF stops[] = {{350, 100}, {470, 100}, {92, 288}, {454, 281}};
    const int n = 4;
    return demo::animate("25_magnifier_zoom", 800, 500, 96, [&](Canvas& c, double t) {
        const double seg = std::min(t * n, n - 1e-9);
        const int k = static_cast<int>(seg);
        const double local = seg - k;
        const PointF from = stops[k == 0 ? 0 : k - 1], to = stops[k];
        const PointF focus = anim::lerp(from, to, anim::ease(Ease::InOutCubic, anim::phase(local, 0.0, 0.35)));
        // zoom up while resting on the clue, then back down before moving on
        const double z = std::sin(3.14159265358979 * anim::phase(local, 0.35, 1.0));
        renderMagnifierAt(c, focus, 1.0 + 2.0 * z);
    }, 1);
}
