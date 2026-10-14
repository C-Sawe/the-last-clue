// Line drawing — Caleb
#include <cmath>
#include <cstdlib>

#include "primitives.h"

// DDA (Digital Differential Analyzer)
// Step one pixel at a time along the axis with the larger change (so no gaps),
// and add a constant floating-point increment to the other coordinate.
//   steps = max(|dx|, |dy|),  xInc = dx / steps,  yInc = dy / steps
void drawLineDDA(Canvas& c, Point a, Point b, Color col) {
    const int dx = b.x - a.x;
    const int dy = b.y - a.y;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { c.setPixel(a.x, a.y, col); return; }

    const double xInc = static_cast<double>(dx) / steps;
    const double yInc = static_cast<double>(dy) / steps;
    double x = a.x, y = a.y;
    for (int i = 0; i <= steps; ++i) {
        c.setPixel(static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y)), col);
        x += xInc;
        y += yInc;
    }
}

// Bresenham — integer-only, works in all 8 octants.
// `err` tracks (scaled) distance between the true line and the pixel grid.
// Each step we move in x, in y, or both, depending on which keeps |err| smallest.
// This general form uses err = dx - dy and compares 2*err against -dy and dx,
// which removes the need for separate code per octant.
void drawLineBresenham(Canvas& c, Point a, Point b, Color col) {
    int x = a.x, y = a.y;
    const int dx = std::abs(b.x - a.x);
    const int dy = std::abs(b.y - a.y);
    const int sx = (a.x < b.x) ? 1 : -1;
    const int sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        c.setPixel(x, y, col);
        if (x == b.x && y == b.y) break;
        const int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }   // step in x
        if (e2 <  dx) { err += dx; y += sy; }   // step in y
    }
}

// Thick line: walk the Bresenham path and stamp a filled disc at every pixel.
void drawThickLine(Canvas& c, Point a, Point b, int thickness, Color col) {
    if (thickness <= 1) { drawLineBresenham(c, a, b, col); return; }
    const int r = thickness / 2;

    int x = a.x, y = a.y;
    const int dx = std::abs(b.x - a.x), dy = std::abs(b.y - a.y);
    const int sx = (a.x < b.x) ? 1 : -1, sy = (a.y < b.y) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        for (int oy = -r; oy <= r; ++oy)
            for (int ox = -r; ox <= r; ++ox)
                if (ox * ox + oy * oy <= r * r) c.setPixel(x + ox, y + oy, col);
        if (x == b.x && y == b.y) break;
        const int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 <  dx) { err += dx; y += sy; }
    }
}
