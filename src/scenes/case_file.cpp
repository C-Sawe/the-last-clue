// Case File panel (left side of the game window) — Latiffa
// Lists the suspects (colour-coded) and the evidence slots the player fills.
// Built from filled rectangles, circles, ellipses, lines and our bitmap font.
#include <string>

#include "../core/font.h"
#include "../polygon/polygon.h"
#include "../primitives/primitives.h"
#include "scenes.h"

namespace {
const Color Panel{34, 36, 46};
const Color Section{48, 51, 64};
const Color Text{230, 230, 236};
const Color Dim{140, 144, 160};

void box(Canvas& c, double x, double y, double w, double h, Color fill, Color edge) {
    scanlineFill(c, makeRect(x, y, w, h), fill);
    drawPolygonOutline(c, makeRect(x, y, w, h), edge);
}

void suspectRow(Canvas& c, int y, Color col, const std::string& name, const std::string& status) {
    box(c, 12, y, 226, 40, Section, Section);
    fillCircle(c, {34, y + 20}, 13, col);
    fillEllipse(c, {34, y + 16}, 5, 6, Color{230, 200, 170});            // tiny head
    fillEllipse(c, {34, y + 28}, 9, 4, Color{230, 200, 170});
    drawCircleMidpoint(c, {34, y + 20}, 13, Palette::Black);
    drawText(c, 56, y + 10, name, Text);
    drawText(c, 56, y + 24, status, Dim);
}

// Evidence slot: filled when collected (with a small icon), outlined when empty.
void slot(Canvas& c, int x, int y, int icon) {
    if (icon == 0) {
        for (int i = 0; i < 64; i += 8) {                                    // dashed empty slot
            drawLineBresenham(c, {x + i, y}, {x + i + 4, y}, Dim);
            drawLineBresenham(c, {x + i, y + 64}, {x + i + 4, y + 64}, Dim);
            drawLineBresenham(c, {x, y + i}, {x, y + i + 4}, Dim);
            drawLineBresenham(c, {x + 64, y + i}, {x + 64, y + i + 4}, Dim);
        }
        drawText(c, x + 29, y + 28, "?", Dim);
        return;
    }
    box(c, x, y, 64, 64, Section, SuspectColour::Highlight);
    if (icon == 1) {                                                         // key
        drawCircleMidpoint(c, {x + 20, y + 32}, 9, SuspectColour::Highlight);
        drawThickLine(c, {x + 29, y + 32}, {x + 52, y + 32}, 3, SuspectColour::Highlight);
        drawThickLine(c, {x + 46, y + 32}, {x + 46, y + 40}, 3, SuspectColour::Highlight);
    } else if (icon == 2) {                                                  // letter
        box(c, x + 14, y + 14, 36, 36, Palette::Paper, Palette::Paper);
        for (int ly = y + 22; ly < y + 46; ly += 6) drawLineBresenham(c, {x + 18, ly}, {x + 45, ly}, Color{90, 90, 140});
    } else {                                                                 // fingerprint
        for (int k = 0; k < 5; ++k) drawEllipseMidpoint(c, {x + 32, y + 32}, 3 + k * 4, 5 + k * 5, Text);
    }
}
}  // namespace

void renderCaseFilePanel(Canvas& c) {
    c.clear(Panel);
    drawText(c, 14, 14, "CASE FILE", Palette::White, 3);
    drawText(c, 14, 42, "CASE 47 - THE STUDY", Dim);
    drawLineBresenham(c, {12, 56}, {237, 56}, Dim);

    drawText(c, 14, 66, "SUSPECTS", SuspectColour::Highlight);
    suspectRow(c, 80, SuspectColour::Butler, "THE BUTLER", "ALIBI: NONE");
    suspectRow(c, 126, SuspectColour::Heiress, "THE HEIRESS", "MOTIVE: WILL");
    suspectRow(c, 172, SuspectColour::Partner, "THE PARTNER", "NOT QUESTIONED");

    drawText(c, 14, 228, "EVIDENCE  3/6", SuspectColour::Highlight);
    const int icons[] = {1, 2, 3, 0, 0, 0};
    for (int i = 0; i < 6; ++i) slot(c, 14 + (i % 3) * 78, 244 + (i / 3) * 78, icons[i]);

    box(c, 12, 410, 226, 32, Color{60, 110, 180}, Palette::Black);
    drawText(c, 125 - textWidth("EVIDENCE BOARD", 2) / 2 + 1, 419, "EVIDENCE BOARD", Palette::White, 2);
    box(c, 12, 452, 226, 32, Color{70, 70, 78}, Palette::Black);
    drawText(c, 125 - textWidth("MAKE DEDUCTION") / 2, 464, "MAKE DEDUCTION", Dim);
}
