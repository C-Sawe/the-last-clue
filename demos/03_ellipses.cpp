// Demo: midpoint ellipse (two regions), filled ellipses, fingerprint ridges.
// Output: output/03_ellipses.bmp
#include "demo_common.h"
#include "primitives/primitives.h"

int main() {
    Canvas c(900, 420, Palette::White);
    demo::title(c, "MIDPOINT ELLIPSE", "REGION 1 + REGION 2");

    // Grid of outlines with varying rx (columns) and ry (rows)
    const int rxs[] = {20, 40, 60};
    const int rys[] = {15, 30, 45};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            drawEllipseMidpoint(c, {80 + i * 110, 95 + j * 100}, rxs[i], rys[j], Palette::Blue);
    demo::caption(c, 60, 395, "RX = 20/40/60   RY = 15/30/45");

    // Filled ellipses — a character head and a rug
    fillEllipse(c, {470, 150}, 45, 60, Color{224, 172, 125});   // head
    drawEllipseMidpoint(c, {470, 150}, 45, 60, Palette::Black);
    fillEllipse(c, {455, 140}, 6, 4, Palette::Black);             // eyes
    fillEllipse(c, {485, 140}, 6, 4, Palette::Black);
    demo::caption(c, 420, 220, "CHARACTER HEAD");

    fillEllipse(c, {470, 320}, 110, 40, Color{140, 40, 50});      // rug
    fillEllipse(c, {470, 320}, 85, 28, Color{180, 120, 60});
    fillEllipse(c, {470, 320}, 55, 16, Color{140, 40, 50});
    demo::caption(c, 425, 372, "CRIME SCENE RUG");

    // Fingerprint ridges: concentric ellipses (future fingerprint puzzle)
    for (int k = 0; k < 11; ++k)
        drawEllipseMidpoint(c, {740, 210}, 10 + k * 7, 16 + k * 10, Color{60, 60, 90});
    demo::caption(c, 680, 340, "FINGERPRINT RIDGES");
    return demo::save(c, "03_ellipses");
}
