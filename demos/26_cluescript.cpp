// Demo (CP3, issue #58): ClueScript programs run by our interpreter and drawn
// by our algorithms, each clipped to its own little "monitor".
// Output: output/26_cluescript.bmp
#include <sstream>

#include "demo_common.h"
#include "polygon/polygon.h"
#include "primitives/primitives.h"
#include "script/script.h"

namespace {
struct Sample { const char* title; const char* code; };

const Sample SAMPLES[] = {
    {"BROKEN STAR (PUZZLE START)", "# the victim's program\nrepeat 5 {\n  forward 120\n  turn 72\n}"},
    {"FIXED STAR (THE ANSWER)", "# the safe wants a star\nrepeat 5 {\n  forward 120\n  turn 144\n}"},
    {"VARIABLES + LOOP", "color blue\nlet d = 6\nrepeat 34 {\n  forward d\n  turn 91\n  d = d + 6\n}"},
    {"SHAPES + IF/ELSE", "color red\ncircle 150 110 50\ncolor green\nellipse 150 110 80 25\nlet n = 3\nif n > 2 {\n  color orange\n  fill_poly 100 220 200 220 150 270\n} else {\n  print \"small\"\n}"},
};
const char* BROKEN[] = {"repeat 5 {\n  forward 120\n  turn 144\n", "repeat 5 {\n  forwrd 120\n}", "let s = 0\nwhile s == 0 {\n  turn 1\n}"};
}  // namespace

int main() {
    Canvas c(1120, 560, Color{236, 236, 240});
    demo::title(c, "CLUESCRIPT INTERPRETER", "LEXER -> PARSER -> AST -> INTERPRETER -> OUR ALGORITHMS");

    script::RunOptions opt;
    opt.start = {90, 140};                                     // where the puzzle's turtle starts
    for (int i = 0; i < 4; ++i) {
        const double ox = 20 + i * 275, oy = 60;
        // monitor: code on top, output screen below
        const Polygon screen = makeRect(ox, oy + 150, 255, 230);
        scanlineFill(c, makeRect(ox - 6, oy - 6, 267, 392), Color{50, 52, 60});
        scanlineFill(c, makeRect(ox, oy, 255, 144), Color{24, 26, 32});
        scanlineFill(c, screen, Palette::White);
        std::istringstream lines(SAMPLES[i].code);
        std::string ln;
        int row = 0;
        while (std::getline(lines, ln) && row < 13) drawText(c, static_cast<int>(ox) + 6, static_cast<int>(oy) + 6 + 11 * row++, ln, Color{150, 230, 160});

        const script::RunResult r = script::run(SAMPLES[i].code, opt);
        // program space 300x300 -> 230x230 screen area
        const Mat3 toScreen = translate(ox + 12, oy + 150) * scale(230.0 / 300, 230.0 / 300);
        const ClipRect clip{ox, oy + 150, ox + 254, oy + 379};
        script::renderDrawing(c, r.drawing, toScreen, clip, 2);
        demo::caption(c, static_cast<int>(ox), static_cast<int>(oy) + 394, SAMPLES[i].title);
        demo::caption(c, static_cast<int>(ox), static_cast<int>(oy) + 408,
                      r.ok ? (std::to_string(r.drawing.size()) + " DRAW COMMANDS, " + std::to_string(r.steps) + " STEPS") : r.error,
                      r.ok ? demo::Muted : Palette::Red);
    }
    // Friendly errors
    int y = 500;
    for (const char* bad : BROKEN) {
        const script::RunResult r = script::run(bad, opt);
        drawText(c, 20, y, "ERROR -> " + r.error, Palette::Red);
        y += 14;
    }
    return demo::save(c, "26_cluescript");
}
