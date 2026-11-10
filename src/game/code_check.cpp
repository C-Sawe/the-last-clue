// Code puzzle validation — Sudheysi (CP3, issue #59)
#include "code_check.h"

#include <cmath>

namespace game {

const char* starStarterCode() {
    return "# ALARM KEY - draws my symbol\n"
           "# (it stopped working...)\n"
           "repeat 5 {\n"
           "  forward 120\n"
           "  turn 72\n"
           "}\n";
}

const char* starHint() {
    return "HINT: A 5-POINT STAR TURNS 144 DEGREES AT EACH POINT (5 X 144 = 720 = TWO FULL TURNS).";
}

script::RunOptions puzzleRunOptions() {
    script::RunOptions o;
    o.start = {90, 140};
    o.heading = 0;
    o.stepLimit = 5000;
    return o;
}

const std::vector<script::DrawCmd>& targetStar() {
    static const std::vector<script::DrawCmd> target =
        script::run("repeat 5 {\n forward 120\n turn 144\n}", puzzleRunOptions()).drawing;
    return target;
}

std::vector<bool> rasterise(const std::vector<script::DrawCmd>& drawing, int size, int thickness) {
    Canvas c(size, size, Palette::White);
    script::renderDrawing(c, drawing, Mat3::identity(), ClipRect{0, 0, size - 1.0, size - 1.0}, thickness);
    std::vector<bool> mask(static_cast<size_t>(size) * size);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) mask[static_cast<size_t>(y) * size + x] = c.getPixel(x, y) != Palette::White;
    return mask;
}

double intersectionOverUnion(const std::vector<bool>& a, const std::vector<bool>& b) {
    size_t inter = 0, uni = 0;
    for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
        inter += (a[i] && b[i]) ? 1 : 0;
        uni += (a[i] || b[i]) ? 1 : 0;
    }
    return uni == 0 ? 0.0 : static_cast<double>(inter) / uni;
}

CodeCheck checkStar(const script::RunResult& r) {
    CodeCheck out;
    if (!r.ok) {
        out.message = "FIX THE ERROR FIRST";
        return out;
    }
    if (r.drawing.empty()) {
        out.message = "NOTHING WAS DRAWN";
        return out;
    }
    static const std::vector<bool> target = rasterise(targetStar());
    out.iou = intersectionOverUnion(rasterise(r.drawing), target);
    const int pct = static_cast<int>(std::lround(out.iou * 100));
    out.pass = out.iou >= 0.85;
    out.message = out.pass ? "SYMBOL ACCEPTED - ALARM DISABLED (MATCH " + std::to_string(pct) + "%)"
                           : "SYMBOL NOT RECOGNISED (MATCH " + std::to_string(pct) + "%)";
    return out;
}

}  // namespace game
