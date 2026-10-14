// Demo: midpoint circle (outline + filled) and how it becomes game parts.
// Output: output/02_circles.bmp
#include "demo_common.h"
#include "primitives/primitives.h"

int main() {
    Canvas c(900, 400, Palette::White);
    demo::title(c, "MIDPOINT CIRCLE", "8-WAY SYMMETRY");

    // Concentric outlines
    for (int r = 10; r <= 130; r += 15) drawCircleMidpoint(c, {170, 210}, r, Palette::Blue);
    demo::caption(c, 95, 360, "CONCENTRIC OUTLINES");

    // Filled circles
    fillCircle(c, {420, 170}, 70, Palette::Green);
    fillCircle(c, {420, 170}, 40, Palette::White);
    fillCircle(c, {420, 170}, 15, Palette::Red);
    drawCircleMidpoint(c, {420, 170}, 70, Palette::Black);
    demo::caption(c, 370, 260, "FILLED (SPANS)");

    // Project use: board pins and the safe dial
    for (int i = 0; i < 4; ++i) {
        const Point p{600 + i * 40, 150};
        fillCircle(c, p, 10, Palette::StringRed);
        fillCircle(c, {p.x - 3, p.y - 3}, 3, Color{255, 170, 170});  // highlight
    }
    demo::caption(c, 585, 180, "EVIDENCE BOARD PINS");

    fillCircle(c, {690, 290}, 55, Palette::DarkGrey);
    fillCircle(c, {690, 290}, 42, Palette::Grey);
    drawCircleMidpoint(c, {690, 290}, 55, Palette::Black);
    fillCircle(c, {690, 290}, 8, Palette::Black);
    demo::caption(c, 760, 285, "SAFE DIAL");
    return demo::save(c, "02_circles");
}
