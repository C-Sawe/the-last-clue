#pragma once
// Text and widget helpers on top of our bitmap font — Latiffa (CP3, issue #46)
//
// The 5x7 font (src/core/font.cpp) draws single lines. Interfaces need more:
//   - word wrapping into a box of a given width
//   - fixed-width (monospace) metrics, so the code editor can map
//     row/column <-> pixel positions
//   - buttons with hover/disabled states, and the toast message bar
#include <string>
#include <vector>

#include "core/canvas.h"

namespace ui {

// Monospace metrics of our font at a given scale.
inline int charW(int scale) { return 6 * scale; }      // 5 px glyph + 1 px gap
inline int lineH(int scale) { return 9 * scale; }      // 7 px glyph + 2 px gap

// Splits text into lines of at most maxChars, breaking at spaces (or mid-word
// if a single word is too long). Existing '\n' always start a new line.
std::vector<std::string> wrap(const std::string& text, int maxChars);

// Draws wrapped text inside a box; returns the number of lines drawn.
int textBox(Canvas& c, double x, double y, double w, double h, const std::string& text,
            Color fg, int scale = 1);

struct Button {
    double x = 0, y = 0, w = 0, h = 0;
    std::string label;
    bool contains(PointF p) const { return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h; }
};
void drawButton(Canvas& c, const Button& b, bool enabled, bool hovered, Color base);

// Message bar across the bottom of a canvas region.
void toastBar(Canvas& c, double x, double y, double w, const std::string& message);

}  // namespace ui
