#pragma once
// Small helpers shared by the demo programs (header-only).
#include <cstdlib>
#include <iostream>
#include <string>

#include "core/canvas.h"
#include "core/font.h"

namespace demo {

const Color Ink   {40, 40, 50};
const Color Muted {130, 130, 140};

// Dark title strip across the top of a demo image.
inline void title(Canvas& c, const std::string& heading, const std::string& sub = "") {
    for (int y = 0; y < 34; ++y)
        for (int x = 0; x < c.width(); ++x) c.setPixel(x, y, Color{30, 32, 40});
    drawText(c, 12, 8, heading, Palette::White, 2);
    if (!sub.empty()) drawText(c, c.width() - textWidth(sub) - 12, 14, sub, Color{170, 175, 190});
}

inline void caption(Canvas& c, int x, int y, const std::string& text, Color col = Ink) {
    drawText(c, x, y, text, col);
}

// Saves to output/<name>.bmp (run demos from the repository root).
inline int save(const Canvas& c, const std::string& name) {
    const std::string path = "output/" + name + ".bmp";
    if (!c.saveBMP(path)) {
        std::cerr << "Could not write " << path << " - run from the repository root.\n";
        return EXIT_FAILURE;
    }
    std::cout << "Wrote " << path << "\n";
    return EXIT_SUCCESS;
}

}  // namespace demo
