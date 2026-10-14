// Demo: DDA vs Bresenham in all 8 octants, plus thick lines.
// Output: output/01_lines.bmp
#include <cmath>

#include "demo_common.h"
#include "primitives/primitives.h"

namespace {
void starburst(Canvas& c, Point centre, int radius, bool useDDA, Color col) {
    const double PI = 3.14159265358979;
    for (int i = 0; i < 24; ++i) {  // 24 directions cover every octant
        const double a = i * 2 * PI / 24;
        Point end{centre.x + static_cast<int>(std::lround(radius * std::cos(a))),
                  centre.y + static_cast<int>(std::lround(radius * std::sin(a)))};
        if (useDDA) drawLineDDA(c, centre, end, col);
        else        drawLineBresenham(c, centre, end, col);
    }
}
}  // namespace

int main() {
    Canvas c(900, 420, Palette::White);
    demo::title(c, "LINE DRAWING", "DDA  VS  BRESENHAM");

    starburst(c, {160, 200}, 120, true, Palette::Blue);
    demo::caption(c, 70, 340, "DDA (FLOATING POINT)");

    starburst(c, {450, 200}, 120, false, Palette::Red);
    demo::caption(c, 345, 340, "BRESENHAM (INTEGER ONLY)");

    // Thick lines — will become evidence-board strings and wires.
    for (int t = 1; t <= 7; t += 2) {
        const int y = 90 + t * 25;
        drawThickLine(c, {640, y}, {860, y - 40}, t, Palette::StringRed);
        demo::caption(c, 640, y + 10, "THICKNESS " + std::to_string(t), demo::Muted);
    }
    demo::caption(c, 640, 340, "THICK LINES");
    demo::caption(c, 12, 395, "BOTH ALGORITHMS HANDLE ALL 8 OCTANTS - 24 DIRECTIONS SHOWN", demo::Muted);
    return demo::save(c, "01_lines");
}
