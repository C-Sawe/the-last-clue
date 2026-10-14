// Midpoint circle algorithm — Caleb
#include "primitives.h"

namespace {
// A circle is symmetric in 8 octants, so every computed point (x, y)
// gives us 8 pixels for free.
void plot8(Canvas& c, Point ctr, int x, int y, Color col) {
    c.setPixel(ctr.x + x, ctr.y + y, col);
    c.setPixel(ctr.x - x, ctr.y + y, col);
    c.setPixel(ctr.x + x, ctr.y - y, col);
    c.setPixel(ctr.x - x, ctr.y - y, col);
    c.setPixel(ctr.x + y, ctr.y + x, col);
    c.setPixel(ctr.x - y, ctr.y + x, col);
    c.setPixel(ctr.x + y, ctr.y - x, col);
    c.setPixel(ctr.x - y, ctr.y - x, col);
}

void hspan(Canvas& c, int x0, int x1, int y, Color col) {
    for (int x = x0; x <= x1; ++x) c.setPixel(x, y, col);
}
}  // namespace

// We compute only the octant from (0, r) to x == y.
// Decision parameter p = f(x+1, y-1/2), where f(x,y) = x^2 + y^2 - r^2.
//   p0 = 1 - r                       (integer form of 5/4 - r)
//   if p < 0  : midpoint is inside  -> keep y,   p += 2x + 3
//   else      : midpoint is outside -> y -= 1,   p += 2(x - y) + 5
void drawCircleMidpoint(Canvas& c, Point centre, int radius, Color col) {
    if (radius < 0) return;
    int x = 0, y = radius;
    int p = 1 - radius;
    while (x <= y) {
        plot8(c, centre, x, y, col);
        if (p < 0) {
            p += 2 * x + 3;
        } else {
            p += 2 * (x - y) + 5;
            --y;
        }
        ++x;
    }
}

// Filled circle: same midpoint walk, but draw horizontal spans between
// symmetric points instead of single pixels.
void fillCircle(Canvas& c, Point centre, int radius, Color col) {
    if (radius < 0) return;
    int x = 0, y = radius;
    int p = 1 - radius;
    while (x <= y) {
        hspan(c, centre.x - x, centre.x + x, centre.y + y, col);
        hspan(c, centre.x - x, centre.x + x, centre.y - y, col);
        hspan(c, centre.x - y, centre.x + y, centre.y + x, col);
        hspan(c, centre.x - y, centre.x + y, centre.y - x, col);
        if (p < 0) {
            p += 2 * x + 3;
        } else {
            p += 2 * (x - y) + 5;
            --y;
        }
        ++x;
    }
}
