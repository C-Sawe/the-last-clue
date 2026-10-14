// Midpoint ellipse algorithm — Caleb
#include "primitives.h"

namespace {
void plot4(Canvas& c, Point ctr, int x, int y, Color col) {
    c.setPixel(ctr.x + x, ctr.y + y, col);
    c.setPixel(ctr.x - x, ctr.y + y, col);
    c.setPixel(ctr.x + x, ctr.y - y, col);
    c.setPixel(ctr.x - x, ctr.y - y, col);
}

void span4(Canvas& c, Point ctr, int x, int y, Color col) {
    for (int i = -x; i <= x; ++i) {
        c.setPixel(ctr.x + i, ctr.y + y, col);
        c.setPixel(ctr.x + i, ctr.y - y, col);
    }
}

// The walk is shared by the outline and the filled version; `fill` picks which.
// f(x,y) = ry^2 x^2 + rx^2 y^2 - rx^2 ry^2  (negative inside, positive outside)
//
// Region 1 (slope magnitude < 1): step x every time, decide whether y drops.
//   p1_0 = ry^2 - rx^2 ry + rx^2 / 4
// Region 2 (slope magnitude >= 1): step y every time, decide whether x grows.
//   p2_0 = ry^2 (x + 1/2)^2 + rx^2 (y - 1)^2 - rx^2 ry^2
// The switch happens when 2 ry^2 x >= 2 rx^2 y.
void midpointEllipse(Canvas& c, Point ctr, int rx, int ry, Color col, bool fill) {
    if (rx < 0 || ry < 0) return;
    const long long rx2 = 1LL * rx * rx, ry2 = 1LL * ry * ry;
    long long x = 0, y = ry;
    long long dx = 0, dy = 2 * rx2 * y;

    auto emit = [&](long long px, long long py) {
        if (fill) span4(c, ctr, static_cast<int>(px), static_cast<int>(py), col);
        else      plot4(c, ctr, static_cast<int>(px), static_cast<int>(py), col);
    };

    // Region 1 (decision values scaled by 4 to stay in integers)
    long long p1 = 4 * ry2 - 4 * rx2 * ry + rx2;
    while (dx < dy) {
        emit(x, y);
        ++x;
        dx += 2 * ry2;
        if (p1 < 0) {
            p1 += 4 * (dx + ry2);
        } else {
            --y;
            dy -= 2 * rx2;
            p1 += 4 * (dx - dy + ry2);
        }
    }

    // Region 2 (scaled by 4 as well)
    long long p2 = ry2 * (2 * x + 1) * (2 * x + 1) + 4 * rx2 * (y - 1) * (y - 1) - 4 * rx2 * ry2;
    while (y >= 0) {
        emit(x, y);
        --y;
        dy -= 2 * rx2;
        if (p2 > 0) {
            p2 += 4 * (rx2 - dy);
        } else {
            ++x;
            dx += 2 * ry2;
            p2 += 4 * (dx - dy + rx2);
        }
    }
}
}  // namespace

void drawEllipseMidpoint(Canvas& c, Point centre, int rx, int ry, Color col) {
    midpointEllipse(c, centre, rx, ry, col, false);
}

void fillEllipse(Canvas& c, Point centre, int rx, int ry, Color col) {
    midpointEllipse(c, centre, rx, ry, col, true);
}
