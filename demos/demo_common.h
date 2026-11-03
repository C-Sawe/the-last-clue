#pragma once
// Small helpers shared by the demo programs (header-only).
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include "core/canvas.h"
#include "core/font.h"
#include "core/gif.h"

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

// ---- CP2: animation output ----------------------------------------------
// Renders `frames` frames of render(canvas, t) with t going 0 -> 1 and writes
//   output/<name>.gif  - the animation (made by our own GIF writer)
//   output/<name>.bmp  - a 2x2 sheet of key frames (t = 0, 1/3, 2/3, 1) for reports
// `holdEnd` repeats the last frame so the loop pauses on the final pose.
inline int animate(const std::string& name, int w, int h, int frames,
                   const std::function<void(Canvas&, double)>& render, int holdEnd = 12) {
    GifWriter gif(w, h, 6);
    Canvas sheet(w, h, Color{30, 32, 40});
    const double keys[] = {0.0, 1.0 / 3, 2.0 / 3, 1.0};
    for (int i = 0; i < frames; ++i) {
        const double t = frames > 1 ? static_cast<double>(i) / (frames - 1) : 1.0;
        Canvas c(w, h);
        render(c, t);
        gif.addFrame(c, i == frames - 1 ? holdEnd : 1);
        for (int k = 0; k < 4; ++k) {
            const int ki = static_cast<int>(keys[k] * (frames - 1) + 0.5);
            if (ki != i) continue;
            const int ox = (k % 2) * (w / 2), oy = (k / 2) * (h / 2);
            for (int y = 0; y < h / 2; ++y)
                for (int x = 0; x < w / 2; ++x) sheet.setPixel(ox + x, oy + y, c.getPixel(x * 2, y * 2));
            drawText(sheet, ox + 6, oy + 6, "T = " + std::to_string(keys[k]).substr(0, 4), Palette::White);
        }
    }
    const std::string gifPath = "output/" + name + ".gif";
    if (!gif.save(gifPath)) {
        std::cerr << "Could not write " << gifPath << " - run from the repository root.\n";
        return EXIT_FAILURE;
    }
    std::cout << "Wrote " << gifPath << " (" << gif.frameCount() << " frames)\n";
    return save(sheet, name);
}

}  // namespace demo
