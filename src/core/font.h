#pragma once
// Tiny 5x7 bitmap font so our images can carry labels without any library.
// Each glyph is 7 rows of 5 bits; a set bit becomes a (scale x scale) block of pixels.
#include <string>
#include "canvas.h"

// Draws text with its top-left corner at (x, y). Lower-case is shown as upper-case.
void drawText(Canvas& c, int x, int y, const std::string& text, Color col, int scale = 1);

// Width in pixels that drawText would use (handy for centring).
int textWidth(const std::string& text, int scale = 1);
