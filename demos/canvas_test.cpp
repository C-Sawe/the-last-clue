// Sanity check for the Canvas + BMP writer. Uses setPixel only.
// Output: output/00_canvas_test.bmp — a colour gradient with a border.
#include "../src/core/canvas.h"
#include <iostream>

int main() {
    Canvas c(400, 300, Palette::White);
    for (int y = 0; y < c.height(); ++y)
        for (int x = 0; x < c.width(); ++x)
            c.setPixel(x, y, Color{static_cast<uint8_t>(x * 255 / c.width()),
                                   static_cast<uint8_t>(y * 255 / c.height()), 150});
    for (int x = 0; x < c.width(); ++x) { c.setPixel(x, 0, Palette::Black); c.setPixel(x, c.height() - 1, Palette::Black); }
    for (int y = 0; y < c.height(); ++y) { c.setPixel(0, y, Palette::Black); c.setPixel(c.width() - 1, y, Palette::Black); }

    const char* path = "output/00_canvas_test.bmp";
    if (!c.saveBMP(path)) { std::cerr << "Could not write " << path << "\n"; return 1; }
    std::cout << "Wrote " << path << "\n";
    return 0;
}
