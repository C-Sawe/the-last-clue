// Demo (CP3, issue #59): how a code puzzle answer is checked.
// Each program's drawing is rasterised with thick lines and compared with the
// target star using Intersection over Union. Green = both, red = only the
// player's drawing, blue = only the target.
// Output: output/28_code_check.bmp
#include "demo_common.h"
#include "game/code_check.h"
#include "polygon/polygon.h"

int main() {
    struct Try { const char* label; const char* code; };
    const Try tries[] = {
        {"STARTER CODE (TURN 72)", game::starStarterCode()},
        {"WRONG LENGTH (FORWARD 80)", "repeat 5 {\n forward 80\n turn 144\n}"},
        {"CORRECT (TURN 144)", "repeat 5 {\n forward 120\n turn 144\n}"},
        {"DIFFERENT CODE, SAME STAR", "let a = 144\nrepeat 10 {\n forward 60\n if 1 { }\n}\n"},
    };
    Canvas c(1240, 420, Palette::White);
    demo::title(c, "CODE PUZZLE CHECK", "RASTERISE + INTERSECTION OVER UNION, PASS AT 85%");

    // The 4th program draws the star in 10 half-length steps: same picture, different code.
    std::string alt = "repeat 5 {\n forward 60\n forward 60\n turn 144\n}";
    const std::vector<bool> target = game::rasterise(game::targetStar());
    for (int i = 0; i < 4; ++i) {
        const std::string code = (i == 3) ? alt : tries[i].code;
        const script::RunResult r = script::run(code, game::puzzleRunOptions());
        const game::CodeCheck check = game::checkStar(r);
        const std::vector<bool> mine = game::rasterise(r.drawing);
        const int ox = 10 + i * 308, oy = 50;
        for (int y = 0; y < 300; ++y)
            for (int x = 0; x < 300; ++x) {
                const bool a = mine[static_cast<size_t>(y) * 300 + x], b = target[static_cast<size_t>(y) * 300 + x];
                const Color col = (a && b) ? Color{40, 170, 70} : a ? Color{220, 60, 60} : b ? Color{80, 120, 230} : Color{245, 245, 248};
                c.setPixel(ox + x, oy + y, col);
            }
        drawPolygonOutline(c, makeRect(ox, oy, 300, 300), Palette::Grey);
        demo::caption(c, ox, oy + 310, i == 3 ? "DIFFERENT CODE, SAME STAR" : tries[i].label);
        demo::caption(c, ox, oy + 326, check.message, check.pass ? Color{30, 140, 60} : Palette::Red);
    }
    demo::caption(c, 10, 400, "GREEN = BOTH DRAWINGS   RED = ONLY THE PLAYER'S   BLUE = ONLY THE TARGET", demo::Muted);
    return demo::save(c, "28_code_check");
}
